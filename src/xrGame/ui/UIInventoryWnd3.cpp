#include "stdafx.h"
#include "UIInventoryWnd.h"
#include "../actor.h"
#include "../silencer.h"
#include "../scope.h"
#include "../grenadelauncher.h"
#include "../WeaponMagazinedWGrenade.h"
#include "../Artifact.h"
#include "../eatable_item.h"
#include "../BottleItem.h" 
#include "../WeaponMagazined.h"
#include "../inventory.h"
#include "../game_base.h"
#include "../game_cl_base.h"
#include "../xr_level_controller.h"
#include "UICellItem.h"
#include "UIListBoxItem.h"
#include "../CustomOutfit.h"
#include "../ActorHelmet.h"
#include "../UICursor.h"
#include "../PDA.h"

void CUIInventoryWnd::EatItem(PIItem itm)
{
	SetCurrentItem(NULL);
	if (!itm->Useful())						return;
//	CActor *pActor = smart_cast<CActor*>(Level().CurrentEntity());
//	if (!pActor)								return; 

	SendEvent_Item_Eat(itm);

	PlaySnd(eInvItemUse);
}

#include "../Medkit.h"
#include "../Antirad.h"
#include "../string_table.h"
void CUIInventoryWnd::ActivatePropertiesBox()
{
	CInventoryOwner* m_pActorInvOwner = smart_cast<CInventoryOwner*>(Level().CurrentViewEntity());
	CInventory&  inv = m_pActorInvOwner->inventory();
	// Флаг-признак для невлючения пункта контекстного меню: Dreess Outfit, если костюм уже надет
	bool bAlreadyDressed = false; 

		
	UIPropertiesBox.RemoveAll();

	CMedkit*			pMedkit				= smart_cast<CMedkit*>			(CurrentIItem());
	CAntirad*			pAntirad			= smart_cast<CAntirad*>			(CurrentIItem());
	CEatableItem*		pEatableItem		= smart_cast<CEatableItem*>		(CurrentIItem());
	CCustomOutfit*		pOutfit				= smart_cast<CCustomOutfit*>	(CurrentIItem());
	CHelmet*			pHelmet				= smart_cast<CHelmet*>			(CurrentIItem());
	CWeapon*			pWeapon				= smart_cast<CWeapon*>			(CurrentIItem());
	CScope*				pScope				= smart_cast<CScope*>			(CurrentIItem());
	CSilencer*			pSilencer			= smart_cast<CSilencer*>		(CurrentIItem());
	CGrenadeLauncher*	pGrenadeLauncher	= smart_cast<CGrenadeLauncher*>	(CurrentIItem());
	CBottleItem*		pBottleItem			= smart_cast<CBottleItem*>		(CurrentIItem());
    
	bool	b_show			= false;

	uint32 slot = CurrentIItem()->GetSlot();

	// ??????? ? ??????????? ???? ????? ?????. Real Wolf.
	bool is_double_slot = slot == RIFLE_SLOT || slot == PISTOL_SLOT;
	if (is_double_slot)
	{
		if (!m_pInv->m_slots[PISTOL_SLOT].m_pIItem || m_pInv->m_slots[PISTOL_SLOT].m_pIItem != CurrentIItem())
		{
			UIPropertiesBox.AddItem("Переместить в слот №1", NULL, INVENTORY_TO_SLOT1_ACTION);
			b_show = true;
		}

		if (!m_pInv->m_slots[RIFLE_SLOT].m_pIItem || m_pInv->m_slots[RIFLE_SLOT].m_pIItem != CurrentIItem())
		{
			UIPropertiesBox.AddItem("Переместить в слот №2", NULL, INVENTORY_TO_SLOT2_ACTION);
			b_show = true;
		}
	}

	if (!pOutfit && !pHelmet && CurrentIItem()->GetSlot() != NO_ACTIVE_SLOT && !m_pInv->m_slots[CurrentIItem()->GetSlot()].m_bPersistent && m_pInv->CanPutInSlot(CurrentIItem()))
	{
		UIPropertiesBox.AddItem("st_move_to_slot", NULL, INVENTORY_TO_SLOT_ACTION);
		b_show = true;
	}
	if (CurrentIItem()->Belt() && m_pInv->CanPutInBelt(CurrentIItem()))
	{
		UIPropertiesBox.AddItem("st_move_on_belt", NULL, INVENTORY_TO_BELT_ACTION);
		b_show = true;
	}

	if (CurrentIItem()->Ruck() && m_pInv->CanPutInRuck(CurrentIItem()) && (CurrentIItem()->GetSlot() == u32(-1) || !m_pInv->m_slots[CurrentIItem()->GetSlot()].m_bPersistent))
	{
		if (!pOutfit)
		{
			if (!pHelmet)
				UIPropertiesBox.AddItem("st_move_to_bag", NULL, INVENTORY_TO_BAG_ACTION);
			else
				UIPropertiesBox.AddItem("st_undress_helmet", NULL, INVENTORY_TO_BAG_ACTION);
		}
		else
			UIPropertiesBox.AddItem("st_undress_outfit", NULL, INVENTORY_TO_BAG_ACTION);

		bAlreadyDressed = true;
		b_show = true;
	}
	if (pOutfit && !bAlreadyDressed)
	{
		UIPropertiesBox.AddItem("st_dress_outfit", NULL, INVENTORY_TO_SLOT_ACTION);
		b_show = true;
	}

	CCustomOutfit* outfit_in_slot = m_pActorInvOwner->GetOutfit();
	if (pHelmet && !bAlreadyDressed && (!outfit_in_slot || outfit_in_slot->bIsHelmetAvaliable))
	{
		UIPropertiesBox.AddItem("st_dress_helmet", NULL, INVENTORY_TO_SLOT_ACTION);
		b_show = true;
	}


	//отсоединение аддонов от вещи
	if(pWeapon)
	{
		if(pWeapon->GrenadeLauncherAttachable() && pWeapon->IsGrenadeLauncherAttached())
		{
			UIPropertiesBox.AddItem("st_detach_gl",  NULL, INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON);
		b_show			= true;
		}
		if(pWeapon->ScopeAttachable() && pWeapon->IsScopeAttached())
		{
			UIPropertiesBox.AddItem("st_detach_scope",  NULL, INVENTORY_DETACH_SCOPE_ADDON);
		b_show			= true;
		}
		if(pWeapon->SilencerAttachable() && pWeapon->IsSilencerAttached())
		{
			UIPropertiesBox.AddItem("st_detach_silencer",  NULL, INVENTORY_DETACH_SILENCER_ADDON);
		b_show			= true;
		}
		if(smart_cast<CWeaponMagazined*>(pWeapon) && IsGameTypeSingle())
		{
			bool b = (0!=pWeapon->GetAmmoElapsed());

			if(!b)
			{
				CUICellItem * itm = CurrentItem();
				for(u32 i=0; i<itm->ChildsCount(); ++i)
				{
					pWeapon		= smart_cast<CWeaponMagazined*>((CWeapon*)itm->Child(i)->m_pData);
					if(pWeapon->GetAmmoElapsed())
					{
						b = true;
						break;
					}
				}
			}

			if(b){
				UIPropertiesBox.AddItem("st_unload_magazine",  NULL, INVENTORY_UNLOAD_MAGAZINE);
				b_show			= true;
			}
		}
	}
	
	//присоединение аддонов к активному слоту (2 или 3)
	if(pScope)
	{
		if(m_pInv->m_slots[PISTOL_SLOT].m_pIItem != NULL &&
		   m_pInv->m_slots[PISTOL_SLOT].m_pIItem->CanAttach(pScope))
		 {
			PIItem tgt = m_pInv->m_slots[PISTOL_SLOT].m_pIItem;
			shared_str str = CStringTable().translate("st_attach_scope_to_pistol");
			str.sprintf("%s %s", str.c_str(), tgt->m_name.c_str());
			UIPropertiesBox.AddItem(str.c_str(), (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 } 
		 if(m_pInv->m_slots[RIFLE_SLOT].m_pIItem != NULL &&
			m_pInv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pScope))
		 {
			PIItem tgt = m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
			shared_str str = CStringTable().translate("st_attach_scope_to_pistol");
			str.sprintf("%s %s", str.c_str(), tgt->m_name.c_str());
			UIPropertiesBox.AddItem(str.c_str(), (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 }
	}
	else if(pSilencer)
	{
		 if(m_pInv->m_slots[PISTOL_SLOT].m_pIItem != NULL &&
		   m_pInv->m_slots[PISTOL_SLOT].m_pIItem->CanAttach(pSilencer))
		 {
			PIItem tgt = m_pInv->m_slots[PISTOL_SLOT].m_pIItem;
			shared_str str = CStringTable().translate("st_attach_silencer_to_pistol");
			str.sprintf("%s %s", str.c_str(), tgt->m_name.c_str());
			UIPropertiesBox.AddItem(str.c_str(), (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 }
		 if(m_pInv->m_slots[RIFLE_SLOT].m_pIItem != NULL &&
			m_pInv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pSilencer))
		 {
			PIItem tgt = m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
			shared_str str = CStringTable().translate("st_attach_silencer_to_pistol");
			str.sprintf("%s %s", str.c_str(), tgt->m_name.c_str());
			UIPropertiesBox.AddItem(str.c_str(), (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 }
	}
	else if(pGrenadeLauncher)
	{
		if (m_pInv->m_slots[PISTOL_SLOT].m_pIItem != NULL &&
			m_pInv->m_slots[PISTOL_SLOT].m_pIItem->CanAttach(pGrenadeLauncher))
		{
			PIItem tgt = m_pInv->m_slots[PISTOL_SLOT].m_pIItem;
			shared_str str = CStringTable().translate("st_attach_gl_to_rifle");
			str.sprintf("%s %s", str.c_str(), tgt->m_name.c_str());
			UIPropertiesBox.AddItem(str.c_str(), (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show = true;
		}
		 if(m_pInv->m_slots[RIFLE_SLOT].m_pIItem != NULL &&
			m_pInv->m_slots[RIFLE_SLOT].m_pIItem->CanAttach(pGrenadeLauncher))
		 {
			PIItem tgt = m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
			shared_str str = CStringTable().translate("st_attach_gl_to_rifle");
			str.sprintf("%s %s", str.c_str(), tgt->m_name.c_str());
			UIPropertiesBox.AddItem(str.c_str(), (void*)tgt, INVENTORY_ATTACH_ADDON);
			b_show			= true;
		 }

	}

	LPCSTR act_str = NULL;
		if (pMedkit || pAntirad)
		{
			act_str = "st_use";
		}
		else if (pBottleItem)
		{
			act_str = "st_drink";
		}
		else if (pEatableItem)
		{
			CObject*	pObj = smart_cast<CObject*>		(CurrentIItem());
			shared_str	section_name = pObj->cNameSect();
			if (!xr_strcmp(section_name, "vodka") || !(xr_strcmp(section_name, "energy_drink")))
			{
				act_str = "st_drink";
			}
			else if (!xr_strcmp(section_name, "bread") || !xr_strcmp(section_name, "kolbasa") || !xr_strcmp(section_name, "conserva"))
			{
				act_str = "st_eat";
			}
			else
			{
				act_str = "st_use";
			}
		}
		if (act_str)
		{
			UIPropertiesBox.AddItem(act_str, NULL, INVENTORY_EAT_ACTION);
			b_show = true;
		}



	bool disallow_drop	= (pOutfit&&bAlreadyDressed);
	disallow_drop		|= !!CurrentIItem()->IsQuestItem();

	if(!disallow_drop)
	{

		UIPropertiesBox.AddItem("st_drop", NULL, INVENTORY_DROP_ACTION);
		b_show			= true;

		if(CurrentItem()->ChildsCount())
			UIPropertiesBox.AddItem("st_drop_all", (void*)33, INVENTORY_DROP_ACTION);
	}

	if(b_show)
	{
		UIPropertiesBox.AutoUpdateSize	();
		UIPropertiesBox.BringAllToTop	();

		Fvector2						cursor_pos;
		Frect							vis_rect;
		GetAbsoluteRect					(vis_rect);
		cursor_pos						= GetUICursor()->GetCursorPosition();
		cursor_pos.sub					(vis_rect.lt);
		UIPropertiesBox.Show			(vis_rect, cursor_pos);
		PlaySnd							(eInvProperties);
	}

//	SetCurrentItem( NULL );
//	UpdateConditionProgressBars();
}

void CUIInventoryWnd::ProcessPropertiesBoxClicked	()
	{
		CUICellItem*	cell_item = CurrentItem();
		if (UIPropertiesBox.GetClickedItem())
		{
			switch (UIPropertiesBox.GetClickedItem()->GetTAG())
			{
			case INVENTORY_TO_SLOT1_ACTION:
				CurrentIItem()->SetSlot(PISTOL_SLOT);
				ToSlot(CurrentItem(), true);
				break;
			case INVENTORY_TO_SLOT2_ACTION:
				CurrentIItem()->SetSlot(RIFLE_SLOT);
				ToSlot(CurrentItem(), true);
				break;
			case INVENTORY_TO_SLOT_ACTION:
				ToSlot(CurrentItem(), true);
				break;
			case INVENTORY_TO_BELT_ACTION:
				ToBelt(CurrentItem(), false);
				break;
			case INVENTORY_TO_BAG_ACTION:
				ToBag(CurrentItem(), false);
				break;
			case INVENTORY_DROP_ACTION:
			{
				void* d = UIPropertiesBox.GetClickedItem()->GetData();
				bool b_all = (d == (void*)33);

				DropCurrentItem(b_all);
			}break;
			case INVENTORY_EAT_ACTION:
				//EatItem(CurrentIItem());
				TryUseItem(cell_item);
				break;
			case INVENTORY_ATTACH_ADDON:
				AttachAddon((PIItem)(UIPropertiesBox.GetClickedItem()->GetData()));
				break;
			case INVENTORY_DETACH_SCOPE_ADDON:
				DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetScopeName());
				break;
			case INVENTORY_DETACH_SILENCER_ADDON:
				DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetSilencerName());
				break;
			case INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON:
				DetachAddon(*(smart_cast<CWeapon*>(CurrentIItem()))->GetGrenadeLauncherName());
				break;
			case INVENTORY_RELOAD_MAGAZINE:
				(smart_cast<CWeapon*>(CurrentIItem()))->Action(kWPN_RELOAD, CMD_START);
				break;
			case INVENTORY_UNLOAD_MAGAZINE:
			{
				CUICellItem * itm = CurrentItem();
				(smart_cast<CWeaponMagazined*>((CWeapon*)itm->m_pData))->UnloadMagazine();
				for (u32 i = 0; i<itm->ChildsCount(); ++i)
				{
					CUICellItem * child_itm = itm->Child(i);
					(smart_cast<CWeaponMagazined*>((CWeapon*)child_itm->m_pData))->UnloadMagazine();
				}
			}break;
			}
		}
	SetCurrentItem(NULL);
	UpdateConditionProgressBars();
}



/*bool CUIInventoryWnd::TryUseItem(PIItem itm)
{
	CBottleItem*		pBottleItem = smart_cast<CBottleItem*>		(itm);
	CMedkit*			pMedkit = smart_cast<CMedkit*>			(itm);
	CAntirad*			pAntirad = smart_cast<CAntirad*>			(itm);
	CEatableItem*		pEatableItem = smart_cast<CEatableItem*>		(itm);


	if (pMedkit || pAntirad || pEatableItem || pBottleItem )
	{
		EatItem(itm);
		return true;
	}
	return false;
}*/

bool CUIInventoryWnd::TryUseItem(CUICellItem* cell_itm)
{
	if (!cell_itm)
	{
		return false;
	}
	PIItem item = (PIItem)cell_itm->m_pData;

	CBottleItem*	pBottleItem = smart_cast<CBottleItem*>	(item);
	CMedkit*		pMedkit = smart_cast<CMedkit*>		(item);
	CAntirad*		pAntirad = smart_cast<CAntirad*>		(item);
	CEatableItem*	pEatableItem = smart_cast<CEatableItem*>	(item);

	if (!(pMedkit || pAntirad || pEatableItem || pBottleItem))
	{
		return false;
	}
	if (!item->Useful())
	{
		return false;
	}
	CInventoryOwner* m_pActorInvOwner = smart_cast<CInventoryOwner*>(Level().CurrentViewEntity());
	u16 recipient = m_pActorInvOwner->object_id();
	if (item->parent_id() != recipient)
	{
		//move_item_from_to	(itm->parent_id(), recipient, itm->object_id());
		cell_itm->OwnerList()->RemoveItem(cell_itm, false);
	}

	SendEvent_Item_Eat(item);
	PlaySnd(eInvItemUse);
	SetCurrentItem( NULL );
	return true;
}


bool CUIInventoryWnd::DropItem(PIItem itm, CUIDragDropListEx* lst)
{
//	if(lst==m_pUIOutfitList)
//	{
//		return TryUseItem			(itm);
//	}
	CUICellItem*	_citem	= lst->ItemsCount() ? lst->GetItemIdx(0) : NULL;
	PIItem _iitem	= _citem ? (PIItem)_citem->m_pData : NULL;

	if(!_iitem)						return	false;
	if(!_iitem->CanAttach(itm))		return	false;
	AttachAddon						(_iitem);

	return							true;
}

// =================== ПОТСВЕТКА АДОНОВ, ПАТРОНОВ. АВТОР GSC =====================

void CUIInventoryWnd::clear_highlight_lists()
{
	m_InvSlot2Highlight->Show(false);
	m_InvSlot3Highlight->Show(false);
	m_HelmetSlotHighlight->Show(false);
	m_OutfitSlotHighlight->Show(false);
	m_DetectorSlotHighlight->Show(false);
	for (u8 i = 0; i<4; i++)
		m_QuickSlotsHighlight[i]->Show(false);
	for (u8 i = 0; i<e_af_count; i++)
		m_ArtefactSlotsHighlight[i]->Show(false);

	m_pUIBagList->clear_select_armament();
	m_pUIBeltList->clear_select_armament();
	m_highlight_clear = true;
}

void CUIInventoryWnd::highlight_item_slot(CUICellItem* cell_item)
{
	PIItem item = (PIItem)cell_item->m_pData;
	if (!item)
		return;

	if (CUIDragDropListEx::m_drag_item)
		return;

	u32 slot_id = item->GetSlot();
	CWeapon* weapon = smart_cast<CWeapon*>(item);
	if (weapon && (slot_id == PISTOL_SLOT || slot_id == RIFLE_SLOT))
	{
		m_InvSlot2Highlight->Show(true);
		m_InvSlot3Highlight->Show(true);
		return;
	}
	CHelmet* helmet = smart_cast<CHelmet*>(item);
	if (helmet && slot_id == HELMET_SLOT)
	{
		m_HelmetSlotHighlight->Show(true);
		return;
	}
	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(item);
	if (outfit && slot_id == OUTFIT_SLOT)
	{
		m_OutfitSlotHighlight->Show(true);
		return;
	}
	CCustomDetector* detector = smart_cast<CCustomDetector*>(item);
	if (detector && DETECTOR_SLOT)
	{
		m_DetectorSlotHighlight->Show(true);
		return;
	}

	CEatableItem* eatable = smart_cast<CEatableItem*>(item);
	if (eatable)
	{
		if (cell_item->OwnerList() && GetType(cell_item->OwnerList()) == iQuickSlot)
			return;

		for (u8 i = 0; i<4; i++)
			m_QuickSlotsHighlight[i]->Show(true);
		return;
	}
	CArtefact* artefact = smart_cast<CArtefact*>(item);
	if (artefact)
	{
		if (cell_item->OwnerList() && GetType(cell_item->OwnerList()) == iwBelt)
			return;

		Ivector2 cap = m_pUIBeltList->CellsCapacity();
		for (u8 i = 0; i<cap.x; i++)
			m_ArtefactSlotsHighlight[i]->Show(true);
		return;
	}
}
void CUIInventoryWnd::set_highlight_item(CUICellItem* cell_item)
{
	PIItem item = (PIItem)cell_item->m_pData;
	if (!item)
	{
		return;
	}
	highlight_item_slot(cell_item);
	highlight_armament(item, m_pUIBeltList);
	highlight_armament(item, m_pUIBagList);

	//		break;

	m_highlight_clear = false;

}

void CUIInventoryWnd::highlight_armament(PIItem item, CUIDragDropListEx* ddlist)
{
	ddlist->clear_select_armament();
	highlight_ammo_for_weapon(item, ddlist);
	highlight_weapons_for_ammo(item, ddlist);
	highlight_weapons_for_addon(item, ddlist);

}

void CUIInventoryWnd::highlight_ammo_for_weapon(PIItem weapon_item, CUIDragDropListEx* ddlist)
{
	VERIFY(weapon_item);
	VERIFY(ddlist);
	static xr_vector<shared_str>	ammo_types;
	ammo_types.clear_not_free();

	CWeapon* weapon = smart_cast<CWeapon*>(weapon_item);
	if (!weapon)
	{
		return;
	}
	ammo_types.assign(weapon->m_ammoTypes.begin(), weapon->m_ammoTypes.end());

	CWeaponMagazinedWGrenade* wg = smart_cast<CWeaponMagazinedWGrenade*>(weapon_item);
	if (wg)
	{
		if (wg->IsGrenadeLauncherAttached() && wg->m_ammoTypes2.size())
		{
			ammo_types.insert(ammo_types.end(), wg->m_ammoTypes2.begin(), wg->m_ammoTypes2.end());
		}
	}

	if (ammo_types.size() == 0)
	{
		return;
	}
	xr_vector<shared_str>::iterator ite = ammo_types.end();

	u32 const cnt = ddlist->ItemsCount();
	for (u32 i = 0; i < cnt; ++i)
	{
		CUICellItem* ci = ddlist->GetItemIdx(i);
		PIItem item = (PIItem)ci->m_pData;
		if (!item)
		{
			continue;
		}
		CWeaponAmmo* ammo = smart_cast<CWeaponAmmo*>(item);
		if (!ammo)
		{
			highlight_addons_for_weapon(weapon_item, ci);
			continue; // for i
		}
		shared_str const& ammo_name = item->object().cNameSect();

		xr_vector<shared_str>::iterator itb = ammo_types.begin();
		for (; itb != ite; ++itb)
		{
			if (ammo_name._get() == (*itb)._get())
			{
				ci->m_select_armament = true;
				break; // itb
			}
		}
	}//for i

}

void CUIInventoryWnd::highlight_weapons_for_ammo(PIItem ammo_item, CUIDragDropListEx* ddlist)
{
	VERIFY(ammo_item);
	VERIFY(ddlist);
	CWeaponAmmo* ammo = smart_cast<CWeaponAmmo*>(ammo_item);
	if (!ammo)
	{
		return;
	}

	shared_str const& ammo_name = ammo_item->object().cNameSect();

	u32 const cnt = ddlist->ItemsCount();
	for (u32 i = 0; i < cnt; ++i)
	{
		CUICellItem* ci = ddlist->GetItemIdx(i);
		PIItem item = (PIItem)ci->m_pData;
		if (!item)
		{
			continue;
		}
		CWeapon* weapon = smart_cast<CWeapon*>(item);
		if (!weapon)
		{
			continue;
		}

		xr_vector<shared_str>::iterator itb = weapon->m_ammoTypes.begin();
		xr_vector<shared_str>::iterator ite = weapon->m_ammoTypes.end();
		for (; itb != ite; ++itb)
		{
			if (ammo_name._get() == (*itb)._get())
			{
				ci->m_select_armament = true;
				break; // for itb
			}
		}

		CWeaponMagazinedWGrenade* wg = smart_cast<CWeaponMagazinedWGrenade*>(item);
		if (!wg || !wg->IsGrenadeLauncherAttached() || !wg->m_ammoTypes2.size())
		{
			continue; // for i
		}
		itb = wg->m_ammoTypes2.begin();
		ite = wg->m_ammoTypes2.end();
		for (; itb != ite; ++itb)
		{
			if (ammo_name._get() == (*itb)._get())
			{
				ci->m_select_armament = true;
				break; // for itb
			}
		}
	}//for i

}

bool CUIInventoryWnd::highlight_addons_for_weapon(PIItem weapon_item, CUICellItem* ci)
{
	PIItem item = (PIItem)ci->m_pData;
	if (!item)
	{
		return false;
	}

	CScope* pScope = smart_cast<CScope*>(item);
	if (pScope && weapon_item->CanAttach(pScope))
	{
		ci->m_select_armament = true;
		return true;
	}

	CSilencer* pSilencer = smart_cast<CSilencer*>(item);
	if (pSilencer && weapon_item->CanAttach(pSilencer))
	{
		ci->m_select_armament = true;
		return true;
	}

	CGrenadeLauncher* pGrenadeLauncher = smart_cast<CGrenadeLauncher*>(item);
	if (pGrenadeLauncher && weapon_item->CanAttach(pGrenadeLauncher))
	{
		ci->m_select_armament = true;
		return true;
	}
	return false;
}

void CUIInventoryWnd::highlight_weapons_for_addon(PIItem addon_item, CUIDragDropListEx* ddlist)
{
	VERIFY(addon_item);
	VERIFY(ddlist);

	CScope*				pScope = smart_cast<CScope*>			(addon_item);
	CSilencer*			pSilencer = smart_cast<CSilencer*>		(addon_item);
	CGrenadeLauncher*	pGrenadeLauncher = smart_cast<CGrenadeLauncher*>	(addon_item);

	if (!pScope && !pSilencer && !pGrenadeLauncher)
	{
		return;
	}

	u32 const cnt = ddlist->ItemsCount();
	for (u32 i = 0; i < cnt; ++i)
	{
		CUICellItem* ci = ddlist->GetItemIdx(i);
		PIItem item = (PIItem)ci->m_pData;
		if (!item)
		{
			continue;
		}
		CWeapon* weapon = smart_cast<CWeapon*>(item);
		if (!weapon)
		{
			continue;
		}

		if (pScope && weapon->CanAttach(pScope))
		{
			ci->m_select_armament = true;
			continue;
		}
		if (pSilencer && weapon->CanAttach(pSilencer))
		{
			ci->m_select_armament = true;
			continue;
		}
		if (pGrenadeLauncher && weapon->CanAttach(pGrenadeLauncher))
		{
			ci->m_select_armament = true;
			continue;
		}

	}//for i
}
