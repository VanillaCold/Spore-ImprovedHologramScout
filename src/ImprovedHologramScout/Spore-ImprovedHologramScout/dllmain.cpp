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
#include <Spore/App/GameSpace.h>

using namespace Simulator;

static bool workaround = false;

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
	(App::cGameModeManager*)(App::IGameModeManager::Get());
	App::AddUpdateFunction(new HologramScoutMod());

	//Simulator::cGameData::SetDefinitionID();

	//UTFWin::UILayout::LoadByID();
	new HologramCombatManager();
}

void Dispose()
{
	// This method is called when the game is closing
}

virtual_detour(GameModeOverride, App::cGameModeManager, App::IGameModeManager, App::IGameMode* ())
{
	App::IGameMode* detoured()
	{
		auto result = original_function(this);
		if (workaround && this->GetGameMode(GameModeIDs::kGameSpace) == result)
		{
			SporeDebugPrint("cool");
			return App::cScenarioMode::Get();
		}
		else
		{
			return result;
		}
	}
};

virtual_detour(GameModeOverrideTwo, App::cGameModeManager, App::IGameModeManager, uint32_t ())
{
	uint32_t detoured()
	{
		auto result = original_function(this);
		if (workaround && GameModeIDs::kGameSpace == result)
		{
			SporeDebugPrint("cool");
			return GameModeIDs::kScenarioMode;
		}
		else
		{
			return result;
		}
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

//FUN_00c3fde0(ResourceKey *param_1,size_t param_2,Property *param_3)
member_detour(TestDetour, Simulator::cGameData, int(ResourceKey*, size_t, App::Property*))
{
	int detoured(ResourceKey* param1, size_t param2, App::Property* param3)
	{
		workaround = true;
		int a = original_function(this, param1, param2, param3);
		workaround = false;
		return a;
	}
};

virtual_detour(TestDetour2, Simulator::cCreatureAnimal, Simulator::cCreatureBase, void(int))
{
	void detoured(int deltaTime)
	{
		if (GameNounManager.GetAvatar() == this)
		{
			workaround = true;
		}
		original_function(this, deltaTime);
		workaround = false;
	}
};

/*static_detour(TestDetour, int* (int*, App::PropertyList*, float, int, int, bool))
{
	int* detoured(int* param1, App::PropertyList * param2, float param3, int param4, int param5, bool param6)
	{
		workaround = true;
		auto a = original_function(param1, param2, param3, param4, param5, param6);
		workaround = false;
		return a;
	}
};*/


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
	GameModeOverride::attach(GetAddress(App::cGameModeManager, GetActiveMode));
	GameModeOverrideTwo::attach(GetAddress(App::cGameModeManager, GetActiveModeID));
	TestDetour::attach(Address(0x00c3fde0));//Address(0x00b550f0));
	TestDetour2::attach(GetAddress(Simulator::cCreatureAnimal, Update));
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

