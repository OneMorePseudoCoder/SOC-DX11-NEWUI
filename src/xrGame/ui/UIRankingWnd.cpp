////////////////////////////////////////////////////////////////////////////
//	Module 		: UIRankingWnd.cpp
//	Created 	: 17.01.2008
//	Author		: Evgeniy Sokolov
//	Description : UI Ranking window class implementation
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "UIRankingWnd.h"

#include "UIXmlInit.h"
#include "UIProgressBar.h" 
#include "UIFrameLineWnd.h"
#include "UIScrollView.h"
#include "UIHelper.h"
#include "UIInventoryUtilities.h"

#include "../actor.h"
#include "../ai_space.h"
#include "../alife_simulator.h"

#include "../script_engine.h"
#include "../character_community.h"
#include "../character_reputation.h"
#include "../relation_registry.h"
#include "../string_table.h"
#include "UICharacterInfo.h"

#define  PDA_RANKING_XML		"pda_ranking.xml"

CUIRankingWnd::CUIRankingWnd()
{
	m_actor_ch_info = NULL;
	m_previous_time = Device.dwTimeGlobal;
	m_delay			= 3000;
}

CUIRankingWnd::~CUIRankingWnd()
{
}

void CUIRankingWnd::Show( bool status )
{
	if ( status )
	{
		m_actor_ch_info->InitCharacter( Actor()->object_id() );
		
		string64 buf;
		sprintf_s( buf, sizeof(buf), "%d %s", Actor()->get_money(), "RU" );
		m_money_value->SetText( buf );
		m_money_value->AdjustWidthToText();
		update_info();
		inherited::Update();
	}


	inherited::Show( status );
}

void CUIRankingWnd::Update()
{
	inherited::Update();
	if ( IsShown() )
	{
		if ( Device.dwTimeGlobal - m_previous_time > m_delay )
		{
			m_previous_time = Device.dwTimeGlobal;
			update_info();
		}
	}
}

void CUIRankingWnd::Init()
{
	Fvector2 pos;
	CUIXml xml;

	xml.Load( CONFIG_PATH, UI_PATH, PDA_RANKING_XML );

	CUIXmlInit::InitWindow( xml, "main_wnd", 0, this );
	m_delay				= (u32)xml.ReadAttribInt( "main_wnd", 0, "delay",	3000 );

	m_actor_ch_info = xr_new<CUICharacterInfo>();
	m_actor_ch_info->SetAutoDelete(true);
	AttachChild(m_actor_ch_info);
	m_actor_ch_info->InitCharacterInfo(&xml, "actor_ch_info");

	m_background		= UIHelper::CreateStatic(xml, "background", this);
	m_center_background	= UIHelper::CreateStatic( xml, "center_background", this );

//	m_actor_ch_info->UICommunityCaption().AdjustWidthToText();
//	pos = m_actor_ch_info->UICommunity().GetWndPos();
//	pos.x = m_actor_ch_info->UICommunityCaption().GetWndPos().x + m_actor_ch_info->UICommunityCaption().GetWndSize().x + 10.0f;
//	m_actor_ch_info->UICommunity().SetWndPos( pos );

	m_money_caption		= UIHelper::CreateStatic( xml, "money_caption", this );
	m_money_caption->AdjustWidthToText();
	pos = m_money_caption->GetWndPos();
	pos.x += m_money_caption->GetWndSize().x + 10.0f;

	m_money_value = UIHelper::CreateStatic(xml, "money_value", this);
	m_money_value->SetWndPos( pos );

	m_center_caption		= UIHelper::CreateStatic( xml, "center_caption", this );

	UIDetailList = xr_new<CUIScrollView>(); 
	UIDetailList->SetAutoDelete(true);
	AttachChild(UIDetailList);
	CUIXmlInit::InitScrollView(xml, "detail_list", 0, UIDetailList);

	XML_NODE* stored_root = xml.GetLocalRoot();
	XML_NODE* node = xml.NavigateToNode( "stat_info", 0 );
	xml.SetLocalRoot( node );

	m_stat_count = (u32)xml.GetNodesNum( node, "stat" );
	u32 value_color = CUIXmlInit::GetColor( xml, "value", 0, 0xFFffffff );

	for ( u8 i = 0; i < m_stat_count; ++i )
	{
		m_stat_caption[i]		= xr_new<CUIStatic>();
		AttachChild				( m_stat_caption[i] );
		m_stat_caption[i]->SetAutoDelete( true );
		CUIXmlInit::InitStatic	( xml, "stat", i, m_stat_caption[i] );
		m_stat_caption[i]->AdjustWidthToText();

		m_stat_info[i]			= xr_new<CUIStatic>();
		AttachChild				( m_stat_info[i] );
		m_stat_info[i]->SetAutoDelete( true );
		CUIXmlInit::InitStatic	( xml, "stat", i, m_stat_info[i] );
		
		m_stat_info[i]->SetTextColor( value_color );
		
		pos.y = m_stat_caption[i]->GetWndPos().y;
		pos.x = m_stat_caption[i]->GetWndPos().x + m_stat_caption[i]->GetWndSize().x + 5.0f;
		m_stat_info[i]->SetWndPos( pos );
	}
	xml.SetLocalRoot( stored_root );

	string256 buf;
	strcpy_s( buf, sizeof(buf), m_center_caption->GetText() );
	strcat_s( buf, sizeof(buf), CStringTable().translate("ui_ranking_center_caption").c_str() );
	m_center_caption->SetText( buf );

	LPCSTR fract_section = "pda_rank_communities";

	VERIFY2( pSettings->section_exist( fract_section ), make_string( "Section [%s] does not exist !", fract_section ) );
	int fract_count = pSettings->line_count( fract_section );
	VERIFY2( fract_count, make_string( "Section [%s] is empty !",       fract_section ) );


}

