#pragma once
#include "UIWindow.h"
#include "../inventory_space.h"
#include "UIDialogWnd.h"

class CInventoryOwner;
class CEatableItem;
class CTrade;
struct CUITradeInternal;

class CInventoryBox;
class CUIPropertiesBox;
class CUIDragDropListEx;
class CUICellItem;
class CUICharacterInfo;
class CUI3tButton;
class CUITextWnd;
class CUIStatic;
class CUIItemInfo;
class ui_actor_state_wnd;

class CUITradeWnd :  public CUIDialogWnd
{
private:
	typedef CUIWindow inherited;


	void						clear_highlight_lists();
	void						set_highlight_item(CUICellItem* cell_item);
	void						highlight_item_slot(CUICellItem* cell_item);
	void						highlight_armament(PIItem item, CUIDragDropListEx* ddlist);
	void						highlight_ammo_for_weapon(PIItem weapon_item, CUIDragDropListEx* ddlist);
	void						highlight_weapons_for_ammo(PIItem ammo_item, CUIDragDropListEx* ddlist);
	bool						highlight_addons_for_weapon(PIItem weapon_item, CUICellItem* ci);
	void						highlight_weapons_for_addon(PIItem addon_item, CUIDragDropListEx* ddlist);
public:
						CUITradeWnd					();
	virtual				~CUITradeWnd				();

	virtual void		Init						();

	virtual void		SendMessage					(CUIWindow *pWnd, s16 msg, void *pData);
	virtual bool		OnKeyboardAction			(int dik, EUIMessages keyboard_action);
	void				InitTrade					(CInventoryOwner* pOur, CInventoryOwner* pOthers);
	

	virtual void 		Draw						();
	virtual void 		Update						();
	virtual void		UpdateActor();
	virtual void		UpdatePartnerBag();
	virtual void 		Show						();
	virtual void 		Hide						();

	void 				DisableAll					();
	void 				EnableAll					();

	void 				SwitchToTalk				();
	void 				StartTrade					();
	void 				StopTrade					();
protected:

	CUITradeInternal*	m_uidata;

	bool				bStarted;
	bool 				ToOurTrade					();
	bool 				ToOthersTrade				();
	bool 				ToOurBag					();
	bool 				ToOthersBag					();
	void 				SendEvent_ItemDrop			(PIItem pItem);
	
	u32					CalcItemsPrice				(CUIDragDropListEx* pList, CTrade* pTrade, bool bBuying);
	float				CalcItemsWeight				(CUIDragDropListEx* pList);

	void				TransferItems				(CUIDragDropListEx* pSellList, CUIDragDropListEx* pBuyList, CTrade* pTrade, bool bBuying);

	void				PerformTrade				();
	void 	    	    OnBtnPerformTradeBuy();
	void        		OnBtnPerformTradeSell();


	void				UpdatePrices				();
	void				ColorizeItem				(CUICellItem* itm, bool b);

	enum EListType{eNone,e1st,e2nd,eBoth};
	void				UpdateLists					(EListType);

	void				FillList					(TIItemContainer& cont, CUIDragDropListEx* list, bool do_colorize);

	bool				m_bDealControlsVisible;

	bool				CanMoveToOther				(PIItem pItem);



	CUICharacterInfo*		m_pUICharacterInfoLeft;
	CUICharacterInfo*		m_pUICharacterInfoRight;



	CUIDragDropListEx*			UIOthersTradeList;
	CUIDragDropListEx*			UIOurTradeList;

	CUIDragDropListEx*	UIOurBagList;
	CUIDragDropListEx*	UIOthersBagList;

	CUITextWnd*			UIOurMoneyStatic;
	CUITextWnd*			UIOtherMoneyStatic;

	CUITextWnd*					m_ActorBottomInfo;
	CUITextWnd*					m_ActorWeight;
	CUITextWnd*					m_ActorWeightMax;

	CUIStatic*					m_PartnerBottomInfo;
	CUITextWnd*					m_PartnerWeight;
	float						m_PartnerWeight_end_x;

	CUIItemInfo*				m_ItemInfo;

	bool						m_item_info_view;
	bool						m_highlight_clear;

	ui_actor_state_wnd*			m_ActorStateInfo;
	// delimiter ------------------------------
	CUIStatic*					m_LeftDelimiter;
	CUIStatic*					m_PartnerTradeCaption;
	CUIStatic*					m_PartnerTradePrice;
	CUIStatic*					m_PartnerTradeWeightMax;

	CUIStatic*					m_RightDelimiter;
	CUIStatic*					m_ActorTradeCaption;
	CUIStatic*					m_ActorTradePrice;
	CUIStatic*					m_ActorTradeWeightMax;


	void					EatItem();

	//указатели игрока и того с кем торгуем
	CInventory*			m_pInv;
	CInventory*			m_pOthersInv;
	CInventoryOwner*	m_pInvOwner;
	CInventoryOwner*	m_pOthersInvOwner;
	CTrade*				m_pTrade;
	CTrade*				m_pOthersTrade;


//	u32					m_iOurTradePrice;
//	u32					m_iOthersTradePrice;


	CUICellItem*		m_pCurrentCellItem;
	TIItemContainer		ruck_list;


	void				SetCurrentItem				(CUICellItem* itm);
	CUICellItem*		CurrentItem					();
	PIItem				CurrentIItem				();
	void				InfoCurItem(CUICellItem* cell_item); //on update item


//	void		xr_stdcall		OnBtnPerformTradeBuy(CUIWindow* w, void* d);
//	void		xr_stdcall		OnBtnPerformTradeSell(CUIWindow* w, void* d);
	bool		xr_stdcall		OnItemDrop			(CUICellItem* itm);
	bool		xr_stdcall		OnItemStartDrag		(CUICellItem* itm);
	bool		xr_stdcall		OnItemDbClick		(CUICellItem* itm);
	bool		xr_stdcall		OnItemSelected		(CUICellItem* itm);
	bool		xr_stdcall		OnItemRButtonClick	(CUICellItem* itm);
	bool		xr_stdcall	    OnItemFocusedUpdate(CUICellItem* itm);
	bool		xr_stdcall	    OnItemFocusReceive(CUICellItem* itm);
	bool		xr_stdcall	    OnItemFocusLost(CUICellItem* itm);

	void				BindDragDropListEnents		(CUIDragDropListEx* lst);



};