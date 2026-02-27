class TrackerManager
{
///------------------------------------------\\\
///				Singleton					 \\\
///------------------------------------------\\\
	protected static ref TrackerManager m_Instance;
	
	static void CreateInstance()
	{
		if(m_Instance != null)
		{
			ErrorEx("Instance already exists!", ErrorExSeverity.WARNING);
			return;
		}
		
		m_Instance = new TrackerManager();
	}
	
	static void DeleteInstance()
	{
		m_Instance = null;
	}
	
	static TrackerManager Instance()
	{
		if(m_Instance == null)
			CreateInstance();
		
		return m_Instance;
	}
///------------------------------------------\\\
	
	void RegisterTracker(TrackerReceiver tracker)
	{
		if(m_Trackers.Count() <= 0)
        	GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TrackerManager.TrackingTick, 5000, true);
		
		int index = m_Trackers.Find(tracker);
		
		if(index != -1)
		{
			ErrorEx("Tracker already registered", ErrorExSeverity.INFO);
			return;
		}
		
		m_Trackers.Insert(tracker);
	}
	
	void UnregisterTracker(TrackerReceiver tracker)
	{
		m_Trackers.RemoveItem(tracker);
		
		if(m_Trackers.Count() <= 0)
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(TrackerManager.TrackingTick);
	}
	
	array<TrackerReceiver> GetActiveTrackers()
	{
		return m_Trackers;
	}
	
	protected ref array<TrackerReceiver> m_Trackers = {};
	
	protected static void TrackingTick()
    {
		ErrorEx("TRACKING TICK", ErrorExSeverity.INFO);
		//NOTE(Felix): Doesn't seem to get Ai players?
		array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);
		
		array<TrackerReceiver> activeTrackers = TrackerManager.Instance().GetActiveTrackers();
		
		map<TrackerReceiver, Param2<float, float>> trackingInfo = FindTargets(players, activeTrackers);
		
		int trackingCount = trackingInfo.Count();
		#ifdef EXPANSIONMODAI
		if( trackingCount > 0 )
		{
		#endif
			for(int i = 0; i < trackingCount; i++)
			{
				TrackerReceiver tracker = trackingInfo.GetKey(i);
				//auto info = trackingInfo.Get(i);
				tracker.SetTrackingInfo(3.0, 4.0);
			}
		
		#ifdef EXPANSIONMODAI
		}
		else if( activeTrackers.Count() > 0 )
		{
			map<TrackerReceiver, Param2<float, float>> aiTrackingInfo = new map<TrackerReceiver, Param2<float, float>>();
			
			array<Object> nearbyObjects = new array<Object>();
           
			const float nearSq = 1000 * 1000;
			foreach (TrackerReceiver currentTracker : activeTrackers)
			{
				float currentDistance = FLT_MAX;
				float angle = 0.0;
				PlayerBase carrier = currentTracker.GetCarrier();
				vector trackerPos = currentTracker.GetCarrier().GetPosition();
				
				GetGame().GetObjectsAtPosition(trackerPos, Math.Sqrt(nearSq), nearbyObjects, null);
				foreach (Object target : nearbyObjects)
				{
					eAIBase targetBase = eAIBase.Cast(target);
					if( targetBase && targetBase.IsAlive() )
					{
						vector targetPos = target.GetPosition();
						float dist = vector.DistanceSq(trackerPos, targetPos);
						
						if( dist < currentDistance )
						{
							currentDistance = dist;
							float dot = vector.Dot(trackerPos, targetPos);
							//x1*y2 - y1*x2 
							float det = (trackerPos[0] * targetPos[1]) - (trackerPos[1] * targetPos[0]);
							angle = Math.Atan2(-det, -dot) + Math.PI;
						}
					}
				}
				
				aiTrackingInfo.Insert(currentTracker, new Param2<float, float>(currentDistance, angle));
			}
			
			for(int j = 0; j < aiTrackingInfo.Count(); i++)
			{
				TrackerReceiver currentTracker2 = aiTrackingInfo.GetKey(j);
				Param2<float, float> aiInfo = aiTrackingInfo.Get(j);
				currentTracker2.SetTrackingInfo(aiInfo.param1, aiInfo.param2);
			}
		}
		#endif
		
		
        //UpdateTrackingStatus();
    };
	
	protected static map<TrackerReceiver, Param2<float, float>> FindTargets(array<Man> targets, array<TrackerReceiver> trackers)
	{
		map<TrackerReceiver, Param2<float, float>> returnMap = new map<TrackerReceiver, Param2<float, float>>();
		foreach (TrackerReceiver tracker : trackers)
		{
			float currentDistance = FLT_MAX;
			float angle = 0.0;
			PlayerBase carrier = tracker.GetCarrier();
			vector trackerPos = tracker.GetCarrier().GetPosition();
			foreach (Man target : targets)
			{
				PlayerBase targetBase = PlayerBase.Cast(target);
				if( !targetBase.IsAlive() || targetBase == carrier ) continue;
				
				#ifdef BVP_HUTNED_TRIGGERS
				if( !GetBVPHuntedTriggersConfig().ShouldTrackPlayer( target.GetIdentity().GetPlainId() ) ) continue;
				#endif
				
				vector targetPos = target.GetPosition();
				float dist = vector.DistanceSq(trackerPos, targetPos);
				
				if( dist < currentDistance )
				{
					currentDistance = dist;
					float dot = vector.Dot(trackerPos, targetPos);
					//x1*y2 - y1*x2 
					float det = (trackerPos[0] * targetPos[1]) - (trackerPos[1] * targetPos[0]);
					angle = Math.Atan2(-det, -dot) + Math.PI;
				}
			}
			
			if(currentDistance < FLT_MAX - FLT_MIN)
			{
				returnMap.Insert(tracker, new Param2<float, float>(currentDistance, angle));
			}
		}
		return returnMap;
	}
	
}

