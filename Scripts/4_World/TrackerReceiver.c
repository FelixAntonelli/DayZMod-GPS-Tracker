class TrackerReceiver extends Inventory_Base
{
    const string TEXTURE_SCREEN_OFF = "\\Tracker\\Data\\screen_off_co.paa";
    const string TEXTURE_SCREEN_ON  = "\\Tracker\\Data\\screen_on_co.paa";
	const string TEXTURE_SCREEN_PLAYER_50 = "\\Tracker\\Data\\screen_distance_50_CO.paa";
	const string TEXTURE_SCREEN_PLAYER_200 = "\\Tracker\\Data\\screen_distance_200_CO.paa";
	const string TEXTURE_SCREEN_PLAYER_600 = "\\Tracker\\Data\\screen_distance_600_CO.paa";
	const string TEXTURE_SCREEN_PLAYER_1000 = "\\Tracker\\Data\\screen_distance_1000_CO.paa";
    const string SELECTION_NAME_SCREEN = "Screen";

    protected const float DETECTION_RADIUS_ENTER = 1000.0;
    protected const float DETECTION_RADIUS_EXIT = 1000.0;

    protected bool m_TargetFound;
	protected bool m_InitialSearch = false;
    protected bool m_TrackerIsTracking;
	
	protected float m_TargetDistance = DETECTION_RADIUS_EXIT;

    void TrackerReceiver()
    {
        RegisterNetSyncVariableBool("m_TargetFound");
		RegisterNetSyncVariableFloat("m_TargetDistance");
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
        if (m_TrackerIsTracking) return;
        
		m_InitialSearch = true;
		
        UpdateTrackingStatus();

        // Updated every 5 seconds, can change if we need to 
        m_TrackerIsTracking = true;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.TrackingTick, 5000, true);
    }

    protected void StopTracking()
    {
        if (!g_Game.IsServer()) return;
        if (!m_TrackerIsTracking) return;
        
        m_TrackerIsTracking = false;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.TrackingTick);
        
        // Reset when stopping
        if (m_TargetFound)
        {
            m_TargetFound = false;
			m_TargetDistance = DETECTION_RADIUS_EXIT;
            SetSynchDirty();
        }
    }

    protected void TrackingTick()
    {
        if (!this || !IsTurnedOn())
        { 
            StopTracking(); 
            return; 
        }
        UpdateTrackingStatus();
    };
	
	protected array<Param2<float, float>> FindTargets(array<Man> targets, array<TrackerReceiver> trackers)
	{
		map<TrackerReceiver, Param2<float, float>> returnMap = new map<TrackerReceiver, Param2<float, float>>();
		foreach(TrackerReceiver tracker : trackers)
		{
			float currentDistance = FLT_MAX;
			float angle = 0.0;
			vector trackerPos = tracker.GetCarrier().GetPosition();
			foreach(Man target : targets)
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
			
			
			returnMap.Insert(tracker, new Param2<float, float>(currentDistance, angle));
		}
		return returnMap;
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