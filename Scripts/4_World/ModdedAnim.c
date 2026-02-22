modded class ModItemRegisterCallbacks
{
    override void RegisterOneHanded(DayZPlayerType pType, DayzPlayerItemBehaviorCfg pBehavior)
    {
        super.RegisterOneHanded(pType, pBehavior);
		DayzPlayerItemBehaviorCfg toolsOneHanded = new DayzPlayerItemBehaviorCfg;
		toolsOneHanded.SetToolsOneHanded();
		pType.AddItemInHandsProfileIK("TrackerReceiver", "dz/anims/workspaces/player/player_main/props/player_main_1h_torch.asi",			toolsOneHanded,				"dz/anims/anm/player/ik/gear/chemlight.anm");
	}
};