//---Dependecies---\\
#define BVP_HUNTED_TRIGGERS 0
//-----------------\\

class TrackerReceiver extends Inventory_Base
{
    const string TEXTURE_SCREEN_OFF         = "\\Tracker\\Data\\screen_off_co.paa";
    const string TEXTURE_SCREEN_ON          = "\\Tracker\\Data\\screen_on_co.paa";
	const string TEXTURE_SCREEN_PLAYER_50   = "\\Tracker\\Data\\screen_distance_50_CO.paa";
	const string TEXTURE_SCREEN_PLAYER_200  = "\\Tracker\\Data\\screen_distance_200_CO.paa";
	const string TEXTURE_SCREEN_PLAYER_600  = "\\Tracker\\Data\\screen_distance_600_CO.paa";
	const string TEXTURE_SCREEN_PLAYER_1000 = "\\Tracker\\Data\\screen_distance_1000_CO.paa";
    const string SELECTION_NAME_SCREEN      = "Screen";
    const string TEXTURE_SCREEN_OFF         = "\\Tracker\\Data\\screen_off_co.paa";
    const string TEXTURE_SCREEN_ON          = "\\Tracker\\Data\\screen_on_co.paa";
	const string TEXTURE_SCREEN_PLAYER_50   = "\\Tracker\\Data\\screen_distance_50_CO.paa";
	const string TEXTURE_SCREEN_PLAYER_200  = "\\Tracker\\Data\\screen_distance_200_CO.paa";
	const string TEXTURE_SCREEN_PLAYER_600  = "\\Tracker\\Data\\screen_distance_600_CO.paa";
	const string TEXTURE_SCREEN_PLAYER_1000 = "\\Tracker\\Data\\screen_distance_1000_CO.paa";
    const string SELECTION_NAME_SCREEN      = "Screen";

    protected static const float DETECTION_RADIUS_ENTER = 1000.0;
    protected static const float DETECTION_RADIUS_EXIT = 1000.0;
    protected static const float DETECTION_RADIUS_ENTER = 1000.0;
    protected static const float DETECTION_RADIUS_EXIT = 1000.0;

	protected bool m_InitialSearch = false;
	protected bool m_InitialSearch = false;
    protected bool m_TrackerIsTracking;
	
	///---Synced Members---///
    protected bool m_TargetFound = false;
	protected float m_TargetDistance = FLT_MAX;
	protected float m_TargetAngle = -1;

    void TrackerReceiver()
	
