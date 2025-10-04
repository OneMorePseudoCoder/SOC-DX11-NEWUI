#include "stdafx.h"
#include "UIInventoryWnd.h"
#include "UISleepWnd.h"
#include "../level.h"
#include "../actor.h"
#include "../ActorCondition.h"
#include "../hudmanager.h"
#include "../inventory.h"
#include "UIInventoryUtilities.h"

#include "UICellItem.h"
#include "UICellItemFactory.h"
#include "UIDragDropListEx.h"
#include "UIDragDropReferenceList.h"
#include "UI3tButton.h"
#include "../player_hud.h"
#include "../CustomOutfit.h"
#include "../ActorHelmet.h"
#include "../string_table.h"

CUICellItem* CUIInventoryWnd::CurrentItem()
{
	return m_pCurrentCellItem;
}

PIItem CUIInventoryWnd::CurrentIItem()
{
	return	(m_pCurrentCellItem)?(PIItem)m_pCurrentCellItem->m_pData : NULL;
}

void CUIInventoryWnd::SetCurrentItem(CUICellItem* itm)
{
	m_pCurrentCellItem = itm;
	if (!itm)
	{
		InfoCurItem(NULL);
	}
}

void CUIInventoryWnd::InfoCurItem(CUICellItem* cell_item)
{
	if (!cell_item)
	{
		m_ItemInfo->InitItem(NULL);
		return;
	}
	PIItem current_item = (PIItem)cell_item->m_pData;

	CInventoryOwner* m_pActorInvOwner = smart_cast<CInventoryOwner*>(Level().CurrentViewEntity());

		PIItem compare_item = NULL;
		u32    compare_slot = current_item->GetSlot();
		if (compare_slot != NO_ACTIVE_SLOT)
		{
			compare_item = m_pActorInvOwner->inventory().m_slots[compare_slot].m_pIItem;
		}
	
	m_ItemInfo->InitItem(cell_item, compare_item, u32(-1));
	float dx_pos = GetWndRect().left;
	fit_in_rect(m_ItemInfo, Frect().set(0.0f, 0.0f, UI_BASE_WIDTH - dx_pos, UI_BASE_HEIGHT), 10.0f, dx_pos);

}

void CUIInventoryWnd::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if(pWnd == &UIPropertiesBox &&	msg==PROPERTY_CLICKED)
	{
		ProcessPropertiesBoxClicked	();
	}else 
	if (UIExitButton == pWnd && BUTTON_CLICKED == msg)
	{
		GetHolder()->StartStopMenu			(this,true);
	}

	CUIWindow::SendMessage(pWnd, msg, pData);
}


void CUIInventoryWnd::InitInventory_delayed()
{
	m_b_need_reinit = true;
}

