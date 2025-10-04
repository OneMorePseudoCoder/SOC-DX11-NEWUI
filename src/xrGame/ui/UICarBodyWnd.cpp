#include "pch_script.h"
#include "UICarBodyWnd.h"
#include "xrUIXmlParser.h"
#include "UIXmlInit.h"
#include "../HUDManager.h"
#include "../level.h"
#include "UICharacterInfo.h"
#include "UIDragDropListEx.h"
#include "UIFrameWindow.h"
#include "UIItemInfo.h"
#include "UIPropertiesBox.h"
#include "../ai/monsters/BaseMonster/base_monster.h"
#include "../inventory.h"
#include "UIInventoryUtilities.h"
#include "UICellItem.h"
#include "UICellItemFactory.h"
#include "../WeaponMagazined.h"
#include "../Actor.h"
#include "../eatable_item.h"
#include "../alife_registry_wrappers.h"
#include "UI3tButton.h"
#include "UIListBoxItem.h"
#include "../InventoryBox.h"
#include "../game_object_space.h"
#include "../script_callback_ex.h"
#include "../script_game_object.h"
#include "../BottleItem.h"
#include "UIHelper.h"
#include "UIInventoryWnd.h"
#include "UIActorStateInfo.h"
#include "../string_table.h"
#include "../CustomOutfit.h"
#include "../Artifact.h"
#include "../silencer.h"
#include "../scope.h"
#include "../grenadelauncher.h"
#include "../WeaponMagazinedWGrenade.h"


void move_item(u16 from_id, u16 to_id, u16 what_id);

CUICarBodyWnd::CUICarBodyWnd()
{
	m_iCurrentActiveSlot = NO_ACTIVE_SLOT;
	m_pInventoryBox = NULL;
	Init();
	Hide();
	m_b_need_update = false;
}

CUICarBodyWnd::~CUICarBodyWnd()
{
	m_pUIOurBagList->ClearAll(true);
	m_pUIOthersBagList->ClearAll(true);
	xr_delete(m_ItemInfo);
}	

void CUICarBodyWnd::Init()
{
	CUIXml							uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, "carbody_new.xml");

	CUIXmlInit						xml_init;

	xml_init.InitWindow(uiXml, "main", 0, this);



	m_pUICharacterInfoLeft = xr_new<CUICharacterInfo>();
	m_pUICharacterInfoLeft->SetAutoDelete(true);
	AttachChild(m_pUICharacterInfoLeft);
	m_pUICharacterInfoLeft->InitCharacterInfo(&uiXml, "actor_ch_info");

	m_pUICharacterInfoRight = xr_new<CUICharacterInfo>();
	m_pUICharacterInfoRight->SetAutoDelete(true);
	AttachChild(m_pUICharacterInfoRight);
	m_pUICharacterInfoRight->InitCharacterInfo(&uiXml, "partner_ch_info");


	m_pUIOurBagList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_list_our", this);
	m_pUIOthersBagList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_list_other", this);

	BindDragDropListEnents(m_pUIOurBagList);
	BindDragDropListEnents(m_pUIOthersBagList);


	xml_init.InitAutoStatic(uiXml, "auto_static", this);

	m_pUIPropertiesBox = xr_new<CUIPropertiesBox>(); m_pUIPropertiesBox->SetAutoDelete(true);
	AttachChild(m_pUIPropertiesBox);
	m_pUIPropertiesBox->InitPropertiesBox(Fvector2().set(0, 0), Fvector2().set(300, 300));
	m_pUIPropertiesBox->Hide();

	m_highlight_clear = true;
	m_item_info_view = false;


	m_ActorMoney = UIHelper::CreateTextWnd(uiXml, "actor_money_static", this);
	m_ActorBottomInfo = UIHelper::CreateTextWnd(uiXml, "actor_weight_caption", this);
	m_ActorWeight = UIHelper::CreateTextWnd(uiXml, "actor_weight", this);
	m_ActorWeightMax = UIHelper::CreateTextWnd(uiXml, "actor_weight_max", this);
	m_ActorBottomInfo->AdjustWidthToText();

	m_PartnerBottomInfo = UIHelper::CreateStatic(uiXml, "partner_weight_caption", this);
	m_PartnerWeight = UIHelper::CreateTextWnd(uiXml, "partner_weight", this);
	m_PartnerBottomInfo->AdjustWidthToText();
	m_PartnerWeight_end_x = m_PartnerWeight->GetWndPos().x;

	m_pUITakeAll = UIHelper::Create3tButton(uiXml, "takeall_button", this);
	UIExitButton = UIHelper::Create3tButton(uiXml, "exit_button", this);

	m_ActorStateInfo = xr_new<ui_actor_state_wnd>();
	m_ActorStateInfo->init_from_xml(uiXml, "actor_state_info");
	m_ActorStateInfo->SetAutoDelete(true);
	AttachChild(m_ActorStateInfo);


	m_ItemInfo = xr_new<CUIItemInfo>();
	m_ItemInfo->InitItemInfo("actor_menu_item.xml");

	SetCurrentItem(NULL);
}

