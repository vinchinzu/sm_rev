# pico-kernel source lists. Desktop MINI_SRCS stays a wildcard.
# Classification: KEEP_SHIP1 / KEEP_SHIP2 / KEEP_SHIP3 / STUB
#
# Inventory of MINI_SHARED_ENGINE_SRCS (CORE_SRCS minus src/main.c,
# src/sm_cpu_infra.c, src/sm_rtl.c, src/predict_cli.c). Every remaining
# src/*.c appears in exactly one of SHIP1 / SHIP2 / SHIP3 / STUB.
#
# Split leftovers (no longer TUs; stale .mini.o may still sit on disk):
#   enemy_fauna.c, enemy_a2_misc.c, enemy_ridley_zebetite.c, gameplay_frame.c
# Those families now live in the per-creature / per-topic files below.
#
# ---------------------------------------------------------------------------
# FUNC16 / Call* notes for sm_rev-t6y (do not implement stubs here)
# ---------------------------------------------------------------------------
# Samus handlers stay KEEP. Do not stub:
#   CallSomeSamusCode, Samus_CallInputHandler, Samus_FrameHandler*,
#   Samus_Movement_*, pose/transition FUNC16 targets in samus_*.c,
#   CallPlmHeaderFunc / CallPlmPreInstr / CallPlmInstr / CallPlmInstrFunc,
#   CallDoorTransitionFunction_Async, CallDoorDefSetupCode,
#   CallGrappleNextFunc (samus_grapple.c is KEEP_SHIP1).
#
# Stubbing these TUs will require FUNC16 / Call* stubs for:
#   CallCinematicFunction, CinematicFunctionOpening,
#     CinematicFunctionBlackoutFromCeres, CinematicFunctionEscapeFromCebes,
#     GameState_1_OpeningCinematic_, GameState_37_CeresGoesBoomWithSamus_,
#     GameState_39_EndingAndCredits_  (cinematics.c)
#   CallEprojInit, CallEprojPreInstr, CallEprojInstr  (eproj_combat.c;
#     eproj_core.c / eproj_pirate.c still call these)
#   CallRidleyFunc, Ridley_Func_*  (enemy_ridley.c; Ceres Ridley calls them)
#   CallHdmaobjPreInstr cases that live in boss_hdma / cinematic_hdma /
#     mother_brain_hdma / samus_xray  (CallHdmaobj* itself is in hdma_core)
#   CallPauseHook, CallUnpauseHook_Async, GameState_12..18  (pause_menu.c)
#   GameState_2_GameOptionsMenu, CallOptions*  (options_menu.c)
#   GameState_4_FileSelectMenus, GameState_5_FileSelectMap
#   GameState_19..25, HandleSamusOutOfHealthAndGameTile, GameState_27
#     (game_over.c — MainGameplay calls the health handler)
#   GameState_26, GameOverMenu  (game_over_menu.c)
#   GameState_41..44, CallDemoRoomDataFunc, CallDemoPreInstr, CallDemoInstr
#   DisplayMessageBox_Async  (message_box.c)
#   HandleSoundEffects, QueueSfx*, QueueMusic_*  (sound_handler.c / spc_player.c)
#   CallMotherBrainFunc and related  (enemy_mother_brain.c)
#   EnemyAiFromAddr / BindEnemyDefAi / EnemyRunPreInstr / EnemyRunInstr
#     leftover rows in enemy_ai_table.c still name STUB creature symbols;
#     drain those rows or stub the named AI when linking ship 2 without fauna.
#   PlmSetup_* in plm_rooms.c if plm_dispatch.c (KEEP_SHIP1) is linked.
# ---------------------------------------------------------------------------

# Authored room + Samus physics/collision/projectiles + PLM core + doors +
# room load + mini kernel API. Brutal: no Ridley, no LS original extras.
PICO_KERNEL_SHIP1_SRCS := \
  src/config.c \
  src/default_controls.c \
  src/util.c \
  src/trig_tables.c \
  src/physics.c \
  src/physics_config.c \
  src/samus_input.c \
  src/samus_motion.c \
  src/samus_jump.c \
  src/samus_air.c \
  src/samus_ball.c \
  src/samus_ground.c \
  src/samus_speed.c \
  src/samus_special_move.c \
  src/samus_collision.c \
  src/samus_collision_block.c \
  src/samus_collision_map.c \
  src/samus_collision_advanced.c \
  src/samus_enemy_collision.c \
  src/samus_pose.c \
  src/samus_runtime.c \
  src/samus_transition.c \
  src/samus_draw.c \
  src/samus_palette.c \
  src/samus_anim_fx.c \
  src/samus_camera_map.c \
  src/samus_resource.c \
  src/samus_status.c \
  src/samus_full_spec.c \
  src/samus_asset_bridge.c \
  src/samus_death.c \
  src/samus_grapple.c \
  src/samus_projectile_state.c \
  src/samus_projectile_weapon.c \
  src/samus_projectile_core.c \
  src/samus_projectile_beam.c \
  src/samus_projectile_block.c \
  src/samus_projectile_view.c \
  src/spritemap_draw.c \
  src/plm_core.c \
  src/plm_blocks.c \
  src/plm_dispatch.c \
  src/plm_draw.c \
  src/plm_preinstr.c \
  src/door_setup.c \
  src/door_transition.c \
  src/decompress.c \
  src/room_header.c \
  src/room_transition.c \
  src/room_setup.c \
  src/room_state_select.c \
  src/room_scrolling.c \
  src/game_init.c \
  src/game_state_extras.c \
  src/irq.c \
  src/palette_fader.c \
  src/multi_samus.c