void CUIInventoryWnd::InitInventory() 
{
	CInventoryOwner *pInvOwner	= smart_cast<CInventoryOwner*>(Level().CurrentEntity());
	if(!pInvOwner)				return;

	m_pInv						= &pInvOwner->inventory();

	UIPropertiesBox.Hide		();
	ClearAllLists				();
	m_pMouseCapturer			= NULL;
	SetCurrentItem				(NULL);

	//Slots
	PIItem _itm							= m_pInv->m_slots[PISTOL_SLOT].m_pIItem;
	if(_itm)
	{
		CUICellItem* itm				= create_cell_item(_itm);
		m_pUIPistolList->SetItem		(itm);
	}

	_itm								= m_pInv->m_slots[RIFLE_SLOT].m_pIItem;
	if(_itm)
	{
		CUICellItem* itm				= create_cell_item(_itm);
		m_pUIAutomaticList->SetItem		(itm);
	}

	_itm								= m_pInv->m_slots[DETECTOR_SLOT].m_pIItem;
	if (_itm)
	{
		CUICellItem* itm = create_cell_item(_itm);
		m_pUIDetectorList->SetItem		(itm);
	}

	_itm								= m_pInv->m_slots[HELMET_SLOT].m_pIItem;
	if (_itm)
	{
		CUICellItem* itm = create_cell_item(_itm);
		m_pUIHelmetList->SetItem		(itm);
	}

	CInventoryOwner* m_pActorInvOwner = smart_cast<CInventoryOwner*>(Level().CurrentViewEntity());
	if (m_pActorInvOwner)
	{
		m_pQuickSlot->ReloadReferences(m_pActorInvOwner);
	}

	PIItem _outfit = m_pInv->m_slots[OUTFIT_SLOT].m_pIItem;
	CUICellItem* outfit = (_outfit) ? create_cell_item(_outfit) : NULL;
	if (outfit)
	{
		m_pUIOutfitList->SetItem(outfit); 
	}

	TIItemContainer::iterator it, it_e;
	for(it=m_pInv->m_belt.begin(),it_e=m_pInv->m_belt.end(); it!=it_e; ++it) 
	{
		CUICellItem* itm			= create_cell_item(*it);
		m_pUIBeltList->SetItem		(itm);
	}

	
	ruck_list		= m_pInv->m_ruck;
	std::sort		(ruck_list.begin(),ruck_list.end(),InventoryUtilities::GreaterRoomInRuck);

	int i=1;
	for(it=ruck_list.begin(),it_e=ruck_list.end(); it!=it_e; ++it,++i) 
	{
		CUICellItem* itm			= create_cell_item(*it);
		m_pUIBagList->SetItem		(itm);
	}
	//fake
	_itm								= m_pInv->m_slots[GRENADE_SLOT].m_pIItem;
	if(_itm)
	{
		CUICellItem* itm				= create_cell_item(_itm);
		m_pUIBagList->SetItem			(itm);
	}

	m_b_need_reinit					= false;
}  

void CUIInventoryWnd::DropCurrentItem(bool b_all)
{

	CActor *pActor			= smart_cast<CActor*>(Level().CurrentEntity());
	if(!pActor)				return;

	if(!b_all && CurrentIItem() && !CurrentIItem()->IsQuestItem())
	{
		SendEvent_Item_Drop		(CurrentIItem());
		SetCurrentItem			(NULL);
	//	InventoryUtilities::UpdateWeight			(UIBagWnd, true);
		return;
	}

	if(b_all && CurrentIItem() && !CurrentIItem()->IsQuestItem())
	{
		u32 cnt = CurrentItem()->ChildsCount();

		for(u32 i=0; i<cnt; ++i){
			CUICellItem*	itm = CurrentItem()->PopChild(NULL);
			PIItem			iitm			= (PIItem)itm->m_pData;
			SendEvent_Item_Drop				(iitm);
		}

		SendEvent_Item_Drop					(CurrentIItem());
		SetCurrentItem						(NULL);
//		InventoryUtilities::UpdateWeight	(UIBagWnd, true);
		return;
	}

}

//------------------------------------------

bool CUIInventoryWnd::TryActiveSlot(CUICellItem* itm)
{
		PIItem	iitem = (PIItem)itm->m_pData;
		u32 slot = iitem->GetSlot();

		if (slot == GRENADE_SLOT)
		{
			PIItem	prev_iitem = GetInventory()->m_slots[slot].m_pIItem;
			if (prev_iitem && (prev_iitem->object().cNameSect() != iitem->object().cNameSect()))
			{
				SendEvent_Item2Ruck(prev_iitem);
				SendEvent_Item2Slot(iitem);
			}
			SendEvent_ActivateSlot(iitem);
			//SendEvent_ActivateSlot2(slot, pOurInvOwner->object_id());
			return true;
		}
		if (slot == DETECTOR_SLOT)
		{ }
		return false;
	//}
}

