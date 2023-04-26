class OVT_RealEstateManagerComponentClass: OVT_OwnerManagerComponentClass
{
};

class OVT_RealEstateManagerComponent: OVT_OwnerManagerComponent
{
	ref map<string, ref vector> m_mHomes;
	
	protected OVT_TownManagerComponent m_Town;
	
	static OVT_RealEstateManagerComponent s_Instance;
	
	protected ref array<IEntity> m_aEntitySearch;
	
	static OVT_RealEstateManagerComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode pGameMode = GetGame().GetGameMode();
			if (pGameMode)
				s_Instance = OVT_RealEstateManagerComponent.Cast(pGameMode.FindComponent(OVT_RealEstateManagerComponent));
		}

		return s_Instance;
	}
	
	void OVT_RealEstateManagerComponent()
	{
		m_mHomes = new map<string, ref vector>;
		m_aEntitySearch = new array<IEntity>;
	}
	
	override void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		
		if (SCR_Global.IsEditMode()) return;		
		
		m_Town = OVT_TownManagerComponent.Cast(GetOwner().FindComponent(OVT_TownManagerComponent));	
	}
	
	void SetHome(int playerId, IEntity building)
	{	
		DoSetHome(playerId, building.GetOrigin());
		Rpc(RpcDo_SetHome, playerId, building.GetOrigin());
	}
	
	void SetHomePos(int playerId, vector pos)
	{	
		DoSetHome(playerId, pos);
		Rpc(RpcDo_SetHome, playerId, pos);
	}
	
	bool IsHome(string playerId, EntityID entityId)
	{
		IEntity building = GetGame().GetWorld().FindEntityByID(entityId);
		float dist = vector.Distance(building.GetOrigin(), m_mHomes[playerId]);
		return dist < 1;
	}
	
	IEntity GetNearestOwned(string playerId, vector pos)
	{
		if(!m_mOwned.Contains(playerId)) return null;
		
		float nearest = 999999;
		IEntity nearestEnt;		
		
		set<RplId> owner = m_mOwned[playerId];
		foreach(RplId id : owner)
		{
			RplComponent rpl = RplComponent.Cast(Replication.FindItem(id));
			IEntity ent = rpl.GetEntity();
			float dist = vector.Distance(ent.GetOrigin(), pos);
			if(dist < nearest)
			{
				nearest = dist;
				nearestEnt = ent;
			}
		}
		
		return nearestEnt;
	}
	
	IEntity GetNearestRented(string playerId, vector pos)
	{
		if(!m_mRented.Contains(playerId)) return null;
		
		float nearest = 999999;
		IEntity nearestEnt;		
		
		set<RplId> owner = m_mRented[playerId];
		foreach(RplId id : owner)
		{
			RplComponent rpl = RplComponent.Cast(Replication.FindItem(id));
			IEntity ent = rpl.GetEntity();
			float dist = vector.Distance(ent.GetOrigin(), pos);
			if(dist < nearest)
			{
				nearest = dist;
				nearestEnt = ent;
			}
		}
		
		return nearestEnt;
	}
	
	IEntity GetNearestBuilding(vector pos, float range = 40)
	{
		m_aEntitySearch.Clear();
		GetGame().GetWorld().QueryEntitiesBySphere(pos, range, null, FilterBuildingToArray, EQueryEntitiesFlags.STATIC);
		
		if(m_aEntitySearch.Count() == 0)
		{
			return null;
		}
		float nearest = range;
		IEntity nearestEnt;	
		
		foreach(IEntity ent : m_aEntitySearch)
		{
			float dist = vector.Distance(ent.GetOrigin(), pos);
			if(dist < nearest)
			{
				nearest = dist;
				nearestEnt = ent;
			}
		}
		return nearestEnt;
	}
	
	bool BuildingIsOwnable(IEntity entity)
	{
		if(!entity) return false;
		if(entity.ClassName() != "SCR_DestructibleBuildingEntity")
		{
			return false;
		}
		
		ResourceName res = entity.GetPrefabData().GetPrefabName();
		foreach(OVT_RealEstateConfig config : m_Config.m_aBuildingTypes)
		{
			foreach(ResourceName s : config.m_aResourceNameFilters)
			{
				if(res.IndexOf(s) > -1) return true;
			}
		}
		return false;
	}
	
	OVT_RealEstateConfig GetConfig(IEntity entity)
	{
		if(!entity) return null;
		if(entity.ClassName() != "SCR_DestructibleBuildingEntity")
		{
			return null;
		}
		
		ResourceName res = entity.GetPrefabData().GetPrefabName();
		foreach(OVT_RealEstateConfig config : m_Config.m_aBuildingTypes)
		{
			foreach(ResourceName s : config.m_aResourceNameFilters)
			{
				if(res.IndexOf(s) > -1) return config;
			}
		}
		return null;
	}
	
	int GetBuyPrice(IEntity entity)
	{
		OVT_RealEstateConfig config = GetConfig(entity);
		if(!config) return 0;
		
		OVT_TownData town = m_Town.GetNearestTown(entity.GetOrigin());
		
		return config.m_BasePrice + (config.m_BasePrice * (config.m_DemandMultiplier * town.population * ((float)town.stability / 100)));
	}
	
	int GetRentPrice(IEntity entity)
	{
		OVT_RealEstateConfig config = GetConfig(entity);
		if(!config) return 0;
		
		OVT_TownData town = m_Town.GetNearestTown(entity.GetOrigin());
		
		return config.m_BaseRent + (config.m_BaseRent * (config.m_DemandMultiplier * town.population * ((float)town.stability / 100)));
	}
	
	bool FilterBuildingToArray(IEntity entity)
	{
		if(entity.ClassName() == "SCR_DestructibleBuildingEntity")
		{
			m_aEntitySearch.Insert(entity);
		}
		return false;
	}
	
	vector GetHome(string playerId)
	{				
		if(m_mHomes.Contains(playerId)) return m_mHomes[playerId];
		
		return "0 0 0";
	}
	
	void TeleportHome(int playerId)
	{
		RpcDo_TeleportHome(playerId);
		Rpc(RpcDo_TeleportHome, playerId);
	}
	
	//RPC Methods
	
	override bool RplSave(ScriptBitWriter writer)
	{
		super.RplSave(writer);
		
		//Send JIP homes
		writer.Write(m_mHomes.Count(), 32); 
		for(int i; i<m_mHomes.Count(); i++)
		{			
			RPL_WritePlayerID(writer, m_mHomes.GetKey(i));
			writer.WriteVector(m_mHomes.GetElement(i));
		}
		
		return true;
	}
	
	override bool RplLoad(ScriptBitReader reader)
	{
		if(!super.RplLoad(reader)) return false;
		
		//Recieve JIP homes
		int length, ownedlength;
		string playerId;
		vector loc;
		
		if (!reader.Read(length, 32)) return false;
		for(int i; i<length; i++)
		{
			if (!RPL_ReadPlayerID(reader, playerId)) return false;
			if (!reader.ReadVector(loc)) return false;		
			
			m_mHomes[playerId] = loc;
		}
		return true;
	}	

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetHome(int playerId, vector loc)
	{
		DoSetHome(playerId, loc);	
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_TeleportHome(int playerId)
	{
		int localId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(SCR_PlayerController.GetLocalControlledEntity());
		if(playerId != localId) return;
		
		string persId = OVT_Global.GetPlayers().GetPersistentIDFromPlayerID(playerId);
		vector spawn = OVT_Global.FindSafeSpawnPosition(m_mHomes[persId]);
		SCR_Global.TeleportPlayer(spawn);
	}
	
	void DoSetHome(int playerId, vector loc)
	{
		string persId = OVT_Global.GetPlayers().GetPersistentIDFromPlayerID(playerId);
		m_mHomes[persId] = loc;
	}
	
	void ~OVT_RealEstateManagerComponent()
	{
		if(m_mHomes)
		{
			m_mHomes.Clear();
			m_mHomes = null;
		}
		if(m_aEntitySearch)
		{
			m_aEntitySearch.Clear();
			m_aEntitySearch = null;
		}
	}
}