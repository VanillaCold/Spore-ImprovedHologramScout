#include "stdafx.h"
#include "HologramCombatManager.h"
#include <Spore/Simulator/SimulatorMessages.h>
#include "HologramUIManager.h"

HologramCombatManager::HologramCombatManager()
{
	sInstance = this;
	MessageManager.AddListener(this, Simulator::SimulatorMessages::kMsgSaveGame);
	MessageManager.AddListener(this, Simulator::SimulatorMessages::kMsgSwitchGameMode);
	MessageManager.AddListener(this, id("SpaceGameAttackUsed"));

	mpLastUsedAbilities = hash_map<uint32_t, PropertyListPtr>();
	mpLastCreatureToAttack = nullptr;
}


HologramCombatManager::~HologramCombatManager()
{
}

HologramCombatManager* HologramCombatManager::Get()
{
	return sInstance;
}

cCreatureAbilityPtr HologramCombatManager::GetLastAbilityUsed(cCreatureBasePtr crt)
{
	auto index = crt->mID;
	auto node = mpLastUsedAbilities.find(index);
	if (node != mpLastUsedAbilities.end())
	{
		cCreatureAbilityPtr newAbility = new Simulator::cCreatureAbility();
		newAbility->Parse(newAbility.get(), node->second.get());
		return newAbility;
	}
	return nullptr;
}

HologramCombatManager* HologramCombatManager::sInstance;


void HologramCombatManager::InitialiseAbilities(bool isSpecial)
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

void HologramCombatManager::SelectCombatant(cCombatantPtr combatant)
{
	mpSelectedCombatant = combatant;
	GameNounManager.GetAvatar()->mpCombatantTarget = combatant.get();
	HologramUIManager::Get()->SelectCombatantUI(combatant);
}

void HologramCombatManager::TriggerSkill(Simulator::cCreatureAbility* ability)
{
	auto avatar = GameNounManager.GetAvatar();
	if (avatar && avatar->mpCombatantTarget && ability)
	{
		auto distance = Math::distance(avatar->mPosition, avatar->mpCombatantTarget->ToSpatialObject()->mPosition);
		//SporeDebugPrint("distance is %f, range is %f", distance, ability->mRange);
		if (floor(distance) > ability->mRange && distance > ability->mRushingRange)
		{
			avatar->WalkTo(1, avatar->mpCombatantTarget->ToSpatialObject()->mPosition, avatar->mpCombatantTarget->ToSpatialObject()->mPosition.Normalized());
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

bool HologramCombatManager::CanUse(byte abilityIndex)
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

// The method that receives the message. The first thing you should do is checking what ID sent this message...
bool HologramCombatManager::HandleMessage(uint32_t messageID, void* message)
{
	switch (messageID)
	{
		case Simulator::SimulatorMessages::kMsgCombatantKilled:
		{
			auto castMessage = (Simulator::CombatantKilledMessage*)(message);
			if (mpLastUsedAbilities.find(castMessage->GetCombatant()->ToGameData()->mID) != mpLastUsedAbilities.end())
			{
				mpLastUsedAbilities.erase(castMessage->GetCombatant()->ToGameData()->mID);
			}
			auto creature = object_cast<Simulator::cCreatureBase>(castMessage->GetCombatant());
			if (creature)
			{
				mpMaxHealthPoints.erase(creature);
			}

			break;
		}
		case id("SpaceGameAttackused"):
		{
			auto castMessage = (AbilityUsedData*)(message);
			PropertyListPtr propList;
			PropManager.GetPropertyList(castMessage->mAbilityKey.instanceID, castMessage->mAbilityKey.groupID, propList);

			if (mpLastUsedAbilities.find(castMessage->mpSourceCreature->mID) != mpLastUsedAbilities.end())
			{
				mpLastUsedAbilities.erase(castMessage->mpSourceCreature->mID);
			}

			mpLastUsedAbilities.emplace(castMessage->mpSourceCreature->mID, propList);
			mpLastCreatureToAttack = castMessage->mpSourceCreature;
		}
	}

	// Return true if the message has been handled. Other listeners will receive the message regardless of the return value.
	return true;
}


// For internal use, do not modify.
int HologramCombatManager::AddRef()
{
	return DefaultRefCounted::AddRef();
}

// For internal use, do not modify.
int HologramCombatManager::Release()
{
	return DefaultRefCounted::Release();
}