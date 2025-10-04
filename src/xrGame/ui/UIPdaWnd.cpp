#include "stdafx.h"
#include "UIPdaWnd.h"
#include "../Pda.h"
#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "UIInventoryUtilities.h"
#include "../HUDManager.h"
#include "../level.h"
#include "../game_cl_base.h"
#include "UINewsWnd.h"
#include "UIStatic.h"
#include "UIFrameWindow.h"
#include "UITabControl.h"
#include "UIPdaContactsWnd.h"
#include "UIMapWnd.h"
#include "UIDiaryWnd.h"
#include "UIFrameLineWnd.h"
#include "UIStalkersRankingWnd.h"
//#include "UIActorInfo.h"
#include "../object_broker.h"
#include "UIMessagesWindow.h"
#include "UIMainIngameWnd.h"
#include "UITabButton.h"
#include "UIRankingWnd.h"
#include "UIHelper.h"
#include "UIHint.h"
#include "UIBtnHint.h"
#include "UITaskWnd.h"

#define		PDA_XML					"pda.xml"
u32			g_pda_info_state		= 0;

void RearrangeTabButtons(CUITabControl* pTab, xr_vector<Fvector2>& vec_sign_places);

CUIPdaWnd::CUIPdaWnd()
{
	pUITaskWnd				= NULL;
	pUILogsWnd				= NULL;
	UIPdaContactsWnd		= NULL;
	UIDiaryWnd				= NULL;
	pUIRankingWnd			= NULL;
	UIStalkersRanking		= NULL;
	m_hint_wnd				= NULL;
	Init					();
}

CUIPdaWnd::~CUIPdaWnd()
{
	delete_data		(pUITaskWnd);
	delete_data		(pUILogsWnd);
	delete_data		(UIPdaContactsWnd);
	delete_data		(UIDiaryWnd);
	delete_data		(pUIRankingWnd);
	delete_data		(UIStalkersRanking);
	delete_data		(m_hint_wnd);
	delete_data		(UINoice);
}

//////////////////////////////////////////////////////////////////////////

void CUIPdaWnd::Init()
{
	CUIXml uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, PDA_XML);

	CUIXmlInit xml_init;
	
	m_pActiveDialog			= NULL;

	xml_init.InitWindow		(uiXml, "main", 0, this);



	UIMainPdaFrame			= xr_new<CUIStatic>(); UIMainPdaFrame->SetAutoDelete(true);
	AttachChild				(UIMainPdaFrame);
	xml_init.InitStatic		(uiXml, "background_static", 0, UIMainPdaFrame);

	//Ёлементы автоматического добавлени€
	xml_init.InitAutoStatic	(uiXml, "auto_static", this);

	m_background = UIHelper::CreateFrameWindow(uiXml, "background", this);
	m_clock = UIHelper::CreateTextWnd(uiXml, "clock_wnd", this);


	m_hint_wnd = UIHelper::CreateHint(uiXml, "hint_wnd");

	if( IsGameTypeSingle() )
	{
		// Oкно карты зова прип€ти
		pUITaskWnd = xr_new<CUITaskWnd>();
		pUITaskWnd->hint_wnd = m_hint_wnd;
		pUITaskWnd->Init();

		// Oкно новостей
		pUILogsWnd = xr_new<CUILogsWnd>();
		pUILogsWnd->Init();

		// Oкно коммуникaции
		UIPdaContactsWnd		= xr_new<CUIPdaContactsWnd>();
		UIPdaContactsWnd->Init	();

		// Oкно новостей
		UIDiaryWnd				= xr_new<CUIDiaryWnd>();
		UIDiaryWnd->Init		();

		// ќкно энциклопедии
	//	UIEncyclopediaWnd		= xr_new<CUIEncyclopediaWnd>();
	//	UIEncyclopediaWnd->Init	();

		// ќкно статистики о актере
		pUIRankingWnd = xr_new<CUIRankingWnd>();
		pUIRankingWnd->Init();

		// ќкно рейтинга сталкеров
		UIStalkersRanking		= xr_new<CUIStalkersRankingWnd>();
		UIStalkersRanking->Init	();

	//	UIEventsWnd				= xr_new<CUIEventsWnd>();
	//	UIEventsWnd->Init		();
	}
	// Tab control
	UITabControl = xr_new<CUITabControl>();
	UITabControl->SetAutoDelete(true);
	AttachChild(UITabControl);
	CUIXmlInit::InitTabControl(uiXml, "tab", 0, UITabControl);
	UITabControl->SetMessageTarget(this);


	
	m_pActiveSection				= eptNoActiveTab;

	UINoice = xr_new<CUIStatic>();
	UINoice->SetAutoDelete(true);
	CUIXmlInit::InitStatic(uiXml, "noice_static", 0, UINoice);

