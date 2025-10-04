////////////////////////////////////////////////////////////////////////////
//	Module 		: UISecondTaskWnd.cpp
//	Created 	: 30.05.2008
//	Author		: Evgeniy Sokolov
//	Description : UI Secondary Task Wnd class impl
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UISecondTaskWnd.h"

#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "UIHelper.h"

#include "UIFrameWindow.h"
#include "UIScrollView.h"
#include "UIStatic.h"
#include "UI3tButton.h"
#include "UICheckButton.h"
#include "UIFrameLineWnd.h"
#include "UIFixedScrollBar.h"
#include "UIHint.h"
#include "UITaskWnd.h"
#include "UITabControl.h"
#include "../GameTaskDefs.h"
#include "../gametask.h"
#include "../map_location.h"
#include "UIInventoryUtilities.h"
#include "../string_table.h"
#include "../level.h"
#include "../gametaskmanager.h"
#include "../actor.h"
#include "UITaskItem.h"
#include "../encyclopedia_article.h"
#include "../alife_registry_wrappers.h"
#include "UITaskDescrWnd.h"

UITaskListWnd::UITaskListWnd()
{
	hint_wnd = NULL;
	m_flags.zero();
}

UITaskListWnd::~UITaskListWnd()
{
	delete_data(m_UITaskInfoWnd);
}

void UITaskListWnd::init_from_xml( CUIXml& xml, LPCSTR path )
{
	VERIFY( hint_wnd );
	CUIXmlInit xml_init;
	CUIXmlInit::InitWindow(xml, path, 0, this);


	XML_NODE*  stored_root = xml.GetLocalRoot();
	XML_NODE*  tmpl_root   = xml.NavigateToNode( path, 0 );
	xml.SetLocalRoot( tmpl_root );
	
	m_background = UIHelper::CreateFrameWindow( xml, "background_frame", this );
	m_caption    = UIHelper::CreateStatic( xml, "t_caption", this );
//	m_counter    = UIHelper::CreateStatic( xml, "t_counter", this );
	m_bt_close   = UIHelper::Create3tButton( xml, "btn_close", this );

	Register( m_bt_close );
	AddCallback( m_bt_close, BUTTON_DOWN, CUIWndCallback::void_function( this, &UITaskListWnd::OnBtnClose ) );

	m_list = xr_new<CUIScrollView>();
	m_list->SetAutoDelete( true );
	AttachChild( m_list );
	CUIXmlInit::InitScrollView( xml, "task_list", 0, m_list );
	m_orig_h = GetHeight();


	m_UIRightWnd = xr_new<CUIWindow>(); 
	m_UIRightWnd->SetAutoDelete(true);
	AttachChild(m_UIRightWnd);
	CUIXmlInit::InitWindow(xml, "right_frame", 0, m_UIRightWnd);

	m_UITaskInfoWnd = xr_new<CUITaskDescrWnd>(); 
	m_UITaskInfoWnd->SetAutoDelete(false);
	m_UITaskInfoWnd->Init(&xml, "right_frame:task_descr_view");

	m_TaskFilter = xr_new<CUITabControl>(); m_TaskFilter->SetAutoDelete(true);
	m_background->AttachChild(m_TaskFilter);
	xml_init.InitTabControl(xml, "filter_tab", 0, m_TaskFilter);
	m_TaskFilter->SetWindowName("filter_tab");
	Register(m_TaskFilter);
	AddCallbackStr("filter_tab", TAB_CHANGED, CUIWndCallback::void_function(this, &UITaskListWnd::OnFilterChanged));

	m_list->SetWindowName("---second_task_list");
//	m_list->m_sort_function = fastdelegate::MakeDelegate( this, &UITaskListWnd::SortingLessFunction );
	m_currFilter = eActiveTask;

	m_flags.set(flMapMode, TRUE);

	m_ui_task_item_xml.Load(CONFIG_PATH, UI_PATH, "pda_map.xml");
	xml.SetLocalRoot( stored_root );
}

bool UITaskListWnd::OnMouseAction( float x, float y, EUIMessages mouse_action )
{
	if ( inherited::OnMouseAction( x, y, mouse_action ) )
	{
		return true;
	}
	return true;
}

