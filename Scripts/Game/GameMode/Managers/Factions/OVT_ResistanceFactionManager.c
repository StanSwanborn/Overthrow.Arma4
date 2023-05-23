class OVT_ResistanceFactionManagerClass: OVT_ComponentClass
{	
}

class OVT_FOBData
{
	int id;
	int faction;	
	vector location;
	EntityID entId;
	ref array<ref EntityID> garrison = {};
	
	bool IsOccupyingFaction()
	{
		return faction == OVT_Global.GetConfig().GetOccupyingFactionIndex();
	}
}

class OVT_Placeable : ScriptAndConfig
{
	[Attribute()]
	string name;
		
	[Attribute(uiwidget: UIWidgets.ResourceAssignArray, desc: "Object Prefabs", params: "et")]
	ref array<ResourceName> m_aPrefabs;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "edds")]
	ResourceName m_tPreview;
	
	[Attribute(defvalue: "100", desc: "Cost (multiplied by difficulty)")]
	int m_iCost;
	
	[Attribute(defvalue: "0", desc: "Place on walls")]
	bool m_bPlaceOnWall;
	
	[Attribute(defvalue: "0", desc: "Can place it anywhere")]
	bool m_bIgnoreLocation;
	
	[Attribute(defvalue: "0", desc: "Cannot place near towns or bases")]
	bool m_bAwayFromTownsBases;
	
	[Attribute(defvalue: "0", desc: "Must be placed near a town")]
	bool m_bNearTown;
	
	[Attribute("", UIWidgets.Object)]
	ref OVT_PlaceableHandler handler;
}

class OVT_Buildable : ScriptAndConfig
{
	[Attribute()]
	string name;
		
	[Attribute(uiwidget: UIWidgets.ResourceAssignArray, desc: "Structure Prefabs", params: "et")]
	ref array<ResourceName> m_aPrefabs;
	
	[Attribute(uiwidget: UIWidgets.ResourceAssignArray, desc: "Furniture Prefabs", params: "et")]
	ref array<ResourceName> m_aFurniturePrefabs;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "edds")]
	ResourceName m_tPreview;
	
	[Attribute(defvalue: "100", desc: "Cost (multiplied by difficulty)")]
	int m_iCost;
	
	[Attribute(defvalue: "0", desc: "Can build at a base")]
	bool m_bBuildAtBase;
	
	[Attribute(defvalue: "0", desc: "Can build in a town")]
	bool m_bBuildInTown;
	
	[Attribute(defvalue: "0", desc: "Can build in a village")]
	bool m_bBuildInVillage;
	
	[Attribute(defvalue: "0", desc: "Can build at an FOB")]
	bool m_bBuildAtFOB;
	
	[Attribute("", UIWidgets.Object)]
	ref OVT_PlaceableHandler handler;
}

class OVT_VehicleUpgrades : ScriptAndConfig
{
	[Attribute()]
	ResourceName m_pBasePrefab;
	
	[Attribute("", UIWidgets.Object)]
	ref array<ref OVT_VehicleUpgrade> m_aUpgrades;	
}

class OVT_VehicleUpgrade : ScriptAndConfig
{
	[Attribute()]
	ResourceName m_pUpgradePrefab;
	
	[Attribute(defvalue: "100", desc: "Cost (multiplied by difficulty)")]
	int m_iCost;
}

class OVT_ResistanceFactionManager: OVT_Component
{	
	[Attribute("", UIWidgets.Object)]
	ref array<ref OVT_Placeable> m_aPlaceables;
	
	[Attribute("", UIWidgets.Object)]
	ref array<ref OVT_Buildable> m_aBuildables;
	
	[Attribute("", UIWidgets.Object)]
	ref array<ref OVT_VehicleUpgrades> m_aVehicleUpgrades;
	
	[Attribute("", UIWidgets.Object)]
	ResourceName m_pHiredCivilianPrefab;
	
