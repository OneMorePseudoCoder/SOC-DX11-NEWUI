#include "stdafx.h"
#include "UITaskWnd.h"
#include "UIMapWnd.h"
#include "../object_broker.h"
#include "UIXmlInit.h"
#include "UIStatic.h"
#include "UI3tButton.h"
#include "UIFrameLineWnd.h"
#include "UISecondTaskWnd.h"
#include "UIMapLegend.h"
#include "UIHelper.h"
#include "UIHint.h"

#include "../gametask.h"
#include "../map_location.h"
#include "../map_location_defs.h"
#include "../map_manager.h"
#include "UIInventoryUtilities.h"
#include "../string_table.h"
#include "../level.h"
#include "../gametaskmanager.h"
#include "../actor.h"
#include "UICheckButton.h"

CUITaskWnd::CUITaskWnd()
{
	hint_wnd = NULL;
}

CUITaskWnd::~CUITaskWnd()
{
	delete_data						(m_pMapWnd);
}

void CUITaskWnd::Init()
{
	CUIXml							xml;
	xml.Load						(CONFIG_PATH, UI_PATH, PDA_TASK_XML);
	VERIFY							(hint_wnd);

	CUIXmlInit::InitWindow			(xml, "main_wnd", 0, this);


	m_pMapWnd						= xr_new<CUIMapWnd>(); 
	m_pMapWnd->SetAutoDelete		(false);
	m_pMapWnd->hint_wnd				= hint_wnd;
	m_pMapWnd->Init					(PDA_TASK_XML,"map_wnd");
	AttachChild						(m_pMapWnd);


	m_BtnTaskListWnd		= UIHelper::Create3tButton( xml, "btn_second_task", this );
	AddCallback				(m_BtnTaskListWnd, BUTTON_CLICKED, CUIWndCallback::void_function(this, &CUITaskWnd::OnShowTaskListWnd));

	m_task_wnd					= xr_new<UITaskListWnd>(); 
	m_task_wnd->SetAutoDelete	(true);
	m_task_wnd->hint_wnd		= hint_wnd;
	m_task_wnd->init_from_xml	(xml, "second_task_wnd");
	m_pMapWnd->AttachChild		(m_task_wnd);
	m_task_wnd->SetMessageTarget(this);
	m_task_wnd->Show			(false);
	m_task_wnd_show				= false;

	m_map_legend_wnd					= xr_new<UIMapLegend>(); 
	m_map_legend_wnd->SetAutoDelete		(true);
	m_map_legend_wnd->init_from_xml		(xml, "map_legend_wnd");
	m_pMapWnd->AttachChild				(m_map_legend_wnd);
	m_map_legend_wnd->SetMessageTarget	(this);
	m_map_legend_wnd->Show				(false);
}

void CUITaskWnd::Update()
{
	if(Actor()->GameTaskManager().ActualFrame() != m_actual_frame)
	{
		ReloadTaskInfo();
	}
	m_pMapWnd->HideCurHint();


	inherited::Update				();
}

void CUITaskWnd::Draw()
{
	inherited::Draw					();
}

void CUITaskWnd::DrawHint()
{
	m_pMapWnd->DrawHint();
}


void CUITaskWnd::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{

	if ( msg == PDA_TASK_SHOW_HINT && pData )
	{
		CGameTask* task = static_cast<CGameTask*>( pData );
		m_pMapWnd->ShowHintTask( task, pWnd );
		return;
	}
	if ( msg == PDA_TASK_HIDE_HINT )
	{
		m_pMapWnd->HideCurHint();
		return;
	}

	inherited::SendMessage(  pWnd, msg, pData );
	CUIWndCallback::OnEvent( pWnd, msg, pData );
}

void CUITaskWnd::ReloadTaskInfo()
{

	m_actual_frame = Actor()->GameTaskManager().ActualFrame();
	
	if(m_task_wnd->IsShown())
		m_task_wnd->UpdateList();

}

void CUITaskWnd::Show(bool status)
{
	inherited::Show			(status);
	m_pMapWnd->Show			(status);
	m_pMapWnd->HideCurHint	();
	m_map_legend_wnd->Show	(false);
	if ( status )
	{
		ReloadTaskInfo();
		m_task_wnd->Show( m_task_wnd_show );
	}
	else
	{
		m_task_wnd->Show(false);
	}
}

