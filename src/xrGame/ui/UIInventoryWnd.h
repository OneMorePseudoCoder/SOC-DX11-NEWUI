#pragma once

#include "UIDialogWnd.h"
#include "UIStatic.h"
#include "UIProgressBar.h"
#include "UIPropertiesBox.h"
#include "UIOutfitSlot.h"
#include "UIItemInfo.h"
#include "../inventory_space.h"
#include "UIHint.h"
#include "../InventoryOwner.h"

class CInventory;
class CUICharacterInfo;
class CArtefact;
class CUI3tButton;
class CUIDragDropListEx;
class CUIDragDropReferenceList;
class CUICellItem;
class CUIItemInfo;
class CUITextWnd;
class CInventoryOwner;
class ui_actor_state_wnd;
class CUIProgressBar;

class CUIInventoryWnd: public CUIDialogWnd
{
private:
	typedef CUIDialogWnd	inherited;
	bool					m_b_need_reinit;

	void					clear_highlight_lists();
	void					set_highlight_item(CUICellItem* cell_item);
	void					highlight_item_slot(CUICellItem* cell_item);
	void					highlight_armament(PIItem item, CUIDragDropListEx* ddlist);
	void					highlight_ammo_for_weapon(PIItem weapon_item, CUIDragDropListEx* ddlist);
	void					highlight_weapons_for_ammo(PIItem ammo_item, CUIDragDropListEx* ddlist);
	bool					highlight_addons_for_weapon(PIItem weapon_item, CUICellItem* ci);
	void					highlight_weapons_for_addon(PIItem addon_item, CUIDragDropListEx* ddlist);

public:
							CUIInventoryWnd				();
	virtual					~CUIInventoryWnd			();

	virtual void			Init						();

	void					InitInventory				();
	void					InitInventory_delayed		();
	virtual bool			StopAnyMove					()					{return false;}

	virtual void			SendMessage					(CUIWindow *pWnd, s16 msg, void *pData);
	virtual bool			OnMouseAction(float x, float y, EUIMessages mouse_action);
	virtual bool			OnKeyboardAction(int dik, EUIMessages keyboard_action);


	IC CInventory*			GetInventory				()					{return m_pInv;}

	virtual void			Update						();
	virtual void			Draw						();

	virtual void			Show						();
	virtual void			Hide						();

	void					AddItemToBag				(PIItem pItem);

	void					UpdateConditionProgressBars();

public:
	CUIDragDropReferenceList*	m_pQuickSlot;

protected:
	enum eInventorySndAction{	eInvSndOpen	=0,
								eInvSndClose,
								eInvItemToSlot,
								eInvItemToBelt,
								eInvItemToRuck,
								eInvProperties,
								eInvDropItem,
								eInvAttachAddon, 
								eInvDetachAddon,
								eInvItemUse,
								eInvSndMax};

	ref_sound					sounds					[eInvSndMax];
	void						PlaySnd					(eInventorySndAction a);


	ui_actor_state_wnd*			m_ActorStateInfo;
	CUICharacterInfo*			m_ActorCharacterInfo;

	CUITextWnd*					m_ActorMoney;
	CUITextWnd*					m_ActorBottomInfo;
	CUITextWnd*					m_ActorWeight;
	CUITextWnd*					m_ActorWeightMax;

	CUI3tButton*				UIExitButton;

	CUIDragDropListEx*			m_pUIBagList;
	CUIDragDropListEx*			m_pUIBeltList;
	CUIDragDropListEx*			m_pUIPistolList;
	CUIDragDropListEx*			m_pUIAutomaticList;
	CUIDragDropListEx*			m_pUIOutfitList;
	CUIDragDropListEx*			m_pUIHelmetList;
	CUIDragDropListEx*			m_pUIDetectorList;
	CUIDragDropListEx*			GetSlotList(u32 slot_idx);
	CUIDragDropListEx*			m_pTrashList;

	enum						{ e_af_count = 5 };
	CUIStatic*					m_belt_list_over[e_af_count];

