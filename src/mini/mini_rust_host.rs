use std::env;
use std::ffi::CString;
use std::os::raw::{c_char, c_int};
use std::process;

const MINI_GAME_WIDTH: c_int = 256;
const MINI_GAME_HEIGHT: c_int = 224;
const BUTTON_X: u16 = 0x40;
const BUTTON_A: u16 = 0x80;
const BUTTON_RIGHT: u16 = 0x100;

#[repr(C)]
struct MiniGameState {
    _private: [u8; 0],
}

extern "C" {
    fn MiniStubs_SetRoomExportPath(path: *const c_char);
    fn MiniCreate(viewport_width: c_int, viewport_height: c_int) -> *mut MiniGameState;
    fn MiniDestroy(state: *mut MiniGameState);
    fn MiniStepButtons(state: *mut MiniGameState, buttons: u16, quit_requested: bool);
    fn MiniStateHash(state: *const MiniGameState) -> u64;
    fn MiniSaveStateSize() -> usize;
    fn MiniSaveState(state: *const MiniGameState, buffer: *mut u8, buffer_size: usize) -> bool;
    fn MiniLoadState(state: *mut MiniGameState, buffer: *const u8, buffer_size: usize) -> bool;
}

struct Options {
    frames: usize,
    room_export_path: Option<String>,
    rollback: bool,
    input_delay: usize,
    rollback_window: usize,
    trace: bool,
}

fn parse_args() -> Result<Options, String> {
    let mut frames = 3usize;
    let mut room_export_path = None;
    let mut rollback = false;
    let mut input_delay = 3usize;
    let mut rollback_window = 32usize;
    let mut trace = false;
    let mut args = env::args().skip(1);
    while let Some(arg) = args.next() {
        match arg.as_str() {
            "--frames" => {
                let value = args.next().ok_or("--frames requires a value")?;
                frames = value
                    .parse::<usize>()
                    .map_err(|_| "--frames requires a positive integer".to_string())?;
                if frames == 0 {
                    return Err("--frames requires a positive integer".to_string());
                }
            }
            "--room-export" => {
                let value = args.next().ok_or("--room-export requires a path")?;
                room_export_path = Some(value);
            }
            "--rollback" => {
                rollback = true;
            }
            "--input-delay" => {
                let value = args.next().ok_or("--input-delay requires a value")?;
                input_delay = value
                    .parse::<usize>()
                    .map_err(|_| "--input-delay requires a positive integer".to_string())?;
                if input_delay == 0 {
                    return Err("--input-delay requires a positive integer".to_string());
                }
            }
            "--rollback-window" => {
                let value = args.next().ok_or("--rollback-window requires a value")?;
                rollback_window = value
                    .parse::<usize>()
                    .map_err(|_| "--rollback-window requires a positive integer".to_string())?;
                if rollback_window == 0 {
                    return Err("--rollback-window requires a positive integer".to_string());
                }
            }
            "--trace" => {
                trace = true;
            }
            "--help" | "-h" => {
                print_help();
                process::exit(0);
            }
            _ => return Err(format!("unknown option: {arg}")),
        }
    }
    let required_window = input_delay.min(frames) + 1;
    if rollback && rollback_window < required_window {
        return Err(format!(
            "--rollback-window must be at least {required_window} for --frames {frames} and --input-delay {input_delay}"
        ));
    }
    Ok(Options {
        frames,
        room_export_path,
        rollback,
        input_delay,
        rollback_window,
        trace,
    })
}

fn print_help() {
    println!(
        "Usage: sm_rev_mini_rs [--frames N] [--room-export PATH] [--rollback] [--input-delay N] [--rollback-window N] [--trace]"
    );
    println!("  --rollback          Run the headless rollback simulation driver.");
    println!("  --input-delay N     Reveal actual delayed input N frames late in rollback mode.");
    println!("  --rollback-window N Keep N pre-step snapshots in the rollback ring.");
    println!("  --trace             Print rollback/resimulation events to stderr.");
}

