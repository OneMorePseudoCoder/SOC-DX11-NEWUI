#include "stdafx.h"
#include "UITradeWnd.h"

#include "xrUIXmlParser.h"
#include "UIXmlInit.h"

#include "UIListBoxItem.h"
#include "../InventoryBox.h"
#include "UIPropertiesBox.h"
#include "../Entity.h"
#include "../HUDManager.h"
#include "../WeaponAmmo.h"
#include "../Actor.h"
#include "../Trade.h"
#include "../UIGameSP.h"
#include "UIInventoryUtilities.h"
#include "../inventoryowner.h"
#include "../eatable_item.h"
#include "../inventory.h"
#include "../level.h"
#include "../string_table.h"
#include "../character_info.h"
#include "UIMultiTextStatic.h"
#include "UI3tButton.h"
#include "UIItemInfo.h"

#include "UICharacterInfo.h"
#include "UIDragDropListEx.h"
#include "UICellItem.h"
#include "UICellItemFactory.h"
#include "UIHelper.h"
#include "../Weapon.h"
#include "../ai/monsters/BaseMonster/base_monster.h"

#include "UIActorStateInfo.h"
#include "../string_table.h"
#include "../CustomOutfit.h"
#include "../Artifact.h"
#include "../silencer.h"
#include "../scope.h"
#include "../WeaponMagazined.h"
#include "../grenadelauncher.h"
#include "../WeaponMagazinedWGrenade.h"

#define				TRADE_XML			"trade.xml"

struct CUITradeInternal{

	//	CUI3tButton*			    UIPerformTradeButton;
	CUI3tButton*				trade_buy_button;
	CUI3tButton*				trade_sell_button;
	CUI3tButton*			    UIToTalkButton;

	SDrawStaticStruct*	UIDealMsg;
};

CUITradeWnd::CUITradeWnd()
	:	m_bDealControlsVisible	(false),
		m_pTrade(NULL),
		m_pOthersTrade(NULL),
		bStarted(false)
{
	m_uidata = xr_new<CUITradeInternal>();
	Init();
	Hide();
	SetCurrentItem			(NULL);
}

CUITradeWnd::~CUITradeWnd()
{
	UIOurBagList->ClearAll(true);
	UIOthersBagList->ClearAll(true);


	UIOurTradeList->ClearAll(true);
	UIOthersTradeList->ClearAll(true);

	xr_delete(m_ItemInfo);
	xr_delete							(m_uidata);
}