	CUIStatic*					m_InvSlot2Highlight;
	CUIStatic*					m_InvSlot3Highlight;
	CUIStatic*					m_HelmetSlotHighlight;
	CUIStatic*					m_OutfitSlotHighlight;
	CUIStatic*					m_DetectorSlotHighlight;
	CUIStatic*					m_QuickSlotsHighlight[4];
	CUIStatic*					m_ArtefactSlotsHighlight[e_af_count];

	void						UpdateOutfit();
	void						MoveArtefactsToBag();

	void						ClearAllLists				();
	void						BindDragDropListEnents		(CUIDragDropListEx* lst);
	EListType					GetType                     (CUIDragDropListEx* l);

	bool						m_item_info_view;
	bool						m_highlight_clear;


	bool		xr_stdcall		OnItemDrop					(CUICellItem* itm);
	bool		xr_stdcall		OnItemStartDrag				(CUICellItem* itm);
	bool		xr_stdcall		OnItemDbClick				(CUICellItem* itm);
	bool		xr_stdcall		OnItemSelected				(CUICellItem* itm);
	bool		xr_stdcall		OnItemRButtonClick			(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusedUpdate			(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusReceive			(CUICellItem* itm);
	bool		xr_stdcall		OnItemFocusLost				(CUICellItem* itm);
	void		xr_stdcall		OnDragItemOnTrash			(CUIDragItem* item, bool b_receive);


	CUIProgressBar*				m_WeaponSlot1_progress;
	CUIProgressBar*				m_WeaponSlot2_progress;
	CUIProgressBar*				m_Helmet_progress;
	CUIProgressBar*				m_Outfit_progress;

	CUITextWnd*					m_QuickSlot1;
	CUITextWnd*					m_QuickSlot2;
	CUITextWnd*					m_QuickSlot3;
	CUITextWnd*					m_QuickSlot4;

	void						UpdateButtonsLayout();

	CUIStatic					UIProgressBack;
	CUIStatic					UIProgressBack_rank;
	CUIProgressBar				UIProgressBarRank;

	CUIPropertiesBox			UIPropertiesBox;
	
//	CInventoryOwner*			m_pActorInvOwner;
	//информация о персонаже
	CUIItemInfo*				m_ItemInfo;

	CInventory*					m_pInv;

	CUICellItem*				m_pCurrentCellItem;

	bool						DropItem					(PIItem itm, CUIDragDropListEx* lst);
//	bool						TryUseItem					(PIItem itm);

	//----------------------	-----------------------------------------------
	void						SendEvent_Item2Slot			(PIItem	pItem);
	void						SendEvent_Item2Belt			(PIItem	pItem);
	void						SendEvent_Item2Ruck			(PIItem	pItem);
	void						SendEvent_Item_Drop			(PIItem	pItem);
	void						SendEvent_Item_Eat			(PIItem	pItem);
	void						SendEvent_ActivateSlot		(PIItem	pItem);
	void						SendEvent_ActivateSlot2		(u32 slot, u32 recipient);

	//---------------------------------------------------------------------

	void						ProcessPropertiesBoxClicked	();
	void						ActivatePropertiesBox		();

	void						DropCurrentItem				(bool b_all);
	void						EatItem						(PIItem itm);
	
	bool						TryActiveSlot				(CUICellItem* itm);
	bool						ToSlot						(CUICellItem* itm, bool force_place);
	bool						ToBag						(CUICellItem* itm, bool b_use_cursor_pos);
	bool						ToBelt						(CUICellItem* itm, bool b_use_cursor_pos);
	bool						TryUseItem					(CUICellItem* cell_itm);
	bool						ToQuickSlot					(CUICellItem* itm);

	void						AttachAddon					(PIItem item_to_upgrade);
	void						DetachAddon					(const char* addon_name);

	void						SetCurrentItem				(CUICellItem* itm);
	CUICellItem*				CurrentItem					();
	PIItem						CurrentIItem				();
	void						InfoCurItem(CUICellItem* cell_item); //on update item

	TIItemContainer				ruck_list;
	u32							m_iCurrentActiveSlot;



};