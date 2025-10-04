// File:		UIMessagesWindow.h
// Description:	Window with MP chat and Game Log ( with PDA messages in single and Kill Messages in MP)
// Created:		22.04.2005
// Author:		Serge Vynnychenko
// Mail:		narrator@gsc-game.kiev.ua
//
// Copyright 2005 GSC Game World

#include "StdAfx.h"

#include "UIMessagesWindow.h"
#include "../level.h"
#include "UIGameLog.h"
#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "UIChatWnd.h"
#include "UIPdaMsgListItem.h"
#include "UIColorAnimatorWrapper.h"
#include "../InfoPortion.h"
#include "../string_table.h"
#include "../game_cl_artefacthunt.h"
#include "UIInventoryUtilities.h"
#include "../game_news.h"

CUIMessagesWindow::CUIMessagesWindow(){
	m_pChatLog = NULL; 
	m_pChatWnd = NULL;
	m_pGameLog = NULL;
	Init(0, 0, UI_BASE_WIDTH, UI_BASE_HEIGHT);
}

CUIMessagesWindow::~CUIMessagesWindow(){
	
}

void CUIMessagesWindow::AddLogMessage(KillMessageStruct& msg){
	m_pGameLog->AddLogMessage(msg);
}

void CUIMessagesWindow::AddLogMessage(const shared_str& msg){
	m_pGameLog->AddLogMessage(*msg);
}

void CUIMessagesWindow::Init(float x, float y, float width, float height){

	CUIXml		 xml;
	u32			color;
	CGameFont*	pFont;

	xml.Load(CONFIG_PATH, UI_PATH, "messages_window.xml");

	m_pGameLog = xr_new<CUIGameLog>();m_pGameLog->SetAutoDelete(true);
	m_pGameLog->Show(true);
	AttachChild(m_pGameLog);
	if ( IsGameTypeSingle() )
	{
		CUIXmlInit::InitScrollView(xml, "sp_log_list", 0, m_pGameLog);
	}
	else
	{
		m_pChatLog			= xr_new<CUIGameLog>(); m_pChatLog->SetAutoDelete(true);
		m_pChatLog->Show	(true);
		AttachChild			(m_pChatLog);
		m_pChatWnd			= xr_new<CUIChatWnd>(m_pChatLog); m_pChatWnd->SetAutoDelete(true);
		AttachChild			(m_pChatWnd);

		CUIXmlInit::InitScrollView(xml, "mp_log_list", 0, m_pGameLog);
		CUIXmlInit::InitFont(xml, "mp_log_list:font", 0, color, pFont);
		m_pGameLog->SetTextAtrib(pFont, color);

		CUIXmlInit::InitScrollView(xml, "chat_log_list", 0, m_pChatLog);
		CUIXmlInit::InitFont(xml, "chat_log_list:font", 0, color, pFont);
		m_pChatLog->SetTextAtrib(pFont, color);
		
		m_pChatWnd->Init	(xml);
	}	

}
 
void CUIMessagesWindow::AddIconedPdaMessage(GAME_NEWS_DATA* news)
{
	CUIPdaMsgListItem *pItem = m_pGameLog->AddPdaMessage();

	LPCSTR time_str = InventoryUtilities::GetTimeAsString(news->receive_time, InventoryUtilities::etpTimeToMinutes).c_str();
	pItem->UITimeText.SetText(time_str);
	pItem->UITimeText.AdjustWidthToText();
	Fvector2 p = pItem->UICaptionText.GetWndPos();
	p.x = pItem->UITimeText.GetWndPos().x + pItem->UITimeText.GetWidth() + 3.0f;
	pItem->UICaptionText.SetWndPos(p);
	pItem->UICaptionText.SetTextST(news->news_caption.c_str());
	pItem->UIMsgText.SetTextST(news->news_text.c_str());
	pItem->UIMsgText.AdjustHeightToText();

	pItem->SetColorAnimation("ui_main_msgs_short", LA_ONLYALPHA | LA_TEXTCOLOR | LA_TEXTURECOLOR, float(news->show_time));
	pItem->UIIcon.InitTexture(news->texture_name.c_str());

	Frect texture_rect;
	texture_rect.lt.set(news->tex_rect.x1, news->tex_rect.y1);
	texture_rect.rb.set(news->tex_rect.x2, news->tex_rect.y2);
	texture_rect.rb.add(texture_rect.lt);
	pItem->UIIcon.GetUIStaticItem().SetTextureRect(texture_rect);

	float h1 = _max(pItem->UIIcon.GetHeight(), pItem->UIMsgText.GetWndPos().y + pItem->UIMsgText.GetHeight());
	pItem->SetHeight(h1 + 3.0f);

	m_pGameLog->SendMessage(pItem, CHILD_CHANGED_SIZE);
}

void CUIMessagesWindow::AddChatMessage(shared_str msg, shared_str author)
{
	 m_pChatLog->AddChatMessage(*msg, *author);
}

void CUIMessagesWindow::SetChatOwner(game_cl_GameState* owner)
{
	if (m_pChatWnd)
		m_pChatWnd->SetOwner(owner);
}

void CUIMessagesWindow::Update()
{
	CUIWindow::Update();
}