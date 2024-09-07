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

virtual_detour(Chocice75_ImprovedHologramScout_OnUseDetour, cGetOutOfUFOToolStrategy, cToolStrategy, bool(cSpaceToolData*))
{
	bool detoured(cSpaceToolData * pTool)
	{
		if (Simulator::GetPlayerEmpire() && GetPlayerEmpire()->GetSpeciesProfile() && GetPlayerEmpire()->GetSpeciesProfile()->mSpeciesKey != ResourceKey(0,0,0))
		{

			bool isSpecial = false;
			App::Property::GetBool(pTool->mpPropList.get(), id("hologramScoutIsImproved"), isSpecial);


			auto captain = GetPlayerEmpire()->mCaptainKey;
			auto species = GetPlayerEmpire()->GetSpeciesProfile();
			HologramScoutMod::Get()->isSpecial = isSpecial;
			//ModAPI::Log("%x!%x.%x", species.groupID, species.instanceID, species.typeID);
			if (!isSpecial)
			{
				GetPlayerEmpire()->mCaptainKey = ResourceKey(0x0, 0x0, 0x0);
			}
			bool result = original_function(this, pTool);

			GetPlayerEmpire()->mCaptainKey = captain;

			GetPlayerEmpire()->SetSpeciesProfile(species);
			//GetPlayerEmpire()->mCaptainKey = captain;
			cCreatureAnimalPtr avatar = GameNounManager.GetAvatar();

			HologramScoutMod::Get()->OpenUI(isSpecial);

			if (result && avatar && isSpecial)
			{

				//cCreatureAnimal* animal = cCreatureAnimal::Create(avatar->mPosition, SpeciesManager.GetSpeciesProfile(captain),1,avatar->mHerd.get(),true);

				cCreatureAnimal* animal2 = cCreatureAnimal::Create(avatar->mPosition, avatar->mpSpeciesProfile, 1, avatar->mHerd.get(), true);

				GameNounManager.SetAvatar(avatar.get());
				animal2->SetScale(0.00000000001);

				HologramScoutMod::Get()->isSpecial = 1;
				avatar->SetCurrentBrainLevel(5);
				avatar->SetScale(0.8f);
				avatar->GetModel()->GetModelWorld()->SetLightingWorld(Simulator::GetPlayerUFO()->GetModel()->GetModelWorld()->GetLightingWorld(0), 0, 1);

				if (avatar->mGeneralFlags |= 0x200)
				{
					avatar->mGeneralFlags -= 0x200;
				}
				//avatar->mFlags = 0x220;
				avatar->mbIsGhost = 0;
				avatar->mbIsTangible = 0;
				avatar->mbKeepPinnedToPlanet = 0;
				avatar->mbEnabled = 1;
				avatar->mCurrentLoudness = 0;
				avatar->mbFixed = 0;
				avatar->mbSupported = false;
			}

			for (auto creature : Simulator::GetData<Simulator::cCreatureAnimal>())
			{
				if (creature != avatar && creature->mbEnabled)
				{
					creature->SetScale(creature->mScale * 0.25f);
					creature->mScale /= 0.25f;
				}
			}

			return result;
		}
		return false;
	}
};

virtual_detour(Chocice75_ImprovedHologramScout_UpdateDetour, cGetOutOfUFOToolStrategy, cToolStrategy, bool(cSpaceToolData*, bool , const char16_t**))
{
	bool detoured(cSpaceToolData * pTool, bool showErrors, const char16_t** ppFailText = nullptr)
	{
		bool result = original_function(this, pTool, showErrors, ppFailText);
		bool isSpecial;
		
		
		App::Property::GetBool(pTool->mpPropList.get(), id("hologramScoutIsImproved"), isSpecial);

		int tScore = 0;
		if (SpacePlayerData::Get()->mCurrentContext == SpaceContext::Planet) { tScore = TerraformingManager.GetTScore(Simulator::GetActivePlanetRecord()); }//TerraformingManager.CalculateTScore(GetActivePlanetRecord()->mAtmosphereScore, GetActivePlanetRecord()->mTemperatureScore); }
		else tScore = 1;

		if (isSpecial == 1 && tScore <= 0)
		{
			//ppFailText = new const char16_t*(u"Cannot be used on T0 planets.");
			showErrors = true;
			return false;
		} 
		else
		{
			return result;
		}
	}
};

static_detour(HologramAudioDetour, void(uint32_t, Audio::AudioTrack, Vector3))
{
	void detoured(uint32_t id, Audio::AudioTrack track, Vector3 pos)
	{
		if (id == 0x73ecda8a && Simulator::IsSpaceGame() && !HologramScoutMod::Get()->isSpecial)
		{
			return original_function(id, track, pos);
		}
		else
		{
			return;
		}
	}
};

void AttachDetours()
{
	Chocice75_ImprovedHologramScout_OnUseDetour::attach(GetAddress(cGetOutOfUFOToolStrategy, OnSelect));
	SetRenderType__detour::attach(GetAddress(App::cViewer, SetRenderType));
	Chocice75_ImprovedHologramScout_UpdateDetour::attach(GetAddress(cGetOutOfUFOToolStrategy, Update));
	//CreatureBaseDetour::attach(GetAddress(Simulator::cCreatureBase, WalkTo));

	HologramAudioDetour::attach(GetAddress(Audio,PlayProceduralAudio));
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

