#include "pch_script.h"
#include "UIInventoryWnd.h"

#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "../string_table.h"
#include "../actor.h"
#include "../uigamesp.h"
#include "../hudmanager.h"
#include "UIMainIngameWnd.h"
#include "../CustomOutfit.h"
#include "../weapon.h"
#include "../script_process.h"
#include "../eatable_item.h"
#include "../inventory.h"
#include "UIInventoryUtilities.h"
using namespace InventoryUtilities;

#include "../InfoPortion.h"
#include "../level.h"
#include "../game_base_space.h"
#include "../entitycondition.h"
#include "../game_cl_base.h"
#include "UISleepWnd.h"
#include "../ActorCondition.h"
#include "UIDragDropListEx.h"
#include "UIDragDropReferenceList.h"
#include "UIOutfitSlot.h"
#include "UI3tButton.h"
#include "UIHelper.h"
#include "UIActorStateInfo.h"
#include "UICharacterInfo.h"

CUIInventoryWnd*	g_pInvWnd = NULL;

CUIInventoryWnd::CUIInventoryWnd()
{
	m_iCurrentActiveSlot				= NO_ACTIVE_SLOT;
	Init								();
	SetCurrentItem						(NULL);

	g_pInvWnd							= this;	
	m_b_need_reinit						= false;
	Hide								();	
}

CUIInventoryWnd::~CUIInventoryWnd()
{
	xr_delete(m_ItemInfo);
	ClearAllLists();
}

