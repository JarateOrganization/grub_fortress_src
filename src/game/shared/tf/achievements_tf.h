//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================


#include "cbase.h"

#ifdef CLIENT_DLL

bool CheckWinNoEnemyCaps( IGameEvent *event, int iRole );
bool IsLocalTFPlayerClass( int iClass );
bool GameRulesAllowsAchievements( void );


//----------------------------------------------------------------------------------------------------------------
// All achievements should derive from this. It takes care of ensuring that MVM mode isn't active for
// non MVM achievements and that MVM is active for MVM achievements
class CBaseTFAchievementSimple : public CBaseAchievement
{
	DECLARE_CLASS( CBaseTFAchievementSimple, CBaseAchievement );
public:
	virtual bool LocalPlayerCanEarn( void );

	virtual void FireGameEvent( IGameEvent *event );
};

//----------------------------------------------------------------------------------------------------------------
// All class specific achievements should derive from this. It takes care of ensuring that the class
// check is performed, and saves needless event handling for other class's achievements.
class CBaseTFAchievement : public CBaseTFAchievementSimple
{
	DECLARE_CLASS( CBaseTFAchievement, CBaseTFAchievementSimple );
public:
	virtual bool LocalPlayerCanEarn( void );
};

//----------------------------------------------------------------------------------------------------------------
// Helper class for achievements that check that the player was playing on a game team for the full round
class CTFAchievementFullRound : public CBaseTFAchievement
{
	DECLARE_CLASS( CTFAchievementFullRound, CBaseTFAchievement );
public:
	void Init() ;
	virtual void ListenForEvents();
	void FireGameEvent_Internal( IGameEvent *event );
	bool PlayerWasInEntireRound( float flRoundTime );

	virtual void Event_OnRoundComplete( float flRoundTime, IGameEvent *event ) = 0 ;
};

class CAchievementTopScoreboard : public CTFAchievementFullRound
{
	DECLARE_CLASS( CAchievementTopScoreboard, CTFAchievementFullRound );

public:
	void Init();
	virtual void ListenForEvents();
	virtual void Event_OnRoundComplete( float flRoundTime, IGameEvent *event );
};

extern CAchievementMgr g_AchievementMgrTF;	// global achievement mgr for TF

#endif // CLIENT_DLL

#define ACHIEVEMENT_LIST(className, achievementID, achievementName, iPointValue) \
	static int className##_YouForgotTheAchievementList;
#include "achievements_tf_list.inc"
#undef ACHIEVEMENT_LIST