#ifndef GAME_H
#define GAME_H

#include <libdl/game.h>
#include <libdl/player.h>

//--------------------------------------------------------------------------
#define SPLEEF_BOARD_DIMENSION							(10)
#define SPLEEF_BOARD_MAX_LAYERS             (2)
#define SPLEEF_BOARD_LAYER_OFFSET						(20.0)
#define SPLEEF_BOARD_BOX_SIZE								(4.0)
#define SPLEEF_BOARD_SPAWN_RADIUS						(SPLEEF_BOARD_BOX_SIZE * ((SPLEEF_BOARD_DIMENSION + SPLEEF_BOARD_DIMENSION) / 6))
#define SPLEEF_BOARD_BOX_MAX								(SPLEEF_BOARD_DIMENSION * SPLEEF_BOARD_DIMENSION * SPLEEF_BOARD_MAX_LAYERS)

//--------------------------------------------------------------------------
enum GameCustomStatIds
{
  CSTAT_MOBYS_DESTROYED
};

//--------------------------------------------------------------------------
enum GameCustomParameterIds
{
  CPARAM_ID_TIMELIMIT,
  CPARAM_ID_POINTS_TO_WIN,
  CPARAM_ID_SPLEEF_LAYERS,
};

//--------------------------------------------------------------------------
enum GameCustomParameterTimelimitValues
{
  CPARAM_TIMELIMIT_NONE,
  CPARAM_TIMELIMIT_5,
  CPARAM_TIMELIMIT_10,
  CPARAM_TIMELIMIT_15,
  CPARAM_TIMELIMIT_20,
  CPARAM_TIMELIMIT_25,
  CPARAM_TIMELIMIT_30,
  CPARAM_TIMELIMIT_COUNT
};

//--------------------------------------------------------------------------
enum GameCustomParameterPointsToWinValues
{
  CPARAM_POINTS_TO_WIN_20,
  CPARAM_POINTS_TO_WIN_25,
  CPARAM_POINTS_TO_WIN_30,
  CPARAM_POINTS_TO_WIN_NONE,
  CPARAM_POINTS_TO_WIN_5,
  CPARAM_POINTS_TO_WIN_10,
  CPARAM_POINTS_TO_WIN_15,
  CPARAM_POINTS_TO_WIN_COUNT
};

//--------------------------------------------------------------------------
enum GameCustomParameterSpleefLayersValues
{
  CPARAM_SPLEEF_LAYERS_2,
  CPARAM_SPLEEF_LAYERS_1,
  CPARAM_SPLEEF_LAYERS_COUNT
};

//--------------------------------------------------------------------------
struct GameState
{
  Moby* SpleefMobys[SPLEEF_BOARD_DIMENSION * SPLEEF_BOARD_DIMENSION * SPLEEF_BOARD_MAX_LAYERS];
};

//--------------------------------------------------------------------------
extern struct GameState State;

//--------------------------------------------------------------------------

#endif // GAME_H