//	RearrangeTabButtons			(UITabControl, m_sign_places_main);
}

void CUIPdaWnd::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	if (pWnd == UITabControl)
	{
		if (TAB_CHANGED == msg){
			SetActiveSubdialog	((EPdaTabs)UITabControl->GetActiveIndex());
		}
	}else 
	{
		R_ASSERT(m_pActiveDialog);
		m_pActiveDialog->SendMessage(pWnd, msg, pData);
	}
}

void CUIPdaWnd::Show()
{
	InventoryUtilities::SendInfoToActor("ui_pda");

	inherited::Show();
}

void CUIPdaWnd::Hide()
{
	inherited::Hide();

	InventoryUtilities::SendInfoToActor("ui_pda_hide");
	HUD().GetUI()->UIMainIngameWnd->SetFlashIconState_(CUIMainIngameWnd::efiPdaTask, false);

}


void CUIPdaWnd::Update()
{
	inherited::Update		();
	m_clock->TextItemControl().SetText(InventoryUtilities::GetGameTimeAsString(InventoryUtilities::etpTimeToMinutes).c_str());

	Device.seqParallel.push_back(fastdelegate::FastDelegate0<>(pUILogsWnd, &CUILogsWnd::PerformWork));
}

void CUIPdaWnd::SetActiveSubdialog(EPdaTabs section)
{
	if(	m_pActiveSection == section) return;

	if (m_pActiveDialog){
		UIMainPdaFrame->DetachChild(m_pActiveDialog);
		m_pActiveDialog->Show(false);
	}

	switch (section) 
	{
	case eptDiary:
		m_pActiveDialog			= smart_cast<CUIWindow*>(UIDiaryWnd);
		InventoryUtilities::SendInfoToActor("ui_pda_events");
		g_pda_info_state		&= ~pda_section::diary;
		break;
	case eptContacts:
		m_pActiveDialog			= smart_cast<CUIWindow*>(UIPdaContactsWnd);
		InventoryUtilities::SendInfoToActor("ui_pda_contacts");
		g_pda_info_state		&= ~pda_section::contacts;
		break;
	case eptMap:
		m_pActiveDialog			= pUITaskWnd;
		g_pda_info_state		&= ~pda_section::map;
		break;
	case enews:
		m_pActiveDialog			= pUILogsWnd;
		g_pda_info_state		&= ~pda_section::news;
		break;


	case eptActorStatistic:
		m_pActiveDialog			= pUIRankingWnd;
		//m_pActiveDialog			= smart_cast<CUIWindow*>(UIActorInfo);
		InventoryUtilities::SendInfoToActor("ui_pda_actor_info");
		g_pda_info_state		&= ~pda_section::statistics;
		break;
	case eptRanking:
		m_pActiveDialog			= smart_cast<CUIWindow*>(UIStalkersRanking);
		g_pda_info_state		&= ~pda_section::ranking;
		InventoryUtilities::SendInfoToActor("ui_pda_ranking");
		break;
//	case eptQuests:
	//	m_pActiveDialog			= smart_cast<CUIWindow*>(UIEventsWnd);
//		g_pda_info_state		&= ~pda_section::quests;
//		break;
	default:
		Msg("not registered button identifier [%d]",UITabControl->GetActiveIndex());
	}
	UIMainPdaFrame->AttachChild		(m_pActiveDialog);
	m_pActiveDialog->Show			(true);

	if (UITabControl->GetActiveIndex() != section)
		UITabControl->SetNewActiveTab(section);

	m_pActiveSection = section;
}


