#include "stdafx.h"
#include "HologramScoutManager.h"

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

void HologramScoutManager::Update(int deltaTime, int deltaGameTime) {

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


	/*if (Simulator::IsSpaceGame() && GameNounManager.GetAvatar())
	{
		if (!isSpecial)
		{
			mbAbilityMode = 0;
		}

		wasActive = 1;


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
		//SporeDebugPrint("%x, %x", avatar->mpSpeciesProfile->mHealthRecoveryRate, avatar->mpSpeciesProfile->mEnergyRecoveryRate);

		auto mode = (App::GameSpace*)GameModeManager.GetActiveMode();

		float time = GameTimeManager.ConvertDeltaTime(deltatime) / 1000.0f;

		//SporeDebugPrint("%f", time);

		avatar->mEnergy += avatar->mpSpeciesProfile->mEnergyRecoveryRate * time;
		avatar->mHealthPoints += avatar->mpSpeciesProfile->mHealthRecoveryRate * time;

		avatar->mEnergy = clamp(0.0f, avatar->mEnergy, avatar->mMaxEnergy);
		avatar->mHealthPoints = clamp(0.0f, avatar->mHealthPoints, mMaxPlayerHealth);
		avatar->mMaxHealthPoints = mMaxPlayerHealth;

		//Selection code
		//Get the camera position and mouse direction
		Vector3 cameraPosition, mouseDir;
		App::GetViewer()->GetCameraToMouse(cameraPosition, mouseDir);


		auto hover = GameViewManager.GetHoveredObject();
		auto combatant = object_cast<Simulator::cCombatant>(hover);
		if (combatant != nullptr)
		{
			mpHoveredCombatant = combatant;
			combatant->ToSpatialObject()->SetIsRolledOver(true);

			UI::SimulatorRollover::ShowRollover(hover);


			//auto wind = WindowManager.GetMainWindow()->FindWindowByID(0x04B47745);
		}

		if (delayedAbility)
		{
			if (mpSelectedCombatant == avatar->mpCombatantTarget)
			{
				TriggerSkill(delayedAbility);
			}
			else
			{
				delayedAbility = nullptr;
			}
		}

		//Get player input
		GetPlayerInput(avatar);
		//update the UI
		UpdateUI();

		if ((!avatar->mpCombatantTarget && mpSelectedCombatant) || (avatar->mpCombatantTarget && avatar->mpCombatantTarget->mHealthPoints <= 0))
		{
			avatar->mpCombatantTarget = nullptr;
			DeselectCombatant();
		}

		if (isSpecial)
		{


			//avatar->mFlags = 0x220;
			//avatar->mbIsGhost = 0;
			//avatar->mbIsTangible = 1;
			//avatar->mbKeepPinnedToPlanet = 1;
			//object_cast<Simulator::cCombatant>(avatar)->field_80 = 0;


		}
		else
		{
			object_cast<Simulator::cCombatant>(avatar)->field_80 = true;
		}
	}
	else
	{
		if (wasActive && Simulator::IsSpaceGame())
		{
			for (auto creature : Simulator::GetData<Simulator::cCreatureAnimal>())
			{
				creature->SetScale(creature->mScale);
				creature->mMaxHealthPoints = 1;
			}
		}
		wasActive = 0;
		isSpecial = 0;
	}

	if (!(Simulator::IsSpaceGame() && GameNounManager.GetAvatar()) && mpLayout)
	{
		if (Simulator::IsSpaceGame())
		{
			CloseUI();
		}
		mpLayout = nullptr;
	}*/
}