bool CUIInventoryWnd::ToSlot(CUICellItem* itm, bool force_place)
{
	CInventoryOwner* m_pActorInvOwner = smart_cast<CInventoryOwner*>(Level().CurrentViewEntity());

	CUIDragDropListEx*	old_owner = itm->OwnerList();
	PIItem	iitem = (PIItem)itm->m_pData;
	u32 _slot = iitem->GetSlot();

	bool b_own_item = (iitem->parent_id() == m_pActorInvOwner->object_id());
	if (_slot == HELMET_SLOT)
	{
		CCustomOutfit* pOutfit = m_pActorInvOwner->GetOutfit();
		if (pOutfit && !pOutfit->bIsHelmetAvaliable)
			return false;
	}

	if (m_pActorInvOwner->inventory().CanPutInSlot(iitem))
	{
		CUIDragDropListEx* new_owner = GetSlotList(_slot);

		if (_slot == GRENADE_SLOT || !new_owner)
			{
				return true; //fake, sorry (((
			}

		if (_slot == OUTFIT_SLOT)
		{
			CCustomOutfit* pOutfit = smart_cast<CCustomOutfit*>(iitem);
			if (pOutfit && !pOutfit->bIsHelmetAvaliable)
			{
				CUIDragDropListEx* helmet_list = GetSlotList(HELMET_SLOT);
				if (helmet_list->ItemsCount() == 1)
				{
					CUICellItem* helmet_cell = helmet_list->GetItemIdx(0);
					ToBag(helmet_cell, false);
				}
			}
		}

		bool result = (!b_own_item) || m_pActorInvOwner->inventory().Slot(iitem);
		VERIFY(result);

		CUICellItem* i = old_owner->RemoveItem(itm, (old_owner == new_owner));

		new_owner->SetItem(i);

		SendEvent_Item2Slot(iitem/*, m_pActorInvOwner->object_id()*/);

		SendEvent_ActivateSlot(iitem);
	//	SendEvent_ActivateSlot2(_slot, m_pActorInvOwner->object_id());

		//ColorizeItem						( itm, false );
		if (_slot == OUTFIT_SLOT)
		{
			MoveArtefactsToBag();
		}

		return								true;
	}
	else
	{ // in case slot is busy
		if (!force_place || _slot == NO_ACTIVE_SLOT) return false;
		if (m_pActorInvOwner->inventory().m_slots[_slot].m_bPersistent && _slot != DETECTOR_SLOT)
		{
			return false;
		}

		PIItem	_iitem = m_pActorInvOwner->inventory().m_slots[_slot].m_pIItem;
		CUIDragDropListEx* slot_list = GetSlotList(_slot);
		VERIFY(slot_list->ItemsCount() == 1);

		CUICellItem* slot_cell = slot_list->GetItemIdx(0);
		VERIFY(slot_cell && ((PIItem)slot_cell->m_pData) == _iitem);

		bool result = ToBag(slot_cell, false);
		VERIFY(result);

		result = ToSlot(itm, false);
		if (b_own_item && result && _slot == DETECTOR_SLOT)
		{
			CCustomDetector* det = smart_cast<CCustomDetector*>(iitem);
			det->ToggleDetector(g_player_hud->attached_item(0) != NULL);
		}
		return result;
	}
}

