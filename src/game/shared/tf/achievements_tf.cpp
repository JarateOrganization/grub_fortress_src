//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================


#include "cbase.h"

#ifdef CLIENT_DLL

#include "achievementmgr.h"
#include "baseachievement.h"
#include "tf_hud_statpanel.h"
#include "c_tf_team.h"
#include "c_tf_player.h"
#include "c_tf_playerresource.h"
#include "tf_gamerules.h"
#include "econ_wearable.h"
#include "achievements_tf.h"
#include "usermessages.h"

// NVNT include for tf2 damage
#include "haptics/haptic_utils.h"

CAchievementMgr g_AchievementMgrTF;	// global achievement mgr for TF

bool CheckWinNoEnemyCaps( IGameEvent *event, int iRole );

// Grace period that we allow a player to start after level init and still consider them to be participating for the full round.  This is fairly generous
// because it can in some cases take a client several minutes to connect with respect to when the server considers the game underway
#define TF_FULL_ROUND_GRACE_PERIOD	( 4 * 60.0f )

bool IsLocalTFPlayerClass( int iClass );


bool CBaseTFAchievementSimple::LocalPlayerCanEarn( void ) 
{ 
	if ( TFGameRules() )
	{
		bool bMVMAchievement = ( m_iAchievementID >= ACHIEVEMENT_TF_MVM_START_RANGE && m_iAchievementID <= ACHIEVEMENT_TF_MVM_END_RANGE );

		if ( bMVMAchievement )
		{
			if ( !TFGameRules()->IsMannVsMachineMode() || ( GetLocalPlayerTeam() != TF_TEAM_PVE_DEFENDERS ) )
			{
				return false;
			}
		}
		else
		{
			if ( TFGameRules()->IsMannVsMachineMode() )
			{
				return false;
			}
		}
	}

	return BaseClass::LocalPlayerCanEarn();
}

void CBaseTFAchievementSimple::FireGameEvent( IGameEvent *event )
{
	if ( !LocalPlayerCanEarn() )
		return;

	BaseClass::FireGameEvent( event );
}


