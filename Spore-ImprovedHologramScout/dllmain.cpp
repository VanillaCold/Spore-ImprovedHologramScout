// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <Spore\Simulator\cGetOutOfUFOToolStrategy.h>
#include "SpawnAvatarCheat.h"
#include <Spore\GameModes.h>
#include "HologramScoutMod.h"
#include <Spore\Simulator\SubSystem\TerraformingManager.h>
#include <Spore/Editors/BakeManager.h>
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
		if (renderType == 15 && Simulator::IsSpaceGame() && HologramScoutMod::Get()->isSpecial == 1) renderType = HologramScoutMod::RenderToUse;
		original_function(this, renderType, arg2);
	}
};

member_detour(CreatureBaseDetour, Simulator::cCreatureBase, void(int, const Vector3&, const Vector3&, float, float))
{
	void detoured(int speedState, const Vector3 & dstPos, const Vector3 & arg_8, float goalStopDistance = 1.0f, float acceptableStopDistance = 2.0f)
	{
		if (this == (cCreatureBase*)GameNounManager.mpAvatar.get() && Simulator::IsSpaceGame() && HologramScoutMod::Get()->isSpecial)
		{
			speedState = 2;
		}
		original_function(this, speedState, dstPos, arg_8, goalStopDistance, acceptableStopDistance);
	}
};

virtual_detour(Chocice75_ImprovedHologramScout_OnUseDetour, cGetOutOfUFOToolStrategy, cToolStrategy, bool(cSpaceToolData*))
{
	bool detoured(cSpaceToolData * pTool)
	{
		if (Simulator::GetPlayerEmpire() && GetPlayerEmpire()->GetSpeciesProfile() && GetPlayerEmpire()->GetSpeciesProfile()->mSpeciesKey != ResourceKey(0,0,0))
		{
			auto captain = GetPlayerEmpire()->mCaptainKey;

			auto species = GetPlayerEmpire()->GetSpeciesProfile();

			//ModAPI::Log("%x!%x.%x", species.groupID, species.instanceID, species.typeID);

			GetPlayerEmpire()->mCaptainKey = ResourceKey(0x0,0x0,0x0);

			bool result = original_function(this, pTool);

			GetPlayerEmpire()->mCaptainKey = captain;

			GetPlayerEmpire()->SetSpeciesProfile(species);
			//GetPlayerEmpire()->mCaptainKey = captain;
			cCreatureAnimalPtr avatar = GameNounManager.GetAvatar();

			bool isSpecial = false;
			if (App::Property::GetBool(pTool->mpPropList.get(), id("hologramScoutIsImproved"), isSpecial) && avatar && isSpecial)
			{
				HologramScoutMod::Get()->isSpecial = 1;
				avatar->SetScale(1.5f);

				//GameNounManager.CreateNest(avatar->mPosition, avatar->mHerd.get());

				avatar->mStandardSpeed = 10;
				avatar->mTurnRate = 10;
				avatar->mDesiredSpeed = 10;
				avatar->GetModel()->GetModelWorld()->SetLightingWorld(Simulator::GetPlayerUFO()->GetModel()->GetModelWorld()->GetLightingWorld(0), 0, 1);
				//avatar->field_101C = 0;
				//avatar->field_166C = 0;
				//avatar->GetModel()->field_5C = 0;

				if (avatar->mGeneralFlags |= 0x200)
				{
					avatar->mGeneralFlags -= 0x200;
				}

				avatar->mFlags = 512;
				avatar->mbIsGhost = 0;
				avatar->mbIsTangible = 0;
				avatar->mbKeepPinnedToPlanet = 0;
				avatar->mbEnabled = 1;
				avatar->mCurrentLoudness = 0;
			}
			return result;
		}
		return false;
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
		if (SpacePlayerData::Get()->mCurrentContext == SpaceContext::Planet) { tScore = TerraformingManager.GetTScore(Simulator::GetActivePlanetRecord()); }//TerraformingManager.CalculateTScore(GetActivePlanetRecord()->mAtmosphereScore, GetActivePlanetRecord()->mTemperatureScore); }
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
	CreatureBaseDetour::attach(GetAddress(Simulator::cCreatureBase, WalkTo));
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