void CUICarBodyWnd::InitCarBody(CInventoryOwner* pOur, CInventoryBox* pInvBox)
{
	m_pOurObject = pOur;
	m_pOthersObject = NULL;
	m_pInventoryBox = pInvBox;
	m_pInventoryBox->m_in_use = true;

	u16 our_id = smart_cast<CGameObject*>(m_pOurObject)->ID();
	m_pUICharacterInfoLeft->InitCharacter(our_id);
//	m_pUIOthersIcon->Show(false);
	m_PartnerBottomInfo->Show(false);
	m_pUICharacterInfoRight->ClearInfo();
	m_pUIPropertiesBox->Hide();
	EnableAll();
	UpdateLists();

}

void CUICarBodyWnd::InitCarBody(CInventoryOwner* pOur, CInventoryOwner* pOthers)
{

	m_pOurObject = pOur;
	m_pOthersObject = pOthers;
	m_pInventoryBox = NULL;

	u16 our_id = smart_cast<CGameObject*>(m_pOurObject)->ID();
	u16 other_id = smart_cast<CGameObject*>(m_pOthersObject)->ID();

	m_pUICharacterInfoLeft->InitCharacter(our_id);
//	m_pUIOthersIcon->Show(true);
	m_PartnerBottomInfo->Show(true);
	CBaseMonster *monster = NULL;
	if (m_pOthersObject) {
		monster = smart_cast<CBaseMonster *>(m_pOthersObject);
		if (monster || m_pOthersObject->use_simplified_visual())
		{
			m_pUICharacterInfoRight->ClearInfo();
			if (monster)
			{
				shared_str monster_tex_name = pSettings->r_string(monster->cNameSect(), "icon");
				m_pUICharacterInfoRight->UIIcon().InitTexture(monster_tex_name.c_str());
				m_pUICharacterInfoRight->UIIcon().SetStretchTexture(true);
			}
		}
		else
		{
			m_pUICharacterInfoRight->InitCharacter(other_id);
		}
	}

	m_pUIPropertiesBox->Hide();
	EnableAll();
	UpdateLists();

	if (!monster){
		CInfoPortionWrapper	*known_info_registry = xr_new<CInfoPortionWrapper>();
		known_info_registry->registry().init(other_id);
		KNOWN_INFO_VECTOR& known_info = known_info_registry->registry().objects();

		KNOWN_INFO_VECTOR_IT it = known_info.begin();
		for (int i = 0; it != known_info.end(); ++it, ++i){
			(*it).info_id;
			NET_Packet		P;
			CGameObject::u_EventGen(P, GE_INFO_TRANSFER, our_id);
			P.w_u16(0);//not used
			P.w_stringZ((*it).info_id);			//сообщение
			P.w_u8(1);						//добавление сообщения
			CGameObject::u_EventSend(P);
		}
		known_info.clear();
		xr_delete(known_info_registry);
	}
}

void CUICarBodyWnd::UpdateLists_delayed()
{
	m_b_need_update = true;
}

#include "UIInventoryUtilities.h"

void CUICarBodyWnd::Hide()
{
	InventoryUtilities::SendInfoToActor("ui_car_body_hide");
	m_pUIOurBagList->ClearAll(true);
	m_pUIOthersBagList->ClearAll(true);
	m_ActorStateInfo->Show(false);
	inherited::Hide();
	if (m_pInventoryBox)
		m_pInventoryBox->m_in_use = false;

}

