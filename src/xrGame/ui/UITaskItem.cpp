#include "pch_script.h"
#include "UITaskItem.h"
#include "UIXmlInit.h"
#include "UI3tButton.h"
#include "../gametask.h"
#include "../string_table.h"
#include "UISecondTaskWnd.h"
#include "UIEditBoxEx.h"
#include "UIEditBox.h"
#include "UIInventoryUtilities.h"
#include "../map_location.h"
#include "../map_manager.h"
#include "../level.h"
#include "../actor.h"
#include "../gametaskmanager.h"

CUITaskItem::CUITaskItem(UITaskListWnd* w)
:m_GameTask			(NULL),
m_TaskObjectiveIdx(u16(-1)),
m_EventsWnd(w),
CUIWindow()
{ 
}

CUITaskItem::~CUITaskItem			()
{}

void CUITaskItem::SetGameTask(CGameTask* gt, u16 obj_idx)				
{ 
	m_GameTask			= gt;
	m_TaskObjectiveIdx	= obj_idx;
	R_ASSERT(m_GameTask->m_Objectives.size() > m_TaskObjectiveIdx);
}

void CUITaskItem::SendMessage				(CUIWindow* pWnd, s16 msg, void* pData)
{
	inherited::SendMessage(pWnd, msg, pData);
	CUIWndCallback::OnEvent(pWnd, msg, pData);
}

SGameTaskObjective*	CUITaskItem::Objective	()	
{
	if (m_TaskObjectiveIdx > m_GameTask->m_Objectives.size())
	{
		Msg("! no objective(%d) for task item %s size=[%d]", m_TaskObjectiveIdx, m_GameTask->m_ID.c_str(), m_GameTask->m_Objectives.size());
		return NULL;
	}
	return &m_GameTask->m_Objectives[m_TaskObjectiveIdx];
}

bool CUITaskItem::OnMouseDown (int mouse_btn)
{
	return inherited::OnMouseDown(mouse_btn);
}

void CUITaskItem::Init				()
{
	SetWindowName					("job_item");
	Register						(this);
	AddCallbackStr						("job_item",BUTTON_CLICKED,CUIWndCallback::void_function(this,&CUITaskItem::OnItemClicked));
}

void CUITaskItem::OnItemClicked(CUIWindow*, void*)
{
	m_EventsWnd->ShowDescription						(GameTask(), ObjectiveIdx());
}


CUITaskRootItem::CUITaskRootItem(UITaskListWnd* w)
:inherited(w), m_can_update(false)
{
	Init();
}

CUITaskRootItem::~CUITaskRootItem		()
{}

void CUITaskRootItem::Init			()
{
	inherited::Init					();

	m_taskImage					= xr_new<CUIStatic>();		m_taskImage->SetAutoDelete(true);			AttachChild(m_taskImage);
	m_captionStatic				= xr_new<CUIStatic>();		m_captionStatic->SetAutoDelete(true);		AttachChild(m_captionStatic);
	m_remTimeStatic				= xr_new<CUIStatic>();		m_remTimeStatic->SetAutoDelete(true);		AttachChild(m_remTimeStatic);

	m_switchDescriptionBtn		= xr_new<CUI3tButton>();	m_switchDescriptionBtn->SetAutoDelete(true); AttachChild(m_switchDescriptionBtn);
	m_captionTime				= xr_new<CUI3tButton>();	m_captionTime->SetAutoDelete(true);			AttachChild(m_captionTime);
	
	CUIXmlInit xml_init;
	CUIXml&						uiXml = m_EventsWnd->m_ui_task_item_xml;
	xml_init.InitWindow			(uiXml,"task_root_item",0,this);

	xml_init.InitStatic			(uiXml,	"task_root_item:image",			0,	m_taskImage);
	xml_init.InitStatic			(uiXml,	"task_root_item:caption",		0,	m_captionStatic);
	xml_init.InitStatic			(uiXml,	"task_root_item:caption_time",	0,	m_captionTime);
	xml_init.InitStatic			(uiXml,	"task_root_item:rem_time",		0,	m_remTimeStatic);
	
	xml_init.Init3tButton		(uiXml,"task_root_item:switch_description_btn",0,m_switchDescriptionBtn);

	m_switchDescriptionBtn->SetWindowName("m_switchDescriptionBtn");
	m_switchDescriptionBtn->SetMessageTarget(this);
	AddCallbackStr				("m_switchDescriptionBtn",BUTTON_CLICKED,CUIWndCallback::void_function(this, &CUITaskRootItem::OnSwitchDescriptionClicked));
}