bool CUIInventoryWnd::ToBag(CUICellItem* itm, bool b_use_cursor_pos)
{
	PIItem	iitem						= (PIItem)itm->m_pData;

	if(GetInventory()->CanPutInRuck(iitem))
	{
		CUIDragDropListEx*	old_owner		= itm->OwnerList();
		CUIDragDropListEx*	new_owner		= NULL;
		if(b_use_cursor_pos){
				new_owner					= CUIDragDropListEx::m_drag_item->BackList();
				VERIFY						(new_owner==m_pUIBagList);
		}else
				new_owner					= m_pUIBagList;


		bool result							= GetInventory()->Ruck(iitem);
		VERIFY								(result);
		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );
		
		result = new_owner->CanSetItem(i);
		if (result || new_owner->IsAutoGrow())
		{

			if (b_use_cursor_pos)
			{
				new_owner->SetItem(i, old_owner->GetDragItemPosition());
			}
			else
			{
				new_owner->SetItem(i);
			}
			SendEvent_Item2Ruck(iitem);
		}
		else
		{
			NET_Packet					P;
			iitem->object().u_EventGen(P, GE_OWNERSHIP_REJECT, iitem->object().H_Parent()->ID());
			P.w_u16(u16(iitem->object().ID()));
			iitem->object().u_EventSend(P);

			CInventoryOwner* m_pActorInvOwner = smart_cast<CInventoryOwner*>(Level().CurrentViewEntity());
			if (m_pActorInvOwner)
			{
				m_pQuickSlot->ReloadReferences(m_pActorInvOwner);
			}
		}
		return true;
	}

	return false;
}

bool CUIInventoryWnd::ToBelt(CUICellItem* itm, bool b_use_cursor_pos)
{
	PIItem	iitem						= (PIItem)itm->m_pData;

	if(GetInventory()->CanPutInBelt(iitem))
	{
		CUIDragDropListEx*	old_owner		= itm->OwnerList();
		CUIDragDropListEx*	new_owner		= NULL;
		if(b_use_cursor_pos){
				new_owner					= CUIDragDropListEx::m_drag_item->BackList();
				VERIFY						(new_owner==m_pUIBeltList);
		}else
				new_owner					= m_pUIBeltList;

		bool result							= GetInventory()->Belt(iitem);
		VERIFY								(result);
		CUICellItem* i						= old_owner->RemoveItem(itm, (old_owner==new_owner) );
		
	//.	UIBeltList.RearrangeItems();
		if(b_use_cursor_pos)
			new_owner->SetItem				(i,old_owner->GetDragItemPosition());
		else
			new_owner->SetItem				(i);

		SendEvent_Item2Belt					(iitem);
		return								true;
	}
	return									false;
}

void CUIInventoryWnd::AddItemToBag(PIItem pItem)
{
	CUICellItem* itm						= create_cell_item(pItem);
	m_pUIBagList->SetItem					(itm);
}

bool CUIInventoryWnd::OnItemStartDrag(CUICellItem* itm)
{
	InfoCurItem(NULL);
	return false; //default behaviour
}

bool CUIInventoryWnd::OnItemSelected(CUICellItem* itm)
{
	SetCurrentItem(itm);
	InfoCurItem(NULL);
	m_item_info_view = false;
	return				false;
}

bool CUIInventoryWnd::OnItemDrop(CUICellItem* itm)
{
	InfoCurItem(NULL);
	CUIDragDropListEx*	old_owner = itm->OwnerList();
	CUIDragDropListEx*	new_owner = CUIDragDropListEx::m_drag_item->BackList();
	if (old_owner == new_owner || !old_owner || !new_owner)
		return false;

	EListType t_new = GetType(new_owner);
	EListType t_old = GetType(old_owner);
	if (t_new == t_old && t_new != iwSlot) return true;

	switch(t_new)
	{
	case iTrashSlot:
	{
		if (CurrentIItem()->IsQuestItem())
			return true;

		if (t_old == iQuickSlot)
		{
		old_owner->RemoveItem(itm, false);
		return true;
		}

		SendEvent_Item_Drop(CurrentIItem());
		SetCurrentItem(NULL);
	}break;
		case iwSlot:{
			uint32 slot = CurrentIItem()->GetSlot();
			if (GetSlotList(slot) == new_owner && t_new != t_old)
				ToSlot(itm, true);

			else if (new_owner == m_pUIPistolList && slot == RIFLE_SLOT)
			{
				CurrentIItem()->SetSlot(PISTOL_SLOT);
				ToSlot(itm, true);
			}
			else if (new_owner == m_pUIAutomaticList && slot == PISTOL_SLOT)
			{
				CurrentIItem()->SetSlot(RIFLE_SLOT);
				ToSlot(itm, true);
			}
		
		}break;
		case iwBag:{
			ToBag	(itm, true);
		}break;
		case iwBelt:{
			ToBelt	(itm, true);
		}break;
		case iQuickSlot:
		{
			ToQuickSlot(itm);
		}break;

	};

	DropItem				(CurrentIItem(), new_owner);
	UpdateConditionProgressBars();
	return true;
}

