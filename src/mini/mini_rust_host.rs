use std::env;
use std::ffi::CString;
use std::os::raw::{c_char, c_int};
use std::process;

const MINI_GAME_WIDTH: c_int = 256;
const MINI_GAME_HEIGHT: c_int = 224;
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
}

fn parse_args() -> Result<Options, String> {
    let mut frames = 3usize;
    let mut room_export_path = None;
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
            "--help" | "-h" => {
                println!("Usage: sm_rev_mini_rs [--frames N] [--room-export PATH]");
                process::exit(0);
            }
            _ => return Err(format!("unknown option: {arg}")),
        }
    }
    Ok(Options {
        frames,
        room_export_path,
    })
}

fn scripted_buttons(frame: usize) -> u16 {
    match frame % 4 {
        1 => BUTTON_RIGHT,
        2 => BUTTON_RIGHT | BUTTON_A,
        _ => 0,
    }
}

fn run() -> Result<(), String> {
    let options = parse_args()?;
    let room_export_path = match &options.room_export_path {
        Some(path) => Some(CString::new(path.as_str()).map_err(|_| "room-export path contains NUL")?),
        None => None,
    };

    unsafe {
        MiniStubs_SetRoomExportPath(
            room_export_path
                .as_ref()
                .map_or(std::ptr::null(), |path| path.as_ptr()),
        );

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

fn main() {
    if let Err(error) = run() {
        eprintln!("{error}");
        process::exit(1);
    }
}