void CUITaskRootItem::SetGameTask(CGameTask* gt, u16 obj_idx)				
{
	inherited::SetGameTask(gt, obj_idx);

	CStringTable		stbl;
	SGameTaskObjective	*obj = &m_GameTask->m_Objectives[m_TaskObjectiveIdx];

	m_taskImage->InitTexture		(*obj->icon_texture_name);

	Frect r							= obj->icon_rect;
	Frect texture_rect;

	texture_rect.lt.set					(r.x1, r.y1);
	texture_rect.rb.set					(r.x2, r.y2);
	texture_rect.rb.add					(texture_rect.lt);
	m_taskImage->SetTextureRect	(texture_rect);
	m_taskImage->SetStretchTexture	(true);
	

	m_captionStatic->TextItemControl()->SetText		(*stbl.translate(m_GameTask->m_Title));
	m_captionStatic->AdjustHeightToText	();
	
	xr_string	txt ="";
	txt			+= *(InventoryUtilities::GetDateAsString(gt->m_ReceiveTime, InventoryUtilities::edpDateToDay));
	txt			+= " ";
	txt			+= *(InventoryUtilities::GetTimeAsString(gt->m_ReceiveTime, InventoryUtilities::etpTimeToMinutes));

	m_captionTime->TextItemControl()->SetText(txt.c_str());
	m_captionTime->SetWndPos(m_captionTime->GetWndPos().x,m_captionStatic->GetWndPos().y+m_captionStatic->GetHeight()+3.0f);

	float h = _max	(m_taskImage->GetWndPos().y+m_taskImage->GetHeight(),m_captionTime->GetWndPos().y+m_captionTime->GetHeight());
	h	= _max(h,m_switchDescriptionBtn->GetWndPos().y+m_switchDescriptionBtn->GetHeight());
	SetHeight						(h);
	
	
	m_curr_descr_mode				= m_EventsWnd->GetDescriptionMode();
	if(m_curr_descr_mode)
		m_switchDescriptionBtn->InitTexture	("ui_icons_newPDA_showtext");
	else
		m_switchDescriptionBtn->InitTexture	("ui_icons_newPDA_showmap");

	m_remTimeStatic->Show			(	GameTask()->Objective(0).TaskState()==eTaskStateInProgress && 
										(GameTask()->m_ReceiveTime!=GameTask()->m_TimeToComplete) );
	
	if( m_remTimeStatic->IsShown() )
	{
		float _height	= GetWndSize().y;
		Fvector2 _pos	= m_captionTime->GetWndPos();
		_pos.y			+= m_captionTime->GetWndSize().y;
		_pos.x			= m_remTimeStatic->GetWndPos().x;

		m_remTimeStatic->SetWndPos(_pos);

		_height			= _max(_height, _pos.y+m_remTimeStatic->GetWndSize().y);
		SetHeight		(_height);
	}
	
}

void CUITaskRootItem::OnMapShown (bool state)
{
	m_switchDescriptionBtn->SetButtonState(state ? CUIButton::BUTTON_NORMAL : CUIButton::BUTTON_PUSHED);
}

void CUITaskRootItem::Update		()
{
	inherited::Update();
	if( m_curr_descr_mode	!= m_EventsWnd->GetDescriptionMode() ){
		m_curr_descr_mode				= m_EventsWnd->GetDescriptionMode();
		if(m_curr_descr_mode)
			m_switchDescriptionBtn->InitTexture	("ui_icons_newPDA_showtext");
		else
			m_switchDescriptionBtn->InitTexture	("ui_icons_newPDA_showmap");
	}

//	m_switchDescriptionBtn->SetButtonState(m_EventsWnd->GetDescriptionMode() ? CUIButton::BUTTON_NORMAL : CUIButton::BUTTON_PUSHED);
	
	if(m_remTimeStatic->IsShown())
	{
		string512									buff, buff2;
		InventoryUtilities::GetTimePeriodAsString	(buff, sizeof(buff), Level().GetGameTime(), GameTask()->m_TimeToComplete);
		sprintf_s										(buff2,"%s %s", *CStringTable().translate("ui_st_time_remains"), buff);
		m_remTimeStatic->TextItemControl()->SetText					(buff2);
	
	}
}

bool CUITaskRootItem::OnDbClick	()
{
	return inherited::OnDbClick();
}

void CUITaskRootItem::OnSwitchDescriptionClicked	(CUIWindow*, void*)
{
	m_switchDescriptionBtn->SetButtonState(m_EventsWnd->GetDescriptionMode() ? CUIButton::BUTTON_PUSHED : CUIButton::BUTTON_NORMAL);

	m_EventsWnd->SetDescriptionMode						(!m_EventsWnd->GetDescriptionMode());
	OnItemClicked										(this, NULL);
	m_can_update = !m_can_update;
}

void CUITaskRootItem::MarkSelected (bool b)
{
}


CUITaskSubItem::CUITaskSubItem(UITaskListWnd* w)
:inherited(w)
{
	Init();
}

CUITaskSubItem::~CUITaskSubItem		()
{}