bool CBaseTFAchievement::LocalPlayerCanEarn( void ) 
{ 
	// Swallow game events if we're not allowed to earn achievements, or if the local player isn't the right class
	if ( !GameRulesAllowsAchievements() )
	{
		return false;
	}

	// Determine class & check it
	if ( m_iAchievementID >= ACHIEVEMENT_START_CLASS_SPECIFIC && m_iAchievementID <= ACHIEVEMENT_END_CLASS_SPECIFIC )
	{
		int iClass = floor( (m_iAchievementID - ACHIEVEMENT_START_CLASS_SPECIFIC) / 100.0f ) + 1;
		if ( !IsLocalTFPlayerClass( iClass ) )
		{
			return false;
		}
	}

	return BaseClass::LocalPlayerCanEarn();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFAchievementFullRound::Init() 
{
	m_iFlags |= ACH_FILTER_FULL_ROUND_ONLY;		
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFAchievementFullRound::ListenForEvents()
{
	ListenForGameEvent( "teamplay_round_win" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFAchievementFullRound::FireGameEvent_Internal( IGameEvent *event )
{
	if ( 0 == Q_strcmp( event->GetName(), "teamplay_round_win" ) )
	{
		C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
		if ( pLocalPlayer )
		{
			// is the player currently on a game team?
			int iTeam = pLocalPlayer->GetTeamNumber();
			if ( iTeam >= FIRST_GAME_TEAM ) 
			{
				float flRoundTime = event->GetFloat( "round_time", 0 );
				if ( flRoundTime > 0 )
				{
					Event_OnRoundComplete( flRoundTime, event );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFAchievementFullRound::PlayerWasInEntireRound( float flRoundTime )
{
	float flTeamplayStartTime = m_pAchievementMgr->GetTeamplayStartTime();
	if ( flTeamplayStartTime > 0 ) 
	{	
		// has the player been present and on a game team since the start of this round (minus a grace period)?
		if ( flTeamplayStartTime < ( gpGlobals->curtime - flRoundTime ) + TF_FULL_ROUND_GRACE_PERIOD )
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Helper function to determine if local player is specified class
//-----------------------------------------------------------------------------
bool IsLocalTFPlayerClass(int iClass)
{
	C_TFPlayer* pLocalPlayer = C_TFPlayer::GetLocalTFPlayer();
	return(pLocalPlayer && pLocalPlayer->IsPlayerClass(iClass));
}

//-----------------------------------------------------------------------------
// Purpose: Query if the gamerules allows achievement progress at this time
//-----------------------------------------------------------------------------
bool GameRulesAllowsAchievements(void)
{
	bool bRetVal = false;

	if ((TFGameRules()->State_Get() < GR_STATE_TEAM_WIN) ||
		(TFGameRules()->State_Get() == GR_STATE_STALEMATE))
	{
		bRetVal = true;
	}

	return bRetVal;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CAchievementTFGrubPlayGameEveryClass : public CTFAchievementFullRound
{
	DECLARE_CLASS( CAchievementTFGrubPlayGameEveryClass, CTFAchievementFullRound );
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_HAS_COMPONENTS | ACH_FILTER_FULL_ROUND_ONLY );
		SetGoal( ( TF_LAST_NORMAL_CLASS - 1 ) - TF_FIRST_NORMAL_CLASS + 1 ); //( TF_LAST_NORMAL_CLASS - 1 ) to exclude the new civilian class
		BaseClass::Init();
	}

	virtual void Event_OnRoundComplete( float flRoundTime, IGameEvent *event )
	{
		float flLastClassChangeTime = m_pAchievementMgr->GetLastClassChangeTime();
		if ( flLastClassChangeTime > 0 ) 
		{					
			// has the player been present and not changed class since the start of this round (minus a grace period)?
			if ( flLastClassChangeTime < ( gpGlobals->curtime - flRoundTime ) + TF_FULL_ROUND_GRACE_PERIOD )
			{
				C_TFPlayer *pTFPlayer = C_TFPlayer::GetLocalTFPlayer();
				if ( pTFPlayer )
				{
					int iClass = pTFPlayer->GetPlayerClass()->GetClassIndex();
					if ( iClass >= TF_FIRST_NORMAL_CLASS && iClass <= ( TF_LAST_NORMAL_CLASS - 1 ) ) //( TF_LAST_NORMAL_CLASS - 1 ) to exclude the new civilian class
					{
						// yes, the achievement is satisfied for this class, set the corresponding bit
						int iBitNumber =( iClass - TF_FIRST_NORMAL_CLASS );
						EnsureComponentBitSetAndEvaluate( iBitNumber );
					}							
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementTFGrubPlayGameEveryClass, ACHIEVEMENT_TFGRUB_PLAY_GAME_EVERYCLASS, "TFGRUB_PLAY_GAME_EVERYCLASS", 5 );

class CAchievementTFGrubPlayGameEveryMap : public CTFAchievementFullRound
{
	DECLARE_CLASS( CAchievementTFGrubPlayGameEveryMap, CTFAchievementFullRound );
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_HAS_COMPONENTS | ACH_FILTER_FULL_ROUND_ONLY );

		static const char *szComponents[] =
		{
			"koth_landfall", "koth_lumberyard", "ctf_warehouse", "koth_well", "bd_bombyard"
		};		
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE( szComponents );
		SetGoal( m_iNumComponents );
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent( "teamplay_round_win" );
	}

	virtual void Event_OnRoundComplete( float flRoundTime, IGameEvent *event )
	{
		float flTeamplayStartTime = m_pAchievementMgr->GetTeamplayStartTime();
		if ( flTeamplayStartTime > 0 ) 
		{	
			// has the player been present and on a game team since the start of this round (minus a grace period)?
			if ( flTeamplayStartTime < ( gpGlobals->curtime - flRoundTime ) + TF_FULL_ROUND_GRACE_PERIOD )
			{
				// yes, the achievement is satisfied for this map, set the corresponding bit
				OnComponentEvent( m_pAchievementMgr->GetMapName() );
			}
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementTFGrubPlayGameEveryMap, ACHIEVEMENT_TFGRUB_PLAY_GAME_EVERYMAP, "TFGRUB_PLAY_GAME_EVERYMAP", 5 );

class CAchievementTFGrubScout_KillWithNailgun : public CBaseTFAchievement
{
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS );
		SetGoal( 50 ); // kill 50 enemies with the nailgun
	}

	virtual void Event_EntityKilled( CBaseEntity *pVictim, CBaseEntity *pAttacker, CBaseEntity *pInflictor, IGameEvent *event ) 
	{
		// Achievement for killing enemies with the nailgun
		if ( event->GetInt( "weaponid" ) == TF_WEAPON_NAILGUN )
		{
			IncrementCount();
		}
	}
};
DECLARE_ACHIEVEMENT( CAchievementTFGrubScout_KillWithNailgun, ACHIEVEMENT_TF_SCOUT_KILL_WITH_NAILGUN, "TF_SCOUT_KILL_WITH_NAILGUN", 5 );

class CAchievementTFGrubScout_KillWithNailgun2 : public CBaseTFAchievement
{
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL | ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS);
		SetGoal(100); // kill 100 enemies with the nailgun
	}

	virtual void Event_EntityKilled(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, IGameEvent* event)
	{
		// Achievement for killing enemies with the nailgun
		if (event->GetInt("weaponid") == TF_WEAPON_NAILGUN)
		{
			IncrementCount();
		}
	}
};
DECLARE_ACHIEVEMENT(CAchievementTFGrubScout_KillWithNailgun2, ACHIEVEMENT_TF_SCOUT_KILL_WITH_NAILGUN2, "TF_SCOUT_KILL_WITH_NAILGUN2", 5);

class CAchievementTFGrubScout_KillWithNailgun3 : public CBaseTFAchievement
{
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL | ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS);
		SetGoal(500); // kill 500 enemies with the nailgun
	}

	virtual void Event_EntityKilled(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, IGameEvent* event)
	{
		// Achievement for killing enemies with the nailgun
		if (event->GetInt("weaponid") == TF_WEAPON_NAILGUN)
		{
			IncrementCount();
		}
	}
};
DECLARE_ACHIEVEMENT(CAchievementTFGrubScout_KillWithNailgun3, ACHIEVEMENT_TF_SCOUT_KILL_WITH_NAILGUN3, "TF_SCOUT_KILL_WITH_NAILGUN3", 5);

class CAchievementTFGrubPlayGame2Fort : public CTFAchievementFullRound
{
	DECLARE_CLASS(CAchievementTFGrubPlayGame2Fort, CTFAchievementFullRound);
	void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL | ACH_HAS_COMPONENTS | ACH_FILTER_FULL_ROUND_ONLY);

		static const char* szComponents[] =
		{
			"ctf_2fort"
		};
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE(szComponents);
		SetGoal(m_iNumComponents);
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("teamplay_round_win");
	}

	virtual void Event_OnRoundComplete(float flRoundTime, IGameEvent* event)
	{
		float flTeamplayStartTime = m_pAchievementMgr->GetTeamplayStartTime();
		if (flTeamplayStartTime > 0)
		{
			// has the player been present and on a game team since the start of this round (minus a grace period)?
			if (flTeamplayStartTime < (gpGlobals->curtime - flRoundTime) + TF_FULL_ROUND_GRACE_PERIOD)
			{
				// yes, the achievement is satisfied for this map, set the corresponding bit
				OnComponentEvent(m_pAchievementMgr->GetMapName());
			}
		}
	}
};
DECLARE_ACHIEVEMENT(CAchievementTFGrubPlayGame2Fort, ACHIEVEMENT_TFGRUB_PLAY_GAME_2FORT, "TFGRUB_PLAY_GAME_2FORT", 5);

#endif // CLIENT_DLL
