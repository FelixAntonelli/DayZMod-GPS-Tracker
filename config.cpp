class CfgPatches
{
	class Tracker
	{
		requiredAddons[]=
		{
			"DZ_Data",
			"DZ_Scripts"
		};
	};
};
class CfgMods
{
	class Tracker
	{
		dir="Tracker";
		picture=""; 
		action="";
		hideName=1;
		hidePicture=1;
		name="Tracker";
		credits=""; 
		author="Bloodshot_VP";
		authorID="0";  
		version="1.0";
		extra=0;
		type="mod";
		dependencies[]=
		{
			"Game",
			"World",
			"Mission"
		};
		class defs
		{
			class gameScriptModule
			{
				value="";
				files[]=
				{
					"Tracker/scripts/3_game"
				};
			};
			class worldScriptModule
			{
				value="";
				files[]=
				{
					"Tracker/scripts/4_world"
				};
			};
			class missionScriptModule
			{
				value="";
				files[]=
				{
					"Tracker/scripts/5_mission"
				};
			};
		};
	};
};
class CfgVehicles
{
	class Inventory_Base;
	class TrackerReceiver: Inventory_Base
	{
		scope = 2;
		displayName = "Tracker Receiver";
		descriptionShort = "A handheld GPS device used to track the location of escapies.";
		model = "\Tracker\Model\bvp_tracker.p3d";
		itemSize[] = {1, 2};
		rotationFlags = 17;
		class EnergyManager
		{
			hasIcon=1;
			autoSwitchOffWhenInCargo=1;
			energyUsagePerSecond=0.0099999998;
			plugType=1;
			attachmentAction=1;
			updateInterval=3;
		};
		inventorySlot[]=
		{
			"WalkieTalkie",
			"Chemlight",
			"TrackerReceiver"
		};
		attachments[]=
		{
			"BatteryD"
		};
		hiddenSelections[] =
		{
			"Device",
			"Screen"
		};
		hiddenSelectionsTextures[] =
		{
			"\Tracker\Data\device_co.paa",
    		"\Tracker\Data\screen_default_CO.paa"
		};
		class InventorySlotsOffsets
		{
			class Chemlight
			{
				position[]={0,0,0};
				orientation[]={90,10,0};
			};
			// class Hands
            // {
            //     position[]={0,-0.02,0};
            //     orientation[]={-65,-20,0};
            // };
		};
	};
};