void CUICarBodyWnd::UpdateLists()
{
	TIItemContainer								ruck_list;
	m_pUIOurBagList->ClearAll(true);
	m_pUIOthersBagList->ClearAll(true);

	ruck_list.clear();
	m_pOurObject->inventory().AddAvailableItems(ruck_list, true);
	std::sort(ruck_list.begin(), ruck_list.end(), InventoryUtilities::GreaterRoomInRuck);

	//Наш рюкзак
	TIItemContainer::iterator it;
	for (it = ruck_list.begin(); ruck_list.end() != it; ++it)
	{
		CUICellItem* itm = create_cell_item(*it);
		ColorizeItem(itm);
		m_pUIOurBagList->SetItem(itm);
	}


	ruck_list.clear();
	if (m_pOthersObject)
		m_pOthersObject->inventory().AddAvailableItems(ruck_list, false);
	else
		m_pInventoryBox->AddAvailableItems(ruck_list);

	std::sort(ruck_list.begin(), ruck_list.end(), InventoryUtilities::GreaterRoomInRuck);

	//Чужой рюкзак
	for (it = ruck_list.begin(); ruck_list.end() != it; ++it)
	{
		CUICellItem* itm = create_cell_item(*it);
		m_pUIOthersBagList->SetItem(itm);
	}

//	InventoryUtilities::UpdateWeight(*m_pUIOurBagWnd);
	m_b_need_update = false;
}

