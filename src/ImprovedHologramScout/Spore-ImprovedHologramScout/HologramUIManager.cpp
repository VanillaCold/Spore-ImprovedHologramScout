#include "stdafx.h"
#include "HologramUIManager.h"
#include "HologramCombatManager.h"
#include "HologramScoutManager.h"
#include "ButtonWinProc.h"
#include "SwitchWinProc.h"

HologramUIManager* HologramUIManager::sInstance;

HologramUIManager::HologramUIManager()
{
	sInstance = this;
}


HologramUIManager::~HologramUIManager()
{
}

void HologramUIManager::Update()
{
	if (Simulator::IsSpaceGame() && GameNounManager.GetAvatar())
	{
		auto combatManager = HologramCombatManager::Get();
		auto mainManager = HologramScoutManager::Get();

		const bool& abilityMode = combatManager->mbAbilityMode;

		auto specialAbilitySlots = mpLayout->FindWindowByID(id("CrtSpecialAbilities"));
		if (specialAbilitySlots)
		{
			specialAbilitySlots->FindWindowByID(id("ScanPanel"))->SetVisible(!abilityMode);
			specialAbilitySlots->FindWindowByID(id("CombatPanel"))->SetVisible(abilityMode);

			object_cast<UTFWin::IButton>(specialAbilitySlots->FindWindowByID(id("CombatButton")))->SetButtonStateFlag(4, abilityMode);
			object_cast<UTFWin::IButton>(specialAbilitySlots->FindWindowByID(id("ScanButton")))->SetButtonStateFlag(4, !abilityMode);

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

		auto selected = combatManager->mpSelectedCombatant;

		if (selected != nullptr)
		{
			//TargetHealthbar
			auto targetUI = mpLayout->FindWindowByID(id("TargetCreatureUI"));
			auto tHealthBar = targetUI->FindWindowByID(id("TargetHealthbar"));
			auto tEnergyBar = targetUI->FindWindowByID(id("TargetHealthbar"));

			auto tHealthCaption = targetUI->FindWindowByID(id("TargetHealthText"));
			auto tEnergyCaption = targetUI->FindWindowByID(id("TargetEnergyText"));

			string16 tHealthText;
			auto tHealth = selected->mHealthPoints;
			tHealthText.sprintf(u"%i", int(selected->mHealthPoints));

			tHealthCaption->SetCaption(tHealthText.c_str());

			float tMaxHealth = selected->mMaxHealthPoints;

			auto crtTarget = object_cast<Simulator::cCreatureBase>(selected);
			if (crtTarget)
			{
				if (combatManager->mpMaxHealthPoints.find(crtTarget) != combatManager->mpMaxHealthPoints.end())
				{
					tMaxHealth = combatManager->mpMaxHealthPoints[crtTarget];
				}

				targetUI->FindWindowByID(id("TargetEnergyContainer"))->SetVisible(true);
				auto creature = object_cast<Simulator::cCreatureBase>(selected);
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

}

void HologramUIManager::OpenUI(bool showAbilities)
{
	auto combatManager = HologramCombatManager::Get();
	combatManager->mbAbilityMode = 0;

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

	if (!showAbilities)
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
		if (combatManager->mpCombatSkills.find(i) != combatManager->mpCombatSkills.end())
		{
			button->SetVisible(true);
			auto window = new UTFWin::Window();

			window->SetControlID(0x0);
			button->AddWindow(window);

			auto& skill = combatManager->mpCombatSkills[i];
			ResourceKey imgKey = ResourceKey(skill->mVerbIconImageID, TypeIDs::png, id("HoloAbilityIcons"));

			UTFWin::ImageDrawable* imageDrawable = new UTFWin::ImageDrawable();
			window->SetDrawable(imageDrawable);
			window->SetShadeColor(Math::Color(255, 255, 255, 240));

			window->SetArea(Math::Rectangle(8, 8, 28, 28));
			UTFWin::ImageDrawable::SetImageForWindow(window, imgKey);

			window->SetVisible(true);
			window->SetFlag(UTFWin::WindowFlags::kWinFlagIgnoreMouse, true);

			for (byte j = 0; j < avatar->GetAbilitiesCount(); j++)
			{
				if (combatManager->mpCombatSkills[i] == avatar->GetAbility(j))
				{
					button->AddWinProc(new ButtonWinProc(j));

					//creatureAbilityRolloverValueDescription

					Math::Vector2 minLevel;
					App::Property::GetVector2(combatManager->mpCombatSkills[i]->mpPropList.get(), 0x04052A86, minLevel);

					string16 tooltip;
					if (minLevel.x != 0)
					{
						tooltip.sprintf(u"%ls - Level %i", combatManager->mpCombatSkills[i]->nName.GetText(), int(minLevel.x));
					}
					else
					{
						tooltip = combatManager->mpCombatSkills[i]->nName.GetText();
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
		if (combatManager->mpScanAbilities.find(i) != combatManager->mpScanAbilities.end())
		{
			button->SetVisible(true);
			auto window = new UTFWin::Window();

			window->SetControlID(0x0);
			button->AddWindow(window);

			auto& skill = combatManager->mpScanAbilities[i];
			ResourceKey imgKey = ResourceKey(skill->mVerbIconImageID, TypeIDs::png, id("HoloAbilityIcons"));

			UTFWin::ImageDrawable* imageDrawable = new UTFWin::ImageDrawable();
			window->SetDrawable(imageDrawable);
			window->SetArea(Math::Rectangle(8, 8, 28, 28));
			UTFWin::ImageDrawable::SetImageForWindow(window, imgKey);

			window->SetVisible(true);
			window->SetFlag(UTFWin::WindowFlags::kWinFlagIgnoreMouse, true);

			for (byte j = 0; j < avatar->GetAbilitiesCount(); j++)
			{
				if (combatManager->mpScanAbilities[i] == avatar->GetAbility(j))
				{
					button->AddWinProc(new ButtonWinProc(j));
					string16 tooltip = combatManager->mpScanAbilities[i]->nName.GetText();
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


void HologramUIManager::CloseUI()
{
	auto container = mpLayout->GetContainerWindow();

	container->DisposeAllWindowFamilies();

	mpLayout = nullptr;

	WindowManager.GetMainWindow()->FindWindowByID(0x065E40F0)->GetParent()->SetVisible(true);
}

void HologramUIManager::SelectCombatantUI(cCombatantPtr combatant)
{
	mpLayout->FindWindowByID(id("TargetCreatureUI"))->SetVisible(true);

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

void HologramUIManager::DeselectUI()
{
	mpLayout->FindWindowByID(id("TargetCreatureUI"))->SetVisible(false);
}

HologramUIManager* HologramUIManager::Get()
{
	return sInstance;
}

// For internal use, do not modify.
int HologramUIManager::AddRef()
{
	return DefaultRefCounted::AddRef();
}

// For internal use, do not modify.
int HologramUIManager::Release()
{
	return DefaultRefCounted::Release();
}

// You can extend this function to return any other types your class implements.
void* HologramUIManager::Cast(uint32_t type) const
{
	CLASS_CAST(Object);
	CLASS_CAST(HologramUIManager);
	return nullptr;
}
