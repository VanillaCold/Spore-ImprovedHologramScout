#include "stdafx.h"
#include "HologramScoutMod.h"
#include <Spore/Simulator/cCreatureGameData.h>
#include "HologramCombatManager.h"
#include <Spore/UI/SimulatorRollovers.h>
#include "ButtonWinProc.h"
#include "SwitchWinProc.h"

#include <Spore/App/GameSpace.h>

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
	deltaTimer = Simulator::cGonzagoTimer();
	deltaTimer.Start();
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

	mpCombatSkills = map<byte, cCreatureAbilityPtr>();
	mpScanAbilities = map<byte, cCreatureAbilityPtr>();

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
	auto deltatime = deltaTimer.GetElapsed().LowPart;
	deltaTimer.Stop();
	if (Simulator::IsSpaceGame() && GameNounManager.GetAvatar())
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

		if ( (!avatar->mpCombatantTarget && mpSelectedCombatant) || (avatar->mpCombatantTarget && avatar->mpCombatantTarget->mHealthPoints <= 0))
		{
			avatar->mpCombatantTarget = nullptr;
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

	deltaTimer.SetTime(LARGE_INTEGER{});
	deltaTimer.Start();
}




void HologramScoutMod::GetPlayerInput(cCreatureBasePtr avatar)
{
	uint32_t num1 = 0x10000001;
	uint32_t num2 = 0x10000002;
	uint32_t num3 = 0x10000003;
	uint32_t num4 = 0x10000004;

	map<byte, cCreatureAbilityPtr> abilities = mpScanAbilities;
	if (mbAbilityMode == 1)
	{
		abilities = mpCombatSkills;
	}

	if (GameInputManager.IsTriggered(num1))
	{
		auto ability = abilities.find(0);
		if (ability != abilities.end())
			TriggerSkill(ability.mpNode->mValue.second.get());
	}
	if (GameInputManager.IsTriggered(num2))
	{
		auto ability = abilities.find(1);
		if (ability != abilities.end())
			TriggerSkill(ability.mpNode->mValue.second.get());
	}
	if (GameInputManager.IsTriggered(num3))
	{
		auto ability = abilities.find(2);
		if (ability != abilities.end())
			TriggerSkill(ability.mpNode->mValue.second.get());
	}
	if (GameInputManager.IsTriggered(num4))
	{
		auto ability = abilities.find(3);
		if (ability != abilities.end())
			TriggerSkill(ability.mpNode->mValue.second.get());
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
	mbAbilityMode = 0;

	auto avatar = GameNounManager.GetAvatar();


	if (mpLayout == nullptr)
	{
		mpLayout = new UTFWin::UILayout();
	}
	mpLayout->LoadByID(id("HoloScoutUI"));
	mpLayout->SetParentWindow(WindowManager.GetMainWindow()->FindWindowByID(0x05E66E88));
	mpLayout->SetVisible(true);
	mpLayout->GetContainerWindow()->AddWinProc(new UTFWin::SimpleLayout());
	mpLayout->GetContainerWindow()->SendToBack(mpLayout->GetContainerWindow());

	mpLayout->FindWindowByID(id("ScanButton"))->AddWinProc(new SwitchWinProc(0));
	mpLayout->FindWindowByID(id("CombatButton"))->AddWinProc(new SwitchWinProc(1));
	
	if (!useAbilities)
	{
		mpLayout->FindWindowByID(id("CrtSpecialAbilities"))->SetVisible(false);
		mpLayout->FindWindowByID(id("CrtBaseAbilities"))->SetVisible(false);
		mpLayout->FindWindowByID(id("InfHealthDisplay"))->SetVisible(true);
	}

	//setup the buttons
	//0x01012020 is the first one

	for (int i = 0; i < 4; i++)
	{
		auto button = mpLayout->FindWindowByID(0x01012020 + i);
		if (mpCombatSkills.find(i) != mpCombatSkills.end())
		{
			button->SetVisible(true);
			auto window = new UTFWin::Window();

			window->SetControlID(0x0);
			button->AddWindow(window);

			auto& skill = mpCombatSkills[i];
			ResourceKey imgKey = ResourceKey(skill->mVerbIconImageID, TypeIDs::png, id("HoloAbilityIcons"));
			
			UTFWin::ImageDrawable* imageDrawable = new UTFWin::ImageDrawable();
			window->SetDrawable(imageDrawable);
			window->SetShadeColor(Math::Color(255, 255, 255, 240));

			window->SetArea(Math::Rectangle(8, 8, 28, 28));
			UTFWin::ImageDrawable::SetImageForWindow(window,imgKey);

			window->SetVisible(true);
			window->SetFlag(UTFWin::WindowFlags::kWinFlagIgnoreMouse, true);

			for (byte j = 0; j < avatar->GetAbilitiesCount(); j++)
			{
				if (mpCombatSkills[i] == avatar->GetAbility(j))
				{
					button->AddWinProc(new ButtonWinProc(j));

					//creatureAbilityRolloverValueDescription
					
					Math::Vector2 minLevel;
					App::Property::GetVector2(mpCombatSkills[i]->mpPropList.get(), 0x04052A86, minLevel);
	
					string16 tooltip;
					if (minLevel.x != 0)
					{
						tooltip.sprintf(u"%ls - Level %i", mpCombatSkills[i]->nName.GetText(), int(minLevel.x));
					}
					else
					{
						tooltip = mpCombatSkills[i]->nName.GetText();
					}
					button->AddWinProc(UTFWin::CreateTooltip(tooltip.c_str()));
					break;
				}
			}

		}
	}

	//Now just do the same for the other abilities.
	for (int i = 0; i < 4; i++)
	{
		auto button = mpLayout->FindWindowByID(0x01012010 + i);
		if (mpScanAbilities.find(i) != mpScanAbilities.end())
		{
			button->SetVisible(true);
			auto window = new UTFWin::Window();

			window->SetControlID(0x0);
			button->AddWindow(window);

			auto& skill = mpScanAbilities[i];
			ResourceKey imgKey = ResourceKey(skill->mVerbIconImageID, TypeIDs::png, id("HoloAbilityIcons"));

			UTFWin::ImageDrawable* imageDrawable = new UTFWin::ImageDrawable();
			window->SetDrawable(imageDrawable);
			window->SetArea(Math::Rectangle(8, 8, 28, 28));
			UTFWin::ImageDrawable::SetImageForWindow(window, imgKey);

			window->SetVisible(true);
			window->SetFlag(UTFWin::WindowFlags::kWinFlagIgnoreMouse, true);

			for (byte j = 0; j < avatar->GetAbilitiesCount(); j++)
			{
				if (mpScanAbilities[i] == avatar->GetAbility(j))
				{
					button->AddWinProc(new ButtonWinProc(j));
					string16 tooltip = mpScanAbilities[i]->nName.GetText();
					button->AddWinProc(UTFWin::CreateTooltip(tooltip.c_str()));
					break;
				}
			}

		}
	}

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

		object_cast<UTFWin::IButton>(specialAbilitySlots->FindWindowByID(id("CombatButton")))->SetButtonStateFlag(4, mbAbilityMode);
		object_cast<UTFWin::IButton>(specialAbilitySlots->FindWindowByID(id("ScanButton")))->SetButtonStateFlag(4, !mbAbilityMode);

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

	if (mpSelectedCombatant)
	{
		//TargetHealthbar
		auto targetUI = mpLayout->FindWindowByID(id("TargetCreatureUI"));
		auto tHealthBar = targetUI->FindWindowByID(id("TargetHealthbar"));
		auto tEnergyBar = targetUI->FindWindowByID(id("TargetHealthbar"));

		auto tHealthCaption = targetUI->FindWindowByID(id("TargetHealthText"));
		auto tEnergyCaption = targetUI->FindWindowByID(id("TargetEnergyText"));

		string16 tHealthText;
		auto tHealth = mpSelectedCombatant->mHealthPoints;
		tHealthText.sprintf(u"%i", int(mpSelectedCombatant->mHealthPoints));

		tHealthCaption->SetCaption(tHealthText.c_str());

		float tMaxHealth = mpSelectedCombatant->mMaxHealthPoints;

		auto crtTarget = object_cast<Simulator::cCreatureBase>(mpSelectedCombatant);
		if (crtTarget)
		{
			auto combatManager = HologramCombatManager::Get();
			if (combatManager->mpMaxHealthPoints.find(crtTarget) != combatManager->mpMaxHealthPoints.end())
			{
				tMaxHealth = combatManager->mpMaxHealthPoints[crtTarget];
			}

			targetUI->FindWindowByID(id("TargetEnergyContainer"))->SetVisible(true);
			auto creature = object_cast<Simulator::cCreatureBase>(mpSelectedCombatant);
			string16 tEnergyText;
			auto tEnergy = creature->mEnergy;
			
			tEnergyText.sprintf(u"%i", int(tEnergy));

			tEnergyCaption->SetCaption(tEnergyText.c_str());
			tEnergyBar->SetArea(Math::Rectangle(0, 0, 68 * tEnergy / creature->mMaxEnergy, 13));

		}
		else
		{
			targetUI->FindWindowByID(id("TargetEnergyContainer"))->SetVisible(false);
		}

		tHealthBar->SetArea(Math::Rectangle(0, 0, 68 * tHealth / tMaxHealth, 13));
	}

}

void HologramScoutMod::CloseUI()
{
	auto container = mpLayout->GetContainerWindow();
	
	container->DisposeAllWindowFamilies();

	//auto special = mpLayout->FindWindowByID(id("CrtSpecialAbilities"));
	//auto baseab = mpLayout->FindWindowByID(id("CrtBaseAbilities"));
	//auto target = mpLayout->FindWindowByID(id("TargetCreatureUI"));
	//auto crtclp = mpLayout->FindWindowByID(id("CreatureClip"));
	//auto bottom = mpLayout->FindWindowByID(id("BottomBar"));
	//container->DisposeWindowFamily(special);
	//container->DisposeWindowFamily(baseab);
	//container->DisposeWindowFamily(target);
	//container->DisposeWindowFamily(bottom);
	//container->DisposeWindowFamily(crtclp);

	
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
			delayedAbility = nullptr;
			avatar->StopMovement();
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

bool HologramScoutMod::CanUse(byte abilityIndex)
{
	auto avatar = GameNounManager.GetAvatar();
	auto ability = avatar->GetAbility(abilityIndex);

	if (!avatar->mpCombatantTarget)
	{
		return false;
	}

	auto distance = Math::distance(avatar->mPosition, avatar->mpCombatantTarget->ToSpatialObject()->mPosition);
	//SporeDebugPrint("distance is %f, range is %f", distance, ability->mRange);
	if (floor(distance) > ability->mRange && distance > ability->mRushingRange)
	{
		return false;
	}
	if (ceil(distance) < ability->mAvatarRangeMin)
	{
		return false;
	}

	if (ability->mEnergyCost > avatar->mEnergy)
	{
		return false;
	}

	if (avatar->mRechargingAbilityBits[abilityIndex])
	{
		return false;
	}

	return true;
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