void CUITradeWnd::Init()
{
	CUIXml								uiXml;
	uiXml.Load(CONFIG_PATH, UI_PATH, TRADE_XML);
	CUIXmlInit							xml_init;

	xml_init.InitWindow					(uiXml, "main", 0, this);


	m_pUICharacterInfoLeft = xr_new<CUICharacterInfo>();
	m_pUICharacterInfoLeft->SetAutoDelete(true);
	AttachChild(m_pUICharacterInfoLeft);
	m_pUICharacterInfoLeft->InitCharacterInfo(&uiXml, "actor_ch_info");

	m_pUICharacterInfoRight = xr_new<CUICharacterInfo>();
	m_pUICharacterInfoRight->SetAutoDelete(true);
	AttachChild(m_pUICharacterInfoRight);
	m_pUICharacterInfoRight->InitCharacterInfo(&uiXml, "partner_ch_info");


	//Списки торговли
	UIOurMoneyStatic = UIHelper::CreateTextWnd(uiXml, "actor_money_static", this);
	UIOtherMoneyStatic = UIHelper::CreateTextWnd(uiXml, "partner_money_static", this);

	m_ActorBottomInfo = UIHelper::CreateTextWnd(uiXml, "actor_weight_caption", this);
	m_ActorWeight = UIHelper::CreateTextWnd(uiXml, "actor_weight", this);
	m_ActorWeightMax = UIHelper::CreateTextWnd(uiXml, "actor_weight_max", this);
	m_ActorBottomInfo->AdjustWidthToText();

	m_PartnerBottomInfo = UIHelper::CreateStatic(uiXml, "partner_weight_caption", this);
	m_PartnerWeight = UIHelper::CreateTextWnd(uiXml, "partner_weight", this);
	m_PartnerBottomInfo->AdjustWidthToText();
	m_PartnerWeight_end_x = m_PartnerWeight->GetWndPos().x;

	m_RightDelimiter = UIHelper::CreateStatic(uiXml, "right_delimiter", this);
	m_ActorTradeCaption = UIHelper::CreateStatic(uiXml, "right_delimiter:trade_caption", m_RightDelimiter);
	m_ActorTradePrice = UIHelper::CreateStatic(uiXml, "right_delimiter:trade_price", m_RightDelimiter);
	m_ActorTradeWeightMax = UIHelper::CreateStatic(uiXml, "right_delimiter:trade_weight_max", m_RightDelimiter);
	m_ActorTradeCaption->AdjustWidthToText();

	m_LeftDelimiter = UIHelper::CreateStatic(uiXml, "left_delimiter", this);
	m_PartnerTradeCaption = UIHelper::CreateStatic(uiXml, "left_delimiter:trade_caption", m_LeftDelimiter);
	m_PartnerTradePrice = UIHelper::CreateStatic(uiXml, "left_delimiter:trade_price", m_LeftDelimiter);
	m_PartnerTradeWeightMax = UIHelper::CreateStatic(uiXml, "left_delimiter:trade_weight_max", m_LeftDelimiter);
	m_PartnerTradeCaption->AdjustWidthToText();


	UIOurTradeList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_actor_trade", this);  //  это актера продажа
	UIOurBagList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_actor_trade_bag", this);
	UIOthersTradeList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_partner_trade", this); //  это сидоровича продажа
	UIOthersBagList = UIHelper::CreateDragDropListEx(uiXml, "dragdrop_partner_bag", this);


	xml_init.InitAutoStatic				(uiXml, "auto_static", this);

	m_ActorStateInfo = xr_new<ui_actor_state_wnd>();
	m_ActorStateInfo->init_from_xml(uiXml, "actor_state_info");
	m_ActorStateInfo->SetAutoDelete(true);
	AttachChild(m_ActorStateInfo);

//	UIPerformTradeButton = UIHelper::Create3tButton(uiXml, "trade_button", this);
	m_uidata->trade_buy_button = UIHelper::Create3tButton(uiXml, "trade_buy_button", this);
	m_uidata->trade_sell_button = UIHelper::Create3tButton(uiXml, "trade_sell_button", this);
	m_uidata->UIToTalkButton = UIHelper::Create3tButton(uiXml, "exit_button", this);



	m_uidata->UIDealMsg					= NULL;
//	SetCurrentItem(NULL);

	m_ItemInfo = xr_new<CUIItemInfo>();
	m_ItemInfo->InitItemInfo("actor_menu_item.xml");

	m_highlight_clear = true;
	m_item_info_view = false;

	BindDragDropListEnents             (UIOurTradeList);
	BindDragDropListEnents              (UIOthersTradeList);
	BindDragDropListEnents              (UIOurBagList);
	BindDragDropListEnents				(UIOthersBagList);
}

void CUITradeWnd::InitTrade(CInventoryOwner* pOur, CInventoryOwner* pOthers)
{
	VERIFY								(pOur);
	VERIFY								(pOthers);

	m_pInvOwner							= pOur;
	m_pOthersInvOwner					= pOthers;

	m_pUICharacterInfoLeft->InitCharacter(m_pInvOwner->object_id());
	m_pUICharacterInfoRight->InitCharacter(m_pOthersInvOwner->object_id());

	m_pInv								= &m_pInvOwner->inventory();
	m_pOthersInv						= pOur->GetTrade()->GetPartnerInventory();
		
	m_pTrade							= pOur->GetTrade();
	m_pOthersTrade						= pOur->GetTrade()->GetPartnerTrade();
    	
	EnableAll							();

	UpdateLists							(eBoth);
}  

void CUITradeWnd::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if (pWnd == m_uidata->UIToTalkButton && msg == BUTTON_CLICKED)
	{
		SwitchToTalk();
	}
	if (pWnd == m_uidata->trade_buy_button && msg == BUTTON_CLICKED)
	{
		OnBtnPerformTradeBuy();
	}
	if (pWnd == m_uidata->trade_sell_button && msg == BUTTON_CLICKED)
	{
		OnBtnPerformTradeSell();
	}


	CUIWindow::SendMessage(pWnd, msg, pData);
}



#include "../xr_level_controller.h"
bool CUITradeWnd::OnKeyboardAction(int dik, EUIMessages keyboard_action)
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


void CUITradeWnd::Draw()
{
	inherited::Draw				();
	m_ItemInfo->Draw();
	if(m_uidata->UIDealMsg)		m_uidata->UIDealMsg->Draw();

}

extern void UpdateCameraDirection(CGameObject* pTo);