void CUIInventoryWnd::Init()
{
	CUIXml							uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, "inventory_new.xml");

	CUIXmlInit						xml_init;
	xml_init.InitWindow(uiXml, "main", 0, this);

	m_ActorCharacterInfo = xr_new<CUICharacterInfo>();
	m_ActorCharacterInfo->SetAutoDelete(true);
	AttachChild(m_ActorCharacterInfo);
	m_ActorCharacterInfo->InitCharacterInfo(&uiXml, "actor_ch_info");

	m_ActorMoney = UIHelper::CreateTextWnd(uiXml, "actor_money_static", this);
	m_ActorBottomInfo = UIHelper::CreateTextWnd(uiXml, "actor_weight_caption", this);
	m_ActorWeight = UIHelper::CreateTextWnd(uiXml, "actor_weight", this);
	m_ActorWeightMax = UIHelper::CreateTextWnd(uiXml, "actor_weight_max", this);
	m_ActorBottomInfo->AdjustWidthToText();

	//Ёлементы автоматического добавлени€
	xml_init.InitAutoStatic				(uiXml, "auto_static", this);

		
	m_InvSlot2Highlight = UIHelper::CreateStatic(uiXml, "inv_slot2_highlight", this);
	m_InvSlot2Highlight->Show(false);
	m_InvSlot3Highlight = UIHelper::CreateStatic(uiXml, "inv_slot3_highlight", this);
	m_InvSlot3Highlight->Show(false);
	m_HelmetSlotHighlight = UIHelper::CreateStatic(uiXml, "helmet_slot_highlight", this);
	m_HelmetSlotHighlight->Show(false);
	m_OutfitSlotHighlight = UIHelper::CreateStatic(uiXml, "outfit_slot_highlight", this);
	m_OutfitSlotHighlight->Show(false);
	m_DetectorSlotHighlight = UIHelper::CreateStatic(uiXml, "detector_slot_highlight", this);
	m_DetectorSlotHighlight->Show(false);
	m_QuickSlotsHighlight[0] = UIHelper::CreateStatic(uiXml, "quick_slot_highlight", this);
	m_QuickSlotsHighlight[0]->Show(false);
	m_ArtefactSlotsHighlight[0] = UIHelper::CreateStatic(uiXml, "artefact_slot_highlight", this);
	m_ArtefactSlotsHighlight[0]->Show(false);

	Fvector2 pos;
	pos = m_QuickSlotsHighlight[0]->GetWndPos();
	float dx = uiXml.ReadAttribFlt("quick_slot_highlight", 0, "dx", 24.0f);
	for (u8 i = 1; i<4; i++)
	{
		pos.x += dx;
		m_QuickSlotsHighlight[i] = UIHelper::CreateStatic(uiXml, "quick_slot_highlight", this);
		m_QuickSlotsHighlight[i]->SetWndPos(pos);
		m_QuickSlotsHighlight[i]->Show(false);
	}
	pos = m_ArtefactSlotsHighlight[0]->GetWndPos();
	dx = uiXml.ReadAttribFlt("artefact_slot_highlight", 0, "dx", 24.0f);
	for (u8 i = 1; i<e_af_count; i++)
	{
		pos.x += dx;
		m_ArtefactSlotsHighlight[i] = UIHelper::CreateStatic(uiXml, "artefact_slot_highlight", this);
		m_ArtefactSlotsHighlight[i]->SetWndPos(pos);
		m_ArtefactSlotsHighlight[i]->Show(false);
	}

	m_pUIBagList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_bag", this);
	m_pUIBeltList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_belt", this);
	m_pUIOutfitList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_outfit", this);
	m_pUIHelmetList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_helmet", this);
	m_pUIPistolList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_pistol", this);
	m_pUIAutomaticList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_automatic", this);
	m_pUIDetectorList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_detector", this);
	m_pTrashList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_trash", this);
	m_pQuickSlot = UIHelper::CreateDragDropReferenceList(uiXml, "dragdrop_quick_slots", this);
	m_pQuickSlot->Initialize();

	m_pTrashList->m_f_item_drop = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIInventoryWnd::OnItemDrop);
	m_pTrashList->m_f_drag_event = CUIDragDropListEx::DRAG_ITEM_EVENT(this, &CUIInventoryWnd::OnDragItemOnTrash);

	m_WeaponSlot1_progress = UIHelper::CreateProgressBar(uiXml, "progess_bar_weapon1", this);
	m_WeaponSlot2_progress = UIHelper::CreateProgressBar(uiXml, "progess_bar_weapon2", this);
	m_Helmet_progress = UIHelper::CreateProgressBar(uiXml, "progess_bar_helmet", this);
	m_Outfit_progress = UIHelper::CreateProgressBar(uiXml, "progess_bar_outfit", this);

	m_QuickSlot1 = UIHelper::CreateTextWnd(uiXml, "quick_slot1_text", this);
	m_QuickSlot2 = UIHelper::CreateTextWnd(uiXml, "quick_slot2_text", this);
	m_QuickSlot3 = UIHelper::CreateTextWnd(uiXml, "quick_slot3_text", this);
	m_QuickSlot4 = UIHelper::CreateTextWnd(uiXml, "quick_slot4_text", this);


	BindDragDropListEnents(m_pUIBagList);
	BindDragDropListEnents(m_pUIBeltList);
	BindDragDropListEnents(m_pUIOutfitList);
	BindDragDropListEnents(m_pUIHelmetList);
	BindDragDropListEnents(m_pUIPistolList);
	BindDragDropListEnents(m_pUIAutomaticList);
	BindDragDropListEnents(m_pUIDetectorList);
	BindDragDropListEnents(m_pTrashList);
	BindDragDropListEnents(m_pQuickSlot);

	m_belt_list_over[0] = UIHelper::CreateStatic(uiXml, "belt_list_over", this);
	pos = m_belt_list_over[0]->GetWndPos();
	dx = uiXml.ReadAttribFlt("belt_list_over", 0, "dx", 10.0f);
	for (u8 i = 1; i < e_af_count; ++i)
	{
		pos.x += dx;
		m_belt_list_over[i] = UIHelper::CreateStatic(uiXml, "belt_list_over", this);
		m_belt_list_over[i]->SetWndPos(pos);
	}

	//pop-up menu
	AttachChild(&UIPropertiesBox);
	UIPropertiesBox.InitPropertiesBox(Fvector2().set(0, 0), Fvector2().set(300, 300));
	UIPropertiesBox.Hide();


	UIExitButton = UIHelper::Create3tButton(uiXml, "exit_button", this);

	m_ActorStateInfo = xr_new<ui_actor_state_wnd>();
	m_ActorStateInfo->init_from_xml(uiXml, "actor_state_info");
	m_ActorStateInfo->SetAutoDelete(true);
	AttachChild(m_ActorStateInfo);