void CUITaskWnd::Reset()
{
	inherited::Reset	();
}

void CUITaskWnd::OnNextTaskClicked()
{
}

void CUITaskWnd::OnPrevTaskClicked()
{
}

void CUITaskWnd::OnShowTaskListWnd( CUIWindow* w, void* d )
{
	m_task_wnd_show = !m_task_wnd_show;
	m_task_wnd->Show( !m_task_wnd->IsShown() );
}

void CUITaskWnd::Show_TaskListWnd(bool status)
{
	m_task_wnd->Show( status );
	m_task_wnd_show = status;
}

void CUITaskWnd::ShowMapLegend( bool status )
{
	m_map_legend_wnd->Show( status );
}

void CUITaskWnd::Switch_ShowMapLegend()
{
	m_map_legend_wnd->Show( !m_map_legend_wnd->IsShown() );
}


// --------------------------------------------------------------------------------------------------
CUITaskItem2::CUITaskItem2() :
	m_owner(NULL),
	m_hint_wt(500),
	show_hint(false),
	show_hint_can(false)
{}

CUITaskItem2::~CUITaskItem2()
{}

CUIStatic* init_static_field(CUIXml& uiXml, LPCSTR path, LPCSTR path2);

void CUITaskItem2::Init(CUIXml& uiXml, LPCSTR path)
{
	CUIXmlInit::InitWindow			(uiXml,path,0,this);
	m_hint_wt						= uiXml.ReadAttribInt(path, 0, "hint_wt", 500);

	string256		buff;
	CUIStatic* S					= NULL;

	strconcat( sizeof(buff), buff, path, ":t_icon" );
	if ( uiXml.NavigateToNode( buff ) )
	{
		S = init_static_field		(uiXml, path, "t_icon");
		AttachChild					(S);
	}
	m_info["t_icon"]				= S;
	
	strconcat( sizeof(buff), buff, path, ":t_icon_over" );
	if ( uiXml.NavigateToNode( buff ) )
	{
		S = init_static_field		(uiXml, path, "t_icon_over");
		AttachChild					(S);
	}
	m_info["t_icon_over"]			= S;
	
	S = init_static_field			(uiXml, path, "t_caption");
	AttachChild						(S);
	m_info["t_caption"]				= S;

	show_hint_can = false;
	show_hint     = false;
}

void CUITaskItem2::InitTask(CGameTask* task)
{
	m_owner							= task;
	CUIStatic* S					= m_info["t_icon"];
	if ( S )
	{
		if ( task )
		{
			S->InitTexture			(task->m_icon_texture_name.c_str());
			S->SetStretchTexture	(true);
			m_info["t_icon_over"]->Show(true);
		}
		else
		{
			S->TextureOff			();
			m_info["t_icon_over"]->Show(false);
		}
	}

	S								= m_info["t_caption"];
	S->TextItemControl()->SetTextST	((task) ? task->m_Title.c_str() : "");
}

void CUITaskItem2::OnFocusReceive()
{
	inherited::OnFocusReceive();
	show_hint_can = true;
	show_hint     = false;
}

void CUITaskItem2::OnFocusLost()
{
	inherited::OnFocusLost();
	show_hint_can = false;
	show_hint     = false;
}

void CUITaskItem2::Update()
{
	inherited::Update();
	if ( m_owner && m_bCursorOverWindow && show_hint_can )
	{
		if ( Device.dwTimeGlobal > ( m_dwFocusReceiveTime + m_hint_wt ) )
		{
			show_hint = true;
			return;
		}
	}
}

void CUITaskItem2::OnMouseScroll( float iDirection )
{
}

bool CUITaskItem2::OnMouseAction( float x, float y, EUIMessages mouse_action )
{
	if ( inherited::OnMouseAction( x, y, mouse_action ) )
	{
		//return true;
	}

	switch ( mouse_action )
	{
	case WINDOW_LBUTTON_DOWN:
	case WINDOW_RBUTTON_DOWN:
	case BUTTON_DOWN:
		show_hint_can = false;
		show_hint     = false;
		break;
	}//switch

	return true;
}

void CUITaskItem2::SendMessage( CUIWindow* pWnd, s16 msg, void* pData )
{
	inherited::SendMessage( pWnd, msg, pData );
}