void CUICarBodyWnd::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if (UIExitButton == pWnd && BUTTON_CLICKED == msg)
	{
		GetHolder()->StartStopMenu(this, true);
	}
	if (BUTTON_CLICKED == msg && m_pUITakeAll == pWnd)
	{
		TakeAll();
	}
	else if (pWnd == m_pUIPropertiesBox &&	msg == PROPERTY_CLICKED)
	{

		if (m_pUIPropertiesBox->GetClickedItem())
		{
			switch (m_pUIPropertiesBox->GetClickedItem()->GetTAG())
			{
			case INVENTORY_EAT_ACTION:	//съесть объект
				EatItem();
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
	}

	inherited::SendMessage(pWnd, msg, pData);
}

void CUICarBodyWnd::DetachAddon(const char* addon_name)
{
	PlaySnd(eInvDetachAddon);
	if (OnClient())
	{
		NET_Packet								P;
		CurrentIItem()->object().u_EventGen(P, GE_ADDON_DETACH, CurrentIItem()->object().ID());
		P.w_stringZ(addon_name);
		CurrentIItem()->object().u_EventSend(P);
	};
	CurrentIItem()->Detach(addon_name, true);

	//спрятать вещь из активного слота в инвентарь на время вызова менюшки
	CActor *pActor = smart_cast<CActor*>(Level().CurrentEntity());
	if (pActor && CurrentIItem() == pActor->inventory().ActiveItem())
	{
		m_iCurrentActiveSlot = pActor->inventory().GetActiveSlot();
		pActor->inventory().Activate(NO_ACTIVE_SLOT);
	}
}

void CUICarBodyWnd::PlaySnd(eInventorySndAction a)
{
	if (sounds[a]._handle())
		sounds[a].play(NULL, sm_2D);
}

void CUICarBodyWnd::Draw()
{
	inherited::Draw();
	m_ItemInfo->Draw();
}


void CUICarBodyWnd::Update()
{
	if (m_b_need_update || m_pOurObject->inventory().ModifyFrame() == Device.dwFrame || (m_pOthersObject&&m_pOthersObject->inventory().ModifyFrame() == Device.dwFrame))


		UpdateLists();
	    m_ActorStateInfo->UpdateActorInfo(m_pOurObject);

	if (m_pOthersObject && (smart_cast<CGameObject*>(m_pOurObject))->Position().distance_to((smart_cast<CGameObject*>(m_pOthersObject))->Position()) > 3.0f)
	{
		GetHolder()->StartStopMenu(this, true);
	}


		if (IsGameTypeSingle())
		{
			string64 buf;
			sprintf_s(buf, "%d RU", m_pOurObject->get_money());
			m_ActorMoney->SetText(buf);
		}

		InventoryUtilities::UpdateWeightStr(*m_ActorWeight, *m_ActorWeightMax, m_pOurObject);

		m_ActorWeight->AdjustWidthToText();
		m_ActorWeightMax->AdjustWidthToText();
		m_ActorBottomInfo->AdjustWidthToText();

		Fvector2 pos = m_ActorWeight->GetWndPos();
		pos.x = m_ActorWeightMax->GetWndPos().x - m_ActorWeight->GetWndSize().x - 5.0f;
		m_ActorWeight->SetWndPos(pos);
		pos.x = pos.x - m_ActorBottomInfo->GetWndSize().x - 5.0f;
		m_ActorBottomInfo->SetWndPos(pos);

		string64 buf;

		LPCSTR kg_str = CStringTable().translate("Кг").c_str();
		float total = CalcItemsWeight(m_pUIOthersBagList);
		sprintf_s(buf, "%.1f %s", total, kg_str);
		m_PartnerWeight->SetText(buf);
		m_PartnerWeight->AdjustWidthToText();

		pos = m_PartnerWeight->GetWndPos();
		pos.x = m_PartnerWeight_end_x - m_PartnerWeight->GetWndSize().x - 5.0f;
		m_PartnerWeight->SetWndPos(pos);
		pos.x = pos.x - m_PartnerBottomInfo->GetWndSize().x - 5.0f;
		m_PartnerBottomInfo->SetWndPos(pos);

		m_ItemInfo->Update();

		inherited::Update();
}


float CUICarBodyWnd::CalcItemsWeight(CUIDragDropListEx* pList)
{
	float res = 0.0f;

	for (u32 i = 0; i < pList->ItemsCount(); ++i)
	{
		CUICellItem* itm = pList->GetItemIdx(i);
		PIItem	iitem = (PIItem)itm->m_pData;
		res += iitem->Weight();
		for (u32 j = 0; j < itm->ChildsCount(); ++j)
		{
			PIItem	jitem = (PIItem)itm->Child(j)->m_pData;
			res += jitem->Weight();
		}
	}
	return res;
}


void CUICarBodyWnd::Show()
{
	InventoryUtilities::SendInfoToActor("ui_car_body");
	inherited::Show();
	m_ActorStateInfo->Show(true);
	m_ActorStateInfo->UpdateActorInfo(m_pOurObject);
}

void CUICarBodyWnd::DisableAll()
{
//	m_pUIOurBagWnd->Enable(false);
//	m_pUIOthersBagWnd->Enable(false);
}

void CUICarBodyWnd::EnableAll()
{
//	m_pUIOurBagWnd->Enable(true);
//	m_pUIOthersBagWnd->Enable(true);
}

CUICellItem* CUICarBodyWnd::CurrentItem()
{
	return m_pCurrentCellItem;
}

PIItem CUICarBodyWnd::CurrentIItem()
{
	return	(m_pCurrentCellItem) ? (PIItem)m_pCurrentCellItem->m_pData : NULL;
}

void CUICarBodyWnd::SetCurrentItem(CUICellItem* itm)
{
	m_pCurrentCellItem = itm;
	if (!itm)
	{
		InfoCurItem(NULL);
	}
}

void CUICarBodyWnd::InfoCurItem(CUICellItem* cell_item)
{
	if (!cell_item)
	{
		m_ItemInfo->InitItem(NULL);
		return;
	}
	PIItem current_item = (PIItem)cell_item->m_pData;

		PIItem compare_item = NULL;
		u32    compare_slot = current_item->GetSlot();
		if (compare_slot != NO_ACTIVE_SLOT)
		{
			compare_item = m_pOurObject->inventory().m_slots[compare_slot].m_pIItem;
		}

		m_ItemInfo->InitItem(cell_item, compare_item, u32(-1));
		float dx_pos = GetWndRect().left;
		fit_in_rect(m_ItemInfo, Frect().set(0.0f, 0.0f, UI_BASE_WIDTH - dx_pos, UI_BASE_HEIGHT), 10.0f, dx_pos);
	}


void CUICarBodyWnd::TakeAll()
{
	u32 cnt = m_pUIOthersBagList->ItemsCount();
	u16 tmp_id = 0;
	if (m_pInventoryBox){
		tmp_id = (smart_cast<CGameObject*>(m_pOurObject))->ID();
	}

	for (u32 i = 0; i<cnt; ++i)
	{
		CUICellItem*	ci = m_pUIOthersBagList->GetItemIdx(i);
		for (u32 j = 0; j<ci->ChildsCount(); ++j)
		{
			PIItem _itm = (PIItem)(ci->Child(j)->m_pData);
			if (m_pOthersObject)
				TransferItem(_itm, m_pOthersObject, m_pOurObject, false);
			else{
				move_item(m_pInventoryBox->ID(), tmp_id, _itm->object().ID());
				//.				Actor()->callback(GameObject::eInvBoxItemTake)( m_pInventoryBox->lua_game_object(), _itm->object().lua_game_object() );
			}

		}
		PIItem itm = (PIItem)(ci->m_pData);
		if (m_pOthersObject)
			TransferItem(itm, m_pOthersObject, m_pOurObject, false);
		else{
			move_item(m_pInventoryBox->ID(), tmp_id, itm->object().ID());
			//.			Actor()->callback(GameObject::eInvBoxItemTake)(m_pInventoryBox->lua_game_object(), itm->object().lua_game_object() );
		}

	}
}


#include "../xr_level_controller.h"

bool CUICarBodyWnd::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	InfoCurItem(NULL);
	if (inherited::OnKeyboardAction(dik, keyboard_action))return true;

	if (keyboard_action == WINDOW_KEY_PRESSED && is_binded(kUSE, dik))
	{
		GetHolder()->StartStopMenu(this, true);
		return true;
		SetCurrentItem(NULL);
	}
	return false;
}