# Landing Site original-loop extras: in-scope enemies, OAM, palette FX,
# HDMA objects, Vector_NMI. Still not Kraid / Draygon / Mother Brain.
# enemy_main / enemy_math / enemy_ai_* are STUB for ship 1; ship 2 KEEPs them.
PICO_KERNEL_SHIP2_SRCS := \
  src/enemy_main.c \
  src/enemy_math.c \
  src/enemy_ai_canon.c \
  src/enemy_ai_table.c \
  src/enemy_tiles.c \
  src/enemy_touch.c \
  src/enemy_shot.c \
  src/enemy_block_collision.c \
  src/enemy_drops.c \
  src/enemy_config.c \
  src/enemy_gunship.c \
  src/eproj_core.c \
  src/eproj_environment.c \
  src/sprite_objects.c \
  src/palette_fx.c \
  src/hdma_core.c \
  src/hdma_power_bomb.c \
  src/room_fx.c \
  src/room_fx_hdma.c \
  src/room_main.c \
  src/anim_tiles.c \
  src/nmi_transfer.c \
  src/hud.c \
  src/save_sram.c

# Ceres elevator, Ceres Ridley, countdown, debris, escape states 32-37,
# Ceres rooms/doors/steam. GameState_32..37 live in game_state_extras.c
# (already KEEP_SHIP1); this list is the remaining Ceres TUs.
# Norfair Ridley (enemy_ridley.c) stays STUB — peel CallRidleyFunc later.
PICO_KERNEL_SHIP3_SRCS := \
  src/enemy_ceres_ridley.c \
  src/enemy_ceres_door.c \
  src/enemy_elevator.c \
  src/eproj_pirate.c \
  src/room_ceres.c \
  src/timer.c

# Title/intro/escape/credits (~340 KB) plus cinematic/boss HDMA.
PICO_STUB_CINEMATIC_SRCS := \
  src/cinematics.c \
  src/cinematic_hdma.c \
  src/boss_hdma.c \
  src/mother_brain_hdma.c

# Pause/map/equipment/file-select/game-over/demo/x-ray/message boxes.
PICO_STUB_MENU_SRCS := \
  src/file_select_menu.c \
  src/file_select_map.c \
  src/options_menu.c \
  src/pause_menu.c \
  src/map_screen.c \
  src/equipment_screen.c \
  src/menu_common.c \
  src/game_over.c \
  src/game_over_menu.c \
  src/message_box.c \
  src/demo_manager.c \
  src/samus_demo.c \
  src/samus_xray.c \
  src/plm_rooms.c

# Audio, SNES emu dispatcher, debug tracing.
PICO_STUB_HOST_SRCS := \
  src/sound_handler.c \
  src/spc_player.c \
  src/sm_dispatcher.c \
  src/tracing.c

# Leftover Bank $86 dispatch (~133 KB) and out-of-scope eproj families.
PICO_STUB_EPROJ_SRCS := \
  src/eproj_combat.c \
  src/eproj_tourian.c \
  src/eproj_pickup.c \
  src/eproj_skree.c \
  src/eproj_kraid.c \
  src/eproj_draygon.c \
  src/eproj_crocomire.c \
  src/eproj_botwoon.c \
  src/eproj_ki_hunter.c

# Bosses: MB/Shitroid, Norfair Ridley+Zebetite, Kraid, Phantoon, Draygon,
# Crocomire, Botwoon, Torizo, Spore Spawn. Ceres Ridley is SHIP3, not here.
PICO_STUB_BOSS_SRCS := \
  src/enemy_mother_brain.c \
  src/enemy_shitroid.c \
  src/enemy_dead_monsters.c \
  src/enemy_ridley.c \
  src/enemy_zebetite.c \
  src/enemy_kraid.c \
  src/enemy_fake_kraid.c \
  src/enemy_phantoon.c \
  src/enemy_draygon.c \
  src/enemy_mini_draygon.c \
  src/enemy_crocomire.c \
  src/enemy_mini_crocomire.c \
  src/enemy_botwoon.c \
  src/enemy_torizo.c \
  src/enemy_torizo_attacks.c \
  src/enemy_torizo_finale.c \
  src/enemy_torizo_projectiles.c \
  src/torizo_config.c \
  src/enemy_spore_spawn.c \
  src/enemy_chozo_shaktool.c

