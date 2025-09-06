//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_bat.h"
#include "decals.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_basedoor.h"
#include "c_tf_player.h"
#include "IEffects.h"
#include "bone_setup.h"
#include "c_tf_gamestats.h"
// Server specific.
#else
#include "doors.h"
#include "tf_player.h"
#include "tf_ammo_pack.h"
#include "tf_gamestats.h"
#include "ilagcompensationmanager.h"
#include "collisionutils.h"
#include "particle_parse.h"
#include "tf_projectile_base.h"
#include "tf_gamerules.h"
#endif
extern ConVar friendlyfire;
extern ConVar tf_scout_stunball_base_speed;
const float DEFAULT_ORNAMENT_EXPLODE_RADIUS = 50.0f;
const float DEFAULT_ORNAMENT_EXPLODE_DAMAGE_MULT = 0.9f;

//=============================================================================
//
// Weapon Bat tables.
//

// CTFBat_Giftwrap --
IMPLEMENT_NETWORKCLASS_ALIASED( TFBat_Giftwrap, DT_TFWeaponBat_Giftwrap )

BEGIN_NETWORK_TABLE( CTFBat_Giftwrap, DT_TFWeaponBat_Giftwrap )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFBat_Giftwrap )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_bat_giftwrap, CTFBat_Giftwrap );
PRECACHE_WEAPON_REGISTER( tf_weapon_bat_giftwrap );
// -- CTFBat_Giftwrap

// CTFBall_Ornament --
IMPLEMENT_NETWORKCLASS_ALIASED( TFBall_Ornament, DT_TFProjectileBall_Ornament )
BEGIN_NETWORK_TABLE( CTFBall_Ornament, DT_TFProjectileBall_Ornament )
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( tf_projectile_ball_ornament, CTFBall_Ornament );
PRECACHE_WEAPON_REGISTER( tf_projectile_ball_ornament );

#define TF_WEAPON_BALL_ORNAMENT_VM_MODEL		"models/weapons/c_models/c_xms_festive_ornament.mdl"
#define TF_WEAPON_BALL_ORNAMENT_MODEL			"models/weapons/c_models/c_xms_festive_ornament.mdl"
// -- CTFBall_Ornament

static string_t s_iszTrainName;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBat_Giftwrap::Spawn( void )
{
	BaseClass::Spawn();

	m_nSkin = ( GetTeamNumber() == TF_TEAM_BLUE ) ? 1 : 0;
}


#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseEntity *CTFBat_Giftwrap::CreateBall( void )
{
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( !pPlayer )
		return NULL;

	// Do another check here, as the player may have moved to an invalid position
	// since the first check (0.1 seconds ago).  This fixes the ball sometimes
	// going through thin geometry, such as windows and spawn blockers.
	if ( !CanCreateBall( pPlayer ) )
		return NULL;

	// Determine the ball's initial location, angles, and velocity.
	Vector vecLocation, vecVelocity;
	QAngle vecAngles;
	AngularImpulse angImpulse;
	GetBallDynamics( vecLocation, vecAngles, vecVelocity, angImpulse, pPlayer );

	// Create the ornament ball.
	CTFBall_Ornament *pBall = CTFBall_Ornament::Create( vecLocation, vecAngles, pPlayer );
	Assert( pBall );
	if ( !pBall )
		return NULL;

	CalcIsAttackCritical();

	pBall->m_iOriginalOwnerID = m_iEnemyBallID;
	m_iEnemyBallID = 0;

	pBall->SetCritical( IsCurrentAttackACrit() );
	pBall->InitGrenade( vecVelocity, angImpulse, pPlayer, GetTFWpnData() );
	pBall->SetLauncher( this );
	pBall->SetOwnerEntity( pPlayer );
	pBall->SetInitialSpeed( tf_scout_stunball_base_speed.GetInt() );
	pBall->m_nSkin = ( pPlayer->GetTeamNumber() == TF_TEAM_BLUE ) ? 1 : 0;

	return pBall;
}


