#include "stdafx.h"
#include "HologramCombatManager.h"
#include <Spore/Simulator/SimulatorMessages.h>

HologramCombatManager::HologramCombatManager()
{
	sInstance = this;
	MessageManager.AddListener(this, Simulator::SimulatorMessages::kMsgSaveGame);
	MessageManager.AddListener(this, Simulator::SimulatorMessages::kMsgSwitchGameMode);
	MessageManager.AddListener(this, id("SpaceGameAttackUsed"));

	mpLastUsedAbilities = hash_map<uint32_t, Simulator::cCreatureAbility*>();
	mpLastCreatureToAttack = nullptr;
}


HologramCombatManager::~HologramCombatManager()
{
}

HologramCombatManager* HologramCombatManager::Get()
{
	return sInstance;
}

Simulator::cCreatureAbility* HologramCombatManager::GetLastAbilityUsed(cCreatureBasePtr crt)
{
	auto index = crt->mID;
	auto node = mpLastUsedAbilities.find(index);
	if (node != mpLastUsedAbilities.end())
	{
		return node->second;
	}
	return nullptr;
}

HologramCombatManager* HologramCombatManager::sInstance;


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
			mpLastUsedAbilities.emplace(castMessage->mpSourceCreature->mID, castMessage->mpAbility);
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