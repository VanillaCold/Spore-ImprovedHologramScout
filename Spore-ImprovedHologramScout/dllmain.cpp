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
			//animal = nullptr;

			bool result = original_function(this, pTool);

			GetPlayerEmpire()->mCaptainKey = captain;

			GetPlayerEmpire()->SetSpeciesProfile(species);
			//GetPlayerEmpire()->mCaptainKey = captain;
			cCreatureAnimalPtr avatar = GameNounManager.GetAvatar();

			HologramScoutMod::Get()->OpenUI(isSpecial);
			avatar->SetCurrentBrainLevel(5);
			avatar->mMaxHealthPoints = max(avatar->mHealthPoints, avatar->mMaxHealthPoints);

			if (result && avatar && isSpecial)
			{

				//cCreatureAnimal* animal = cCreatureAnimal::Create(avatar->mPosition, SpeciesManager.GetSpeciesProfile(captain),1,avatar->mHerd.get(),true);

				cCreatureAnimal* animal2 = cCreatureAnimal::Create(avatar->mPosition, avatar->mpSpeciesProfile, 1, avatar->mHerd.get(), true);

				animal2->SetCurrentBrainLevel(5);
				avatar->mMaxHealthPoints = max(avatar->mMaxHealthPoints, max(animal2->mHealthPoints, animal2->mMaxHealthPoints));
				GameNounManager.SetAvatar(avatar.get());
				animal2->SetScale(0.00000000001);

				avatar->mHealthPoints = avatar->mMaxHealthPoints;

				HologramScoutMod::Get()->isSpecial = 1;
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
					creature->mMaxHealthPoints = max(creature->mMaxHealthPoints, creature->mHealthPoints);
					creature->mHealthPoints = max(creature->mMaxHealthPoints, creature->mHealthPoints);
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

//This is needed to ensure that creatures in the space stage don't always deal 9999 damage like they do in vanilla. 
virtual_detour(OverrideCreatureDamageDetour, Simulator::cCreatureBase, Simulator::cCombatant, int(float, uint32_t, int, const Vector3&, cCombatant*))
{
	int detoured(float damage, uint32_t attackerPoliticalID, int unk, const Vector3 & unkPos, cCombatant * pAttacker)
	{
		//Get the attacker, and verify it's the space-stage.
		if (pAttacker && Simulator::IsSpaceGame())
		{
			//Cast the attacker to a creature.
			auto creature = object_cast<Simulator::cCreatureBase>(pAttacker);
			if (creature) //if it is one,
			{
				//Get the current ability index being used,
				uint32_t attackIndex = creature->mCurrentAttackIdx;
				//and get the ability from that.
				auto ability = creature->GetAbility(attackIndex);
				//If it's a valid ability,
				if (ability)
				{
					//set the damage to ability->mDamage, so that it isn't overpowered.
					damage = ability->mDamage;
				}
			}
		}
		SporeDebugPrint("Damage is %f, caused by political ID of 0x%x", damage, attackerPoliticalID);
		return original_function(this, damage, attackerPoliticalID, unk, unkPos, pAttacker);
	}
};

void AttachDetours()
{
	Chocice75_ImprovedHologramScout_OnUseDetour::attach(GetAddress(cGetOutOfUFOToolStrategy, OnSelect));
	SetRenderType__detour::attach(GetAddress(App::cViewer, SetRenderType));
	Chocice75_ImprovedHologramScout_UpdateDetour::attach(GetAddress(cGetOutOfUFOToolStrategy, Update));
	//CreatureBaseDetour::attach(GetAddress(Simulator::cCreatureBase, WalkTo));

	HologramAudioDetour::attach(GetAddress(Audio,PlayProceduralAudio));
	OverrideCreatureDamageDetour::attach(Address(0x00bfcf10));
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