//-----------------------------------------------------------------------------
void CTFBall_Ornament::Precache( void )
{
	PrecacheScriptSound( "BallBuster.OrnamentImpactRange" );
	PrecacheScriptSound( "BallBuster.OrnamentImpact" );
	PrecacheScriptSound( "BallBuster.HitBall" );
	PrecacheScriptSound( "BallBuster.HitFlesh" );
	PrecacheScriptSound( "BallBuster.HitWorld" );
	PrecacheScriptSound( "BallBuster.DrawCatch" );
	PrecacheScriptSound( "BallBuster.Ornament_DrawCatch" );
	PrecacheScriptSound( "BallBuster.Ball_HitWorld" );

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
CTFBall_Ornament *CTFBall_Ornament::Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner )
{
	CTFBall_Ornament* pBall = static_cast< CTFBall_Ornament * >( CBaseAnimating::CreateNoSpawn( "tf_projectile_ball_ornament", vecOrigin, vecAngles, pOwner ) );
	if ( pBall )
	{
		DispatchSpawn( pBall );
	}

	return pBall;
}


//-----------------------------------------------------------------------------
const char *CTFBall_Ornament::GetBallModelName( void ) const
{
	return TF_WEAPON_BALL_ORNAMENT_MODEL;
}


//-----------------------------------------------------------------------------
const char *CTFBall_Ornament::GetBallViewModelName( void ) const
{
	return TF_WEAPON_BALL_ORNAMENT_VM_MODEL;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#define FLIGHT_TIME_TO_MAX_STUN_GIFT	0.8f
void CTFBall_Ornament::ApplyBallImpactEffectOnVictim( CBaseEntity *pOther )
{
	if ( !pOther || !pOther->IsPlayer() )
		return;

	CTFPlayer* pPlayer = ToTFPlayer( pOther );
	if ( !pPlayer )
		return;

	CTFPlayer *pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pOwner )
		return;

	if ( m_bTouched )
		return;

	// Can't bleed an invul player.
	if ( pPlayer->m_Shared.IsInvulnerable() || pPlayer->m_Shared.InCond( TF_COND_INVULNERABLE_WEARINGOFF ) )
		return;

	bool bIsCriticalHit = IsCritical();
	float flBleedTime = 5.0f;
	bool bIsLongRangeHit = false;

	// long distance hit is always a crit
	float flLifeTime = gpGlobals->curtime - m_flCreationTime;
	if ( flLifeTime >= FLIGHT_TIME_TO_MAX_STUN_GIFT )
	{
		bIsCriticalHit = true;
		bIsLongRangeHit = true;
	}

	// just do the bleed effect directly since the bleed
	// attribute comes from the inflictor, which is the bat.
	pPlayer->m_Shared.MakeBleed( pOwner, (CTFBat_Giftwrap *)GetLauncher(), flBleedTime );

	// Apply particle effect to victim (the remaining effects happen inside Explode)
	DispatchParticleEffect( "xms_ornament_glitter", PATTACH_POINT_FOLLOW, pPlayer, "head" );

	// Give 'em a love tap.
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();
	trace_t *pNewTrace = const_cast<trace_t*>( pTrace );

	CBaseEntity *pInflictor = GetLauncher();
	CTakeDamageInfo info;
	info.SetAttacker( GetOwnerEntity() );
	info.SetInflictor( pInflictor ); 
	info.SetWeapon( pInflictor );
	info.SetDamage( GetDamage() );
	info.SetDamageCustom( TF_DMG_CUSTOM_BASEBALL );
	info.SetDamageForce( GetDamageForce() );
	info.SetDamagePosition( GetAbsOrigin() );
	int iDamageType = GetDamageType();
	if ( bIsCriticalHit )
		iDamageType |= DMG_CRITICAL;
	info.SetDamageType( iDamageType );

	// Hurt 'em.
	Vector dir;
	AngleVectors( GetAbsAngles(), &dir );
	pPlayer->DispatchTraceAttack( info, dir, pNewTrace );
	ApplyMultiDamage();

	// the ball shatters
	UTIL_Remove( this );

	m_bTouched = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBall_Ornament::PipebombTouch( CBaseEntity *pOther )
{
	if ( !ShouldBallTouch( pOther ) )
		return;

	trace_t pTrace;
	Vector velDir = GetAbsVelocity();
	VectorNormalize( velDir );
	Vector vecSpot = GetAbsOrigin() - velDir * 32;
	UTIL_TraceLine( vecSpot, vecSpot + velDir * 64, MASK_SOLID, this, COLLISION_GROUP_NONE, &pTrace );

	if ( pOther == GetThrower() )
		return;

	// Explode (does radius damage, triggers particles and sound effects).
	Explode( &pTrace, DMG_BLAST|DMG_PREVENT_PHYSICS_FORCE );

	if ( ( !InSameTeam( pOther ) || friendlyfire.GetBool() ) && pOther->m_takedamage != DAMAGE_NO)
	{
		ApplyBallImpactEffectOnVictim( pOther );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBall_Ornament::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	BaseClass::VPhysicsCollision( index, pEvent );

	int otherIndex = !index;
	CBaseEntity *pHitEntity = pEvent->pEntities[otherIndex];

	if ( !pHitEntity )
		return;

	// Break if we hit the world.
	if ( pHitEntity->IsWorld() )
	{
		// Explode immediately next frame. (Can't explode in the collision callback.)
		m_vCollisionVelocity = pEvent->preVelocity[index];
		SetContextThink( &CTFBall_Ornament::VPhysicsCollisionThink, gpGlobals->curtime, "OrnamentCollisionThink" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBall_Ornament::VPhysicsCollisionThink( void )
{
	trace_t pTrace;
	Vector velDir = m_vCollisionVelocity;
	VectorNormalize( velDir );
	Vector vecSpot = GetAbsOrigin() - velDir * 16;
	UTIL_TraceLine( vecSpot, vecSpot + velDir * 32, MASK_SOLID, this, COLLISION_GROUP_NONE, &pTrace );

	Explode( &pTrace, DMG_BLAST|DMG_PREVENT_PHYSICS_FORCE );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBall_Ornament::Explode( trace_t *pTrace, int bitsDamageType )
{
	// Create smashed glass particles when we explode
	CTFPlayer* pOwner = ToTFPlayer( GetOwnerEntity() );
	if ( pOwner && pOwner->GetTeamNumber() == TF_TEAM_RED )
	{
		DispatchParticleEffect( "xms_ornament_smash_red", GetAbsOrigin(), GetAbsAngles() );
	}
	else
	{
		DispatchParticleEffect( "xms_ornament_smash_blue", GetAbsOrigin(), GetAbsAngles() );
	}

	Vector vecOrigin = GetAbsOrigin();

	// sound effects
	EmitSound_t params;
	params.m_flSoundTime = 0;
	params.m_pflSoundDuration = 0;
	params.m_pSoundName = "BallBuster.OrnamentImpact";
	CPASFilter filter( vecOrigin );
	filter.RemoveRecipient( pOwner );
	EmitSound( filter, entindex(), params );
	CSingleUserRecipientFilter attackerFilter( pOwner );
	EmitSound( attackerFilter, pOwner->entindex(), params );

	// Explosion damage is some fraction of our base damage
	float flExplodeDamage = GetDamage() * DEFAULT_ORNAMENT_EXPLODE_DAMAGE_MULT;

	// Do radius damage
	Vector vecBlastForce(0.0f, 0.0f, 0.0f);
	CTakeDamageInfo info( this, GetThrower(), m_hLauncher, vecBlastForce, GetAbsOrigin(), flExplodeDamage, bitsDamageType, TF_DMG_CUSTOM_BASEBALL, &vecOrigin );
	CTFRadiusDamageInfo radiusinfo( &info, vecOrigin, DEFAULT_ORNAMENT_EXPLODE_RADIUS, nullptr, 0.0f, 0.0f );
	TFGameRules()->RadiusDamage( radiusinfo );

	UTIL_Remove( this );
}

#endif

