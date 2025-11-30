//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_fireaxe.h"

#ifdef GAME_DLL
#include "tf_player.h"
#else
#include "c_tf_player.h"
#endif
//Set any FireAxe particles and their attachements below.
#define FIREAXE_BEACON_PARTICLE	 "balefulcandle"
#define FIREAXE_BEACON_ATTACHMENT	"candle"

#define FIREAXE_DRG_PARTICLE	"drg_3rd_idle"
#define FIREAXE_DRG_ATTACHMENT		"electrode_0"

//=============================================================================
//
// Weapon FireAxe tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFFireAxe, DT_TFWeaponFireAxe )

BEGIN_NETWORK_TABLE( CTFFireAxe, DT_TFWeaponFireAxe )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFFireAxe )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_fireaxe, CTFFireAxe );
PRECACHE_WEAPON_REGISTER( tf_weapon_fireaxe );

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFFireAxe::GetInitialAfterburnDuration() const 
{ 
	int iAddBurningDamageType = 0;
	CALL_ATTRIB_HOOK_INT( iAddBurningDamageType, set_dmgtype_ignite );
	if ( iAddBurningDamageType )
	{
		return 7.5f;
	}

	return BaseClass::GetInitialAfterburnDuration();
}
#endif
void CTFFireAxe::Precache( void )
{
	PrecacheParticleSystem( "balefulcandle" );
	PrecacheParticleSystem( "drg_3rd_idle" );
	return BaseClass::Precache();
}
bool CTFFireAxe::Holster( CBaseCombatWeapon *pSwitchingTo )
{
#ifdef CLIENT_DLL
	StopClientEffects();
#endif

	return BaseClass::Holster( pSwitchingTo );
}
#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Certain FireAxes have particles attached to the weapon.
//-----------------------------------------------------------------------------
void CTFFireAxe::UpdateVisibility( void )
{
	BaseClass::UpdateVisibility();
	StartClientEffects();
}
void CTFFireAxe::StartClientEffects( void )
{
	StopClientEffects();

	C_TFPlayer *pOwner = GetTFPlayerOwner();

	if ( !pOwner || pOwner->m_Shared.IsStealthed() || !GetOwner() || GetOwner()->GetActiveWeapon() != this )
		return;

	const char *pszParticleEffect = NULL;
	const char *pszAttachmentName = NULL;

	m_hEffectOwner = NULL;
	bool bIsVM = false;

	int iBeacon = 0;
	int iDRG = 0;
	CALL_ATTRIB_HOOK_FLOAT(iBeacon, mod_ghostly_dash);
	CALL_ATTRIB_HOOK_INT(iDRG, damage_all_connected);
	
	if (iBeacon) {
		pszParticleEffect = FIREAXE_BEACON_PARTICLE;
		pszAttachmentName = FIREAXE_BEACON_ATTACHMENT;
	} 
	else if (iDRG) {
		pszParticleEffect = FIREAXE_DRG_PARTICLE;
		pszAttachmentName = FIREAXE_DRG_ATTACHMENT;
	} 
	else return;
	
	if ( ( pOwner == C_TFPlayer::GetLocalTFPlayer() ) && ( ::input->CAM_IsThirdPerson() == false ) )
	{
		m_hEffectOwner = pOwner->GetViewModel();
		bIsVM = true;
	}
	else
	{
		m_hEffectOwner = this;
	}

	if ( m_hEffectOwner )
	{
		int iAttachment = m_hEffectOwner->LookupAttachment( pszAttachmentName );
		if ( iAttachment ) { //Check to make sure attachment is valid.
			m_pFireAxeEffect = m_hEffectOwner->ParticleProp()->Create( pszParticleEffect, PATTACH_POINT_FOLLOW, iAttachment );
			
			if ( bIsVM )
			{
				m_pFireAxeEffect->SetIsViewModelEffect( true );
				ClientLeafSystem()->SetRenderGroup( m_pFireAxeEffect->RenderHandle(), RENDER_GROUP_VIEW_MODEL_TRANSLUCENT );
			}
		}
	}
}
void CTFFireAxe::StopClientEffects( void )
{
	if ( m_pFireAxeEffect && m_hEffectOwner )
	{
		m_hEffectOwner->ParticleProp()->StopEmission( m_pFireAxeEffect );
		m_pFireAxeEffect = NULL;
		m_hEffectOwner = NULL;
	}
}
//-----------------------------------------------------------------------------
// Purpose: Show the meter if we have mod_soul_defense
//-----------------------------------------------------------------------------
bool CTFFireAxe::ShouldDrawMeter() const
{
	float flSoulDefense = 0.f;
	CALL_ATTRIB_HOOK_FLOAT( flSoulDefense, mod_soul_defense );
	if ( flSoulDefense > 0.f )
	{
		return true;
	}
	
	return BaseClass::ShouldDrawMeter();
}
#endif