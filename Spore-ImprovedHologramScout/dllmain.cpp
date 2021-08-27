// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <Spore\Simulator\cGetOutOfUFOToolStrategy.h>
#include "SpawnAvatarCheat.h"
#include <Spore\GameModes.h>
#include "HologramScoutMod.h"
#include <Spore\Simulator\SubSystem\TerraformingManager.h>
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
}

void Dispose()
{
	// This method is called when the game is closing
}

member_detour(SetRenderType__detour, App::cViewer, void(int, bool)) {
	void detoured(int renderType, bool arg2) {
		if (renderType == 15 && Simulator::IsSpaceGame() && HologramScoutMod::Get()->isSpecial == 1) renderType = 0;
		original_function(this, renderType, arg2);
	}
};

virtual_detour(Chocice75_ImprovedHologramScout_OnUseDetour, cGetOutOfUFOToolStrategy, cToolStrategy, bool(cSpaceToolData*))
{
	bool detoured(cSpaceToolData * pTool)
	{
		auto captain = GetPlayerEmpire()->mCaptainKey;
		GetPlayerEmpire()->mCaptainKey = GetPlayerEmpire()->GetSpeciesProfile()->field_504;
		bool result = original_function(this, pTool);
		cCreatureAnimalPtr avatar = GameNounManager.GetAvatar();
		
		bool isSpecial;
		if (App::Property::GetBool(pTool->mpPropList.get(), id("hologramScoutIsImproved"), isSpecial) && avatar)
		{
			HologramScoutMod::Get()->isSpecial = 1;
			avatar->SetScale(1);
		}
		GetPlayerEmpire()->mCaptainKey = captain;
		return result;
	}
};

virtual_detour(Chocice75_ImprovedHologramScout_UpdateDetour, cGetOutOfUFOToolStrategy, cToolStrategy, bool(cSpaceToolData*, bool , char16_t*))
{
	bool detoured(cSpaceToolData * pTool, bool showErrors, char16_t* ppFailText = nullptr)
	{
		bool result = original_function(this, pTool, showErrors, ppFailText);
		bool isSpecial;
		App::Property::GetBool(pTool->mpPropList.get(), id("hologramScoutIsImproved"), isSpecial);

		int tScore = 0;
		if (SpacePlayerData::Get()->mCurrentContext == SpaceContext::kSpaceContextPlanet) { tScore = TerraformingManager.GetTScore(Simulator::GetActivePlanetRecord()); }//TerraformingManager.CalculateTScore(GetActivePlanetRecord()->mAtmosphereScore, GetActivePlanetRecord()->mTemperatureScore); }
		else tScore = 1;

		if (isSpecial == 1 && tScore <= 0)
		{
			ppFailText = u"Cannot be used on T0 planets.";
			showErrors = true;
			return false;
		} 
		else
		{
			return result;
		}
	}
};

void AttachDetours()
{
	Chocice75_ImprovedHologramScout_OnUseDetour::attach(GetAddress(cGetOutOfUFOToolStrategy, OnSelect));
	SetRenderType__detour::attach(GetAddress(App::cViewer, SetRenderType));
	Chocice75_ImprovedHologramScout_UpdateDetour::attach(GetAddress(cGetOutOfUFOToolStrategy, Update));
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

