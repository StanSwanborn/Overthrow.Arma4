class OVT_Faction : SCR_Faction
{
	[Attribute(uiwidget: UIWidgets.ResourceAssignArray, desc: "Faction groups (all)", params: "et", category: "Faction Groups")]
	ref array<ResourceName> m_aGroupPrefabSlots;
	
	[Attribute(uiwidget: UIWidgets.ResourceAssignArray, desc: "Faction groups (Light Infantry)", params: "et", category: "Faction Groups")]
	ref array<ResourceName> m_aGroupInfantryPrefabSlots;
	
	[Attribute(uiwidget: UIWidgets.ResourceAssignArray, desc: "Faction groups (Heavy Infantry)", params: "et", category: "Faction Groups")]
	ref array<ResourceName> m_aHeavyInfantryPrefabSlots;
	
	[Attribute(uiwidget: UIWidgets.ResourceAssignArray, desc: "Faction groups (AT)", params: "et", category: "Faction Groups")]
	ref array<ResourceName> m_aGroupATPrefabSlots;
	
	[Attribute(uiwidget: UIWidgets.ResourceAssignArray, desc: "Faction groups (Special Forces)", params: "et", category: "Faction Groups")]
	ref array<ResourceName> m_aGroupSpecialPrefabSlots;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Faction group (Sniper)", params: "et", category: "Faction Groups")]
	ResourceName m_aGroupSniperPrefab;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Faction group (Light Town Patrol)", params: "et", category: "Faction Groups")]
	ResourceName m_aLightTownPatrolPrefab;
	
	[Attribute(uiwidget: UIWidgets.ResourceAssignArray, desc: "Faction vehicles (all)", params: "et", category: "Faction Vehicles")]
	ref array<ResourceName> m_aVehiclePrefabSlots;
	
	[Attribute(uiwidget: UIWidgets.ResourceAssignArray, desc: "Faction vehicles (Unarmed)", params: "et", category: "Faction Vehicles")]
	ref array<ResourceName> m_aVehicleUnarmedPrefabSlots;
	
	[Attribute(uiwidget: UIWidgets.ResourceAssignArray, desc: "Faction vehicles (Light)", params: "et", category: "Faction Vehicles")]
	ref array<ResourceName> m_aVehicleLightPrefabSlots;
	
	[Attribute(uiwidget: UIWidgets.ResourceAssignArray, desc: "Faction vehicles (Heavy)", params: "et", category: "Faction Vehicles")]
	ref array<ResourceName> m_aVehicleHeavyPrefabSlots;
		
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Flag Pole Prefab", params: "et", category: "Faction Objects")]
	ResourceName m_aFlagPolePrefab;
	
}