void CUITaskSubItem::Init			()
{
	inherited::Init					();
	CUIXml&						uiXml = m_EventsWnd->m_ui_task_item_xml;

	m_stateStatic			= xr_new<CUIStatic>();		m_stateStatic->SetAutoDelete(true);				AttachChild(m_stateStatic);
	m_descriptionStatic		= xr_new<CUIStatic>();		m_descriptionStatic->SetAutoDelete(true);		AttachChild(m_descriptionStatic);
	m_ActiveObjectiveStatic	= xr_new<CUIStatic>();		m_ActiveObjectiveStatic->SetAutoDelete(true);	AttachChild(m_ActiveObjectiveStatic);
	m_showDescriptionBtn	= xr_new<CUI3tButton>();	m_showDescriptionBtn->SetAutoDelete(true);		AttachChild(m_showDescriptionBtn);

	m_showDescriptionBtn->SetWindowName		("m_showDescriptionBtn");
	m_showDescriptionBtn->SetMessageTarget	(this);

	AddCallbackStr						("m_showDescriptionBtn",BUTTON_CLICKED,CUIWndCallback::void_function(this, &CUITaskSubItem::OnShowDescriptionClicked));


	CUIXmlInit xml_init;
	xml_init.InitWindow				(uiXml,	"task_sub_item",							0,	this);
	xml_init.InitStatic				(uiXml,	"task_sub_item:state_image",				0,	m_stateStatic);
	xml_init.InitStatic				(uiXml,	"task_sub_item:description",				0,	m_descriptionStatic);
	xml_init.InitStatic				(uiXml,	"task_sub_item:active_objecttive_image",	0,	m_ActiveObjectiveStatic);
	xml_init.Init3tButton			(uiXml,	"task_sub_item:show_descr_btn",			0,	m_showDescriptionBtn);


	m_active_color					= xml_init.GetColor(uiXml, "task_sub_item:description:text_colors:active",		0, 0x00);
	m_failed_color					= xml_init.GetColor(uiXml, "task_sub_item:description:text_colors:failed",		0, 0x00);
	m_accomplished_color			= xml_init.GetColor(uiXml, "task_sub_item:description:text_colors:accomplished",0, 0x00);
}

void CUITaskSubItem::SetGameTask	(CGameTask* gt, u16 obj_idx)				
{
	inherited::SetGameTask			(gt, obj_idx);

	CStringTable		stbl;
	SGameTaskObjective	*obj = &m_GameTask->m_Objectives[m_TaskObjectiveIdx];

	
	m_descriptionStatic->TextItemControl()->SetText				(*stbl.translate(obj->description));
	m_descriptionStatic->AdjustHeightToText		();
	float h = _max(	m_ActiveObjectiveStatic->GetWndPos().y+m_ActiveObjectiveStatic->GetHeight(),
					m_descriptionStatic->GetWndPos().y+ m_descriptionStatic->GetHeight());
	SetHeight									(h);
	switch (obj->TaskState())
	{
//.		case eTaskUserDefined:
		case eTaskStateInProgress:
			m_stateStatic->InitTexture				("ui_icons_PDA_subtask_active");
			m_descriptionStatic->SetTextureColor		(m_active_color);
			break;
		case eTaskStateFail:
			m_stateStatic->InitTexture				("ui_icons_PDA_subtask_failed");
			m_descriptionStatic->SetTextureColor		(m_failed_color);
			break;
		case eTaskStateCompleted:
			m_stateStatic->InitTexture				("ui_icons_PDA_subtask_accomplished");
			m_descriptionStatic->SetTextureColor		(m_accomplished_color);
			break;
		default:
			NODEFAULT;
	};
	
}

void CUITaskSubItem::Update					()
{
	inherited::Update						();
	SGameTaskObjective	*obj				= &m_GameTask->m_Objectives[m_TaskObjectiveIdx];
	bool bIsActive							= (Actor()->GameTaskManager().ActiveObjective() == obj); 
	m_ActiveObjectiveStatic->Show			(bIsActive);
	m_showDescriptionBtn->Show				(m_EventsWnd->ItemHasDescription(this));

}

bool CUITaskSubItem::OnDbClick()
{
	SGameTaskObjective	*obj					= &m_GameTask->m_Objectives[m_TaskObjectiveIdx];
	if(obj->TaskState()!=eTaskStateInProgress)	return true;

	bool bIsActive								= (Actor()->GameTaskManager().ActiveObjective() == obj); 
	Actor()->GameTaskManager().SetActiveTask((!bIsActive)?m_GameTask->m_ID:"", m_TaskObjectiveIdx);
	return										true;
}

void CUITaskSubItem::OnActiveObjectiveClicked()
{
	m_EventsWnd->ShowDescription			(GameTask(), ObjectiveIdx());
}

void CUITaskSubItem::OnShowDescriptionClicked (CUIWindow*, void*)
{
	m_EventsWnd->ShowDescription						(GameTask(), ObjectiveIdx());
}

void CUITaskSubItem::OnMapShown (bool state)
{
}

void CUITaskSubItem::MarkSelected (bool b)
{
	m_showDescriptionBtn->SetButtonState		(b ? CUIButton::BUTTON_PUSHED : CUIButton::BUTTON_NORMAL);
}