bool CUIInventoryWnd::OnItemDbClick(CUICellItem* itm) 
{
	SetCurrentItem(itm);
	InfoCurItem(NULL);
//	if(TryUseItem((PIItem)itm->m_pData))		
//		return true;

	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	EListType t_old						= GetType(old_owner);

	switch(t_old)
		{
		case iwSlot:
		{
			ToBag	(itm, false);
		}break;

		case iwBag:
		{

			if ( TryUseItem(itm))
			{
				break;
			}
			if (TryActiveSlot(itm))
			{
				break;
			}

			PIItem item = (PIItem)itm->m_pData;
			// ??? ??????? ????? ???????? ????????? ????. Real Wolf.
			uint32 slot = item->GetSlot();
			if (slot == PISTOL_SLOT || slot == RIFLE_SLOT)
			{
				uint32 double_slot = slot == PISTOL_SLOT ? RIFLE_SLOT : PISTOL_SLOT;
				if (GetInventory()->m_slots[slot].m_pIItem)
					if (!GetInventory()->m_slots[double_slot].m_pIItem)
						item->SetSlot(double_slot);
			}

			if (!ToSlot(itm, false))
			{
				if (!ToBelt(itm, false))
					ToSlot(itm, true);
			}break;
		}

		case iwBelt:
		{
			ToBag	(itm, false);
		}break;
		case iQuickSlot:
		{
			ToQuickSlot(itm);
		}break;

	};

	UpdateConditionProgressBars();
	return true;
}


bool CUIInventoryWnd::OnItemRButtonClick(CUICellItem* itm)
{
	SetCurrentItem(itm);
	InfoCurItem(NULL);
	ActivatePropertiesBox();
    m_item_info_view = false;
	return false;
}

bool CUIInventoryWnd::OnItemFocusReceive(CUICellItem* itm)
{
	InfoCurItem(NULL);
	m_item_info_view = true;
	itm->m_selected = true;
	set_highlight_item(itm);
	return true;
}

bool CUIInventoryWnd::OnItemFocusLost(CUICellItem* itm)
{
	if (itm)
	{
		itm->m_selected = false;
	}
	InfoCurItem(NULL);
	clear_highlight_lists();

	return true;
}

bool CUIInventoryWnd::OnItemFocusedUpdate(CUICellItem* itm)
{
	if (itm)
	{
		itm->m_selected = true;
		if (m_highlight_clear)
		{
			set_highlight_item(itm);
		}
	}
	VERIFY(m_ItemInfo);
	if (Device.dwTimeGlobal < itm->FocusReceiveTime() + m_ItemInfo->delay)
	{
		return true; //false
	}
	if (CUIDragDropListEx::m_drag_item || UIPropertiesBox.IsShown() || !m_item_info_view)
	{
		return true;
	}
	InfoCurItem(itm);
	return true;
}

CUIDragDropListEx* CUIInventoryWnd::GetSlotList(u32 slot_idx)
{
	if(slot_idx == NO_ACTIVE_SLOT /*|| GetInventory()->m_slots[slot_idx].m_bPersistent*/)	
	{
		return NULL;
	}
	switch (slot_idx)
	{
		case PISTOL_SLOT:
			return m_pUIPistolList;
			break;
		case RIFLE_SLOT:
			return m_pUIAutomaticList;
			break;
		case DETECTOR_SLOT:
			return m_pUIDetectorList;
			break;
		case OUTFIT_SLOT:
			return m_pUIOutfitList;
			break;
		case HELMET_SLOT:
			return m_pUIHelmetList;
			break;

	};
	return NULL; 
}

