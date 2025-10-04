#pragma once
#include "UIDialogWnd.h"
#include "UIListBoxItem.h"
#include "UIWndCallback.h"

class CGameTask;
class CUIStatic;
class CUIButton;
class SGameTaskObjective;
class UITaskListWnd;
class CUIEditBoxEx;
class CUIEditBox;

class CUITaskItem : public CUIWindow, public CUIWndCallback
{
	typedef		CUIWindow	inherited;
protected:
	CGameTask*				m_GameTask;
	u16						m_TaskObjectiveIdx;
	void __stdcall	OnItemClicked			(CUIWindow*, void*);
	void					Init			();
public:
	CUITaskItem(UITaskListWnd* w);
	virtual			~CUITaskItem			();
	virtual void	SendMessage				(CUIWindow* pWnd, s16 msg, void* pData = NULL);

	virtual bool	OnMouseDown				(int mouse_btn);
					
	virtual void	SetGameTask				(CGameTask* gt, u16 obj_idx);

	virtual void	MarkSelected			(bool b) = 0;
	virtual void    OnMapShown				(bool state) = 0;

	CGameTask*		GameTask				()	{return m_GameTask;}
	u16				ObjectiveIdx			()	{return m_TaskObjectiveIdx;}
	SGameTaskObjective*	Objective			();

	UITaskListWnd*	m_EventsWnd;
};

class CUI3tButton;

class CUITaskRootItem : public CUITaskItem
{
	typedef		CUITaskItem	inherited;
protected:
	bool			m_can_update;
	CUIStatic*		m_taskImage;
	CUIStatic*		m_captionStatic;
	CUIStatic*		m_captionTime;
	CUIStatic*		m_remTimeStatic;
	CUI3tButton*	m_switchDescriptionBtn;
	bool			m_curr_descr_mode;
	void			Init					();
public:	
	CUITaskRootItem(UITaskListWnd* w);
	virtual			~CUITaskRootItem		();
	virtual void	Update					();
	virtual void	SetGameTask				(CGameTask* gt, u16 obj_idx);
	void __stdcall	OnSwitchDescriptionClicked(CUIWindow*, void*);

	virtual void	MarkSelected			(bool b);
	virtual bool	OnDbClick				();
	virtual void    OnMapShown				(bool state);

};

class CUITaskSubItem :public CUITaskItem
{
	typedef			CUITaskItem	inherited;
	u32				m_active_color;
	u32				m_failed_color;
	u32				m_accomplished_color;
protected:
	CUIStatic*		m_ActiveObjectiveStatic;
	CUI3tButton*	m_showDescriptionBtn;
	CUIStatic*		m_descriptionStatic;
	CUIStatic*		m_stateStatic;

	void			Init					();

public:	
	CUITaskSubItem(UITaskListWnd* w);
	virtual			~CUITaskSubItem			();
	virtual void	Update					();
	virtual void	SetGameTask				(CGameTask* gt, u16 obj_idx);
			void	OnActiveObjectiveClicked();
	void __stdcall	OnShowDescriptionClicked(CUIWindow*, void*);
	virtual void	MarkSelected			(bool b);
	virtual bool	OnDbClick				();
	virtual void    OnMapShown				(bool state);
};
