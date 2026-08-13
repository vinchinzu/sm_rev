#ifndef SM_MINI_TEST_ROOM_H_
#define SM_MINI_TEST_ROOM_H_

#include "mini/mini_game.h"

// Create a test room with basic collision geometry for golden tests
// Uses authored movement system (ROM-free physics)
MiniGameState *MiniTestRoom_CreateWithFloor(int viewport_width, int viewport_height);

#endif  // SM_MINI_TEST_ROOM_H_
