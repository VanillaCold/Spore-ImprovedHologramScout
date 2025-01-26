#include "stdafx.h"
#include "HologramUIManager.h"
#include "HologramCombatManager.h"
#include "HologramScoutManager.h"

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

void HologramUIManager::DeselectUI()
{
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
