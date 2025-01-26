#include "stdafx.h"
#include "HologramScoutManager.h"
#include "HologramCombatManager.h"
#include "HologramUIManager.h"

/// AUTOGENERATED METHODS ///

int HologramScoutManager::AddRef() {
	return Simulator::cStrategy::AddRef();
}
int HologramScoutManager::Release() {
	return Simulator::cStrategy::Release();
}

const char* HologramScoutManager::GetName() const {
	return "Spore_ImprovedHologramScout::HologramScoutManager";
}

bool HologramScoutManager::Write(Simulator::ISerializerStream* stream)
{
	return Simulator::ClassSerializer(this, ATTRIBUTES).Write(stream);
}
bool HologramScoutManager::Read(Simulator::ISerializerStream* stream)
{
	return Simulator::ClassSerializer(this, ATTRIBUTES).Read(stream);
}

/// END OF AUTOGENERATED METHODS ///
////////////////////////////////////

Simulator::Attribute HologramScoutManager::ATTRIBUTES[] = {
	// Add more attributes here
	// This one must always be at the end
	Simulator::Attribute()
};

void HologramScoutManager::Initialize() {
	
}

void HologramScoutManager::Dispose() {
	
}

void HologramScoutManager::Update(int deltaTime, int deltaGameTime) 
{
	auto combatManager = HologramCombatManager::Get();

	// Check if the hologram scout is active.
	if (Simulator::IsSpaceGame() && GameNounManager.GetAvatar())
	{
		cCreatureAnimalPtr avatar = GameNounManager.GetAvatar();
		
		// Perform beam-down-exclusive functionality.
		if (mbHasBeamedDown)
		{
			//do stuff
		}
		else
		{
			object_cast<Simulator::cCombatant>(avatar)->field_80 = true;
		}
		mbWasActive = true;
		
		// Rescale each creature.
		RescaleCreatures();

		
		float time = deltaGameTime / 1000.0f;

		avatar->mEnergy += avatar->mpSpeciesProfile->mEnergyRecoveryRate * time;
		avatar->mHealthPoints += avatar->mpSpeciesProfile->mHealthRecoveryRate * time;

		avatar->mEnergy = clamp(0.0f, avatar->mEnergy, avatar->mMaxEnergy);
		avatar->mHealthPoints = clamp(0.0f, avatar->mHealthPoints, mMaxPlayerHealth);
		avatar->mMaxHealthPoints = mMaxPlayerHealth;

		// Select creatures.
		cCombatantPtr hovered = GetHoveredCombatant();
		GetPlayerInput();

		if ((!avatar->mpCombatantTarget && combatManager->mpSelectedCombatant) || (avatar->mpCombatantTarget && avatar->mpCombatantTarget->mHealthPoints <= 0))
		{
			avatar->mpCombatantTarget = nullptr;
			combatManager->mpSelectedCombatant = nullptr;
			HologramUIManager::Get()->DeselectUI();
		}

	}

	else
	{
		if (Simulator::IsSpaceGame() && mbWasActive)
		{
			for (auto creature : Simulator::GetData<Simulator::cCreatureAnimal>())
			{
				creature->SetScale(creature->mScale);
				creature->mMaxHealthPoints = 1;
			}
		}
		mbHasBeamedDown = false;
		mbWasActive = false;
	}
}

void HologramScoutManager::RescaleCreatures()
{
	auto avatar = GameNounManager.GetAvatar();
	//Make sure that each creature is sized appropriately.
	for (auto creature : Simulator::GetData<Simulator::cCreatureAnimal>())
	{
		if (creature->mbEnabled)
		{
			if (creature != avatar)
			{
				creature->SetScale(creature->mScale * 0.5f);
				creature->mScale /= (0.5f);

				auto combatManager = HologramCombatManager::Get();
				if (combatManager->mpMaxHealthPoints.find(creature.get()) == combatManager->mpMaxHealthPoints.end())
				{
					creature->mMaxHealthPoints = max(creature->mHealthPoints, creature->mMaxHealthPoints);
					combatManager->mpMaxHealthPoints.emplace(creature.get(), creature->mMaxHealthPoints);
				}
				else
				{
					creature->mMaxHealthPoints = combatManager->mpMaxHealthPoints[creature.get()];
				}
			}
		}
	}
}

Simulator::cCombatant* HologramScoutManager::GetHoveredCombatant()
{
	auto hover = GameViewManager.GetHoveredObject();
	auto combatant = object_cast<Simulator::cCombatant>(hover);
	if (combatant != nullptr)
	{
		combatant->ToSpatialObject()->SetIsRolledOver(true);

		UI::SimulatorRollover::ShowRollover(hover);

		return combatant;
	}
	return nullptr;
}

void HologramScoutManager::GetPlayerInput()
{
	auto avatar = GameNounManager.GetAvatar();
	auto combatManager = HologramCombatManager::Get();
	uint32_t num1 = 0x10000001;
	uint32_t num2 = 0x10000002;
	uint32_t num3 = 0x10000003;
	uint32_t num4 = 0x10000004;

	map<byte, cCreatureAbilityPtr> abilities = combatManager->mpScanAbilities;
	if (combatManager->mbAbilityMode == 1)
	{
		abilities = combatManager->mpCombatSkills;
	}

	if (GameInputManager.IsTriggered(num1))
	{
		auto ability = abilities.find(0);
		if (ability != abilities.end())
			combatManager->TriggerSkill(ability.mpNode->mValue.second.get());
	}
	if (GameInputManager.IsTriggered(num2))
	{
		auto ability = abilities.find(1);
		if (ability != abilities.end())
			combatManager->TriggerSkill(ability.mpNode->mValue.second.get());
	}
	if (GameInputManager.IsTriggered(num3))
	{
		auto ability = abilities.find(2);
		if (ability != abilities.end())
			combatManager->TriggerSkill(ability.mpNode->mValue.second.get());
	}
	if (GameInputManager.IsTriggered(num4))
	{
		auto ability = abilities.find(3);
		if (ability != abilities.end())
			combatManager->TriggerSkill(ability.mpNode->mValue.second.get());
	}


	if (GameInputManager.IsTriggered(0x00000031))
	{
		if (mbPressedTab == 0)
		{
			mbAbilityMode = !mbAbilityMode;
		}
		mbPressedTab = 1;
	}
	else
	{
		mbPressedTab = 0;
	}

	if (GameInputManager.IsTriggered(0x00000007)) //Spacebar
	{
		if (!mbPressedSpace)
		{
			mbPressedSpace = 1;
			delayedAbility = nullptr;
			if (avatar->mbSupported || CreatureGameData.mNumFlapsAllowed > 0)
			{
				if (avatar->mbSupported)
				{
					auto flight = avatar->GetAbility(avatar->GetAbilityIndexByType(18));
					if (flight)
					{
						float gliding;
						App::Property::GetFloat(flight->mpPropList.get(), 0x02CC974A, gliding);
						CreatureGameData.mNumFlapsAllowed = round(gliding) * 2;
					}
				}
				else
				{
					CreatureGameData.mNumFlapsAllowed -= 1;
				}

				avatar->DoJump(50 * (!isSpecial));
			}
			else
			{
				CreatureGameData.mNumFlapsAllowed = 0;
			}
		}
	}
	else
	{
		mbPressedSpace = false;
	}


	bool leftClick = GameInputManager.IsTriggered(0x00000016);
	if (leftClick) //Left click
	{
		//SporeDebugPrint("click!");
		if (combatManager->mpHoveredCombatant)
		{
			combatManager->SelectCombatant(mpHoveredCombatant);
		}
	}
}
