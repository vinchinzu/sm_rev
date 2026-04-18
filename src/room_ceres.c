// Ceres room-main helpers extracted from Bank $89.

#include "ida_types.h"
#include "variables.h"
#include "funcs.h"

#define kCeresElevatorMode7Table ((uint16*)RomFixedPtr(0x89ad5f))

enum {
  kRoomMainAsmVar0 = 0,
  kRoomMainAsmVar2 = 2,
  kCeresElevatorLeftX = 112,
  kCeresElevatorRightX = 144,
  kCeresElevatorTopY = 75,
  kCeresElevatorBottomY = 128,
};

static inline uint16 ReadRoomMainAsmWord(uint16 offset) {
  return *(uint16 *)(room_main_asm_variables + offset);
}

static inline int16 ReadRoomMainAsmSignedWord(uint16 offset) {
  return *(int16 *)(room_main_asm_variables + offset);
}

static inline void WriteRoomMainAsmWord(uint16 offset, uint16 value) {
  *(uint16 *)(room_main_asm_variables + offset) = value;
}

static uint16 AdvanceCeresElevatorFrame(uint16 frame) {
  if ((int16)frame < 0)
    return frame == 0x8001 ? 0 : frame - 1;
  return frame == 67 ? (uint16)-32700 : frame + 1;
}

static bool SamusIsOnCeresElevator(void) {
  return (int16)(kCeresElevatorLeftX - samus_x_pos) < 0
      && (int16)(kCeresElevatorRightX - samus_x_pos) >= 0
      && sign16(samus_y_pos - kCeresElevatorBottomY)
      && !sign16(samus_y_pos - kCeresElevatorTopY)
      && !samus_y_speed
      && !samus_y_subspeed;
}

void RoomCode_CeresElevatorShaft(void) {  // 0x89ACC3
  if ((ceres_status & 0x8000) == 0)
    return;

  if (SamusIsOnCeresElevator() && game_state == kGameState_8_MainGameplay) {
    CallSomeSamusCode(2);
    screen_fade_delay = 0;
    screen_fade_counter = 0;
    game_state = kGameState_32_MadeItToCeresElevator;
  }

  WriteRoomMainAsmWord(kRoomMainAsmVar2, ReadRoomMainAsmWord(kRoomMainAsmVar2) - 1);
  if (ReadRoomMainAsmSignedWord(kRoomMainAsmVar2) < 0) {
    uint16 frame_index = ReadRoomMainAsmWord(kRoomMainAsmVar0);
    uint16 table_index = (uint16)(6 * frame_index) >> 1;
    WriteRoomMainAsmWord(kRoomMainAsmVar2, kCeresElevatorMode7Table[table_index]);
    reg_M7B = kCeresElevatorMode7Table[table_index + 1];
    reg_M7C = -reg_M7B;
    reg_M7A = kCeresElevatorMode7Table[table_index + 2];
    reg_M7D = reg_M7A;
    WriteRoomMainAsmWord(kRoomMainAsmVar0, AdvanceCeresElevatorFrame(frame_index));
  }
}
