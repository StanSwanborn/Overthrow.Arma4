[EPF_ComponentSaveDataType(OVT_ResistanceFactionManager), BaseContainerProps()]
class OVT_ResistanceSaveDataClass : EPF_ComponentSaveDataClass
{
};

[EDF_DbName.Automatic()]
class OVT_ResistanceSaveData : EPF_ComponentSaveData
{
	ref array<ref OVT_FOBData> m_FOBs;
	string m_sPlayerFactionKey;
	
	override EPF_EReadResult ReadFrom(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{		
		OVT_ResistanceFactionManager resistance = OVT_ResistanceFactionManager.Cast(component);
		
		m_FOBs = new array<ref OVT_FOBData>;
		m_sPlayerFactionKey = OVT_Global.GetConfig().m_sPlayerFaction;
		
		foreach(OVT_FOBData fob : resistance.m_FOBs)
		{
			fob.garrison.Clear();
			foreach(EntityID id : fob.garrisonEntities)
			{
				SCR_AIGroup aigroup = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByID(id));
				if(!aigroup) continue;
				if(aigroup.GetAgentsCount() > 0)
				{
					ResourceName res = EPF_Utils.GetPrefabName(aigroup);
					fob.garrison.Insert(res);					
				}
			}	
			m_FOBs.Insert(fob);
		}
		
		return EPF_EReadResult.OK;
	}
	
	override EPF_EApplyResult ApplyTo(IEntity owner, GenericComponent component, EPF_ComponentSaveDataClass attributes)
	{
		OVT_ResistanceFactionManager resistance = OVT_ResistanceFactionManager.Cast(component);
				
		OVT_Global.GetConfig().m_sPlayerFaction = m_sPlayerFactionKey;
		OVT_Global.GetConfig().m_iPlayerFactionIndex = GetGame().GetFactionManager().GetFactionIndex(GetGame().GetFactionManager().GetFactionByKey(m_sPlayerFactionKey));
		
		foreach(OVT_FOBData fob : m_FOBs)
		{	
			fob.id = resistance.m_FOBs.Count();		
			resistance.m_FOBs.Insert(fob);						
		}
				
		return EPF_EApplyResult.OK;
	}
}