void UITaskListWnd::OnMouseScroll( float iDirection )
{
	if ( iDirection == WINDOW_MOUSE_WHEEL_UP )
		m_list->ScrollBar()->TryScrollDec();
	else if ( iDirection == WINDOW_MOUSE_WHEEL_DOWN )
		m_list->ScrollBar()->TryScrollInc();
}
void UITaskListWnd::Show( bool status )
{
	inherited::Show( status );
	GetMessageTarget()->SendMessage( this, PDA_TASK_HIDE_HINT, NULL );
	if(status)
		//m_UITaskInfoWnd->Show(status);
		UpdateList();
}

void UITaskListWnd::OnFocusReceive()
{
	inherited::OnFocusReceive();
	GetMessageTarget()->SendMessage( this, PDA_TASK_HIDE_HINT, NULL );
}

void UITaskListWnd::OnFocusLost()
{
	inherited::OnFocusLost();
	GetMessageTarget()->SendMessage( this, PDA_TASK_HIDE_HINT, NULL );
}

void UITaskListWnd::Update()
{
	if (m_flags.test(flNeedReload))
	{
		UpdateList();
		m_flags.set(flNeedReload, FALSE);
	}
	inherited::Update();
//	UpdateCounter();
}

void UITaskListWnd::SendMessage( CUIWindow* pWnd, s16 msg, void* pData )
{
	GetMessageTarget()->SendMessage( pWnd, msg, pData );
	inherited::SendMessage( pWnd, msg, pData );
	CUIWndCallback::OnEvent( pWnd, msg, pData );
}

void UITaskListWnd::OnBtnClose( CUIWindow* w, void* d )
{
	CUITaskWnd* wnd = smart_cast<CUITaskWnd*>(GetParent()->GetParent());
	if(wnd)
		wnd->Show_TaskListWnd(false);
//	Show( false );
	m_bt_close->SetButtonState(CUIButton::BUTTON_NORMAL);
}

void UITaskListWnd::Reload()
{
	m_flags.set(flNeedReload, TRUE);
}

void UITaskListWnd::UpdateList()
{
	int prev_scroll_pos	= m_list->GetCurrentScrollPos	();

	m_list->Clear();
	
	u32 count_for_check = 0;

	if (!g_actor)				return;
	vGameTasks& tasks = Actor()->GameTaskManager().GameTasks();
	vGameTasks::iterator itb = tasks.begin();
	vGameTasks::iterator ite = tasks.end();
	CGameTask* task = NULL;

	for ( ; itb != ite; ++itb )
	{
		task = (*itb).game_task;
		R_ASSERT(task);
		R_ASSERT(task->m_Objectives.size() > 0);

		if (!Filter(task))		continue;
		CUITaskItem* pTaskItem = NULL;

		for (u16 i = 0; i < task->m_Objectives.size(); ++i)
		{
			if (i == 0){
				pTaskItem = xr_new<CUITaskRootItem>(this);
			}
			else{
				pTaskItem = xr_new<CUITaskSubItem>(this);
			}
			pTaskItem->SetGameTask(task, i);
			m_list->AddWindow(pTaskItem, true);
		}

	}// for
	m_list->SetScrollPos(prev_scroll_pos);
}

/*bool UITaskListWnd::SortingLessFunction( CUIWindow* left, CUIWindow* right )
{
//	UITaskListWndItem* lpi = smart_cast<UITaskListWndItem*>(left);
//	UITaskListWndItem* rpi = smart_cast<UITaskListWndItem*>(right);
//	VERIFY( lpi && rpi );
//	return ( lpi->get_priority_task() > rpi->get_priority_task() );
}*/

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void UITaskListWnd::OnFilterChanged(CUIWindow* w, void*)
{
	m_currFilter = (ETaskFilters)m_TaskFilter->GetActiveIndex();
	UpdateList();
	if (!GetDescriptionMode())
		SetDescriptionMode(true);
}

bool UITaskListWnd::Filter(CGameTask* t)
{
	ETaskState task_state = t->m_Objectives[0].TaskState();
	//	bool bprimary_only			= m_primary_or_all_filter_btn->GetCheck();

	return (false) ||
		(
		(true) &&
		(
		(m_currFilter == eAccomplishedTask	&& task_state == eTaskStateCompleted) ||
		(m_currFilter == eFailedTask			&& task_state == eTaskStateFail) ||
		(m_currFilter == eActiveTask			&& task_state == eTaskStateInProgress)
		)
		);
}

