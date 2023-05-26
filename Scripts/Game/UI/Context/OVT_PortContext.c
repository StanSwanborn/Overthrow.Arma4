class OVT_PortContext : OVT_UIContext
{	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Inventory Item Layout", params: "layout")]
	ResourceName m_ItemLayout;
	
	protected int m_SelectedResource = -1;
	protected ResourceName m_SelectedResourceName;
	
	override void PostInit()
	{
		if(SCR_Global.IsEditMode()) return;
		m_Economy.m_OnPlayerMoneyChanged.Insert(OnPlayerMoneyChanged);
	}
	
	protected void OnPlayerMoneyChanged(string playerId, int amount)
	{
		if(playerId == m_sPlayerID && m_bIsActive)
		{
			TextWidget money = TextWidget.Cast(m_wRoot.FindAnyWidget("PlayerMoney"));		
			money.SetText("$" + amount);
		}
	}
	
	override void OnShow()
	{	
		
		Widget take1 = m_wRoot.FindAnyWidget("Buy10Button");
		ButtonActionComponent action = ButtonActionComponent.Cast(take1.FindHandler(ButtonActionComponent));		
		action.GetOnAction().Insert(BuyTen);
		
		Widget take10 = m_wRoot.FindAnyWidget("Buy100Button");
		action = ButtonActionComponent.Cast(take10.FindHandler(ButtonActionComponent));		
		action.GetOnAction().Insert(BuyHundred);
		
		Widget closeButton = m_wRoot.FindAnyWidget("CloseButton");
		SCR_NavigationButtonComponent btn = SCR_NavigationButtonComponent.Cast(closeButton.FindHandler(SCR_NavigationButtonComponent));		
		btn.m_OnClicked.Insert(CloseLayout);
		
		Refresh();		
	}
	
	void Refresh()
	{		
		Widget container = m_wRoot.FindAnyWidget("Inventory");
		WorkspaceWidget workspace = GetGame().GetWorkspace(); 
		
		int wi = 0;
		
		while(container.GetChildren())
			container.RemoveChild(container.GetChildren());
		
		array<ResourceName> done = new array<ResourceName>;
		
		foreach(OVT_ShopInventoryConfig shop : m_Economy.m_aShopConfigs)
		{			
			if(shop.type == OVT_ShopType.SHOP_VEHICLE) continue;
			
			foreach(OVT_ShopInventoryItem item : shop.m_aInventoryItems)
			{
				array<SCR_EntityCatalogEntry> entries();
				m_Economy.FindInventoryItems(item.m_eItemType, item.m_eItemMode, item.m_sFind, entries);
				
				foreach(SCR_EntityCatalogEntry entry : entries)
				{
					ResourceName prefab = entry.GetPrefab();
					if(done.Contains(prefab)) continue;
					done.Insert(prefab);
					
					if(wi == 0 && m_SelectedResource == -1){
						SelectItem(prefab);
					}
					
					Widget ww = workspace.CreateWidgets(m_ItemLayout, container);
					OVT_PortItemComponent card = OVT_PortItemComponent.Cast(ww.FindHandler(OVT_PortItemComponent));
					
					int id = m_Economy.GetInventoryId(prefab);
											
					card.Init(prefab, m_Economy.GetPrice(id), this);
					
					wi++;
				}
			}
		}		
	}
	
	override void SelectItem(ResourceName res)
	{
		int id = m_Economy.GetInventoryId(res);
		m_SelectedResource = id;
		m_SelectedResourceName = res;
		TextWidget typeName = TextWidget.Cast(m_wRoot.FindAnyWidget("SelectedTypeName"));
		ItemPreviewWidget img = ItemPreviewWidget.Cast(m_wRoot.FindAnyWidget("SelectedTypeImage"));
		TextWidget details = TextWidget.Cast(m_wRoot.FindAnyWidget("SelectedDetails"));
		TextWidget desc = TextWidget.Cast(m_wRoot.FindAnyWidget("SelectedDescription"));
		img.SetResolutionScale(1, 1);
				
		IEntity spawnedItem = GetGame().SpawnEntityPrefabLocal(Resource.Load(m_Economy.GetResource(id)));
									
		InventoryItemComponent inv = InventoryItemComponent.Cast(spawnedItem.FindComponent(InventoryItemComponent));
		if(inv){
			SCR_ItemAttributeCollection attr = SCR_ItemAttributeCollection.Cast(inv.GetAttributes());
			if(attr)
			{
				
				UIInfo info = attr.GetUIInfo();
				typeName.SetText(info.GetName());
				details.SetText("$" + m_Economy.GetPrice(id).ToString());
				desc.SetText(info.GetDescription());
			}
		}
		SCR_EntityHelper.DeleteEntityAndChildren(spawnedItem);
		
		ItemPreviewManagerEntity manager = GetGame().GetItemPreviewManager();
		if (!manager)
			return;
		
		manager.SetPreviewItemFromPrefab(img, res);		
	}
	
	override void RegisterInputs()
	{
		super.RegisterInputs();
		if(!m_InputManager) return;
				
		m_InputManager.AddActionListener("MenuBack", EActionTrigger.DOWN, CloseLayout);
	}
	
	override void UnregisterInputs()
	{
		super.UnregisterInputs();
		if(!m_InputManager) return;
				
		m_InputManager.RemoveActionListener("MenuBack", EActionTrigger.DOWN, CloseLayout);
	}	
	
	void Buy(int qty)
	{		
		SCR_CompartmentAccessComponent compartment = SCR_CompartmentAccessComponent.Cast(m_Owner.FindComponent(SCR_CompartmentAccessComponent));
		if(!compartment) return;
			
		IEntity entity = compartment.GetVehicle();
		if(entity)
		{	
			OVT_Global.GetServer().ImportToVehicle(m_SelectedResource, qty, entity, m_iPlayerID);
		}		
	}	
	
	void BuyTen(Widget src, float value = 1, EActionTrigger reason = EActionTrigger.DOWN)
	{
		Buy(10);
	}
	
	void BuyHundred(Widget src, float value = 1, EActionTrigger reason = EActionTrigger.DOWN)
	{
		Buy(100);
	}
	
	void ~OVT_PortContext()
	{
		if(!m_Economy) return;
		m_Economy.m_OnPlayerMoneyChanged.Remove(OnPlayerMoneyChanged);
	}
}