#include "../Medkit.h"
#include "../Antirad.h"


void CUICarBodyWnd::ActivatePropertiesBox()
{
	if (m_pInventoryBox)	return;

	m_pUIPropertiesBox->RemoveAll();

	CEatableItem*			pEatableItem = smart_cast<CEatableItem*>			(CurrentIItem());
	CMedkit*				pMedkit = smart_cast<CMedkit*>						(CurrentIItem());
	CAntirad*				pAntirad = smart_cast<CAntirad*>					(CurrentIItem());
	CBottleItem*			pBottleItem = smart_cast<CBottleItem*>				(CurrentIItem());
	CWeapon*				pWeapon = smart_cast<CWeapon*>						(CurrentIItem());
	CScope*					pScope = smart_cast<CScope*>						(CurrentIItem());
	CSilencer*				pSilencer = smart_cast<CSilencer*>					(CurrentIItem());
	CGrenadeLauncher*		pGrenadeLauncher = smart_cast<CGrenadeLauncher*>	(CurrentIItem());
	bool					b_show = false;

	//отсоединение аддонов от вещи
	if (pWeapon)
	{
		if (pWeapon->GrenadeLauncherAttachable() && pWeapon->IsGrenadeLauncherAttached())
		{
			m_pUIPropertiesBox->AddItem("st_detach_gl", NULL, INVENTORY_DETACH_GRENADE_LAUNCHER_ADDON);
			b_show = true;
		}
		if (pWeapon->ScopeAttachable() && pWeapon->IsScopeAttached())
		{
			m_pUIPropertiesBox->AddItem("st_detach_scope", NULL, INVENTORY_DETACH_SCOPE_ADDON);
			b_show = true;
		}
		if (pWeapon->SilencerAttachable() && pWeapon->IsSilencerAttached())
		{
			m_pUIPropertiesBox->AddItem("st_detach_silencer", NULL, INVENTORY_DETACH_SILENCER_ADDON);
			b_show = true;
		}
		if (smart_cast<CWeaponMagazined*>(pWeapon) && IsGameTypeSingle())
		{
			bool b = (0 != pWeapon->GetAmmoElapsed());

			if (!b)
			{
				CUICellItem * itm = CurrentItem();
				for (u32 i = 0; i<itm->ChildsCount(); ++i)
				{
					pWeapon = smart_cast<CWeaponMagazined*>((CWeapon*)itm->Child(i)->m_pData);
					if (pWeapon->GetAmmoElapsed())
					{
						b = true;
						break;
					}
				}
			}

			if (b){
				m_pUIPropertiesBox->AddItem("st_unload_magazine", NULL, INVENTORY_UNLOAD_MAGAZINE);
				b_show = true;
			}
		}
	}

	LPCSTR _action = NULL;

	if (pMedkit || pAntirad)
	{
		_action = "st_use";
	}
	else if (pEatableItem)
	{
		if (pBottleItem)
			_action = "st_drink";
		else
			_action = "st_eat";
	}

	if (_action)
	{
		m_pUIPropertiesBox->AddItem(_action, NULL, INVENTORY_EAT_ACTION);
		b_show = true;
	}

	if (b_show){
		m_pUIPropertiesBox->AutoUpdateSize();
		m_pUIPropertiesBox->BringAllToTop();

		Fvector2						cursor_pos;
		Frect							vis_rect;

		GetAbsoluteRect(vis_rect);
		cursor_pos = GetUICursor()->GetCursorPosition();
		cursor_pos.sub(vis_rect.lt);
		m_pUIPropertiesBox->Show(vis_rect, cursor_pos);

	}
}