class CUITrashIcon :public ICustomDrawDragItem
{
	CUIStatic			m_icon;
public:
	CUITrashIcon()
	{
		m_icon.SetWndSize(Fvector2().set(29.0f*UI().get_current_kx(), 36.0f));
		m_icon.SetStretchTexture(true);
		//		m_icon.SetAlignment		(waCenter);
		m_icon.InitTexture("ui_inGame2_inv_trash");
	}
	virtual void		OnDraw(CUIDragItem* drag_item)
	{
		Fvector2 pos = drag_item->GetWndPos();
		Fvector2 icon_sz = m_icon.GetWndSize();
		Fvector2 drag_sz = drag_item->GetWndSize();

		pos.x -= icon_sz.x;
		pos.y += drag_sz.y;

		m_icon.SetWndPos(pos);
		//		m_icon.SetWndSize(sz);
		m_icon.Draw();
	}

};

void CUIInventoryWnd::OnDragItemOnTrash(CUIDragItem* item, bool b_receive)
{
	if (b_receive && !CurrentIItem()->IsQuestItem())
		item->SetCustomDraw(xr_new<CUITrashIcon>());
	else
		item->SetCustomDraw(NULL);
}

void CUIInventoryWnd::UpdateOutfit()
{
	CInventoryOwner* m_pActorInvOwner = smart_cast<CInventoryOwner*>(Level().CurrentViewEntity());

	for (u8 i = 0; i < e_af_count; ++i)
	{
		m_belt_list_over[i]->SetVisible(true);
	}
	u32 af_count = m_pActorInvOwner->inventory().BeltWidth();
	VERIFY(0 <= af_count && af_count <= 5);


	VERIFY(m_pUIBeltList);
	PIItem         ii_outfit = m_pActorInvOwner->inventory().m_slots[OUTFIT_SLOT].m_pIItem;
	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(ii_outfit);
	if (!ii_outfit || !outfit)
	{
		MoveArtefactsToBag();
		return;
	}

	Ivector2 afc;
	afc.x = af_count;//1;
	afc.y = 1;//af_count;

	m_pUIBeltList->SetCellsCapacity(afc);
	for (u8 i = 0; i < af_count; ++i)
	{
		m_belt_list_over[i]->SetVisible(false);
	}
}

void CUIInventoryWnd::MoveArtefactsToBag()
{
	while (m_pUIBeltList->ItemsCount())
	{
		CUICellItem* ci = m_pUIBeltList->GetItemIdx(0);
		VERIFY(ci && ci->m_pData);
		ToBag(ci, false);
	}//for i
	m_pUIBeltList->ClearAll(true);
}

void CUIInventoryWnd::UpdateButtonsLayout()
{
	string32 tmp;
	LPCSTR str = CStringTable().translate("quick_use_str_1").c_str();
	strncpy_s(tmp, sizeof(tmp), str, 3);
	if (tmp[2] == ',')
		tmp[1] = '\0';
	m_QuickSlot1->SetTextST(tmp);

	str = CStringTable().translate("quick_use_str_2").c_str();
	strncpy_s(tmp, sizeof(tmp), str, 3);
	if (tmp[2] == ',')
		tmp[1] = '\0';
	m_QuickSlot2->SetTextST(tmp);

	str = CStringTable().translate("quick_use_str_3").c_str();
	strncpy_s(tmp, sizeof(tmp), str, 3);
	if (tmp[2] == ',')
		tmp[1] = '\0';
	m_QuickSlot3->SetTextST(tmp);

	str = CStringTable().translate("quick_use_str_4").c_str();
	strncpy_s(tmp, sizeof(tmp), str, 3);
	if (tmp[2] == ',')
		tmp[1] = '\0';
	m_QuickSlot4->SetTextST(tmp);

	UpdateConditionProgressBars();
}

