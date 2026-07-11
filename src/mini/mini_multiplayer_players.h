#ifndef SM_MINI_MULTIPLAYER_PLAYERS_H_
#define SM_MINI_MULTIPLAYER_PLAYERS_H_

#include "mini_game.h"

int MiniMultiplayerPlayers_NormalizeCount(int player_count);
uint16 MiniMultiplayerPlayers_InputButtons(const MiniInputState *input,
                                           int player_index);

MiniSamusCoreState MiniMultiplayerPlayers_CoreFromGlobals(
    const MiniGameState *state);
void MiniMultiplayerPlayers_CopyCoreToGlobals(const MiniSamusCoreState *samus);
void MiniMultiplayerPlayers_SaveRuntime(MiniGameState *state, int player_index);
void MiniMultiplayerPlayers_LoadRuntime(const MiniGameState *state,
                                        int player_index);
void MiniMultiplayerPlayers_UpdateScreenPositions(MiniGameState *state);
void MiniMultiplayerPlayers_SyncFromOriginalRuntime(MiniGameState *state);
void MiniMultiplayerPlayers_InitializePlayerOne(MiniGameState *state);
void MiniMultiplayerPlayers_InitializePlayerTwo(MiniGameState *state);
void MiniMultiplayerPlayers_ApplyJoypad(const MiniGameState *state,
                                        int player_index);
void MiniMultiplayerPlayers_FaceEachOther(MiniGameState *state);
void MiniMultiplayerPlayers_RunPostMovementChecksForStored(
    MiniGameState *state);
void MiniMultiplayerPlayers_RunPostMovementChecksFromMultiSamus(
    MiniGameState *state);

#endif  // SM_MINI_MULTIPLAYER_PLAYERS_H_