void CUITradeWnd::Update()
{

	EListType et					= eNone;

	if(m_pInv->ModifyFrame()==Device.dwFrame && m_pOthersInv->ModifyFrame()==Device.dwFrame){
		et = eBoth;
	}else if(m_pInv->ModifyFrame()==Device.dwFrame){
		et = e1st;
	}else if(m_pOthersInv->ModifyFrame()==Device.dwFrame){
		et = e2nd;
	}
	if(et!=eNone)
		UpdateLists					(et);

	inherited::Update				();
	UpdateCameraDirection			(smart_cast<CGameObject*>(m_pOthersInvOwner));

	if(m_uidata->UIDealMsg){
		m_uidata->UIDealMsg->Update();
		if( !m_uidata->UIDealMsg->IsActual()){
			HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_mine");
			HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_other");
			m_uidata->UIDealMsg			= NULL;
		}
	}
	m_ItemInfo->Update();
	m_ActorStateInfo->UpdateActorInfo(m_pInvOwner);

	UpdateActor();
	UpdatePartnerBag();
}


void CUITradeWnd::UpdateActor()
{
	if (IsGameTypeSingle())
	{
		string64 buf;
		sprintf_s(buf, "%d RU", m_pInvOwner->get_money());
		UIOurMoneyStatic->SetText(buf);
	}

//	CActor* actor = smart_cast<CActor*>(m_pActorInvOwner);
//	if (actor)
//	{
//		CWeapon* wp = smart_cast<CWeapon*>(actor->inventory().ActiveItem());
//		if (wp)
//		{
		//	wp->ForceUpdateAmmo();
//		}
	//}//actor

	InventoryUtilities::UpdateWeightStr(*m_ActorWeight, *m_ActorWeightMax, m_pInvOwner);

	m_ActorWeight->AdjustWidthToText();
	m_ActorWeightMax->AdjustWidthToText();
	m_ActorBottomInfo->AdjustWidthToText();

	Fvector2 pos = m_ActorWeight->GetWndPos();
	pos.x = m_ActorWeightMax->GetWndPos().x - m_ActorWeight->GetWndSize().x - 5.0f;
	m_ActorWeight->SetWndPos(pos);
	pos.x = pos.x - m_ActorBottomInfo->GetWndSize().x - 5.0f;
	m_ActorBottomInfo->SetWndPos(pos);
}

void CUITradeWnd::UpdatePartnerBag()
{
	string64 buf;

	CBaseMonster* monster = smart_cast<CBaseMonster*>(m_pOthersInvOwner);
	if (monster || m_pOthersInvOwner->use_simplified_visual())
	{
		m_PartnerWeight->SetText("");
	}
	else if (m_pOthersInvOwner->InfinitiveMoney())
	{
		UIOtherMoneyStatic->SetText("--- RU");
	}
	else
	{
		sprintf_s(buf, "%d RU", m_pOthersInvOwner->get_money());
		UIOtherMoneyStatic->SetText(buf);
	}

	LPCSTR kg_str = CStringTable().translate("Кг").c_str();
	float total = CalcItemsWeight(UIOthersBagList);
	sprintf_s(buf, "%.1f %s", total, kg_str);
	m_PartnerWeight->SetText(buf);
	m_PartnerWeight->AdjustWidthToText();

	Fvector2 pos = m_PartnerWeight->GetWndPos();
	pos.x = m_PartnerWeight_end_x - m_PartnerWeight->GetWndSize().x - 5.0f;
	m_PartnerWeight->SetWndPos(pos);
	pos.x = pos.x - m_PartnerBottomInfo->GetWndSize().x - 5.0f;
	m_PartnerBottomInfo->SetWndPos(pos);
}



#include "UIInventoryUtilities.h"
void CUITradeWnd::Show()
{
	InventoryUtilities::SendInfoToActor("ui_trade");
	inherited::Show					(true);
	inherited::Enable				(true);

	SetCurrentItem					(NULL);
	ResetAll						();
	m_uidata->UIDealMsg				= NULL;

	m_ActorStateInfo->Show(true);
	m_ActorStateInfo->UpdateActorInfo(m_pInvOwner);
}

void CUITradeWnd::Hide()
{
	InventoryUtilities::SendInfoToActor("ui_trade_hide");
	inherited::Show					(false);
	inherited::Enable				(false);
	if(bStarted)
		StopTrade					();
	
	m_uidata->UIDealMsg				= NULL;
	m_ActorStateInfo->Show(false);

	if(HUD().GetUI()->UIGame()){
		HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_mine");
		HUD().GetUI()->UIGame()->RemoveCustomStatic("not_enough_money_other");
	}

	UIOurBagList->ClearAll(true);
	UIOurTradeList->ClearAll(true);
	UIOthersBagList->ClearAll(true);
	UIOthersTradeList->ClearAll(true);

}

