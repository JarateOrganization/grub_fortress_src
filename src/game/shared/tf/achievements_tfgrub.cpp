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
// Purpose: 
//-----------------------------------------------------------------------------
class CAchievementTFPlayGameEveryClass : public CTFAchievementFullRound
{
	DECLARE_CLASS( CAchievementTFPlayGameEveryClass, CTFAchievementFullRound );
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
DECLARE_ACHIEVEMENT( CAchievementTFPlayGameEveryClass, ACHIEVEMENT_TF_PLAY_GAME_EVERYCLASS, "TF_PLAY_GAME_EVERYCLASS", 3 );

class CAchievementTFPlayGameEveryMap : public CTFAchievementFullRound
{
	DECLARE_CLASS( CAchievementTFPlayGameEveryMap, CTFAchievementFullRound );
	void Init() 
	{
		SetFlags( ACH_SAVE_GLOBAL | ACH_HAS_COMPONENTS | ACH_FILTER_FULL_ROUND_ONLY );

		static const char *szComponents[] =
		{
			"ctf_warehouse", "koth_landfall", "koth_lumberyard"
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
DECLARE_ACHIEVEMENT( CAchievementTFPlayGameEveryMap, ACHIEVEMENT_TF_PLAY_GAME_EVERYMAP, "TF_PLAY_GAME_EVERYMAP", 3 );

#endif // CLIENT_DLL
