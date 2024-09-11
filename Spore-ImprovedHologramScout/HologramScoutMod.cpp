#include "stdafx.h"
#include "HologramScoutMod.h"
#include <Spore/Simulator/cCreatureGameData.h>

HologramScoutMod* HologramScoutMod::sInstance;
byte HologramScoutMod::RenderToUse;

HologramScoutMod::HologramScoutMod()
{
	RenderToUse = 0;
	mpLayout = nullptr;
	sInstance = this;

	isSpecial = 0;
	wasActive = 0;
	mbPressedSpace = 0;
}


HologramScoutMod::~HologramScoutMod()
{
}

HologramScoutMod* HologramScoutMod::Get()
{
	return sInstance;
}

void HologramScoutMod::InitialiseAbilities(bool isSpecial = false)
{
	auto avatar = GameNounManager.GetAvatar();
	if (Simulator::IsSpaceGame() && avatar)
	{
		for (int i = 0; i < avatar->GetAbilitiesCount(); i++)
		{
			auto ability = avatar->GetAbility(i);
			if (ability->mType == 30 || ability->mType == 62 || ability->mType == 74)
			{
				mpCombatSkills.emplace(0, ability);
			}
			if (ability->mType == 31 || ability->mType == 61 || ability->mType == 73)
			{
				mpCombatSkills.emplace(1, ability);
			}
			if (ability->mType == 33 || ability->mType == 65 || ability->mType == 75)
			{
				mpCombatSkills.emplace(2, ability);
			}
			if (ability->mType == 32 || ability->mType == 66 || ability->mType == 72)
			{
				mpCombatSkills.emplace(3, ability);
			}

			if (ability->mType == 29)
			{
				mpScanAbilities.emplace(0, ability);
			}
		}
	}
}