# Out-of-scope fauna (ex-enemy_fauna / enemy_a2_misc / A8 leftovers).
PICO_STUB_FAUNA_SRCS := \
  src/enemy_ki_hunter.c \
  src/enemy_space_pirates.c \
  src/enemy_baby_metroid.c \
  src/enemy_escape_typewriter.c \
  src/enemy_metroid.c \
  src/enemy_mochtroid.c \
  src/enemy_zoomer.c \
  src/enemy_skree.c \
  src/enemy_sidehopper.c \
  src/enemy_waver.c \
  src/enemy_metalee.c \
  src/enemy_fireflea.c \
  src/enemy_slug.c \
  src/enemy_roach.c \
  src/enemy_bang.c \
  src/enemy_reflec.c \
  src/enemy_shutter.c \
  src/enemy_falling_platform.c \
  src/enemy_goofball_rio.c \
  src/enemy_rinka.c \
  src/enemy_beyblade_turtle.c \
  src/enemy_hopping_blobs.c \
  src/enemy_spike_plant.c \
  src/enemy_spikey_shell.c \
  src/enemy_gripper.c \
  src/enemy_ripper.c \
  src/enemy_flies.c \
  src/enemy_fireball.c \
  src/enemy_lavaquake_rocks.c \
  src/enemy_hirising.c \
  src/enemy_lava_seahorse.c \
  src/enemy_walking_lava_seahorse.c \
  src/enemy_norfair_lava_man.c \
  src/enemy_fune.c \
  src/enemy_fire_geyser.c \
  src/enemy_boulder.c \
  src/enemy_nuclear_waffle.c \
  src/enemy_spikey_platform.c \
  src/enemy_beetom.c \
  src/enemy_yapping_maw.c \
  src/enemy_evir_projectile.c \
  src/enemy_kago.c \
  src/enemy_morph_ball_eye.c \
  src/enemy_blue_brinstar_face_block.c \
  src/enemy_wrecked_ship.c \
  src/enemy_maridia_candy.c \
  src/enemy_maridia_fish.c \
  src/enemy_maridia_floater.c \
  src/enemy_maridia_large_snail.c \
  src/enemy_maridia_puffer.c \
  src/enemy_maridia_snail.c

PICO_STUB_SRCS := \
  $(PICO_STUB_CINEMATIC_SRCS) \
  $(PICO_STUB_MENU_SRCS) \
  $(PICO_STUB_HOST_SRCS) \
  $(PICO_STUB_EPROJ_SRCS) \
  $(PICO_STUB_BOSS_SRCS) \
  $(PICO_STUB_FAUNA_SRCS)

# Mini gameplay kernel API used by pico-kernel. Host/SDL/browser/climb stay off.
PICO_MINI_RUNTIME_KEEP_SRCS := \
  src/mini/mini_game.c \
  src/mini/mini_system.c \
  src/mini/mini_platform_stubs.c \
  src/mini/mini_ppu_stub.c \
  src/mini/mini_room_adapter.c \
  src/mini/mini_authored_movement.c \
  src/mini/mini_content_scope.c \
  src/mini/mini_door_transition.c \
  src/mini/mini_enemy_runtime.c \
  src/mini/mini_enemy_metadata.c \
  src/mini/mini_asset_bootstrap.c \
  src/mini/mini_editor_bridge.c \
  src/mini/mini_editor_path.c \
  src/mini/mini_rom_bootstrap.c \
  src/mini/mini_room_fx.c \
  src/mini/mini_run_mode.c

# omit-from-pico: desktop host, renderer, replay I/O, climb, multiplayer/browser.
PICO_MINI_RUNTIME_OMIT_SRCS := \
  src/mini/mini_main.c \
  src/mini/mini_runtime.c \
  src/mini/mini_renderer.c \
  src/mini/mini_record.c \
  src/mini/mini_input_script.c \
  src/mini/mini_backdrop.c \
  src/mini/mini_replay.c \
  src/mini/mini_predict.c \
  src/mini/mini_net_bridge.c \
  src/mini/mini_multiplayer_combat.c \
  src/mini/mini_climb_endless.c \
  src/mini/mini_generated_background.c \
  src/mini/mini_wram_peek.c

# Default pico-kernel = ship 1 only
PICO_KERNEL_SRCS := $(PICO_KERNEL_SHIP1_SRCS) $(PICO_MINI_RUNTIME_KEEP_SRCS)