void CUITradeWnd::StartTrade()
{
	if (m_pTrade)					m_pTrade->TradeCB(true);
	if (m_pOthersTrade)				m_pOthersTrade->TradeCB(true);
	bStarted						= true;
}

void CUITradeWnd::StopTrade()
{
	if (m_pTrade)					m_pTrade->TradeCB(false);
	if (m_pOthersTrade)				m_pOthersTrade->TradeCB(false);
	bStarted						= false;
}

#include "../trade_parameters.h"
bool CUITradeWnd::CanMoveToOther(PIItem pItem)
{
	if (!pItem->CanTrade())
		return false;

	if (!m_pOthersInvOwner->trade_parameters().enabled(
		CTradeParameters::action_buy(0), pItem->object().cNameSect()))
	{
		return false;
	}

	if (pItem->GetCondition()<m_pOthersInvOwner->trade_parameters().buy_item_condition_factor)
		return false;

	float r1				= CalcItemsWeight(UIOurTradeList);	// our
	float r2				= CalcItemsWeight(UIOthersTradeList);	// other
	 
	float itmWeight			= pItem->Weight();
	float otherInvWeight	= m_pOthersInv->CalcTotalWeight();
	float otherMaxWeight	= m_pOthersInv->GetMaxWeight();

	if (otherInvWeight - r2 + r1 + itmWeight > otherMaxWeight)
	{
		return false;
	}
	return true;

}

void move_item(CUICellItem* itm, CUIDragDropListEx* from, CUIDragDropListEx* to)
{
	CUICellItem* _itm		= from->RemoveItem	(itm, false);
	to->SetItem				(_itm);
}

bool CUITradeWnd::ToOurTrade()
{
	if (!CanMoveToOther(CurrentIItem()))	return false;

	move_item				(CurrentItem(), UIOurBagList, UIOurTradeList);
	UpdatePrices			();
	return					true;
}

bool CUITradeWnd::ToOthersTrade()
{
	move_item				(CurrentItem(), UIOthersBagList, UIOthersTradeList);
	UpdatePrices			();

	return					true;
}

bool CUITradeWnd::ToOurBag()
{
	move_item				(CurrentItem(), UIOurTradeList, UIOurBagList);
	UpdatePrices			();
	
	return					true;
}

bool CUITradeWnd::ToOthersBag()
{
	move_item				(CurrentItem(), UIOthersTradeList, UIOthersBagList);
	UpdatePrices			();

	return					true;
}

float CUITradeWnd::CalcItemsWeight(CUIDragDropListEx* pList)
{
	float res = 0.0f;

	for(u32 i=0; i<pList->ItemsCount(); ++i)
	{
		CUICellItem* itm	= pList->GetItemIdx	(i);
		PIItem	iitem		= (PIItem)itm->m_pData;
		res					+= iitem->Weight();
		for(u32 j=0; j<itm->ChildsCount(); ++j){
			PIItem	jitem		= (PIItem)itm->Child(j)->m_pData;
			res					+= jitem->Weight();
		}
	}
	return res;
}

u32 CUITradeWnd::CalcItemsPrice(CUIDragDropListEx* pList, CTrade* pTrade, bool bBuying)
{
	u32 res = 0;

	for (u32 i = 0; i < pList->ItemsCount(); ++i)
	{
		CUICellItem* itm = pList->GetItemIdx(i);
		PIItem iitem = (PIItem)itm->m_pData;
		res += pTrade->GetItemPrice(iitem, bBuying);

		for (u32 j = 0; j < itm->ChildsCount(); ++j)
		{
			PIItem jitem = (PIItem)itm->Child(j)->m_pData;
			res += pTrade->GetItemPrice(jitem, bBuying);
		}
	}
	return res;
}



void CUITradeWnd::DisableAll()
{
//	m_uidata->UIOurBagWnd.Enable			(false);
//	m_uidata->UIOthersBagWnd.Enable			(false);
//	m_uidata->UIOurTradeWnd.Enable			(false);
//	m_uidata->UIOthersTradeWnd.Enable		(false);
}

void CUITradeWnd::EnableAll()
{
//	UIOurBagWnd->Enable(true);
//	m_uidata->UIOthersBagWnd.Enable			(true);
//	m_uidata->UIOurTradeWnd.Enable			(true);
//	m_uidata->UIOthersTradeWnd.Enable		(true);
}

