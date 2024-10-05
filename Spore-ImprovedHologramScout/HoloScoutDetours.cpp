#include "stdafx.h"
#include "Detours.h"
#include <Spore/Simulator/SubSystem/TerraformingManager.h>

bool Chocice75_ImprovedHologramScout_OnUseDetour::DETOUR(cSpaceToolData* pTool)
{
	if (Simulator::GetPlayerEmpire() && GetPlayerEmpire()->GetSpeciesProfile() && GetPlayerEmpire()->GetSpeciesProfile()->mSpeciesKey != ResourceKey(0, 0, 0))
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
		avatar->SetScale(1.5f);

		HologramScoutMod::Get()->InitialiseAbilities(isSpecial);
		avatar->SetPoliticalID(Simulator::GetPlayerEmpire()->mPoliticalID);

		if (result && avatar && isSpecial)
		{

			cCreatureAnimal* animal = cCreatureAnimal::Create(Vector3(0, 0, 0), avatar->mpSpeciesProfile, 1, avatar->mHerd.get(), true);

			cCreatureAnimal* animal2 = cCreatureAnimal::Create(Vector3(0, 0, 0), species, 1, avatar->mHerd.get(), true);

			animal->SetCurrentBrainLevel(5);
			avatar->mMaxHealthPoints = max(avatar->mMaxHealthPoints, max(animal2->mHealthPoints, animal2->mMaxHealthPoints));
			HologramScoutMod::Get()->mMaxPlayerHealth = avatar->mMaxHealthPoints;
			GameNounManager.SetAvatar(avatar.get());
			animal->SetScale(0.00000000001);

			GameNounManager.DestroyInstance(animal);


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

bool Chocice75_ImprovedHologramScout_UpdateDetour::DETOUR(cSpaceToolData* pTool, bool showErrors, const char16_t** ppFailText = nullptr)
{
	bool result = original_function(this, pTool, showErrors, ppFailText);
	bool isSpecial;


	App::Property::GetBool(pTool->mpPropList.get(), id("hologramScoutIsImproved"), isSpecial);

	int tScore = 0;
	if (SpacePlayerData::Get()->mCurrentContext == SpaceContext::Planet) 
	{ 
		tScore = TerraformingManager.GetTScore(Simulator::GetActivePlanetRecord()); 
	}
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