//Load sounds

	XML_NODE* stored_root				= uiXml.GetLocalRoot		();
	uiXml.SetLocalRoot					(uiXml.NavigateToNode		("action_sounds",0));
	::Sound->create						(sounds[eInvSndOpen],		uiXml.Read("snd_open",			0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvSndClose],		uiXml.Read("snd_close",			0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvItemToSlot],	uiXml.Read("snd_item_to_slot",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvItemToBelt],	uiXml.Read("snd_item_to_belt",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvItemToRuck],	uiXml.Read("snd_item_to_ruck",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvProperties],	uiXml.Read("snd_properties",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvDropItem],		uiXml.Read("snd_drop_item",		0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvAttachAddon],	uiXml.Read("snd_attach_addon",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvDetachAddon],	uiXml.Read("snd_detach_addon",	0,	NULL),st_Effect,sg_SourceType);
	::Sound->create						(sounds[eInvItemUse],		uiXml.Read("snd_item_use",		0,	NULL),st_Effect,sg_SourceType);

	uiXml.SetLocalRoot					(stored_root);

	m_ItemInfo = xr_new<CUIItemInfo>();
	m_ItemInfo->InitItemInfo("actor_menu_item.xml");

	m_highlight_clear = true;
	SetCurrentItem(NULL);
	m_item_info_view = false;
}

EListType CUIInventoryWnd::GetType(CUIDragDropListEx* l)
{
	if(l==m_pUIBagList)			return iwBag;
	if(l==m_pUIBeltList)		return iwBelt;
	if(l==m_pUIDetectorList)	return iwSlot;
	if(l==m_pUIAutomaticList)	return iwSlot;
	if(l==m_pUIPistolList)		return iwSlot;
	if(l==m_pUIOutfitList)		return iwSlot;
	if(l==m_pUIHelmetList)		return iwSlot;
	if(l==m_pTrashList)			return iTrashSlot;
	if(l==m_pQuickSlot)			return iQuickSlot;


	NODEFAULT;
#ifdef DEBUG
	return iwSlot;
#endif // DEBUG
}

void CUIInventoryWnd::PlaySnd(eInventorySndAction a)
{
	if (sounds[a]._handle())
        sounds[a].play					(NULL, sm_2D);
}


bool CUIInventoryWnd::OnMouseAction(float x, float y, EUIMessages mouse_action)
{
	if (m_b_need_reinit)
		return true;

	//вызов дополнительного меню по правой кнопке
	if (mouse_action == WINDOW_RBUTTON_DOWN)
	{
		if (UIPropertiesBox.IsShown())
		{
			UIPropertiesBox.Hide();
			return						true;
		}
	}

	CUIWindow::OnMouseAction(x, y, mouse_action);

	return true; // always returns true, because ::StopAnyMove() == true;
}

void CUIInventoryWnd::Draw()
{
	CUIWindow::Draw						();
	m_ItemInfo->Draw();
	HUD().GetUI()->UIMainIngameWnd->DrawZoneMap();
	HUD().GetUI()->UIMainIngameWnd->DrawMainIndicatorsForInventory();
}



void CUIInventoryWnd::Update()
{
	if(m_b_need_reinit)
		InitInventory					();

		CInventoryOwner* m_pActorInvOwner = smart_cast<CInventoryOwner*>(Level().CurrentViewEntity());
			if (m_pActorInvOwner)
				{
					UpdateOutfit();

					UpdateButtonsLayout();
					//UpdateConditionProgressBars();
					HUD().GetUI()->UIMainIngameWnd->UpdateZoneMap();
				}

			m_ActorCharacterInfo->InitCharacter(m_pActorInvOwner->object_id());

			InventoryUtilities::UpdateWeightStr(*m_ActorWeight, *m_ActorWeightMax, m_pActorInvOwner);
			m_ActorWeight->AdjustWidthToText();
			m_ActorWeightMax->AdjustWidthToText();
			m_ActorBottomInfo->AdjustWidthToText();
			Fvector2 pos = m_ActorWeight->GetWndPos();
			pos.x = m_ActorWeightMax->GetWndPos().x - m_ActorWeight->GetWndSize().x - 5.0f;
			m_ActorWeight->SetWndPos(pos);
			pos.x = pos.x - m_ActorBottomInfo->GetWndSize().x - 5.0f;
			m_ActorBottomInfo->SetWndPos(pos);

			m_ActorStateInfo->UpdateActorInfo(m_pActorInvOwner);
			m_ItemInfo->Update();

			if (IsGameTypeSingle())
			{
				string64 buf;
				sprintf_s(buf, "%d RU", m_pActorInvOwner->get_money());
				m_ActorMoney->SetText(buf);
			}

	//	m_pQuickSlot->ReloadReferences(m_pActorInvOwner);
	CUIWindow::Update					();
}

