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
#include "c_tf_player.h"
#include "IEffects.h"
#include "bone_setup.h"
// Server specific.
#else
#include "tf_player.h"
#endif
extern ConVar friendlyfire;

//=============================================================================
//
// Weapon Bat tables.
//

// TFBat --
IMPLEMENT_NETWORKCLASS_ALIASED( TFBat, DT_TFWeaponBat )

BEGIN_NETWORK_TABLE( CTFBat, DT_TFWeaponBat )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFBat )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_bat, CTFBat );
PRECACHE_WEAPON_REGISTER( tf_weapon_bat );
// -- TFBat


// TFBat_Fish --
IMPLEMENT_NETWORKCLASS_ALIASED( TFBat_Fish, DT_TFWeaponBat_Fish )

BEGIN_NETWORK_TABLE( CTFBat_Fish, DT_TFWeaponBat_Fish )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFBat_Fish )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_bat_fish, CTFBat_Fish );
PRECACHE_WEAPON_REGISTER( tf_weapon_bat_fish );
// -- TFBat_Fish

//=============================================================================
//
// CTFBat
//

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFBat::CTFBat()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFBat::Smack( void )
{
	BaseClass::Smack();

#ifdef GAME_DLL
	if ( BatDeflects() )
	{
#ifdef TF_RAID_MODE
		if ( TFGameRules()->IsRaidMode() )
		{
		}
		else
#endif // TF_RAID_MODE
		{
			DeflectProjectiles();
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFBat::PlayDeflectionSound( bool bPlayer )
{
	WeaponSound( MELEE_HIT_WORLD );
}
