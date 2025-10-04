#pragma once

#include "UIDialogWnd.h"
#include "UIStatic.h"
#include "UIEditBox.h"
#include "../inventory_space.h"
#include "../InventoryOwner.h"

class CUIDragDropListEx;
class CUIItemInfo;
class CUICharacterInfo;
class CUIPropertiesBox;
class CUI3tButton;
class CUICellItem;
class CInventoryBox;
class CInventoryOwner;
class CUITextWnd;
class ui_actor_state_wnd;

class CUICarBodyWnd : public CUIDialogWnd
{
private:
	typedef CUIDialogWnd	inherited;
	bool					m_b_need_update;

	void ColorizeItem(CUICellItem* itm);

	void						clear_highlight_lists();
	void						set_highlight_item(CUICellItem* cell_item);
	void						highlight_item_slot(CUICellItem* cell_item);
	void						highlight_armament(PIItem item, CUIDragDropListEx* ddlist);
	void						highlight_ammo_for_weapon(PIItem weapon_item, CUIDragDropListEx* ddlist);
	void						highlight_weapons_for_ammo(PIItem ammo_item, CUIDragDropListEx* ddlist);
	bool						highlight_addons_for_weapon(PIItem weapon_item, CUICellItem* ci);
	void						highlight_weapons_for_addon(PIItem addon_item, CUIDragDropListEx* ddlist);
public:
	CUICarBodyWnd();
	virtual					~CUICarBodyWnd();

	virtual void			Init();
	virtual bool			StopAnyMove(){ return true; }

	virtual void			SendMessage(CUIWindow *pWnd, s16 msg, void *pData);

	void					InitCarBody(CInventoryOwner* pOurInv, CInventoryOwner* pOthersInv);
	void					InitCarBody(CInventoryOwner* pOur, CInventoryBox* pInvBox);
	virtual void			Draw();
	virtual void			Update();

	virtual void			Show();
	virtual void			Hide();

	void					DisableAll();
	void					EnableAll();
	virtual bool			OnKeyboardAction(int dik, EUIMessages keyboard_action);

	void					UpdateLists_delayed();

protected:
	bool						m_item_info_view;
	bool						m_highlight_clear;

	enum eInventorySndAction{
		eInvSndOpen = 0,
		eInvSndClose,
		eInvItemToSlot,
		eInvItemToBelt,
		eInvItemToRuck,
		eInvProperties,
		eInvDropItem,
		eInvAttachAddon,
		eInvDetachAddon,
		eInvItemUse,
		eInvSndMax
	};

	ref_sound					sounds[eInvSndMax];
	void						PlaySnd(eInventorySndAction a);

	CInventoryOwner*		m_pOurObject;
	CInventoryOwner*		m_pOthersObject;
	CInventoryBox*			m_pInventoryBox;



	CUIDragDropListEx*		m_pUIOurBagList;
	CUIDragDropListEx*		m_pUIOthersBagList;

	CUIItemInfo*			m_ItemInfo;
	CUICharacterInfo*		m_pUICharacterInfoLeft;
	CUICharacterInfo*		m_pUICharacterInfoRight; 
	CUIPropertiesBox*		m_pUIPropertiesBox;
	CUI3tButton*			m_pUITakeAll;
	CUI3tButton*			UIExitButton;

	CUITextWnd*					m_ActorMoney;
	CUITextWnd*					m_ActorBottomInfo;
	CUITextWnd*					m_ActorWeight;
	CUITextWnd*					m_ActorWeightMax;

	CUIStatic*					m_PartnerBottomInfo;
	CUITextWnd*					m_PartnerWeight;
	float						m_PartnerWeight_end_x;

	float				CalcItemsWeight(CUIDragDropListEx* pList);

	ui_actor_state_wnd*			m_ActorStateInfo;

	CUICellItem*			m_pCurrentCellItem;

	void					UpdateLists();

	void					ActivatePropertiesBox();
	void					EatItem();

	bool					ToOurBag();
	bool					ToOthersBag();

	void					SetCurrentItem(CUICellItem* itm);
	CUICellItem*			CurrentItem();
	PIItem					CurrentIItem();
	void					InfoCurItem(CUICellItem* cell_item); //on update item

	// Взять все
	void					TakeAll();
	void					DetachAddon(const char* addon_name);

	bool		xr_stdcall	OnItemDrop(CUICellItem* itm);
	bool		xr_stdcall	OnItemStartDrag(CUICellItem* itm);
	bool		xr_stdcall	OnItemDbClick(CUICellItem* itm);
	bool		xr_stdcall	OnItemSelected(CUICellItem* itm);
	bool		xr_stdcall	OnItemRButtonClick(CUICellItem* itm);
	bool		xr_stdcall	OnItemFocusedUpdate(CUICellItem* itm);
	bool		xr_stdcall	OnItemFocusReceive(CUICellItem* itm);
	bool		xr_stdcall	OnItemFocusLost(CUICellItem* itm);

	bool					TransferItem(PIItem itm, CInventoryOwner* owner_from, CInventoryOwner* owner_to, bool b_check);
	void					BindDragDropListEnents(CUIDragDropListEx* lst);

	u32							m_iCurrentActiveSlot;

};