void CUIRankingWnd::update_info()
{
	get_value_from_script();
}

void CUIRankingWnd::get_value_from_script()
{
	string128 buf;
	InventoryUtilities::GetTimePeriodAsString( buf, sizeof(buf), Level().GetStartGameTime(), Level().GetGameTime() );
	m_stat_info[0]->SetText( buf );

	for ( u8 i = 1; i < m_stat_count; ++i )
	{
		luabind::functor<LPCSTR>	funct;
		R_ASSERT( ai().script_engine().functor( "pda.get_stat", funct ) );
		LPCSTR str = funct( i );
		m_stat_info[i]->SetTextST( str );
	}
}

///////////////////////////////////////////////////////////////////////////////////////////

void CUIRankingWnd::FillReputationDetails(CUIXml* xml, LPCSTR path)
{
	XML_NODE* _list_node = xml->NavigateToNode("relation_communities_list", 0);
	int cnt = xml->GetNodesNum("relation_communities_list", 0, "r");

	CHARACTER_COMMUNITY						comm;


	CHARACTER_REPUTATION					rep_actor, rep_neutral;
	rep_actor.set(Actor()->Reputation());
	rep_neutral.set(NEUTAL_REPUTATION);

	CHARACTER_GOODWILL d_neutral = CHARACTER_REPUTATION::relation(rep_actor.index(), rep_neutral.index());


	string64 buff;
	for (int i = 0; i<cnt; ++i)
	{
		CUIActorStaticticDetail* itm = xr_new<CUIActorStaticticDetail>();
		itm->Init(xml, path, 0);
		comm.set(xml->Read(_list_node, "r", i, "unknown_community"));
		itm->m_text1->SetTextST(*(comm.id()));

		CHARACTER_GOODWILL	gw = RELATION_REGISTRY().GetCommunityGoodwill(comm.index(), Actor()->ID());
		gw += CHARACTER_COMMUNITY::relation(Actor()->Community(), comm.index());
		gw += d_neutral;

		itm->m_text2->SetTextST(InventoryUtilities::GetGoodwillAsText(gw));
		itm->m_text2->SetTextColor(InventoryUtilities::GetGoodwillColor(gw));

		sprintf_s(buff, "%d", gw);
		itm->m_text3->SetTextST(buff);

		UIDetailList->AddWindow(itm, true);
	}
}

void CUIActorStaticticDetail::Init(CUIXml* xml, LPCSTR path, int idx)
{
	XML_NODE* _stored_root = xml->GetLocalRoot();

	CUIXmlInit							xml_init;
	xml_init.InitWindow(*xml, path, idx, this);

	xml->SetLocalRoot(xml->NavigateToNode(path, idx));

	m_text0 = xr_new<CUIStatic>(); 
	m_text0->SetAutoDelete(true);
	AttachChild(m_text0);
	xml_init.InitStatic(*xml, "text_0", 0, m_text0);

	m_text1 = xr_new<CUIStatic>(); 
	m_text1->SetAutoDelete(true);
	AttachChild(m_text1);
	xml_init.InitStatic(*xml, "text_1", 0, m_text1);

	m_text2 = xr_new<CUIStatic>(); 
	m_text2->SetAutoDelete(true);
	AttachChild(m_text2);
	xml_init.InitStatic(*xml, "text_2", 0, m_text2);

	m_text3 = xr_new<CUIStatic>(); 
	m_text3->SetAutoDelete(true);
	AttachChild(m_text3);
	xml_init.InitStatic(*xml, "text_3", 0, m_text3);

	xml_init.InitAutoStaticGroup(*xml, "auto", 0, this);

	xml->SetLocalRoot(_stored_root);
}