fn scripted_buttons(frame: usize) -> u16 {
    match frame % 4 {
        1 => BUTTON_RIGHT,
        2 => BUTTON_RIGHT | BUTTON_A,
        _ => 0,
    }
}

fn rollback_predicted_buttons(frame: usize) -> u16 {
    match frame % 8 {
        0 | 1 | 2 | 3 => BUTTON_RIGHT,
        _ => 0,
    }
}

fn rollback_actual_buttons(frame: usize) -> u16 {
    let delayed_remote = match frame % 12 {
        2 | 3 | 9 => BUTTON_A,
        6 => BUTTON_X,
        _ => 0,
    };
    rollback_predicted_buttons(frame) | delayed_remote
}

fn run_basic_host(options: &Options) -> Result<(), String> {
    unsafe {
        let state = MiniCreate(MINI_GAME_WIDTH, MINI_GAME_HEIGHT);
        if state.is_null() {
            return Err("MiniCreate failed".to_string());
        }

        let snapshot_size = MiniSaveStateSize();
        let mut snapshot = vec![0u8; snapshot_size];
        if !MiniSaveState(state, snapshot.as_mut_ptr(), snapshot.len()) {
            MiniDestroy(state);
            return Err("MiniSaveState failed".to_string());
        }

        for frame in 0..options.frames {
            MiniStepButtons(state, scripted_buttons(frame), false);
        }
        let state_hash = MiniStateHash(state);

        if !MiniLoadState(state, snapshot.as_ptr(), snapshot.len()) {
            MiniDestroy(state);
            return Err("MiniLoadState failed".to_string());
        }
        let rewind_hash = MiniStateHash(state);
        MiniDestroy(state);

        println!(
            "{{\"build\":\"mini-rust-host\",\"frames\":{},\"state_hash\":\"0x{:016x}\",\"rewind_hash\":\"0x{:016x}\",\"snapshot_size\":{}}}",
            options.frames, state_hash, rewind_hash, snapshot_size
        );
    }

    Ok(())
}

struct SnapshotSlot {
    frame: usize,
    bytes: Vec<u8>,
}

struct SnapshotRing {
    slots: Vec<Option<SnapshotSlot>>,
    snapshot_size: usize,
}

impl SnapshotRing {
    fn new(capacity: usize, snapshot_size: usize) -> Self {
        let slots = (0..capacity).map(|_| None).collect();
        Self {
            slots,
            snapshot_size,
        }
    }

    fn save(&mut self, state: *const MiniGameState, frame: usize) -> Result<(), String> {
        let index = frame % self.slots.len();
        let mut bytes = match self.slots[index].take() {
            Some(slot) => slot.bytes,
            None => vec![0u8; self.snapshot_size],
        };
        unsafe {
            if !MiniSaveState(state, bytes.as_mut_ptr(), bytes.len()) {
                return Err(format!("MiniSaveState failed for frame {frame}"));
            }
        }
        self.slots[index] = Some(SnapshotSlot { frame, bytes });
        Ok(())
    }

    fn load(&self, state: *mut MiniGameState, frame: usize) -> Result<(), String> {
        let index = frame % self.slots.len();
        let slot = self.slots[index]
            .as_ref()
            .filter(|slot| slot.frame == frame)
            .ok_or_else(|| {
                format!(
                    "snapshot for frame {frame} expired from rollback ring of {} slots",
                    self.slots.len()
                )
            })?;
        unsafe {
            if !MiniLoadState(state, slot.bytes.as_ptr(), slot.bytes.len()) {
                return Err(format!("MiniLoadState failed for frame {frame}"));
            }
        }
        Ok(())
    }
}

struct FrameRecord {
    predicted_buttons: u16,
    actual_buttons: u16,
    active_buttons: u16,
    hash_after: u64,
}

struct RollbackEvent {
    arrival_frame: usize,
    changed_frame: usize,
    predicted_buttons: u16,
    actual_buttons: u16,
    resimulated_frames: usize,
    hash_after_resim: u64,
}

struct DesyncReport {
    after_frame: usize,
    rollback_hash: u64,
    reference_hash: u64,
}