void CUITradeWnd::UpdatePrices()
{
	LPCSTR kg_str = CStringTable().translate("Кг").c_str();

	UpdateActor();
	UpdatePartnerBag();

	u32 actor_price = CalcItemsPrice(UIOurTradeList, m_pOthersTrade, true);
	u32 partner_price = CalcItemsPrice(UIOthersTradeList, m_pOthersTrade, false);

	string64 buf;
	sprintf_s(buf, "%d RU", actor_price);		m_ActorTradePrice->SetText(buf);	m_ActorTradePrice->AdjustWidthToText();
	sprintf_s(buf, "%d RU", partner_price);	    m_PartnerTradePrice->SetText(buf);	m_PartnerTradePrice->AdjustWidthToText();


	float actor_weight = CalcItemsWeight(UIOurTradeList);
	float partner_weight = CalcItemsWeight(UIOthersTradeList);

	sprintf_s(buf, "(%.1f %s)", actor_weight, kg_str);		m_ActorTradeWeightMax->SetText(buf);
	sprintf_s(buf, "(%.1f %s)", partner_weight, kg_str);	m_PartnerTradeWeightMax->SetText(buf);

	Fvector2 pos = m_ActorTradePrice->GetWndPos();
	pos.x = m_ActorTradeWeightMax->GetWndPos().x - m_ActorTradePrice->GetWndSize().x - 5.0f;
	m_ActorTradePrice->SetWndPos(pos);
	pos.x = pos.x - m_ActorTradeCaption->GetWndSize().x - 5.0f;
	m_ActorTradeCaption->SetWndPos(pos);

	pos = m_PartnerTradePrice->GetWndPos();
	pos.x = m_PartnerTradeWeightMax->GetWndPos().x - m_PartnerTradePrice->GetWndSize().x - 5.0f;
	m_PartnerTradePrice->SetWndPos(pos);
	pos.x = pos.x - m_PartnerTradeCaption->GetWndSize().x - 5.0f;
	m_PartnerTradeCaption->SetWndPos(pos);
}

void CUITradeWnd::PerformTrade()
{

	if (UIOurTradeList->ItemsCount() == 0 && UIOthersTradeList->ItemsCount() == 0)
		return;

	int actor_money = (int)m_pInvOwner->get_money();
	int partner_money = (int)m_pOthersInvOwner->get_money();
	int actor_price = (int)CalcItemsPrice(UIOurTradeList, m_pOthersTrade, true);
	int partner_price = (int)CalcItemsPrice(UIOthersTradeList, m_pOthersTrade, false);

	int delta_price = actor_price - partner_price;
	actor_money += delta_price;
	partner_money -= delta_price;

	if ((actor_money >= 0) && (partner_money >= 0) && (actor_price >= 0 || partner_price > 0))
	{
		m_pOthersTrade->OnPerformTrade(partner_price, actor_price);

		TransferItems(UIOurTradeList, UIOthersBagList, m_pOthersTrade, true);
		TransferItems(UIOthersTradeList, UIOurBagList, m_pOthersTrade, false);
	}
	else
	{
		if (actor_money < 0)
		{
			m_uidata->UIDealMsg = HUD().GetUI()->UIGame()->AddCustomStatic("not_enough_money_mine", true);

		}
		else if (partner_money < 0)
		{
			m_uidata->UIDealMsg = HUD().GetUI()->UIGame()->AddCustomStatic("not_enough_money_other", true);
		}
		else
		{
			m_uidata->UIDealMsg = HUD().GetUI()->UIGame()->AddCustomStatic("trade_dont_make", true);

		}

		m_uidata->UIDealMsg->m_endTime = Device.fTimeGlobal + 2.0f;// sec
	}
	SetCurrentItem(NULL);

}