void CUIInventoryWnd::Show()
{
	InitInventory();
	inherited::Show();

	if (!IsGameTypeSingle())
	{
		CActor *pActor = smart_cast<CActor*>(Level().CurrentEntity());
		if (!pActor) return;

		pActor->SetWeaponHideState(INV_STATE_INV_WND, true);

		SendInfoToActor("ui_inventory");

		Update();
		PlaySnd(eInvSndOpen);

	}
	VERIFY(HUD().GetUI() && HUD().GetUI()->UIMainIngameWnd);
	HUD().GetUI()->UIMainIngameWnd->ShowZoneMap(true);
}

void CUIInventoryWnd::Hide()
{
	PlaySnd								(eInvSndClose);
	inherited::Hide						();

	SendInfoToActor						("ui_inventory_hide");
	ClearAllLists						();

	//достать вещь в активный слот
	CActor *pActor = smart_cast<CActor*>(Level().CurrentEntity());
	if(pActor && m_iCurrentActiveSlot != NO_ACTIVE_SLOT && 
		pActor->inventory().m_slots[m_iCurrentActiveSlot].m_pIItem)
	{
		pActor->inventory().Activate(m_iCurrentActiveSlot);
		m_iCurrentActiveSlot = NO_ACTIVE_SLOT;
	}

	if (!IsGameTypeSingle())
	{
		CActor *pActor		= smart_cast<CActor*>(Level().CurrentEntity());
		if(!pActor)			return;

		pActor->SetWeaponHideState(INV_STATE_INV_WND, false);
	}
	VERIFY(HUD().GetUI() && HUD().GetUI()->UIMainIngameWnd);
	HUD().GetUI()->UIMainIngameWnd->ShowZoneMap(false);
}

void CUIInventoryWnd::AttachAddon(PIItem item_to_upgrade)
{
	PlaySnd										(eInvAttachAddon);
	R_ASSERT									(item_to_upgrade);
	if (OnClient())
	{
		NET_Packet								P;
		item_to_upgrade->object().u_EventGen	(P, GE_ADDON_ATTACH, item_to_upgrade->object().ID());
		P.w_u32									(CurrentIItem()->object().ID());
		item_to_upgrade->object().u_EventSend	(P);
	};

	item_to_upgrade->Attach						(CurrentIItem(), true);


	//спр€тать вещь из активного слота в инвентарь на врем€ вызова менюшки
	CActor *pActor								= smart_cast<CActor*>(Level().CurrentEntity());
	if(pActor && item_to_upgrade == pActor->inventory().ActiveItem())
	{
			m_iCurrentActiveSlot				= pActor->inventory().GetActiveSlot();
			pActor->inventory().Activate		(NO_ACTIVE_SLOT);
	}
	SetCurrentItem								(NULL);
}

void CUIInventoryWnd::DetachAddon(const char* addon_name)
{
	PlaySnd										(eInvDetachAddon);
	if (OnClient())
	{
		NET_Packet								P;
		CurrentIItem()->object().u_EventGen		(P, GE_ADDON_DETACH, CurrentIItem()->object().ID());
		P.w_stringZ								(addon_name);
		CurrentIItem()->object().u_EventSend	(P);
	};
	CurrentIItem()->Detach						(addon_name, true);

	//спр€тать вещь из активного слота в инвентарь на врем€ вызова менюшки
	CActor *pActor								= smart_cast<CActor*>(Level().CurrentEntity());
	if(pActor && CurrentIItem() == pActor->inventory().ActiveItem())
	{
			m_iCurrentActiveSlot				= pActor->inventory().GetActiveSlot();
			pActor->inventory().Activate		(NO_ACTIVE_SLOT);
	}
}


