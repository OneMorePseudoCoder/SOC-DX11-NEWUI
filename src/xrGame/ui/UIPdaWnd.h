#pragma once

#include "UIDialogWnd.h"
#include "UIPdaAux.h"
#include "../encyclopedia_article_defs.h"

class CInventoryOwner;
class CUIFrameLineWnd;
class CUIFrameWindow;
class CUIButton;
class CUITabControl;
class CUIStatic;
class CUITextWnd;
class CUITaskWnd;
//class CUIEncyclopediaWnd;
class CUIDiaryWnd;
class CUIActorInfoWnd;
class CUIRankingWnd;
class CUIStalkersRankingWnd;
class CUIPdaContactsWnd;
class UIHint;
class CUILogsWnd;

class CUIPdaWnd: public CUIDialogWnd
{
private:
	typedef CUIDialogWnd	inherited;

	CUIFrameWindow*			m_background;

protected:

	CUITextWnd*				m_clock;
	// кнопки PDA
	CUITabControl*			UITabControl;

	// Установить игровое время
	//void					UpdateDateTime					();
//	void					DrawUpdatedSections				();
protected:
	// Бэкграунд
	CUIStatic*				UIMainPdaFrame;
//	CUIStatic*				m_updatedSectionImage;
//	CUIStatic*				m_oldSectionImage;
	CUIStatic*				UINoice;

	// Текущий активный диалог
	CUIWindow*				m_pActiveDialog;
	EPdaTabs				m_pActiveSection;
	xr_vector<Fvector2>		m_sign_places_main;

	UIHint*					m_hint_wnd;

public:
	// Поддиалоги PDA
	CUITaskWnd*				pUITaskWnd;
	CUILogsWnd*				pUILogsWnd;
	CUIPdaContactsWnd*		UIPdaContactsWnd;
	CUIDiaryWnd*			UIDiaryWnd;
	CUIRankingWnd*			pUIRankingWnd;
	CUIStalkersRankingWnd*	UIStalkersRanking;
	virtual void			Reset				();
public:
							CUIPdaWnd			();
	virtual					~CUIPdaWnd			();

	virtual void 			Init				();

	virtual void 			SendMessage			(CUIWindow* pWnd, s16 msg, void* pData = NULL);

	virtual void 			Draw				();
	virtual void 			Update				();
	virtual void 			Show				();
	virtual void 			Hide				();
	virtual bool			OnMouseAction(float x, float y, EUIMessages mouse_action) { CUIDialogWnd::OnMouseAction(x, y, mouse_action); return true; } //always true because StopAnyMove() == false
	
			UIHint*			get_hint_wnd() const { return m_hint_wnd; }
			void			DrawHint();


	void			Show_SecondTaskWnd(bool status);

	void					SetActiveSubdialog	(EPdaTabs section);
	virtual bool			StopAnyMove			(){return false;}

			void			PdaContentsChanged	(pda_section::part type);
			void			UpdatePda();
};			