void CUITradeWnd::OnBtnPerformTradeBuy()
{
	if (UIOthersTradeList->ItemsCount() == 0)
	{
		return;
	}

	int actor_money = (int)m_pInvOwner->get_money();
	int partner_money = (int)m_pOthersInvOwner->get_money();
	int actor_price = 0;
	int partner_price = (int)CalcItemsPrice(UIOthersTradeList, m_pOthersTrade, false);

	int delta_price = actor_price - partner_price;
	actor_money += delta_price;
	partner_money -= delta_price;

	if ((actor_money >= 0) /*&& ( partner_money >= 0 )*/ && (actor_price >= 0 || partner_price > 0))
	{
		m_pOthersTrade->OnPerformTrade(partner_price, actor_price);

		//		TransferItems( m_pTradeActorList,   m_pTradePartnerBagList, m_partner_trade, true );
		TransferItems(UIOthersTradeList, UIOurBagList, m_pOthersTrade, false);
	}
	else
	{
		if (actor_money < 0)
		{
			m_uidata->UIDealMsg = HUD().GetUI()->UIGame()->AddCustomStatic("not_enough_money_mine", true);
		}
		//else if ( partner_money < 0 )
		//{
		//	CallMessageBoxOK( "not_enough_money_partner" );
		//}
		else
		{
			m_uidata->UIDealMsg = HUD().GetUI()->UIGame()->AddCustomStatic("trade_dont_make", true);
		}
		m_uidata->UIDealMsg->m_endTime = Device.fTimeGlobal + 2.0f;// sec
	}


	SetCurrentItem(NULL);

	//	UpdateItemsPlace();
}
void CUITradeWnd::OnBtnPerformTradeSell()
{
	if (UIOurTradeList->ItemsCount() == 0)
	{
		return;
	}

	int actor_money = (int)m_pInvOwner->get_money();
	int partner_money = (int)m_pOthersInvOwner->get_money();
	int actor_price = (int)CalcItemsPrice(UIOurTradeList, m_pOthersTrade, true);
	int partner_price = 0;

	int delta_price = actor_price - partner_price;
	actor_money += delta_price;
	partner_money -= delta_price;

	if ((actor_money >= 0) && (partner_money >= 0) && (actor_price >= 0 || partner_price > 0))
	{
		m_pOthersTrade->OnPerformTrade(partner_price, actor_price);

		TransferItems(UIOurTradeList, UIOthersBagList, m_pOthersTrade, true);
		//		TransferItems( m_pTradePartnerList,	m_pTradeActorBagList,	m_partner_trade, false );
	}
	else
	{
		/*		if ( actor_money < 0 )
		{
		CallMessageBoxOK( "not_enough_money_actor" );
		}
		else */if (partner_money < 0)
		{
			m_uidata->UIDealMsg = HUD().GetUI()->UIGame()->AddCustomStatic("not_enough_money_other", true);
		}
		else
		{
			m_uidata->UIDealMsg = HUD().GetUI()->UIGame()->AddCustomStatic("trade_dont_make", true);

		}

	    m_uidata->UIDealMsg->m_endTime = Device.fTimeGlobal + 2.0f;// sec
	  }

	SetCurrentItem(NULL);
}


void CUITradeWnd::TransferItems(CUIDragDropListEx* pSellList,
								CUIDragDropListEx* pBuyList,
								CTrade* pTrade,
								bool bBuying)
{
	while(pSellList->ItemsCount())
	{
		CUICellItem* itm	=	pSellList->RemoveItem(pSellList->GetItemIdx(0),false);
		pTrade->TransferItem	((PIItem)itm->m_pData, bBuying);
		pBuyList->SetItem		(itm);
	}

	pTrade->pThis.inv_owner->set_money ( pTrade->pThis.inv_owner->get_money(), true );
	pTrade->pPartner.inv_owner->set_money( pTrade->pPartner.inv_owner->get_money(), true );
}

void CUITradeWnd::UpdateLists(EListType mode)
{
	if(mode==eBoth||mode==e1st){
		UIOurBagList->ClearAll(true);
		UIOurTradeList->ClearAll(true);
	}

	if(mode==eBoth||mode==e2nd){
		UIOthersBagList->ClearAll(true);
		UIOthersTradeList->ClearAll(true);
	}

	UpdatePrices						();


	if(mode==eBoth||mode==e1st)
	{
		ruck_list.clear					();
   		m_pInv->AddAvailableItems		(ruck_list, true);
		std::sort						(ruck_list.begin(),ruck_list.end(),InventoryUtilities::GreaterRoomInRuck);
		FillList						(ruck_list, UIOurBagList, true);
	}

	if(mode==eBoth||mode==e2nd)
	{
		ruck_list.clear					();
		m_pOthersInv->AddAvailableItems	(ruck_list, true);
		std::sort						(ruck_list.begin(),ruck_list.end(),InventoryUtilities::GreaterRoomInRuck);
		FillList						(ruck_list, UIOthersBagList, false);
	}




}

void CUITradeWnd::FillList	(TIItemContainer& cont, CUIDragDropListEx* dragDropList, bool do_colorize)
{
	TIItemContainer::iterator it	= cont.begin();
	TIItemContainer::iterator it_e	= cont.end();

	for(; it != it_e; ++it) 
	{
		CUICellItem* itm			= create_cell_item	(*it);
		if(do_colorize)				ColorizeItem		(itm, CanMoveToOther(*it));
		dragDropList->SetItem(itm);
	}

}

