#include <libdl/stdio.h>
#include <libdl/random.h>
#include <libdl/net.h>
#include <libdl/spawnpoint.h>
#include <libdl/area.h>
#include <libdl/color.h>
#include <libdl/collision.h>
#include <libdl/time.h>
#include "game.h"
#include "cgm.h"
#include "cgm_score.h"
#include "cgm_netmsg.h"

#ifndef PLAYER_SPAWN_CUBOID_IDX
#define PLAYER_SPAWN_CUBOID_IDX (1)
#endif


//--------------------------------------------------------------------------
struct GameState State = {};

//--------------------------------------------------------------------------
int spleefResolveCParamTimelimit(void)
{
  int paramValue = cgmGetParameter(CPARAM_ID_TIMELIMIT);
  switch (paramValue)
  {
    case CPARAM_TIMELIMIT_NONE: return 0;
    case CPARAM_TIMELIMIT_5: return 5;
    case CPARAM_TIMELIMIT_10: return 10;
    case CPARAM_TIMELIMIT_15: return 15;
    case CPARAM_TIMELIMIT_20: return 20;
    case CPARAM_TIMELIMIT_25: return 25;
    case CPARAM_TIMELIMIT_30: return 30;
    default: return 0;
  }
}

//--------------------------------------------------------------------------
int spleefResolveCParamPointsToWin(void)
{
  int paramValue = cgmGetParameter(CPARAM_ID_POINTS_TO_WIN);
  switch (paramValue)
  {
    case CPARAM_POINTS_TO_WIN_20: return 20;
    case CPARAM_POINTS_TO_WIN_25: return 25;
    case CPARAM_POINTS_TO_WIN_30: return 30;
    case CPARAM_POINTS_TO_WIN_NONE: return 0;
    case CPARAM_POINTS_TO_WIN_5: return 5;
    case CPARAM_POINTS_TO_WIN_10: return 10;
    case CPARAM_POINTS_TO_WIN_15: return 15;
    default: return 20;
  }
}

//--------------------------------------------------------------------------
int spleefResolveSpleefLayers(void)
{
  int paramValue = cgmGetParameter(CPARAM_ID_SPLEEF_LAYERS);
  switch (paramValue)
  {
    case CPARAM_SPLEEF_LAYERS_2: return 2;
    case CPARAM_SPLEEF_LAYERS_1: return 1;
    default: return 2;
  }
}

//--------------------------------------------------------------------------
void spleefGetResurrectPoint(Player* player, VECTOR outPos, VECTOR outRot, int firstRes)
{
  GameSettings* gameSettings = gameGetSettings();
  float idx = player->PlayerId / (float)gameSettings->PlayerCount;
  float theta = MATH_TAU * idx;

  vector_copy(outPos, &spawnPointGet(PLAYER_SPAWN_CUBOID_IDX)->M0[12]);
  outPos[0] += (cosf(theta) * SPLEEF_BOARD_SPAWN_RADIUS);
  outPos[1] += (sinf(theta) * SPLEEF_BOARD_SPAWN_RADIUS);

  vector_write(outRot, 0);
  outRot[2] = clampAngle(theta - MATH_PI);
}

//--------------------------------------------------------------------------
void onDestroySpleefMobyMessageReceived(int fromClientId, struct DestroySpleefMobyMessage *msg)
{
  Moby* moby = State.SpleefMobys[msg->MobyIdx];
  if (!moby || mobyIsDestroyed(moby))
    return;

  // destroy
  mobyDestroy(moby);
  State.SpleefMobys[msg->MobyIdx] = NULL;

  // inc
  cgmScoreIncCustomPlayerIntStat(msg->PlayerId, CSTAT_MOBYS_DESTROYED, 1, 0);
}

//--------------------------------------------------------------------------
void spleefMobyUpdate(Moby* moby)
{
	int i;
	GameSettings* gameSettings = gameGetSettings();
	MobyColDamage* colDamage = mobyGetDamage(moby, 0xfffffff, 0);

	if (moby->CollDamage != -1 && colDamage && colDamage->Moby == moby)
	{
		int damageClientId = colDamage->Damager->NetObjectGid.HostId;
		if (gameGetMyClientId() == damageClientId)
		{
			int playerId = 0;
			for (playerId = 0; playerId < GAME_MAX_PLAYERS; ++playerId)
			{
				if (gameSettings->PlayerClients[playerId] == damageClientId)
					break;
			}
			if (playerId == GAME_MAX_PLAYERS)
				playerId = -1;

			for (i = 0; i < SPLEEF_BOARD_BOX_MAX; ++i)
			{
				if (State.SpleefMobys[i] == moby)
				{
					// tell other clients we destroyed the instance
          struct DestroySpleefMobyMessage msg = 
          {
            .MobyIdx = i,
            .PlayerId = playerId
          };
          cgmNetMsgBroadcast_DestroySpleefMoby(&msg);
					return;
				}
			}

			return;
		}

		// remove damage
		moby->CollDamage = -1;
	}
}