struct RollbackSummary {
    frames: usize,
    input_delay: usize,
    rollback_window: usize,
    snapshot_size: usize,
    rollback_count: usize,
    resimulated_frames: usize,
    final_hash: u64,
    reference_hash: u64,
    first_desync: Option<DesyncReport>,
    events: Vec<RollbackEvent>,
}

fn save_and_step(
    state: *mut MiniGameState,
    ring: &mut SnapshotRing,
    frames: &mut [FrameRecord],
    frame: usize,
) -> Result<(), String> {
    ring.save(state, frame)?;
    unsafe {
        MiniStepButtons(state, frames[frame].active_buttons, false);
        frames[frame].hash_after = MiniStateHash(state);
    }
    Ok(())
}

fn resimulate_from(
    state: *mut MiniGameState,
    ring: &mut SnapshotRing,
    frames: &mut [FrameRecord],
    changed_frame: usize,
    current_frame: usize,
) -> Result<u64, String> {
    ring.load(state, changed_frame)?;
    for frame in changed_frame..current_frame {
        save_and_step(state, ring, frames, frame)?;
    }
    unsafe { Ok(MiniStateHash(state)) }
}

fn resolve_delayed_input(
    state: *mut MiniGameState,
    ring: &mut SnapshotRing,
    frames: &mut [FrameRecord],
    changed_frame: usize,
    current_frame: usize,
    trace: bool,
) -> Result<Option<RollbackEvent>, String> {
    let actual_buttons = frames[changed_frame].actual_buttons;
    if frames[changed_frame].active_buttons == actual_buttons {
        return Ok(None);
    }

    let predicted_buttons = frames[changed_frame].predicted_buttons;
    frames[changed_frame].active_buttons = actual_buttons;
    let hash_after_resim = resimulate_from(state, ring, frames, changed_frame, current_frame)?;
    let event = RollbackEvent {
        arrival_frame: current_frame,
        changed_frame,
        predicted_buttons,
        actual_buttons,
        resimulated_frames: current_frame - changed_frame,
        hash_after_resim,
    };
    if trace {
        eprintln!(
            "rollback: arrival_frame={} changed_frame={} predicted=0x{:04x} actual=0x{:04x} resimulated={} hash=0x{:016x}",
            event.arrival_frame,
            event.changed_frame,
            event.predicted_buttons,
            event.actual_buttons,
            event.resimulated_frames,
            event.hash_after_resim
        );
    }
    Ok(Some(event))
}

fn reference_desync(frames: &[FrameRecord]) -> Result<(u64, Option<DesyncReport>), String> {
    unsafe {
        let state = MiniCreate(MINI_GAME_WIDTH, MINI_GAME_HEIGHT);
        if state.is_null() {
            return Err("MiniCreate failed for reference run".to_string());
        }

        let mut first_desync = None;
        let mut reference_hash = MiniStateHash(state);
        for (frame, record) in frames.iter().enumerate() {
            MiniStepButtons(state, record.actual_buttons, false);
            reference_hash = MiniStateHash(state);
            if first_desync.is_none() && record.hash_after != reference_hash {
                first_desync = Some(DesyncReport {
                    after_frame: frame + 1,
                    rollback_hash: record.hash_after,
                    reference_hash,
                });
            }
        }

        MiniDestroy(state);
        Ok((reference_hash, first_desync))
    }
}