	///---Synced Members---///
    protected bool m_TargetFound = false;
	protected float m_TargetDistance = FLT_MAX;
	protected float m_TargetAngle = -1;

    void TrackerReceiver()
    {
        RegisterNetSyncVariableBool("m_TargetFound");
		RegisterNetSyncVariableFloat("m_TargetDistance");
		RegisterNetSyncVariableFloat("m_TargetAngle");
    }

	PlayerBase GetCarrier()
	{
		return PlayerBase.Cast(GetHierarchyRootPlayer());
	}
	
		RegisterNetSyncVariableFloat("m_TargetDistance");
		RegisterNetSyncVariableFloat("m_TargetAngle");
    }

	PlayerBase GetCarrier()
	{
		return PlayerBase.Cast(GetHierarchyRootPlayer());
	}
	
    override void EEInit()
    {
        super.EEInit();
        
        if (g_Game.IsServer())
        {
            if (IsTurnedOn())
            {
                StartTracking();
            }
        }
        
        if (!g_Game.IsDedicatedServer())
        {
            UpdateScreenLocal();
        }
    }

    override void EEDelete(EntityAI parent)
    {
        super.EEDelete(parent);
        
        if (g_Game.IsServer())
        {
            StopTracking();
        }
    }

    protected void StartTracking()
    {
        if (!g_Game.IsServer()) return;
        
        // Updated every 5 seconds, can change if we need to 
        m_TrackerIsTracking = true;
		
		TrackerManager.Instance().RegisterTracker(this);
		
		TrackerManager.Instance().RegisterTracker(this);
    }

    protected void StopTracking()
    {
        if (!g_Game.IsServer())	return;
		
		m_TrackerIsTracking = false;
		
		TrackerManager.Instance().UnregisterTracker(this);
        if (!g_Game.IsServer())	return;
		
		m_TrackerIsTracking = false;
		
		TrackerManager.Instance().UnregisterTracker(this);
        
        // Reset when stopping
        //if (m_TargetFound)
        //{
        //    m_TargetFound = false;
		//	m_TargetDistance = FLT_MAX;
        //    SetSynchDirty();
       // }
        //if (m_TargetFound)
        //{
        //    m_TargetFound = false;
		//	m_TargetDistance = FLT_MAX;
        //    SetSynchDirty();
       // }
    }

    
	
    
	
    protected void UpdateTrackingStatus()
    {
        PlayerBase carrier = PlayerBase.Cast(GetHierarchyRootPlayer());
        vector refPos = GetPosition();
        if (carrier) refPos = carrier.GetPosition();
        
        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);
        
        bool foundNear = false;
		float dist = DETECTION_RADIUS_EXIT;
        
        float nearSq = DETECTION_RADIUS_ENTER * DETECTION_RADIUS_ENTER;
        if (m_TargetFound) nearSq = DETECTION_RADIUS_EXIT * DETECTION_RADIUS_EXIT;
        
        foreach (Man p : players)
        {
            PlayerBase pb = PlayerBase.Cast(p);
            if (!pb || !pb.IsAlive() || pb == carrier) continue;
            
            float distTemp = Math.Sqrt(vector.DistanceSq(refPos, pb.GetPosition()));
            if (distTemp <= dist)
            {
                foundNear = true;
				dist = distTemp;
				
				//if( !m_InitialSearch ) break;
            }
        }
        
        // Added check for expansion ai, may not be needed anymore
        #ifdef EXPANSIONMODAI
        if (!foundNear)
        {
            array<Object> nearbyObjects = new array<Object>();
            GetGame().GetObjectsAtPosition(refPos, Math.Sqrt(nearSq), nearbyObjects, null);
            
            foreach (Object obj : nearbyObjects)
            {
                eAIBase ai = eAIBase.Cast(obj);
                if (ai && ai.IsAlive() && ai != carrier)
                {
                    float aiDistTemp = Math.Sqrt(vector.DistanceSq(refPos, ai.GetPosition()));
                    if (aiDistTemp <= dist)
                    {
                        //Print("[BVP_Tracker] Expansion AI detected: " + ai.GetType() + " at distance " + Math.Sqrt(aiDistSq).ToString() + "m");
                        foundNear = true;
						dist = aiDistTemp;
						
                        //if( !m_InitialSearch ) break;
                    }
                }
            }
        }
        #endif
        
        if (foundNear)
        {
            m_TargetFound = foundNear;
			m_TargetDistance = dist;
			m_InitialSearch = false;
            SetSynchDirty();
        }
    }

	
	
    override void SetActions()
	{
		super.SetActions();
		
		AddAction(ActionTurnOnWhileInHands);
		AddAction(ActionTurnOffWhileInHands);
	}

    bool IsTurnedOn()
	{
		return GetCompEM() && GetCompEM().IsWorking();
	}

	void UpdateScreen()
	{
		UpdateScreenLocal();
        SetSynchDirty();
	}
	
    void UpdateScreenLocal()
    {
        int selectionIdx = GetHiddenSelectionIndex(SELECTION_NAME_SCREEN);

        if (selectionIdx != -1)
        {
            if (IsTurnedOn())
            {
                if (m_TargetFound)
                {
					ErrorEx(string.Format("Angle to target is %f", m_TargetAngle ), ErrorExSeverity.WARNING);
					string textureName = "";
                    if(m_TargetDistance <= 50.0)
					{
						textureName = TEXTURE_SCREEN_PLAYER_50;
					}
					else if(m_TargetDistance > 50.0 && m_TargetDistance <= 200)
					{
						textureName = TEXTURE_SCREEN_PLAYER_200;
					}
					else if(m_TargetDistance > 200.0 && m_TargetDistance <= 600)
					{
						textureName = TEXTURE_SCREEN_PLAYER_600;
					}
					else if(m_TargetDistance > 600.0 && m_TargetDistance <= 1000)
					{
						textureName = TEXTURE_SCREEN_PLAYER_1000;
					}
					else
					{
						textureName = TEXTURE_SCREEN_ON;
					}
					
					SetObjectTexture(selectionIdx, textureName);
                }
                else
                {
                    SetObjectTexture(selectionIdx, TEXTURE_SCREEN_ON);
                }
            {
                if (m_TargetFound)
                {
					ErrorEx(string.Format("Angle to target is %f", m_TargetAngle ), ErrorExSeverity.WARNING);
					string textureName = "";
                    if(m_TargetDistance <= 50.0)
					{
						textureName = TEXTURE_SCREEN_PLAYER_50;
					}
					else if(m_TargetDistance > 50.0 && m_TargetDistance <= 200)
					{
						textureName = TEXTURE_SCREEN_PLAYER_200;
					}
					else if(m_TargetDistance > 200.0 && m_TargetDistance <= 600)
					{
						textureName = TEXTURE_SCREEN_PLAYER_600;
					}
					else if(m_TargetDistance > 600.0 && m_TargetDistance <= 1000)
					{
						textureName = TEXTURE_SCREEN_PLAYER_1000;
					}
					else
					{
						textureName = TEXTURE_SCREEN_ON;
					}
					
					SetObjectTexture(selectionIdx, textureName);
                }
                else
                {
                    SetObjectTexture(selectionIdx, TEXTURE_SCREEN_ON);
                }
            }
            else
            {
                SetObjectTexture(selectionIdx, TEXTURE_SCREEN_OFF);
            }
        }
    }
	
	void SetTrackingInfo(float distance, float angle)
	{
		m_TargetFound = true;
		m_TargetDistance = distance;
		m_TargetAngle = angle;
		
		SetSynchDirty();
	}
    }
	
	void SetTrackingInfo(float distance, float angle)
	{
		m_TargetFound = true;
		m_TargetDistance = distance;
		m_TargetAngle = angle;
		
		SetSynchDirty();
	}

    override void OnWorkStart()
    {
        if (g_Game.IsServer())
        {
            StartTracking();
        }
        
        UpdateScreen();
    }
    
    override void OnWorkStop()
    {
        if (g_Game.IsServer())
        {
            StopTracking();
        }
        
        UpdateScreen();
    }
	
	override void OnWork(float consumed_energy)
	{
		if (g_Game.IsServer())
        {
            return;
        }
        
        UpdateScreenLocal();
	}
    
    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();
        
        UpdateScreenLocal();
    }
}