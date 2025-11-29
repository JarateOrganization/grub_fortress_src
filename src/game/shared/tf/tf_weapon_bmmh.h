//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================
#ifndef TF_WEAPON_COMPOUND_BOW_H
#define TF_WEAPON_COMPOUND_BOW_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_gun.h"
#include "tf_weapon_pipebomblauncher.h"

// Client specific.
#ifdef CLIENT_DLL
#define CTFBMMH C_TFBMMH
#else
#endif

//=============================================================================
//
// TF Weapon Bow
//
class CTFBMMH : public CTFPipebombLauncher
{
public:

	DECLARE_CLASS( CTFBMMH, CTFPipebombLauncher );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

// Server specific.
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CTFBMMH();
	~CTFBMMH() {}

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_DISPENSER_GUN; }
	virtual int		GetAmmoPerShot()					{ return RemapValClamped(gpGlobals->curtime - GetInternalChargeBeginTime(), 0, GetChargeMaxTime(), 0, 100 ); }
	
	// Override SecondaryAttack to cancel charge instead of detonating
	virtual void	SecondaryAttack();
	
	// Override PrimaryAttack to respect charge cancel delay
	virtual void	PrimaryAttack();
	
	// Override ItemPostFrame to prevent firing during cancel delay
	virtual void	ItemPostFrame();
	
	// Override LaunchGrenade to ensure proper charge reset
	virtual void	LaunchGrenade();
	
	// Override to store metal cost in projectile
	virtual CBaseEntity *FireProjectile( CTFPlayer *pPlayer );

private:
	float m_flChargeCancelTime;

public:
	CTFBMMH( const CTFBMMH & ) {}
};

#endif // TF_WEAPON_COMPOUND_BOW_H