fn run_rollback_host(options: &Options) -> Result<RollbackSummary, String> {
    let mut frame_records = (0..options.frames)
        .map(|frame| {
            let predicted_buttons = rollback_predicted_buttons(frame);
            FrameRecord {
                predicted_buttons,
                actual_buttons: rollback_actual_buttons(frame),
                active_buttons: predicted_buttons,
                hash_after: 0,
            }
        })
        .collect::<Vec<_>>();

    unsafe {
        let state = MiniCreate(MINI_GAME_WIDTH, MINI_GAME_HEIGHT);
        if state.is_null() {
            return Err("MiniCreate failed".to_string());
        }

        let snapshot_size = MiniSaveStateSize();
        let mut ring = SnapshotRing::new(options.rollback_window, snapshot_size);
        let mut current_frame = 0usize;
        let mut next_delivery_frame = 0usize;
        let mut events = Vec::new();
        let mut resimulated_frames = 0usize;

        while current_frame < options.frames {
            save_and_step(state, &mut ring, &mut frame_records, current_frame)?;
            current_frame += 1;

            while next_delivery_frame < options.frames
                && next_delivery_frame + options.input_delay <= current_frame
            {
                if let Some(event) = resolve_delayed_input(
                    state,
                    &mut ring,
                    &mut frame_records,
                    next_delivery_frame,
                    current_frame,
                    options.trace,
                )? {
                    resimulated_frames += event.resimulated_frames;
                    events.push(event);
                }
                next_delivery_frame += 1;
            }
        }

        while next_delivery_frame < options.frames {
            if let Some(event) = resolve_delayed_input(
                state,
                &mut ring,
                &mut frame_records,
                next_delivery_frame,
                current_frame,
                options.trace,
            )? {
                resimulated_frames += event.resimulated_frames;
                events.push(event);
            }
            next_delivery_frame += 1;
        }

        let final_hash = MiniStateHash(state);
        MiniDestroy(state);

        let (reference_hash, first_desync) = reference_desync(&frame_records)?;

        Ok(RollbackSummary {
            frames: options.frames,
            input_delay: options.input_delay,
            rollback_window: options.rollback_window,
            snapshot_size,
            rollback_count: events.len(),
            resimulated_frames,
            final_hash,
            reference_hash,
            first_desync,
            events,
        })
    }
}

fn print_rollback_summary(summary: &RollbackSummary) {
    print!(
        "{{\"build\":\"mini-rust-host\",\"mode\":\"rollback\",\"frames\":{},\"input_delay\":{},\"rollback_window\":{},\"snapshot_size\":{},\"rollbacks\":{},\"resimulated_frames\":{},\"state_hash\":\"0x{:016x}\",\"reference_hash\":\"0x{:016x}\",\"desync\":{}",
        summary.frames,
        summary.input_delay,
        summary.rollback_window,
        summary.snapshot_size,
        summary.rollback_count,
        summary.resimulated_frames,
        summary.final_hash,
        summary.reference_hash,
        if summary.first_desync.is_some() { "true" } else { "false" }
    );

    if let Some(desync) = &summary.first_desync {
        print!(
            ",\"first_desync\":{{\"after_frame\":{},\"rollback_hash\":\"0x{:016x}\",\"reference_hash\":\"0x{:016x}\"}}",
            desync.after_frame, desync.rollback_hash, desync.reference_hash
        );
    }

    print!(",\"events\":[");
    for (index, event) in summary.events.iter().enumerate() {
        if index != 0 {
            print!(",");
        }
        print!(
            "{{\"arrival_frame\":{},\"changed_frame\":{},\"predicted\":\"0x{:04x}\",\"actual\":\"0x{:04x}\",\"resimulated_frames\":{},\"hash_after_resim\":\"0x{:016x}\"}}",
            event.arrival_frame,
            event.changed_frame,
            event.predicted_buttons,
            event.actual_buttons,
            event.resimulated_frames,
            event.hash_after_resim
        );
    }
    println!("]}}");
}

fn run() -> Result<(), String> {
    let options = parse_args()?;
    let room_export_path = match &options.room_export_path {
        Some(path) => {
            Some(CString::new(path.as_str()).map_err(|_| "room-export path contains NUL")?)
        }
        None => None,
    };

    unsafe {
        MiniStubs_SetRoomExportPath(
            room_export_path
                .as_ref()
                .map_or(std::ptr::null(), |path| path.as_ptr()),
        );
    }

    if options.rollback {
        let summary = run_rollback_host(&options)?;
        let desynced = summary.first_desync.is_some();
        print_rollback_summary(&summary);
        if desynced {
            return Err("rollback desync detected against reference run".to_string());
        }
        Ok(())
    } else {
        run_basic_host(&options)
    }
}

fn main() {
    if let Err(error) = run() {
        eprintln!("{error}");
        process::exit(1);
    }
}