void UITaskListWnd::SetDescriptionMode(bool bMap)
{
	if (bMap){
		m_UIRightWnd->DetachChild(m_UITaskInfoWnd);
		//m_UIRightWnd->AttachChild(m_UIMapWnd);
	}
	else{
		//m_UIRightWnd->DetachChild(m_UIMapWnd);
		m_UIRightWnd->AttachChild(m_UITaskInfoWnd);
	}
	m_flags.set(flMapMode, bMap);
	u32 sz = m_list->GetSize();
	for (u32 i = 0; i<sz; ++i)
	{
		CUITaskItem* itm = smart_cast<CUITaskItem*>(m_list->GetItem(i));
		itm->OnMapShown(bMap);
	}

}

bool UITaskListWnd::GetDescriptionMode()
{
	return !!m_flags.test(flMapMode);
}
#include "../HUDManager.h"
#include "../UI.h"
#include "../UIGameSp.h"
#include "UIPdaWnd.h"
#include "UIMapWnd.h"
void UITaskListWnd::ShowDescription(CGameTask* t, int idx)
{
	if (GetDescriptionMode())
	{//map
		SGameTaskObjective& o = t->Objective(idx);
		CMapLocation* ml = o.LinkedMapLocation();

		if (ml&&ml->SpotEnabled())
		{
			CUIGameSP* ui_game_sp = smart_cast<CUIGameSP*>(HUD().GetUI()->UIGame());
			ui_game_sp->PdaMenu->pUITaskWnd->m_pMapWnd->SetTargetMap(ml->GetLevelName(), ml->GetPosition(), true);
		}

		//if (ml&&ml->SpotEnabled())
		//	m_UIMapWnd->SetTargetMap(ml->GetLevelName(), ml->GetPosition(), true);
	}
	else
	{//articles
		SGameTaskObjective& o = t->Objective(0);
		idx = 0;

		m_UITaskInfoWnd->ClearAll();

		if (Actor()->encyclopedia_registry->registry().objects_ptr())
		{
			string512	need_group;
			if (0 == idx){
				strcpy_s(need_group, *t->m_ID);
			}
			else
				if (o.article_key.size())
				{
					sprintf_s(need_group, "%s/%s", *t->m_ID, *o.article_key);
				}
				else
				{
					sprintf_s(need_group, "%s/%d", *t->m_ID, idx);
				}

			ARTICLE_VECTOR::const_iterator it = Actor()->encyclopedia_registry->registry().objects_ptr()->begin();

			for (; it != Actor()->encyclopedia_registry->registry().objects_ptr()->end(); ++it)
			{
				if (ARTICLE_DATA::eTaskArticle == it->article_type)
				{
					CEncyclopediaArticle	A;
					A.Load(it->article_id);

					const shared_str& group = A.data()->group;

					if (strstr(group.c_str(), need_group) == group.c_str())
					{
						u32 sz = xr_strlen(need_group);
						if (group.size() == sz || group.c_str()[sz] == '/')
							m_UITaskInfoWnd->AddArticle(&A);
					}
					else
						if (o.article_id.size() && it->article_id == o.article_id)
						{
							CEncyclopediaArticle			A;
							A.Load(it->article_id);
							m_UITaskInfoWnd->AddArticle(&A);
						}
				}
			}
		}
	}

	int sz = m_list->GetSize();

	for (int i = 0; i<sz; ++i)
	{
		CUITaskItem* itm = (CUITaskItem*)m_list->GetItem(i);

		if ((itm->GameTask() == t) && (itm->ObjectiveIdx() == idx))
			itm->MarkSelected(true);
		else
			itm->MarkSelected(false);
	}
}

bool UITaskListWnd::ItemHasDescription(CUITaskItem* itm)
{
	if (itm->ObjectiveIdx() == 0)// root
	{
		bool bHasLocation = itm->GameTask()->HasLinkedMapLocations();
		return bHasLocation;
	}
	else
	{
		SGameTaskObjective	*obj = itm->Objective();
		if (!obj)
			return false;
		CMapLocation* ml = obj->LinkedMapLocation();
		bool bHasLocation = (NULL != ml);
		bool bIsMapMode = GetDescriptionMode();
		bool b = (bIsMapMode&&bHasLocation&&ml->SpotEnabled());
		return b;
	}
}

void UITaskListWnd::Reset()
{
	inherited::Reset();
	Reload();
}