void CUIInventoryWnd::UpdateConditionProgressBars()
{
	CInventoryOwner* m_pActorInvOwner = smart_cast<CInventoryOwner*>(Level().CurrentViewEntity());
	PIItem itm = m_pActorInvOwner->inventory().ItemFromSlot(PISTOL_SLOT);
	if (itm)
	{
		m_WeaponSlot1_progress->SetProgressPos(iCeil(itm->GetCondition()*15.0f) / 15.0f);
	}
	else
		m_WeaponSlot1_progress->SetProgressPos(0);

	itm = m_pActorInvOwner->inventory().ItemFromSlot(RIFLE_SLOT);
	if (itm)
		m_WeaponSlot2_progress->SetProgressPos(iCeil(itm->GetCondition()*15.0f) / 15.0f);
	else
		m_WeaponSlot2_progress->SetProgressPos(0);

	itm = m_pActorInvOwner->inventory().ItemFromSlot(OUTFIT_SLOT);
	if (itm)
		m_Outfit_progress->SetProgressPos(iCeil(itm->GetCondition()*15.0f) / 15.0f);
	else
		m_Outfit_progress->SetProgressPos(0);

	itm = m_pActorInvOwner->inventory().ItemFromSlot(HELMET_SLOT);
	if (itm)
	m_Helmet_progress->SetProgressPos(iCeil(itm->GetCondition()*15.0f) / 15.0f);
	else
	m_Helmet_progress->SetProgressPos(0);
	

	//Highlight 'equipped' items in actor bag
	CUIDragDropListEx* slot_list = m_pUIBagList;
	u32 const cnt = slot_list->ItemsCount();
	for (u32 i = 0; i < cnt; ++i)
	{
		CUICellItem* ci = slot_list->GetItemIdx(i);
		PIItem item = (PIItem)ci->m_pData;
		if (!item)
			continue;

		if (item->m_highlight_equipped && item->m_pCurrentInventory && item->m_pCurrentInventory->ItemFromSlot(item->GetSlot()) == item)
			ci->m_select_equipped = true;
		else
			ci->m_select_equipped = false;
	}
}



#include "../eatable_item_object.h"
bool CUIInventoryWnd::ToQuickSlot(CUICellItem* itm)
{
	PIItem iitem = (PIItem)itm->m_pData;
	CEatableItemObject* eat_item = smart_cast<CEatableItemObject*>(iitem);
	if (!eat_item)
		return false;

	//Update: Should not be necessary now
	//Alundaio: Fix deep recursion if placing icon greater then col/row set in actor_menu.xml
	 	/*Ivector2 iWH = iitem->GetInvGridRect().rb;
	if (iWH.x > 1 || iWH.y > 1)
	return false; */
	//Alundaio: END

	u8 slot_idx = u8(m_pQuickSlot->PickCell(GetUICursor()->GetCursorPosition()).x);
	if (slot_idx == 255)
		return false;

	if (!m_pQuickSlot->CanSetItem(itm))
		return false;

	m_pQuickSlot->SetItem(create_cell_item(iitem), GetUICursor()->GetCursorPosition());
	strcpy_s(ACTOR_DEFS::g_quick_use_slots[slot_idx], iitem->m_section_id.c_str());
	return true;
}

void CUIInventoryWnd::ClearAllLists()
{
	m_pUIBagList->ClearAll					(true);
	m_pUIBeltList->ClearAll					(true);
	m_pUIOutfitList->ClearAll				(true);
	m_pUIHelmetList->ClearAll				(true);
	m_pUIPistolList->ClearAll				(true);
	m_pUIAutomaticList->ClearAll			(true);
	m_pUIDetectorList->ClearAll				(true);
	m_pQuickSlot->ClearAll(true);
}