void HologramScoutMod::Update()
{
	if (Simulator::IsSpaceGame() && GameNounManager.GetAvatar())
	{
		wasActive = 1;
		
		
		auto avatar = GameNounManager.GetAvatar();
		//Make sure that each creature is sized appropriately.
		for (auto creature : Simulator::GetData<Simulator::cCreatureAnimal>())
		{
			if (creature->mbEnabled)
			{
				if (creature != avatar)
				{
					creature->SetScale(creature->mScale * 0.25f);
					creature->mScale /= (0.25f);
					creature->mMaxHealthPoints = max(creature->mHealthPoints, creature->mMaxHealthPoints);

					//creature->mEnergy += creature->mpSpeciesProfile->mEnergyRecoveryRate;
					//creature->mHealthPoints += creature->mpSpeciesProfile->mHealthRecoveryRate;

					//creature->mEnergy = clamp(0.0f, creature->mEnergy, creature->mMaxEnergy);
					//creature->mHealthPoints = clamp(0.0f, creature->mHealthPoints, creature->mMaxHealthPoints);
				}
			}
		}
		//SporeDebugPrint("%x, %x", avatar->mpSpeciesProfile->mHealthRecoveryRate, avatar->mpSpeciesProfile->mEnergyRecoveryRate);

		//avatar->mEnergy += avatar->mpSpeciesProfile->mEnergyRecoveryRate;
		//avatar->mHealthPoints += avatar->mpSpeciesProfile->mHealthRecoveryRate;

		//avatar->mEnergy = clamp(0.0f, avatar->mEnergy, avatar->mMaxEnergy);
		//avatar->mHealthPoints = clamp(0.0f, avatar->mHealthPoints, mMaxPlayerHealth);

		//Selection code
		//Get the camera position and mouse direction
		Vector3 cameraPosition, mouseDir;
		App::GetViewer()->GetCameraToMouse(cameraPosition, mouseDir);
		//and also get the view direction.
		auto viewDir = CameraManager.GetViewer()->GetViewTransform().GetRotation().Row(1);
		//Initialize a vector of spaital objects
		vector<cSpatialObjectPtr> objects;

		//Raycast to find all items in the view.
		if (GameViewManager.RaycastAll(cameraPosition, cameraPosition + mouseDir * 1000.0f, objects, true))
		{
			cCombatantPtr comb;
			//For each, check if it's a combatant.
			for each (cSpatialObjectPtr obj in objects)
			{
				if (object_cast<Simulator::cCombatant>(obj))
				{
					//if it is, then set comb to be the object and then break from the foreach loop.
					comb = object_cast<Simulator::cCombatant>(obj);
					break;
				}
			}

			//If the combatant is neither null nor the player avatar,
			if (comb != nullptr && comb != object_cast<Simulator::cCombatant>(avatar))
			{
				//Check if the terrain intersects with it.
				if (PlanetModel.mpTerrain->Raycast(cameraPosition, comb->ToSpatialObject()->mPosition) == Vector3(0, 0, 0))
				{
					//SporeDebugPrint("raycasted!");

					//SporeDebugPrint("%f, %f", comb->mHealthPoints, comb->mMaxHealthPoints);
					//If the hovered combatant isn't the same as the one just found,
					if (mpHoveredCombatant != comb)
					{
						//set the hovered combatant to not be hovered, and set the new one to be the hovered combatant.
						comb->ToSpatialObject()->SetIsRolledOver(true);
						if (mpHoveredCombatant)
						{
							mpHoveredCombatant->ToSpatialObject()->SetIsRolledOver(false);
						}
						mpHoveredCombatant = comb;
					}
				}
				else //if the terrain intersects, set the hovered combatant to nullptr.
				{
					comb = nullptr;
					mpHoveredCombatant = nullptr;
				}
			}
			else
			{
				//If it's not null, but it should no longer be hoveed,
				if (mpHoveredCombatant != nullptr)
				{
					//set it to not be hovered.
					mpHoveredCombatant->ToSpatialObject()->SetIsRolledOver(false);
				}
			}
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

		if (!avatar->mpCombatantTarget && mpSelectedCombatant)
		{
			DeselectCombatant();
		}
			
		if (isSpecial)
		{
			

			//avatar->mFlags = 0x220;
			avatar->mbIsGhost = 0;
			avatar->mbIsTangible = 1;
			avatar->mbKeepPinnedToPlanet = 0;
			avatar->mbEnabled = 1;
			avatar->mbFixed = 0;
			avatar->mbIsInvalid = 0;
			avatar->mbSelfPowered = true;
			avatar->mCurrentLoudness = 0;
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
	}
}




void HologramScoutMod::GetPlayerInput(cCreatureBasePtr avatar)
{
	uint32_t num1 = 0x10000001;
	uint32_t num2 = 0x10000002;
	uint32_t num3 = 0x10000003;
	uint32_t num4 = 0x10000004;

	map<byte, Simulator::cCreatureAbility*> abilities = mpScanAbilities;
	if (mbAbilityMode)
	{
		abilities = mpCombatSkills;
	}

	if (GameInputManager.IsTriggered(num1))
	{
		auto ability = abilities.find(0);
		if (ability != abilities.end())
			TriggerSkill(ability.mpNode->mValue.second);
	}
	if (GameInputManager.IsTriggered(num2))
	{
		auto ability = abilities.find(1);
		if (ability != abilities.end())
			TriggerSkill(ability.mpNode->mValue.second);
	}
	if (GameInputManager.IsTriggered(num3))
	{
		auto ability = abilities.find(2);
		if (ability != abilities.end())
			TriggerSkill(ability.mpNode->mValue.second);
	}
	if (GameInputManager.IsTriggered(num4))
	{
		auto ability = abilities.find(3);
		if (ability != abilities.end())
			TriggerSkill(ability.mpNode->mValue.second);
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
		if (mpHoveredCombatant)
		{
			SelectCombatant(mpHoveredCombatant);
		}
	}
}


void HologramScoutMod::OpenUI(bool useAbilities)
{
	if (mpLayout == nullptr)
	{
		mpLayout = new UTFWin::UILayout();
	}
	mpLayout->LoadByID(id("HoloScoutUI"));
	mpLayout->SetParentWindow(WindowManager.GetMainWindow()->FindWindowByID(0x05E66E88));
	mpLayout->SetVisible(true);
	mpLayout->GetContainerWindow()->AddWinProc(new UTFWin::SimpleLayout());
	mpLayout->GetContainerWindow()->SendToBack(mpLayout->GetContainerWindow());
	
	if (!useAbilities)
	{
		mpLayout->FindWindowByID(id("CrtSpecialAbilities"))->SetVisible(false);
		mpLayout->FindWindowByID(id("CrtBaseAbilities"))->SetVisible(false);
		mpLayout->FindWindowByID(id("InfHealthDisplay"))->SetVisible(true);
	}
	auto avatar = GameNounManager.GetAvatar();
	auto imageKey = ResourceKey(avatar->mSpeciesKey.instanceID, TypeIDs::png, avatar->mSpeciesKey.groupID);

	auto imageDraw = new UTFWin::ImageDrawable();
	ImagePtr image;
	UTFWin::Image::GetImage(imageKey, image);
	imageDraw->SetImage(image.get());
	mpLayout->FindWindowByID(id("CreatureIcon"))->SetDrawable(imageDraw);

	WindowManager.GetMainWindow()->FindWindowByID(0x065E40F0)->GetParent()->SetVisible(false);
}

void HologramScoutMod::UpdateUI()
{
	auto specialAbilitySlots = mpLayout->FindWindowByID(id("CrtSpecialAbilities"));
	if (specialAbilitySlots)
	{
		specialAbilitySlots->FindWindowByID(id("ScanPanel"))->SetVisible(!mbAbilityMode);
		specialAbilitySlots->FindWindowByID(id("CombatPanel"))->SetVisible(mbAbilityMode);
	}

	// Update stats UI.
	//doing this so that we won't need to recursively search all windows repeatedly
	auto statDisplay = mpLayout->FindWindowByID(id("StatDisplay"));
	auto avatar = GameNounManager.GetAvatar();
	if (!avatar || !statDisplay)
	{
		CloseUI();
		return;
	}

	auto healthText = statDisplay->FindWindowByID(id("CrtHealthText"));

	string16 text;
	text.sprintf(u"%i", int(avatar->mHealthPoints));

	healthText->SetCaption(text.c_str());
	float healthPercent = avatar->mHealthPoints / avatar->mMaxHealthPoints;
	auto healthBar = statDisplay->FindWindowByID(id("CrtHealthBar"));
	healthBar->SetArea(Math::Rectangle(0, 0, 68 * healthPercent, 13));
	healthBar->SetFlag(UTFWin::WindowFlags::kWinFlagClip, true);
	healthBar->GetParent()->SetFlag(UTFWin::WindowFlags::kWinFlagClip, true);


	auto energyText = statDisplay->FindWindowByID(id("CrtEnergyDisplay"));

	string16 energyCaption;
	energyCaption.sprintf(u"%i", int(avatar->mEnergy));

	energyText->SetCaption(energyCaption.c_str());
	float energyPercent = avatar->mEnergy / avatar->mMaxEnergy;
	auto energyBar = statDisplay->FindWindowByID(id("CrtEnergyBar"));
	energyBar->SetArea(Math::Rectangle(0, 0, 68 * energyPercent, 13));

}

void HologramScoutMod::CloseUI()
{
	mpLayout = nullptr;
	WindowManager.GetMainWindow()->FindWindowByID(0x065E40F0)->GetParent()->SetVisible(true);
}



// You can extend this function to return any other types your class implements.
void* HologramScoutMod::Cast(uint32_t type) const
{
	CLASS_CAST(Object);
	CLASS_CAST(HologramScoutMod);
	return nullptr;
}

void HologramScoutMod::SelectCombatant(cCombatantPtr combatant)
{
	GameNounManager.GetAvatar()->mpCombatantTarget = combatant.get();
	mpLayout->FindWindowByID(id("TargetCreatureUI"))->SetVisible(true);

	mpSelectedCombatant = combatant;
	SporeDebugPrint("Selected combatant!");

	if (object_cast<Simulator::cSpatialObject>(combatant))
	{
		Simulator::cSpatialObject* spatial = object_cast<Simulator::cSpatialObject>(combatant);
		auto key = spatial->GetModelKey();

		if (object_cast<Simulator::cCreatureBase>(combatant))
		{
			key = object_cast<Simulator::cCreatureBase>(combatant)->mSpeciesKey;
		}

		SporeDebugPrint("(0x%x!0x%x.0x%x)", key.groupID, key.instanceID, key.typeID);
		auto imageKey = ResourceKey(key.instanceID, TypeIDs::png, key.groupID);

		auto window = mpLayout->FindWindowByID(id("TargetCreatureImage"));
		if (ResourceManager.GetResource(imageKey, nullptr))
		{
			auto imageDraw = new UTFWin::ImageDrawable();
			ImagePtr image;
			UTFWin::Image::GetImage(imageKey, image);
			imageDraw->SetImage(image.get());
			window->SetDrawable(imageDraw);
			window->Revalidate();
			window->SetVisible(true);
		}
		else
		{
			window->SetVisible(false);
		}
	}
}

void HologramScoutMod::DeselectCombatant()
{
	mpLayout->FindWindowByID(id("TargetCreatureUI"))->SetVisible(false);
}




void HologramScoutMod::TriggerSkill(Simulator::cCreatureAbility* ability)
{
	auto avatar = GameNounManager.GetAvatar();
	if (avatar && avatar->mpCombatantTarget && ability)
	{
		auto distance = Math::distance(avatar->mPosition, avatar->mpCombatantTarget->ToSpatialObject()->mPosition);
		//SporeDebugPrint("distance is %f, range is %f", distance, ability->mRange);
		if (floor(distance) > ability->mRange && distance > ability->mRushingRange)
		{
			avatar->WalkTo(1,avatar->mpCombatantTarget->ToSpatialObject()->mPosition, avatar->mpCombatantTarget->ToSpatialObject()->mPosition.Normalized());
			delayedAbility = ability;
			return;
		}
		if (ceil(distance) < ability->mAvatarRangeMin)
		{
			return;
		}

		if (ability->mEnergyCost > avatar->mEnergy)
		{
			return;
		}

		int abilityCount = avatar->GetAbilitiesCount();
		int index = -1;
		for (int i = 0; i < abilityCount; i++)
		{
			if (avatar->GetAbility(i) == ability)
			{
				index = i;
				break;
			}
		}
		if (index != -1)
		{
			avatar->PlayAbility(index);
		}
		else
		{
			delayedAbility = nullptr;
		}
	}
	else
	{
		delayedAbility = nullptr;
	}
}


// For internal use, do not modify.
int HologramScoutMod::AddRef()
{
	return DefaultRefCounted::AddRef();
}

// For internal use, do not modify.
int HologramScoutMod::Release()
{
	return DefaultRefCounted::Release();
}
