//========= Copyright Custom Fortress 2, All rights reserved. ============//
//
// Purpose: Custom Server Browser MK.IV
//
//=============================================================================//

#ifndef CF_SERVERBROWSER_H
#define CF_SERVERBROWSER_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/ComboBox.h>
#include <vgui/IScheme.h>
#include "steam/steam_api.h"
#include "steam/isteammatchmaking.h"

namespace vgui
{
	class PropertySheet;
	class Button;
	class ListPanel;
	class ImagePanel;
}

//-----------------------------------------------------------------------------
// Purpose: Server Browser Main Dialog
//-----------------------------------------------------------------------------
class CCFServerBrowser : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CCFServerBrowser, vgui::Frame );

public:
	CCFServerBrowser( vgui::VPANEL parent );
	virtual ~CCFServerBrowser();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void OnCommand( const char *command );
	virtual void OnKeyCodePressed( vgui::KeyCode code );
	virtual void Activate();
	
	MESSAGE_FUNC( OnServerSelected, "ItemSelected" );
	
	void ShowServerInfo( uint32 serverIP, uint16 serverPort );
	void RefreshServerList();
	
	static CCFServerBrowser *GetInstance();
	static void ShowDialog();

private:
	vgui::PropertySheet *m_pGameTabs;
	vgui::Label *m_pTitleLabel;
	
	static CCFServerBrowser *s_pInstance;
};

//-----------------------------------------------------------------------------
// Purpose: Internet Games Tab
//-----------------------------------------------------------------------------
class CInternetGamesPage : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CInternetGamesPage, vgui::EditablePanel );

public:
	CInternetGamesPage( vgui::Panel *parent );
	virtual ~CInternetGamesPage();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void OnCommand( const char *command );
	
	MESSAGE_FUNC_PARAMS( OnItemSelected, "ItemSelected", kv );
	
	void RefreshServerList();
	void ConnectToSelectedServer();
	void ShowServerInfo();
	
	// Steam callbacks
	void OnServerResponded( gameserveritem_t &server );
	void OnRefreshComplete( HServerListRequest hRequest, EMatchMakingServerResponse response );

protected:
	vgui::ListPanel *m_pGameList;
	vgui::Button *m_pConnectButton;
	vgui::Button *m_pRefreshButton;
	vgui::Button *m_pServerInfoButton;
	vgui::ComboBox *m_pGameFilter;
	vgui::ComboBox *m_pLocationFilter;
	vgui::TextEntry *m_pMapFilter;
	
	HServerListRequest m_hServerListRequest;
	bool m_bRefreshing;
	
	CCallResult< CInternetGamesPage, LobbyMatchList_t > m_CallResultLobbyMatchList;
};

//-----------------------------------------------------------------------------
// Purpose: Server Info Dialog
//-----------------------------------------------------------------------------
class CDialogGameInfo : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CDialogGameInfo, vgui::Frame );

public:
	CDialogGameInfo( vgui::Panel *parent );
	virtual ~CDialogGameInfo();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void OnCommand( const char *command );
	
	void SetServerInfo( gameserveritem_t *pServer );
	void UpdateServerInfo( uint32 serverIP, uint16 serverPort );
	
private:
	void QueryServerInfo();
	void OnServerResponded( HServerQuery hRequest, gameserveritem_t *pServer, bool bIOFailure );
	void OnPlayerDetailsResponse( HServerQuery hRequest, int iPlayer, const char *pszName, int iScore, float flTimePlayed );
	void OnRulesResponse( HServerQuery hRequest, const char *pszRuleName, const char *pszRuleValue );
	
	vgui::ImagePanel *m_pMapImage;
	vgui::Label *m_pServerName;
	vgui::Label *m_pServerStatusLabel;
	vgui::Label *m_pMapName;
	vgui::Label *m_pGameTags;
	vgui::Label *m_pPlayerCountLabel;
	vgui::Label *m_pPingLabel;
	vgui::Label *m_pServerTypeLabel;
	vgui::Label *m_pRegionLabel;
	vgui::TextEntry *m_pDescription;
	vgui::ListPanel *m_pAddonsList;
	vgui::ListPanel *m_pPlayerList;
	vgui::Button *m_pConnectButton;
	vgui::Button *m_pCloseButton;
	vgui::ToggleButton *m_pFavoriteButton;
	vgui::ToggleButton *m_pBlacklistButton;
	vgui::Button *m_pMoreInfoButton;
	vgui::ImagePanel *m_pWarningIcon;
	vgui::ImagePanel *m_pLanguageFlag;
	vgui::ImagePanel *m_pRegionFlag;
	
	uint32 m_unServerIP;
	uint16 m_usServerPort;
	HServerQuery m_hServerQuery;
	HServerQuery m_hPlayerQuery;
	HServerQuery m_hRulesQuery;
	
	gameserveritem_t m_ServerInfo;
};

//-----------------------------------------------------------------------------
// Purpose: Favorite Games Tab
//-----------------------------------------------------------------------------
class CFavoriteGamesPage : public CInternetGamesPage
{
	DECLARE_CLASS_SIMPLE( CFavoriteGamesPage, CInternetGamesPage );

public:
	CFavoriteGamesPage( vgui::Panel *parent );
	virtual void RefreshServerList();
};

//-----------------------------------------------------------------------------
// Purpose: History Games Tab
//-----------------------------------------------------------------------------
class CHistoryGamesPage : public CInternetGamesPage
{
	DECLARE_CLASS_SIMPLE( CHistoryGamesPage, CInternetGamesPage );

public:
	CHistoryGamesPage( vgui::Panel *parent );
	virtual void RefreshServerList();
};

//-----------------------------------------------------------------------------
// Purpose: Friends Games Tab
//-----------------------------------------------------------------------------
class CFriendsGamesPage : public CInternetGamesPage
{
	DECLARE_CLASS_SIMPLE( CFriendsGamesPage, CInternetGamesPage );

public:
	CFriendsGamesPage( vgui::Panel *parent );
	virtual void RefreshServerList();
};

//-----------------------------------------------------------------------------
// Purpose: LAN Games Tab
//-----------------------------------------------------------------------------
class CLanGamesPage : public CInternetGamesPage
{
	DECLARE_CLASS_SIMPLE( CLanGamesPage, CInternetGamesPage );

public:
	CLanGamesPage( vgui::Panel *parent );
	virtual void RefreshServerList();
};

//-----------------------------------------------------------------------------
// Purpose: Spectate Games Tab
//-----------------------------------------------------------------------------
class CSpectateGamesPage : public CInternetGamesPage
{
	DECLARE_CLASS_SIMPLE( CSpectateGamesPage, CInternetGamesPage );

public:
	CSpectateGamesPage( vgui::Panel *parent );
	virtual void RefreshServerList();
};

//-----------------------------------------------------------------------------
// Purpose: Blacklisted Servers Tab
//-----------------------------------------------------------------------------
class CBlacklistedServersPage : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CBlacklistedServersPage, vgui::EditablePanel );

public:
	CBlacklistedServersPage( vgui::Panel *parent );
	virtual ~CBlacklistedServersPage();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void OnCommand( const char *command );
	
	void RefreshServerList();
	void RemoveFromBlacklist();
	
private:
	vgui::ListPanel *m_pGameList;
	vgui::Button *m_pRemoveButton;
	vgui::Button *m_pRefreshButton;
};

#endif // CF_SERVERBROWSER_H
