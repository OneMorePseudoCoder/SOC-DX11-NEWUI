#pragma once
#include "UIWindow.h"
#include "UIWndCallback.h"
#include "../associative_vector.h"
#include "../GameTaskDefs.h"
#include "UICheckButton.h"

class CUIMapWnd;
class CUIStatic;
class CGameTask;
class CUIXml;
class CUITaskItem2;
class CUI3tButton;
class CUIFrameLineWnd;
class CUIFrameWindow;
class CUICheckButton;
class UITaskListWnd;
class UIMapLegend;
class UIHint;
class CMapLocation;

class CUITaskWnd			:	public CUIWindow, 
								public CUIWndCallback
{
private:
	typedef CUIWindow		inherited;

	CUIStatic*				m_right_bottom_background;

	CUI3tButton*			m_BtnTaskListWnd;
	u32						m_actual_frame;


	bool					m_bTreasuresEnabled;
	bool					m_bQuestNpcsEnabled;
	bool					m_bSecondaryTasksEnabled;
	bool					m_bPrimaryObjectsEnabled;

	UITaskListWnd*			m_task_wnd;
	bool					m_task_wnd_show;
	UIMapLegend*			m_map_legend_wnd;

public:
	UIHint*					hint_wnd;
	CUIMapWnd*				m_pMapWnd;
public:
								CUITaskWnd				();
	virtual						~CUITaskWnd				();
	virtual void				SendMessage				(CUIWindow* pWnd, s16 msg, void* pData);
			void				Init					();
	virtual void				Update					();
	virtual void				Draw					();
			void				DrawHint				();
	virtual void				Show					(bool status);
	virtual void				Reset					();

			void				ReloadTaskInfo			();
			void				ShowMapLegend			(bool status);
			void				Switch_ShowMapLegend	();


			void				Show_TaskListWnd		(bool status);

private:

	void						OnNextTaskClicked		();
	void 						OnPrevTaskClicked		();
	void xr_stdcall				OnShowTaskListWnd		(CUIWindow* w, void* d);

};

class CUITaskItem2 : public CUIWindow
{
private:
	typedef CUIWindow			inherited;

	associative_vector<shared_str, CUIStatic*>			m_info;
	CGameTask*											m_owner;
public:
								CUITaskItem2				();
	virtual						~CUITaskItem2			();

	virtual void 				OnFocusReceive			();
	virtual void	 			OnFocusLost				();
	virtual void				Update					();
	virtual void				OnMouseScroll			(float iDirection);
	virtual bool				OnMouseAction					(float x, float y, EUIMessages mouse_action);
	virtual void				SendMessage				(CUIWindow* pWnd, s16 msg, void* pData);

	void						Init					(CUIXml& uiXml, LPCSTR path);
	void						InitTask				(CGameTask* task);
	CGameTask*					OwnerTask				()							{return m_owner;}

public:
	bool						show_hint_can;
	bool						show_hint;

protected:
	u32							m_hint_wt;
};