void	CUIInventoryWnd::SendEvent_ActivateSlot	(PIItem	pItem)
{
	NET_Packet						P;
	pItem->object().u_EventGen		(P, GEG_PLAYER_ACTIVATE_SLOT, pItem->object().H_Parent()->ID());
	P.w_u32							(pItem->GetSlot());
	pItem->object().u_EventSend		(P);
}

void CUIInventoryWnd::SendEvent_ActivateSlot2(u32 slot, u32 recipient)
{
	NET_Packet						P;
	CGameObject::u_EventGen(P, GEG_PLAYER_ACTIVATE_SLOT, recipient);
	P.w_u32(slot);
	CGameObject::u_EventSend(P);
}

void	CUIInventoryWnd::SendEvent_Item2Slot			(PIItem	pItem)
{
	NET_Packet						P;
	pItem->object().u_EventGen		(P, GEG_PLAYER_ITEM2SLOT, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	pItem->object().u_EventSend		(P);
	g_pInvWnd->PlaySnd				(eInvItemToSlot);
};

void	CUIInventoryWnd::SendEvent_Item2Belt			(PIItem	pItem)
{
	NET_Packet						P;
	pItem->object().u_EventGen		(P, GEG_PLAYER_ITEM2BELT, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	pItem->object().u_EventSend		(P);
	g_pInvWnd->PlaySnd				(eInvItemToBelt);
};

void	CUIInventoryWnd::SendEvent_Item2Ruck			(PIItem	pItem)
{
	NET_Packet						P;
	pItem->object().u_EventGen		(P, GEG_PLAYER_ITEM2RUCK, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	pItem->object().u_EventSend		(P);

	g_pInvWnd->PlaySnd				(eInvItemToRuck);
};

void	CUIInventoryWnd::SendEvent_Item_Drop(PIItem	pItem)
{
	pItem->SetDropManual			(TRUE);

	if( OnClient() )
	{
		NET_Packet					P;
		pItem->object().u_EventGen	(P, GE_OWNERSHIP_REJECT, pItem->object().H_Parent()->ID());
		P.w_u16						(pItem->object().ID());
		pItem->object().u_EventSend(P);
	}
	g_pInvWnd->PlaySnd				(eInvDropItem);
};

void	CUIInventoryWnd::SendEvent_Item_Eat			(PIItem	pItem)
{
	R_ASSERT						(pItem->m_pCurrentInventory==m_pInv);
	NET_Packet						P;
	pItem->object().u_EventGen		(P, GEG_PLAYER_ITEM_EAT, pItem->object().H_Parent()->ID());
	P.w_u16							(pItem->object().ID());
	pItem->object().u_EventSend		(P);
};

void CUIInventoryWnd::BindDragDropListEnents(CUIDragDropListEx* lst)
{
	lst->m_f_item_drop = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIInventoryWnd::OnItemDrop);
	lst->m_f_item_start_drag = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIInventoryWnd::OnItemStartDrag);
	lst->m_f_item_db_click = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIInventoryWnd::OnItemDbClick);
	lst->m_f_item_selected = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIInventoryWnd::OnItemSelected);
	lst->m_f_item_rbutton_click = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIInventoryWnd::OnItemRButtonClick);
	lst->m_f_item_focused_update = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIInventoryWnd::OnItemFocusedUpdate);
	lst->m_f_item_focus_received = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIInventoryWnd::OnItemFocusReceive);
	lst->m_f_item_focus_lost = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUIInventoryWnd::OnItemFocusLost);
}


#include "../xr_level_controller.h"
#include <dinput.h>

bool CUIInventoryWnd::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	if (m_b_need_reinit)
		return true;

	if (UIPropertiesBox.GetVisible())
		UIPropertiesBox.OnKeyboardAction(dik, keyboard_action);

	if (is_binded(kDROP, dik))
	{
		if (WINDOW_KEY_PRESSED == keyboard_action && CurrentIItem() && !CurrentIItem()->IsQuestItem())

			SendEvent_Item_Drop(CurrentIItem());
			SetCurrentItem(NULL);

			DropCurrentItem(false);
		return true;
	}
	if (inherited::OnKeyboardAction(dik, keyboard_action))return true;
	return false;

}