void CUICarBodyWnd::EatItem()
{
	CActor *pActor = smart_cast<CActor*>(Level().CurrentEntity());
	if (!pActor)					return;

	CUIDragDropListEx* owner_list = CurrentItem()->OwnerList();
	if (owner_list == m_pUIOthersBagList)
	{
		u16 owner_id = (m_pInventoryBox) ? m_pInventoryBox->ID() : smart_cast<CGameObject*>(m_pOthersObject)->ID();

		move_item(owner_id, //from
			Actor()->ID(), //to
			CurrentIItem()->object().ID());
	}

	NET_Packet					P;
	CGameObject::u_EventGen(P, GEG_PLAYER_ITEM_EAT, Actor()->ID());
	P.w_u16(CurrentIItem()->object().ID());
	CGameObject::u_EventSend(P);

}


bool CUICarBodyWnd::OnItemDrop(CUICellItem* itm)
{
	InfoCurItem(NULL);
	CUIDragDropListEx*	old_owner = itm->OwnerList();
	CUIDragDropListEx*	new_owner = CUIDragDropListEx::m_drag_item->BackList();

	if (old_owner == new_owner || !old_owner || !new_owner || (false && new_owner == m_pUIOthersBagList&&m_pInventoryBox))
		return true;

	if (m_pOthersObject)
	{
		if (TransferItem(CurrentIItem(),
			(old_owner == m_pUIOthersBagList) ? m_pOthersObject : m_pOurObject,
			(old_owner == m_pUIOurBagList) ? m_pOthersObject : m_pOurObject,
			(old_owner == m_pUIOurBagList)
			)
			)
		{
			CUICellItem* ci = old_owner->RemoveItem(CurrentItem(), false);
			new_owner->SetItem(ci);
		}
	}
	else
	{
		u16 tmp_id = (smart_cast<CGameObject*>(m_pOurObject))->ID();

		bool bMoveDirection = (old_owner == m_pUIOthersBagList);

		move_item(
			bMoveDirection ? m_pInventoryBox->ID() : tmp_id,
			bMoveDirection ? tmp_id : m_pInventoryBox->ID(),
			CurrentIItem()->object().ID());


		//		Actor()->callback		(GameObject::eInvBoxItemTake)(m_pInventoryBox->lua_game_object(), CurrentIItem()->object().lua_game_object() );

		CUICellItem* ci = old_owner->RemoveItem(CurrentItem(), false);
		new_owner->SetItem(ci);
	}
	SetCurrentItem(NULL);

	return				true;
}

bool CUICarBodyWnd::OnItemStartDrag(CUICellItem* itm)
{
	InfoCurItem(NULL);
	return				false; //default behaviour
}

bool CUICarBodyWnd::OnItemDbClick(CUICellItem* itm)
{
	InfoCurItem(NULL);
	CUIDragDropListEx*	old_owner = itm->OwnerList();
	CUIDragDropListEx*	new_owner = (old_owner == m_pUIOthersBagList) ? m_pUIOurBagList : m_pUIOthersBagList;

	if (m_pOthersObject)
	{
		if (TransferItem(CurrentIItem(),
			(old_owner == m_pUIOthersBagList) ? m_pOthersObject : m_pOurObject,
			(old_owner == m_pUIOurBagList) ? m_pOthersObject : m_pOurObject,
			(old_owner == m_pUIOurBagList)
			)
			)
		{
			CUICellItem* ci = old_owner->RemoveItem(CurrentItem(), false);
			new_owner->SetItem(ci);
		}
	}
	else
	{
		if (false && old_owner == m_pUIOurBagList) return true;
		bool bMoveDirection = (old_owner == m_pUIOthersBagList);

		u16 tmp_id = (smart_cast<CGameObject*>(m_pOurObject))->ID();
		move_item(
			bMoveDirection ? m_pInventoryBox->ID() : tmp_id,
			bMoveDirection ? tmp_id : m_pInventoryBox->ID(),
			CurrentIItem()->object().ID());
		//.		Actor()->callback		(GameObject::eInvBoxItemTake)(m_pInventoryBox->lua_game_object(), CurrentIItem()->object().lua_game_object() );

	}


	return						true;
}