bool CUITradeWnd::OnItemStartDrag(CUICellItem* itm)
{
	InfoCurItem(NULL);
	return false; //default behaviour
}

bool CUITradeWnd::OnItemSelected(CUICellItem* itm)
{
	SetCurrentItem		(itm);
	InfoCurItem(NULL);
	m_item_info_view = false;
	return				false;
}

bool CUITradeWnd::OnItemRButtonClick(CUICellItem* itm)
{
	SetCurrentItem				(itm);
	InfoCurItem(NULL);
	m_item_info_view = false;
	return						false;
}


bool CUITradeWnd::OnItemDrop(CUICellItem* itm)
{
	InfoCurItem(NULL);
	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	CUIDragDropListEx*	new_owner		= CUIDragDropListEx::m_drag_item->BackList();
	if(old_owner==new_owner || !old_owner || !new_owner)
					return false;

	if(old_owner==UIOurBagList && new_owner==UIOurTradeList)
		ToOurTrade				();
	else if(old_owner==UIOurTradeList && new_owner==UIOurBagList)
		ToOurBag				();
	else if(old_owner==UIOthersBagList && new_owner == UIOthersTradeList)
		ToOthersTrade			();
	else if(old_owner== UIOthersTradeList && new_owner==UIOthersBagList)
		ToOthersBag				();

	return true;
}

bool CUITradeWnd::OnItemDbClick(CUICellItem* itm)
{
	InfoCurItem(NULL);
	SetCurrentItem						(itm);
	CUIDragDropListEx*	old_owner		= itm->OwnerList();
	
	if(old_owner == UIOurBagList)
		ToOurTrade				();
	else if(old_owner == UIOurTradeList)
		ToOurBag				();
	else if(old_owner == UIOthersBagList)
		ToOthersTrade			();
	else if(old_owner == UIOthersTradeList)
		ToOthersBag				();
	else
		R_ASSERT2(false, "wrong parent for cell item");

	return true;
}


CUICellItem* CUITradeWnd::CurrentItem()
{
	return m_pCurrentCellItem;
}

PIItem CUITradeWnd::CurrentIItem()
{
	return	(m_pCurrentCellItem)?(PIItem)m_pCurrentCellItem->m_pData : NULL;
}

void CUITradeWnd::SetCurrentItem(CUICellItem* itm)
{
	if (m_pCurrentCellItem == itm) return;
	m_pCurrentCellItem = itm;
	if (!itm)
	{
		InfoCurItem(NULL);
	}
	
//	if(!m_pCurrentCellItem)		return;

	CUIDragDropListEx* owner	= itm->OwnerList();
	bool bBuying				= (owner==UIOurBagList) || (owner == UIOurTradeList);


}

void CUITradeWnd::SwitchToTalk()
{
	GetMessageTarget()->SendMessage		(this, TRADE_WND_CLOSED);
}

void CUITradeWnd::BindDragDropListEnents(CUIDragDropListEx* lst)
{
	lst->m_f_item_drop = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUITradeWnd::OnItemDrop);
	lst->m_f_item_start_drag = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUITradeWnd::OnItemStartDrag);
	lst->m_f_item_db_click = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUITradeWnd::OnItemDbClick);
	lst->m_f_item_selected = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUITradeWnd::OnItemSelected);
	lst->m_f_item_rbutton_click = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUITradeWnd::OnItemRButtonClick);
	lst->m_f_item_focused_update = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUITradeWnd::OnItemFocusedUpdate);
	lst->m_f_item_focus_received = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUITradeWnd::OnItemFocusReceive);
	lst->m_f_item_focus_lost = CUIDragDropListEx::DRAG_CELL_EVENT(this, &CUITradeWnd::OnItemFocusLost);
}

void CUITradeWnd::ColorizeItem(CUICellItem* itm, bool b)
{
	//lost alpha starts
	PIItem iitem = (PIItem)itm->m_pData;
	if (!b)
		itm->SetTextureColor(color_rgba(255, 100, 100, 255));
	else if (iitem->m_eItemPlace == eItemPlaceSlot || iitem->m_eItemPlace == eItemPlaceBelt)
		itm->SetTextureColor(color_rgba(100, 255, 100, 255));
}

