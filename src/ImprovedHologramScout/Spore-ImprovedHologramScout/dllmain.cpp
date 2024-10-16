// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <Spore\Simulator\cGetOutOfUFOToolStrategy.h>
#include "SpawnAvatarCheat.h"
#include <Spore\GameModes.h>
#include "HologramScoutMod.h"
#include <Spore\Simulator\SubSystem\TerraformingManager.h>
#include <Spore/Editors/BakeManager.h>
#include "Detours.h"
#include "HologramCombatManager.h"
#include <SourceCode/DLL/Application.h>

#include <Spore/App/cGameModeManager.h>

using namespace Simulator;

void Initialize()
{
	// This method is executed when the game starts, before the user interface is shown
	// Here you can do things such as:
	//  - Add new cheats
	//  - Add new simulator classes
	//  - Add new game modes
	//  - Add new space tools
	//  - Change materials
	CheatManager.AddCheat("SpawnAvatar", new SpawnAvatarCheat());

	App::AddUpdateFunction(new HologramScoutMod());

	//UTFWin::UILayout::LoadByID();
	new HologramCombatManager();
}

void Dispose()
{
	// This method is called when the game is closing
}

static_detour(RolloverDetour, UI::SimulatorRollover* (int*, UI::SimulatorRolloverID, Simulator::cObjectPoolIndex))
{
	UI::SimulatorRollover* detoured(int* gamedata, UI::SimulatorRolloverID rolloverID, Simulator::cObjectPoolIndex unk)
	{
		//SporeDebugPrint("%x, %x, %x", gamedata, rolloverID, unk);
		return original_function(gamedata, rolloverID, unk);
	}
};

member_detour(RolloverUIDetour, UTFWin::UILayout, bool(ResourceKey&, bool, uint32_t))
{
	bool detoured(ResourceKey& resourceKey, bool unk, uint32_t param)
	{
		if (resourceKey.instanceID == id("Rollover_Creature") && Simulator::IsSpaceGame())
		{
			resourceKey.instanceID = id("Rollover_CreatureSPG");
		}
		return original_function(this, resourceKey, unk, param);
	}
};


void AttachDetours()
{
	Chocice75_ImprovedHologramScout_OnUseDetour::attach(GetAddress(cGetOutOfUFOToolStrategy, OnSelect));
	SetRenderType__detour::attach(GetAddress(App::cViewer, SetRenderType));
	Chocice75_ImprovedHologramScout_UpdateDetour::attach(GetAddress(cGetOutOfUFOToolStrategy, Update));
	//CreatureBaseDetour::attach(GetAddress(Simulator::cCreatureBase, WalkTo));

	HologramAudioDetour::attach(GetAddress(Audio,PlayProceduralAudio));
	OverrideCreatureDamageDetour::attach(Address(ModAPI::ChooseAddress(0x00bfc500, 0x00bfcf10)));
	PlayAbilityDetour::attach(GetAddress(Simulator::cCreatureBase, PlayAbility));

	//RolloverDetour::attach(GetAddress(UI::SimulatorRollover, ShowRollover));
	RolloverUIDetour::attach(GetAddress(UTFWin::UILayout, Load));
	// Call the attach() method on any detours you want to add
	// For example: cViewer_SetRenderType_detour::attach(GetAddress(cViewer, SetRenderType));
}


// Generally, you don't need to touch any code here
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		ModAPI::AddPostInitFunction(Initialize);
		ModAPI::AddDisposeFunction(Dispose);

		PrepareDetours(hModule);
		AttachDetours();
		CommitDetours();
		break;

	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