bool CUICarBodyWnd::OnItemSelected(CUICellItem* itm)
{
	SetCurrentItem(itm);
	InfoCurItem(NULL);
	m_item_info_view = false;
	return				false;
}

bool CUICarBodyWnd::OnItemRButtonClick(CUICellItem* itm)
{
	SetCurrentItem(itm);
	InfoCurItem(NULL);
	m_item_info_view = false;
	ActivatePropertiesBox();
	return						false;
}

bool CUICarBodyWnd::OnItemFocusReceive(CUICellItem* itm)
{
	InfoCurItem(NULL);
	m_item_info_view = true;
	itm->m_selected = true;
		set_highlight_item( itm );
	return true;
}

bool CUICarBodyWnd::OnItemFocusLost(CUICellItem* itm)
{
	if (itm)
	{
		itm->m_selected = false;
	}
	InfoCurItem(NULL);
	clear_highlight_lists();

	return true;
}

bool CUICarBodyWnd::OnItemFocusedUpdate(CUICellItem* itm)
{
	if (itm)
	{
		itm->m_selected = true;
			if ( m_highlight_clear )
			{
				set_highlight_item( itm );
			}
	}
	VERIFY(m_ItemInfo);
	if (Device.dwTimeGlobal < itm->FocusReceiveTime() + m_ItemInfo->delay)
	{
		return true; //false
	}
	if (CUIDragDropListEx::m_drag_item || m_pUIPropertiesBox->IsShown() || !m_item_info_view)
		{
			return true;
		}
	InfoCurItem(itm);
	return true;
}


void move_item(u16 from_id, u16 to_id, u16 what_id)
{
	NET_Packet P;
	CGameObject::u_EventGen(P, GE_OWNERSHIP_REJECT, from_id);

	P.w_u16(what_id);
	CGameObject::u_EventSend(P);

	//другому инвентарю - взять вещь 
	CGameObject::u_EventGen(P, GE_OWNERSHIP_TAKE, to_id);
	P.w_u16(what_id);
	CGameObject::u_EventSend(P);

}

bool CUICarBodyWnd::TransferItem(PIItem itm, CInventoryOwner* owner_from, CInventoryOwner* owner_to, bool b_check)
{
	VERIFY(NULL == m_pInventoryBox);
	CGameObject* go_from = smart_cast<CGameObject*>(owner_from);
	CGameObject* go_to = smart_cast<CGameObject*>(owner_to);

	if (smart_cast<CBaseMonster*>(go_to))	return false;
	if (b_check)
	{
		float invWeight = owner_to->inventory().CalcTotalWeight();
		float maxWeight = owner_to->inventory().GetMaxWeight();
		float itmWeight = itm->Weight();
		if (invWeight + itmWeight >= maxWeight)	return false;
	}

	move_item(go_from->ID(), go_to->ID(), itm->object().ID());

	return true;
}

void CUICarBodyWnd::BindDragDropListEnents(CUIDragDropListEx* lst)
{
	lst->m_f_item_drop = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUICarBodyWnd::OnItemDrop);
	lst->m_f_item_start_drag = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUICarBodyWnd::OnItemStartDrag);
	lst->m_f_item_db_click = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUICarBodyWnd::OnItemDbClick);
	lst->m_f_item_selected = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUICarBodyWnd::OnItemSelected);
	lst->m_f_item_rbutton_click = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUICarBodyWnd::OnItemRButtonClick);
	lst->m_f_item_focused_update = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUICarBodyWnd::OnItemFocusedUpdate);
	lst->m_f_item_focus_received = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUICarBodyWnd::OnItemFocusReceive);
	lst->m_f_item_focus_lost = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUICarBodyWnd::OnItemFocusLost);
}

