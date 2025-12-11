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
	virtual ~CTFBMMH() {}

	virtual int		GetWeaponID( void ) const			{ return TF_WEAPON_DISPENSER_GUN; }
	virtual int		GetAmmoPerShot( void );
	
	// Override SecondaryAttack to cancel charge instead of detonating
	virtual void	SecondaryAttack( void );
	
	// Override PrimaryAttack to respect charge cancel delay
	virtual void	PrimaryAttack( void );
	
	// Override ItemPostFrame to prevent firing during cancel delay
	virtual void	ItemPostFrame( void );
	
	// Override LaunchGrenade to ensure proper charge reset
	virtual void	LaunchGrenade( void );
	
	// Override to store metal cost in projectile
	virtual CBaseEntity *FireProjectile( CTFPlayer *pPlayer );

private:
	CNetworkVar( float, m_flChargeCancelTime );

	CTFBMMH( const CTFBMMH & );
};

#endif // TF_WEAPON_COMPOUND_BOW_H
