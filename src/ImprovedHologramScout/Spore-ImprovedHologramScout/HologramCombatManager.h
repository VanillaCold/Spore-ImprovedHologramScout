#pragma once

#include <Spore\BasicIncludes.h>

#define HologramCombatManagerPtr intrusive_ptr<HologramCombatManager>

struct AbilityUsedData
{
	ResourceKey mAbilityKey;
	Simulator::cCreatureAbility* mpAbility;
	cCreatureBasePtr mpSourceCreature;
	cCombatantPtr mpCreatureTarget;
};

class HologramCombatManager 
	: public App::IMessageListener
	, public DefaultRefCounted
{
protected:
	static HologramCombatManager* sInstance;
	hash_map<uint32_t, PropertyListPtr> mpLastUsedAbilities;
public:
	static const uint32_t TYPE = id("HologramCombatManager");

	HologramCombatManager();
	~HologramCombatManager();

	int AddRef() override;
	int Release() override;

	static HologramCombatManager* Get();

	cCreatureAbilityPtr GetLastAbilityUsed(cCreatureBasePtr);
	hash_map<Simulator::cCreatureBase*, float> mpMaxHealthPoints;
	cCreatureBasePtr mpLastCreatureToAttack;
	
	// This is the function you have to implement, called when a message you registered to is sent.
	bool HandleMessage(uint32_t messageID, void* message) override;
};