void CUICarBodyWnd::ColorizeItem(CUICellItem* itm)
{
	//LOST ALPHA starts
	PIItem iitem = (PIItem)itm->m_pData;
	if (iitem->m_eItemPlace == eItemPlaceSlot || iitem->m_eItemPlace == eItemPlaceBelt)
		itm->SetTextureColor(color_rgba(100, 255, 100, 255));
}



// =================== ПОТСВЕТКА АДОНОВ, ПАТРОНОВ. АВТОР GSC =====================

void CUICarBodyWnd::clear_highlight_lists()
{
	//	m_InvSlot2Highlight->Show(false);
	//	m_InvSlot3Highlight->Show(false);
	//	m_HelmetSlotHighlight->Show(false);
	//	m_OutfitSlotHighlight->Show(false);
	//	m_DetectorSlotHighlight->Show(false);
	//	for (u8 i = 0; i<4; i++)
	//	m_QuickSlotsHighlight[i]->Show(false);
	//	for (u8 i = 0; i<e_af_count; i++)
	//		m_ArtefactSlotsHighlight[i]->Show(false);

	m_pUIOurBagList->clear_select_armament();
	m_pUIOthersBagList->clear_select_armament();
	m_highlight_clear = true;
}
void CUICarBodyWnd::highlight_item_slot(CUICellItem* cell_item)
{
	PIItem item = (PIItem)cell_item->m_pData;
	if (!item)
		return;

	if (CUIDragDropListEx::m_drag_item)
		return;

	CWeapon* weapon = smart_cast<CWeapon*>(item);
	//	CHelmet* helmet = smart_cast<CHelmet*>(item);
	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(item);
	//	CCustomDetector* detector = smart_cast<CCustomDetector*>(item);
	CEatableItem* eatable = smart_cast<CEatableItem*>(item);
	CArtefact* artefact = smart_cast<CArtefact*>(item);

	//	if (weapon)
	//	{
	//		m_InvSlot2Highlight->Show(true);
	//		m_InvSlot3Highlight->Show(true);
	//		return;
	//	}
	//	if(helmet)
	//	{
	//		m_HelmetSlotHighlight->Show(true);
	//		return;
	//	}
	//	if (outfit)
	//	{
	//		m_OutfitSlotHighlight->Show(true);
	//		return;
	//	}
	//	if(detector)
	//	{
	//		m_DetectorSlotHighlight->Show(true);
	//		return;
	//	}
	/*
	if (eatable)
	{
	if (cell_item->OwnerList() && GetType(cell_item->OwnerList()) == iQuickSlot)
	return;

	for (u8 i = 0; i<4; i++)
	m_QuickSlotsHighlight[i]->Show(true);
	return;
	}*/
	/*	if (artefact)
	{
	if (cell_item->OwnerList() && GetType(cell_item->OwnerList()) == iwBelt)
	return;

	Ivector2 cap = m_pUIBeltList->CellsCapacity();
	for (u8 i = 0; i<cap.x; i++)
	m_ArtefactSlotsHighlight[i]->Show(true);
	return;
	}*/
}
void CUICarBodyWnd::set_highlight_item(CUICellItem* cell_item)
{
	PIItem item = (PIItem)cell_item->m_pData;
	if (!item)
	{
		return;
	}
	//	highlight_item_slot(cell_item);

	highlight_armament(item, m_pUIOurBagList);
	highlight_armament(item, m_pUIOthersBagList);

	//		break;

	m_highlight_clear = false;

}

void CUICarBodyWnd::highlight_armament(PIItem item, CUIDragDropListEx* ddlist)
{
	ddlist->clear_select_armament();
	highlight_ammo_for_weapon(item, ddlist);
	highlight_weapons_for_ammo(item, ddlist);
	highlight_weapons_for_addon(item, ddlist);

}

void CUICarBodyWnd::highlight_ammo_for_weapon(PIItem weapon_item, CUIDragDropListEx* ddlist)
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

void CUICarBodyWnd::highlight_weapons_for_ammo(PIItem ammo_item, CUIDragDropListEx* ddlist)
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

bool CUICarBodyWnd::highlight_addons_for_weapon(PIItem weapon_item, CUICellItem* ci)
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

void CUICarBodyWnd::highlight_weapons_for_addon(PIItem addon_item, CUIDragDropListEx* ddlist)
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