void CUITradeWnd::InfoCurItem(CUICellItem* cell_item)
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
		compare_item = m_pInvOwner->inventory().ItemFromSlot(compare_slot);
	}

	CInventoryOwner* item_owner = smart_cast<CInventoryOwner*>(current_item->m_pCurrentInventory->GetOwner());
	u32 item_price = u32(-1);
	if (item_owner && item_owner == m_pInvOwner)
		item_price = m_pOthersTrade->GetItemPrice(current_item, true);
	else
		item_price = m_pOthersTrade->GetItemPrice(current_item, false);

	CWeaponAmmo* ammo = smart_cast<CWeaponAmmo*>(current_item);
	if (ammo)
	{
		for (u32 j = 0; j < cell_item->ChildsCount(); ++j)
		{
			u32 tmp_price = 0;
			PIItem jitem = (PIItem)cell_item->Child(j)->m_pData;
			CInventoryOwner* ammo_owner = smart_cast<CInventoryOwner*>(jitem->m_pCurrentInventory->GetOwner());
			if (ammo_owner && ammo_owner == m_pInvOwner)
				tmp_price = m_pOthersTrade->GetItemPrice(jitem, true);
			else
				tmp_price = m_pOthersTrade->GetItemPrice(jitem, false);

			item_price += tmp_price;
		}
	}
		if (!current_item->CanTrade() ||
			(!m_pOthersInvOwner->trade_parameters().enabled(CTradeParameters::action_buy(0),
			current_item->object().cNameSect()) &&
			item_owner && item_owner == m_pInvOwner)
			)
			m_ItemInfo->InitItem(cell_item, compare_item, u32(-1), "Торговец не интересуется подобными предметами.");
		else if (current_item->GetCondition() < m_pOthersInvOwner->trade_parameters().buy_item_condition_factor)
			m_ItemInfo->InitItem(cell_item, compare_item, u32(-1), "Предмет в слишком плохом состоянии для продажи.");
		else
			m_ItemInfo->InitItem(cell_item, compare_item, item_price);

	float dx_pos = GetWndRect().left;
	fit_in_rect(m_ItemInfo, Frect().set(0.0f, 0.0f, UI_BASE_WIDTH - dx_pos, UI_BASE_HEIGHT), 10.0f, dx_pos);
}

bool CUITradeWnd::OnItemFocusReceive(CUICellItem* itm)
{
	InfoCurItem(NULL);
	SetCurrentItem(NULL);
	m_item_info_view = true;
	itm->m_selected = true;
	set_highlight_item(itm);
	return true;
}

bool CUITradeWnd::OnItemFocusLost(CUICellItem* itm)
{
	if (itm)
	{
		itm->m_selected = false;
	}
	InfoCurItem(NULL);
	SetCurrentItem(NULL);
	clear_highlight_lists();

	return true;
}

bool CUITradeWnd::OnItemFocusedUpdate(CUICellItem* itm)
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
	if (CUIDragDropListEx::m_drag_item || !m_item_info_view /*|| m_pUIPropertiesBox->IsShown() || !m_item_info_view*/)
	{
		return true;
	}
	InfoCurItem(itm);
	return true;
}

// =================== ПОТСВЕТКА АДОНОВ, ПАТРОНОВ. АВТОР GSC =====================

void CUITradeWnd::clear_highlight_lists()
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


	UIOurTradeList->clear_select_armament();
	UIOurBagList->clear_select_armament();
	UIOthersTradeList->clear_select_armament();
	UIOthersBagList->clear_select_armament();

	m_highlight_clear = true;
}
void CUITradeWnd::highlight_item_slot(CUICellItem* cell_item)
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
void CUITradeWnd::set_highlight_item(CUICellItem* cell_item)
{
	PIItem item = (PIItem)cell_item->m_pData;
	if (!item)
	{
		return;
	}
	//	highlight_item_slot(cell_item);

	highlight_armament(item, UIOurTradeList);
	highlight_armament(item, UIOurBagList);
	highlight_armament(item, UIOthersTradeList);
	highlight_armament(item, UIOthersBagList);

	//		break;

	m_highlight_clear = false;

}

void CUITradeWnd::highlight_armament(PIItem item, CUIDragDropListEx* ddlist)
{
	ddlist->clear_select_armament();
	highlight_ammo_for_weapon(item, ddlist);
	highlight_weapons_for_ammo(item, ddlist);
	highlight_weapons_for_addon(item, ddlist);

}

void CUITradeWnd::highlight_ammo_for_weapon(PIItem weapon_item, CUIDragDropListEx* ddlist)
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

void CUITradeWnd::highlight_weapons_for_ammo(PIItem ammo_item, CUIDragDropListEx* ddlist)
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

bool CUITradeWnd::highlight_addons_for_weapon(PIItem weapon_item, CUICellItem* ci)
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

void CUITradeWnd::highlight_weapons_for_addon(PIItem addon_item, CUIDragDropListEx* ddlist)
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