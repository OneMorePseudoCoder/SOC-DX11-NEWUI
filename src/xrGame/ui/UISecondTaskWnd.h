////////////////////////////////////////////////////////////////////////////
//	Module 		: UISecondTaskWnd.h
//	Created 	: 30.05.2008
//	Author		: Evgeniy Sokolov
//	Description : UI Secondary Task Wnd class
////////////////////////////////////////////////////////////////////////////

#ifndef UI_SECOND_TASK_WND_H_INCLUDED
#define UI_SECOND_TASK_WND_H_INCLUDED

#include "UIWindow.h"
#include "UIWndCallback.h"
#include "UIXmlInit.h"

#define PDA_TASK_XML	"pda_map.xml"

class CUIXml;
class CUIFrameWindow;
class CUIScrollView;
class CUIStatic;
class CUI3tButton;
class CUICheckButton;
class CUIFrameLineWnd;
class CGameTask;
class UIHint;
class CUITaskDescrWnd;
class CUITaskItem;
class CUITabControl;
class CUITaskWnd;

class UITaskListWnd : public CUIWindow, public CUIWndCallback
{
	enum ETaskFilters{
		eActiveTask = 0,
		eAccomplishedTask,
		eFailedTask,
		//.						eOwnTask,
		eMaxTask
	};
	enum EEventWndFlags{
		flNeedReload = (1 << 0),
		flMapMode = (1 << 1),
	};
	Flags16						m_flags;
	ETaskFilters				m_currFilter;
	CUIWindow*					m_UIRightWnd;
	CUITaskDescrWnd*			m_UITaskInfoWnd;
	CUITabControl*				m_TaskFilter;

	bool						Filter(CGameTask* t);
	void __stdcall				OnFilterChanged(CUIWindow*, void*);

private:
	typedef CUIWindow	inherited;

public:
	UITaskListWnd();
	virtual			~UITaskListWnd		();

			void	init_from_xml		( CUIXml& xml, LPCSTR path );

	virtual bool	OnMouseAction		( float x, float y, EUIMessages mouse_action );
	virtual void 	OnMouseScroll		(float iDirection);
	virtual void	Show				( bool status );
	virtual void 	OnFocusReceive		(); 
	virtual void	OnFocusLost			();
	virtual void	Update				();
	virtual void	SendMessage			( CUIWindow* pWnd, s16 msg, void* pData );

			void	UpdateList			();


protected:
	void xr_stdcall	OnBtnClose			( CUIWindow* w, void* d);
//	bool xr_stdcall	SortingLessFunction	( CUIWindow* left, CUIWindow* right );

//			void	UpdateCounter		();
public:
	UIHint*				hint_wnd;

private: // m_
	CUIFrameWindow*		m_background;
	CUIScrollView*		m_list;
	
	CUIStatic*			m_caption;
//	CUIStatic*			m_counter;
	CUI3tButton*		m_bt_close;

//	u32					m_activ_task_count;
	float				m_orig_h;

public:
	void				SetDescriptionMode(bool bMap);
	bool				GetDescriptionMode();
	void				ShowDescription(CGameTask* t, int idx);
	bool				ItemHasDescription(CUITaskItem*);

	void				Reload();
	virtual void		Reset();
	CUIXml				m_ui_task_item_xml;

}; // class UITaskListWnd

// -------------------------------------------------------------------------------------------------

#endif 