//--------------------------------------------------------------------------
void spleefSpawnGrid(void)
{
  Moby** mobys = State.SpleefMobys;
  int spawnedCount = 0;
  VECTOR start;
  VECTOR pos;
  int layers = spleefResolveSpleefLayers();

  vector_copy(start, &spawnPointGet(PLAYER_SPAWN_CUBOID_IDX)->M0[12]);
  start[0] -= SPLEEF_BOARD_DIMENSION * 0.5 * SPLEEF_BOARD_BOX_SIZE;
  start[1] -= SPLEEF_BOARD_DIMENSION * 0.5 * SPLEEF_BOARD_BOX_SIZE;
  start[2] -= 10;
	vector_copy(pos, start);

	// Spawn boxes
  int k, i, j;
	for (k = 0; k < layers; ++k)
	{
		for (i = 0; i < SPLEEF_BOARD_DIMENSION; ++i)
		{
			for (j = 0; j < SPLEEF_BOARD_DIMENSION; ++j)
			{
				// delete old one
				int boxId = (k * SPLEEF_BOARD_DIMENSION * SPLEEF_BOARD_DIMENSION) + (i * SPLEEF_BOARD_DIMENSION) + j;
				if (!mobys[boxId] || mobys[boxId]->OClass != MOBY_ID_DARK_CATHEDRAL_SECRET_PLATFORM || mobyIsDestroyed(mobys[boxId]))
				{
					// spawn
					Moby* spawnedMoby = mobys[boxId] = mobySpawn(MOBY_ID_DARK_CATHEDRAL_SECRET_PLATFORM, 0);

					if (spawnedMoby)
					{
						vector_copy(spawnedMoby->Position, pos);

						spawnedMoby->UpdateDist = 0xFF;
						spawnedMoby->Drawn = 0x01;
						spawnedMoby->DrawDist = 0x0080;
						spawnedMoby->Opacity = 0x80;
						spawnedMoby->State = 1;
            spawnedMoby->ModeBits |= MOBY_MODE_BIT_CAN_BE_DAMAGED;
            spawnedMoby->PrimaryColor = colorLerp(0x404040, 0x808080, ((j + k + i) % 2) / 1.0);

						spawnedMoby->Scale = (float)0.021 * SPLEEF_BOARD_BOX_SIZE;
						spawnedMoby->GuberMoby = 0;
						spawnedMoby->PUpdate = &spleefMobyUpdate;

						++spawnedCount;
					}
				}

				pos[1] += SPLEEF_BOARD_BOX_SIZE;
			}

			pos[0] += SPLEEF_BOARD_BOX_SIZE;
			pos[1] = start[1];
		}

		pos[0] = start[0];
		pos[1] = start[1];
		pos[2] -= SPLEEF_BOARD_LAYER_OFFSET;
	}

}

//--------------------------------------------------------------------------
void spleefResetRound(void)
{
  spleefSpawnGrid();
}

//--------------------------------------------------------------------------
void modDraw(void)
{
  
}

//--------------------------------------------------------------------------
void modUpdate(void)
{
}

//--------------------------------------------------------------------------
void modCleanup(void)
{
  
}

//--------------------------------------------------------------------------
void modStart(void)
{
}

//--------------------------------------------------------------------------
void modInit(void)
{
  spleefResetRound();

  // set target points
  cgmScoreSetCustomTarget(spleefResolveCParamPointsToWin());
}

//--------------------------------------------------------------------------
void modLoad(void)
{
  // hook spawn
  HOOK_JAL(0x00610724, &spleefGetResurrectPoint);
  HOOK_JAL(0x005e2d44, &spleefGetResurrectPoint);

  // update timelimit
  GameOptions* gameOptions = gameGetOptions();
  gameOptions->GameFlags.MultiplayerGameFlags.Timelimit = spleefResolveCParamTimelimit();
}