void CUIPdaWnd::Draw()
{
	inherited::Draw									();
	DrawHint();
	UINoice->Draw(); // over all
}

void CUIPdaWnd::DrawHint()
{
	if (m_pActiveDialog == pUITaskWnd)
	{
		pUITaskWnd->DrawHint();
	}
	else if (m_pActiveDialog == pUIRankingWnd)
	{
	//	pUIRankingWnd->DrawHint();
	}
	m_hint_wnd->Draw();
}


void CUIPdaWnd::PdaContentsChanged	(pda_section::part type)
{
	bool b = true;

//	if(type==pda_section::encyclopedia)
//	{
//		UIEncyclopediaWnd->ReloadArticles	();
//	}
//	else
//	if(type==pda_section::news)
//	{
//		UIDiaryWnd->AddNews					();
//		UIDiaryWnd->MarkNewsAsRead			(UIDiaryWnd->IsShown());
//	}
//	else
//	if(type==pda_section::quests){
//		UIEventsWnd->Reload					();
//	}
//else
	if(type==pda_section::contacts){
		UIPdaContactsWnd->Reload		();
		b = false;
	}

	if(b){
		g_pda_info_state |= type;
		HUD().GetUI()->UIMainIngameWnd->SetFlashIconState_(CUIMainIngameWnd::efiPdaTask, true);
	}

}
void draw_sign(CUIStatic* s, Fvector2& pos)
{
	s->SetWndPos(pos);
	s->Draw();
}


void CUIPdaWnd::Show_SecondTaskWnd(bool status)
{
	if (status)
	{
		SetActiveSubdialog(eptMap);
	}
	pUITaskWnd->Show_TaskListWnd(status);
}

void CUIPdaWnd::UpdatePda()
{
	pUILogsWnd->UpdateNews();
	if (m_pActiveDialog == pUITaskWnd)
	{
		pUITaskWnd->ReloadTaskInfo();
	}
}

void CUIPdaWnd::Reset()
{
	inherited::Reset		();
	if (pUITaskWnd)			pUITaskWnd->Reset();
	if (pUILogsWnd)			pUILogsWnd->ResetAll();
	if (UIPdaContactsWnd)	UIPdaContactsWnd->Reset	();
	if (UIDiaryWnd)			UIDiaryWnd->Reset		();
	if (pUIRankingWnd)		pUITaskWnd->Reset		();
	if (UIStalkersRanking)	UIStalkersRanking->Reset();
}

void RearrangeTabButtons(CUITabControl* pTab, xr_vector<Fvector2>& vec_sign_places)
{
	TABS_VECTOR *	btn_vec = pTab->GetButtonsVector();
	TABS_VECTOR::iterator it = btn_vec->begin();
	TABS_VECTOR::iterator it_e = btn_vec->end();

	Fvector2					pos;
	pos.set((*it)->GetWndPos());
	float						size_x;

	for (; it != it_e; ++it)
	{
		(*it)->SetWndPos(pos);
		(*it)->AdjustWidthToText();
		size_x = (*it)->GetWndSize().x + 30.0f;
		(*it)->SetWidth(size_x);
		pos.x += size_x - 6.0f;
	}

	pTab->SetWidth(pos.x + 5.0f);
	pos.x = pTab->GetWndPos().x - pos.x;
	pos.y = pTab->GetWndPos().y;
	pTab->SetWndPos(pos);
}