	ref array<ref OVT_FOBData> m_FOBs;
	ref array<EntityID> m_Placed;
	ref array<EntityID> m_Built;
	
	ref array<string> m_Officers;
	ref map<ref string,ref vector> m_mCamps;
	
	OVT_PlayerManagerComponent m_Players;
	
	protected IEntity m_TempVehicle;
	protected SCR_AIGroup m_TempGroup;
	
	OVT_ResistanceFactionStruct m_LoadStruct;
	
	static OVT_ResistanceFactionManager s_Instance;
	
	static OVT_ResistanceFactionManager GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode pGameMode = GetGame().GetGameMode();
			if (pGameMode)
				s_Instance = OVT_ResistanceFactionManager.Cast(pGameMode.FindComponent(OVT_ResistanceFactionManager));
		}

		return s_Instance;
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_Players = OVT_Global.GetPlayers();
	}
	
	void OVT_ResistanceFactionManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_FOBs = new array<ref OVT_FOBData>;	
		m_Placed = new array<EntityID>;	
		m_Built = new array<EntityID>;	
		m_Officers = new array<string>;
		m_mCamps = new map<ref string,ref vector>;
	}
	
	void Init(IEntity owner)
	{
		//Register vehicle upgrade resources with the economy
		OVT_EconomyManagerComponent economy = OVT_Global.GetEconomy();
		foreach(OVT_VehicleUpgrades upgrades : m_aVehicleUpgrades)
		{
			foreach(OVT_VehicleUpgrade upgrade : upgrades.m_aUpgrades)
			{
				economy.RegisterResource(upgrade.m_pUpgradePrefab);				
			}
		}
	}
	
	void PostGameStart()
	{
		GetGame().GetCallqueue().CallLater(SpawnFOBs, 0);
	}
	
	protected void SpawnFOBs()
	{
		if(!m_LoadStruct) return;
		
		OVT_Faction resistance = m_Config.GetPlayerFaction();
		
		foreach(OVT_FOBStruct struct : m_LoadStruct.fobs)
		{
			OVT_FOBData fob = new OVT_FOBData;
			fob.location = struct.pos;
			fob.faction = struct.faction;
			if(!fob.IsOccupyingFaction())
			{
				foreach(int res : struct.garrison)
				{
					ResourceName resource = m_LoadStruct.rdb[res];
					int prefabIndex = resistance.m_aGroupPrefabSlots.Find(resource);
					if(prefabIndex > -1)
					{
						AddGarrisonFOB(fob, prefabIndex,false);
					}							
				}
				Rpc(RpcDo_RegisterFOB, struct.pos);
			}
			m_FOBs.Insert(fob);			
		}
	}
	
	bool IsOfficer(int playerId)
	{
		string persId = OVT_Global.GetPlayers().GetPersistentIDFromPlayerID(playerId);
		return m_Officers.Contains(persId);
	}
	
	bool IsLocalPlayerOfficer()
	{
		return IsOfficer(SCR_PlayerController.GetLocalPlayerId());
	}
	
	void AddOfficer(int playerId)
	{
		RpcDo_AddOfficer(playerId);
		Rpc(RpcDo_AddOfficer, playerId);
	}
	
	IEntity PlaceItem(int placeableIndex, int prefabIndex, vector pos, vector angles, int playerId, bool runHandler = true)
	{
		OVT_ResistanceFactionManager config = OVT_Global.GetResistanceFaction();
		OVT_Placeable placeable = config.m_aPlaceables[placeableIndex];
		ResourceName res = placeable.m_aPrefabs[prefabIndex];
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		vector mat[4];
		Math3D.AnglesToMatrix(angles, mat);
		mat[3] = pos;
		params.Transform = mat;
		
		IEntity entity = GetGame().SpawnEntityPrefab(Resource.Load(res), GetGame().GetWorld(), params);
		
		if(placeable.handler && runHandler)
		{
			placeable.handler.OnPlace(entity, playerId);
		}
		
		RegisterPlaceable(entity);
		
		return entity;
	}
	
	IEntity BuildItem(int buildableIndex, int prefabIndex, vector pos, vector angles, int playerId, bool runHandler = true)
	{
		OVT_ResistanceFactionManager config = OVT_Global.GetResistanceFaction();
		OVT_Buildable buildable = config.m_aBuildables[buildableIndex];
		ResourceName res = buildable.m_aPrefabs[prefabIndex];
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		vector mat[4];
		Math3D.AnglesToMatrix(angles, mat);
		mat[3] = pos;
		params.Transform = mat;
		
		IEntity entity = GetGame().SpawnEntityPrefab(Resource.Load(res), GetGame().GetWorld(), params);
		
		if(buildable.handler && runHandler)
		{
			buildable.handler.OnPlace(entity, playerId);
		}
		
		RegisterBuildable(entity);
		
		return entity;
	}
	
	void AddGarrison(int baseId, int prefabIndex, bool takeSupporters = true)
	{
		OVT_BaseData base = OVT_Global.GetOccupyingFaction().m_Bases[baseId];
		OVT_Faction faction = m_Config.GetPlayerFaction();
		ResourceName res = faction.m_aGroupPrefabSlots[prefabIndex];
				
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = base.location;
		
		IEntity entity = GetGame().SpawnEntityPrefab(Resource.Load(res), GetGame().GetWorld(), params);
		SCR_AIGroup group = SCR_AIGroup.Cast(entity);
		AddPatrolWaypoints(group, base);
			
		base.garrison.Insert(entity.GetID());	
		
		if(takeSupporters)
		{
			OVT_Global.GetTowns().TakeSupportersFromNearestTown(base.location, group.m_aUnitPrefabSlots.Count());
		}
	}
	
	void AddGarrisonFOB(OVT_FOBData fob, int prefabIndex, bool takeSupporters = true)
	{		
		OVT_Faction faction = m_Config.GetPlayerFaction();
		ResourceName res = faction.m_aGroupPrefabSlots[prefabIndex];
				
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = fob.location + "1 0 0";
		
		IEntity entity = GetGame().SpawnEntityPrefab(Resource.Load(res), GetGame().GetWorld(), params);
		SCR_AIGroup group = SCR_AIGroup.Cast(entity);
		fob.garrison.Insert(entity.GetID());
		
		AIWaypoint wp = m_Config.SpawnDefendWaypoint(fob.location);
		group.AddWaypoint(wp);	
		
		if(takeSupporters)
		{
			OVT_Global.GetTowns().TakeSupportersFromNearestTown(fob.location, group.m_aUnitPrefabSlots.Count());
		}
	}
	
	protected void AddPatrolWaypoints(SCR_AIGroup aigroup, OVT_BaseData base)
	{
		OVT_BaseControllerComponent controller = OVT_Global.GetOccupyingFaction().GetBase(base.entId);
		array<AIWaypoint> queueOfWaypoints = new array<AIWaypoint>();
		
		if(controller.m_AllCloseSlots.Count() > 2)
		{
			AIWaypoint firstWP;
			for(int i=0; i< 3; i++)
			{
				IEntity randomSlot = GetGame().GetWorld().FindEntityByID(controller.m_AllCloseSlots.GetRandomElement());
				AIWaypoint wp = m_Config.SpawnPatrolWaypoint(randomSlot.GetOrigin());
				if(i==0) firstWP = wp;
				queueOfWaypoints.Insert(wp);
				
				AIWaypoint wait = m_Config.SpawnWaitWaypoint(randomSlot.GetOrigin(), s_AIRandomGenerator.RandFloatXY(45, 75));								
				queueOfWaypoints.Insert(wait);
			}
			AIWaypointCycle cycle = AIWaypointCycle.Cast(m_Config.SpawnWaypoint(m_Config.m_pCycleWaypointPrefab, firstWP.GetOrigin()));
			cycle.SetWaypoints(queueOfWaypoints);
			cycle.SetRerunCounter(-1);
			aigroup.AddWaypoint(cycle);
		}
	}
	
	void RegisterPlaceable(IEntity ent)
	{
		m_Placed.Insert(ent.GetID());
	}
	
	void RegisterBuildable(IEntity ent)
	{
		m_Built.Insert(ent.GetID());
	}
	
	void RegisterFOB(IEntity ent, int playerId)
	{	
		vector pos = ent.GetOrigin();	
		OVT_FOBData fob = new OVT_FOBData;
		fob.faction = OVT_Global.GetConfig().GetPlayerFactionIndex();
		fob.location = pos;
		m_FOBs.Insert(fob);
				
		Rpc(RpcDo_RegisterFOB, pos);
		m_Players.HintMessageAll("PlacedFOB",-1,playerId);
	}
	
	void RegisterCamp(IEntity ent, int playerId)
	{
		string persId = OVT_Global.GetPlayers().GetPersistentIDFromPlayerID(playerId);
		vector pos = ent.GetOrigin();
		if(m_mCamps.Contains(persId))
		{
			GetGame().GetWorld().QueryEntitiesBySphere(m_mCamps[persId], 15, null, FindAndDeleteCamps, EQueryEntitiesFlags.ALL);
		}
		m_mCamps[persId] = pos;
		
		OVT_PlayerData player = OVT_Global.GetPlayers().GetPlayer(persId);
		if(player)
		{
			player.camp = pos;
		}
				
		Rpc(RpcDo_RegisterCamp, pos, playerId);
	}
	
	protected bool FindAndDeleteCamps(IEntity ent)
	{
		string res = ent.GetPrefabData().GetPrefabName();
		if(res.Contains("TentSmallUS"))
		{
			SCR_EntityHelper.DeleteEntityAndChildren(ent);
		}
		return false;
	}
	
	float DistanceToNearestCamp(vector pos)
	{
		float nearest = -1;
		for(int i =0; i<m_mCamps.Count(); i++)
		{
			float dist = vector.Distance(pos, m_mCamps.GetElement(i));
			if(nearest == -1 || dist < nearest) nearest = dist;
		}
		return nearest;
	}
	
	vector GetNearestFOB(vector pos)
	{
		vector nearestBase;
		float nearest = -1;
		foreach(OVT_FOBData fob : m_FOBs)
		{
			if(fob.IsOccupyingFaction()) continue;
			float distance = vector.Distance(fob.location, pos);
			if(nearest == -1 || distance < nearest){
				nearest = distance;
				nearestBase = fob.location;
			}
		}
		return nearestBase;
	}
	
	OVT_FOBData GetNearestFOBData(vector pos)
	{
		OVT_FOBData nearestBase;
		float nearest = -1;
		foreach(OVT_FOBData fob : m_FOBs)
		{
			if(fob.IsOccupyingFaction()) continue;
			float distance = vector.Distance(fob.location, pos);
			if(nearest == -1 || distance < nearest){
				nearest = distance;
				nearestBase = fob;
			}
		}
		return nearestBase;
	}
	
	protected void MoveInGunner()
	{
		array<AIAgent> agents = {};
		m_TempGroup.GetAgents(agents);
		if(agents.Count() == 0) return;
		
		AIAgent dude = agents[0];
		IEntity ent = dude.GetControlledEntity();
		
		SCR_CompartmentAccessComponent access = SCR_CompartmentAccessComponent.Cast(ent.FindComponent(SCR_CompartmentAccessComponent));
		if(!access) return;
		
		access.MoveInVehicle(m_TempVehicle, ECompartmentType.Turret);
	}
	
	void SpawnGunner(RplId turret, int playerId = -1, bool takeSupporter = true)
	{
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(turret));
		if(!rpl) return;
		
		IEntity turretEntity = rpl.GetEntity();	
		IEntity vehicle = turretEntity.GetParent();
		if(!vehicle) return;		
				
		IEntity group = GetGame().SpawnEntityPrefab(Resource.Load(m_pHiredCivilianPrefab));
		SCR_AIGroup aigroup = SCR_AIGroup.Cast(group);
		if(!aigroup) return;
		
		m_TempVehicle = vehicle;
		m_TempGroup = aigroup;
		
		GetGame().GetCallqueue().CallLater(MoveInGunner, 5);
		
		if(takeSupporter)
		{
			OVT_Global.GetTowns().TakeSupportersFromNearestTown(turretEntity.GetOrigin());
		}		
	}
	
	//RPC Methods	
	override bool RplSave(ScriptBitWriter writer)
	{	
			
		//Send JIP FOBs
		writer.WriteInt(m_FOBs.Count()); 
		for(int i=0; i<m_FOBs.Count(); i++)
		{
			OVT_FOBData fob = m_FOBs[i];
			writer.Write(fob.faction, 32);
			writer.WriteVector(fob.location);
		}
		
		//Send JIP officers
		writer.WriteInt(m_Officers.Count()); 
		for(int i=0; i<m_Officers.Count(); i++)
		{
			writer.WriteString(m_Officers[i]);			
		}
		
		//Send JIP Camps
		writer.WriteInt(m_mCamps.Count()); 
		for(int i=0; i<m_mCamps.Count(); i++)
		{
			writer.WriteString(m_mCamps.GetKey(i));
			writer.WriteVector(m_mCamps.GetElement(i));
		}
		
		return true;
	}
	
	override bool RplLoad(ScriptBitReader reader)
	{		
				
		//Recieve JIP FOBs
		int length;
		string id;
		vector pos;
		
		if (!reader.ReadInt(length)) return false;
		for(int i=0; i<length; i++)
		{			
			OVT_FOBData fob = new OVT_FOBData;			
			if (!reader.Read(fob.faction, 32)) return false;
			if (!reader.ReadVector(fob.location)) return false;
			m_FOBs.Insert(fob);
		}
		
		//Recieve JIP Officers
		if (!reader.ReadInt(length)) return false;
		for(int i=0; i<length; i++)
		{			
			if (!reader.ReadString(id)) return false;
			m_Officers.Insert(id);
		}
		
		//Recieve JIP Camps
		if (!reader.ReadInt(length)) return false;
		for(int i=0; i<length; i++)
		{			
			if (!reader.ReadString(id)) return false;
			if (!reader.ReadVector(pos)) return false;
			m_mCamps[id] = pos;
		}
		return true;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_RegisterFOB(vector pos)
	{
		OVT_FOBData fob = new OVT_FOBData;
		fob.location = pos;
		fob.faction = m_Config.GetPlayerFactionIndex();
		m_FOBs.Insert(fob);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_RegisterCamp(vector pos, int playerId)
	{
		string persId = OVT_Global.GetPlayers().GetPersistentIDFromPlayerID(playerId);
		m_mCamps[persId] = pos;
		OVT_PlayerData player = OVT_Global.GetPlayers().GetPlayer(persId);
		if(player)
		{
			player.camp = pos;
		}
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_AddOfficer(int playerId)
	{
		string persId = OVT_Global.GetPlayers().GetPersistentIDFromPlayerID(playerId);
		if(m_Officers.Contains(persId)) return;
		
		m_Officers.Insert(persId);
		if(playerId == SCR_PlayerController.GetLocalPlayerId())
		{
			SCR_HintManagerComponent.GetInstance().ShowCustom("#OVT-NewOfficerYou", "", 10, true);
		}else{
			string playerName = GetGame().GetPlayerManager().GetPlayerName(playerId);
			SCR_HintManagerComponent.GetInstance().ShowCustom(playerName + " #OVT-NewOfficer", "", 10, true);
		}
	}
	
	void ~OVT_ResistanceFactionManager()
	{		
		if(m_FOBs)
		{
			m_FOBs.Clear();
			m_FOBs = null;
		}			
	}
}