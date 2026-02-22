class TrackerReceiver extends Inventory_Base
{
    const string TEXTURE_SCREEN_OFF = "\\Tracker\\Data\\screen_off_co.paa";
    const string TEXTURE_SCREEN_ON  = "\\Tracker\\Data\\screen_on_co.paa";
    const string SELECTION_NAME_SCREEN = "Screen";

    protected float DETECTION_RADIUS_ENTER = 80.0;
    protected float DETECTION_RADIUS_EXIT = 100.0;

    protected bool m_TargetFound;
    protected bool m_TrackerIsTracking;

    void BVP_TrackerReceiver()
    {
        RegisterNetSyncVariableBool("m_TargetFound");
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

    protected void UpdateTrackingStatus()
    {
        PlayerBase carrier = PlayerBase.Cast(GetHierarchyRootPlayer());
        vector refPos = GetPosition();
        if (carrier) refPos = carrier.GetPosition();
        
        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);
        
        bool foundNear = false;
        
        float nearSq = DETECTION_RADIUS_ENTER * DETECTION_RADIUS_ENTER;
        if (m_TargetFound) nearSq = DETECTION_RADIUS_EXIT * DETECTION_RADIUS_EXIT;
        
        foreach (Man p : players)
        {
            PlayerBase pb = PlayerBase.Cast(p);
            if (!pb || !pb.IsAlive() || pb == carrier) continue;
            
            float distSq = vector.DistanceSq(refPos, pb.GetPosition());
            if (distSq <= nearSq)
            {
                foundNear = true;
                break;
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
                    float aiDistSq = vector.DistanceSq(refPos, ai.GetPosition());
                    if (aiDistSq <= nearSq)
                    {
                        //Print("[BVP_Tracker] Expansion AI detected: " + ai.GetType() + " at distance " + Math.Sqrt(aiDistSq).ToString() + "m");
                        foundNear = true;
                        break;
                    }
                }
            }
        }
        #endif
        
        if (foundNear != m_TargetFound)
        {
            m_TargetFound = foundNear;
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
        int selectionIdx = GetHiddenSelectionIndex(SELECTION_NAME_SCREEN);
        string texture = GetObjectTexture(selectionIdx);
        if (selectionIdx != -1)
        {
            if (IsTurnedOn())
            {
                SetObjectTexture(selectionIdx, TEXTURE_SCREEN_ON);
            }
            else
            {
                SetObjectTexture(selectionIdx, TEXTURE_SCREEN_OFF);
            }
        }
        
        SetSynchDirty();
    }

    override void OnWorkStart()
    {
        UpdateScreen();
    }
    
    override void OnWorkStop()
    {
        UpdateScreen();
    }
    
    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();
        
        UpdateScreen();
    }
}