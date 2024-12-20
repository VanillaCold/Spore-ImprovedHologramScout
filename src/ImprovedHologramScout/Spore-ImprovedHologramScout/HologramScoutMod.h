#pragma once

#include <Spore\BasicIncludes.h>

#define HologramScoutModPtr intrusive_ptr<HologramScoutMod>

class HologramScoutMod
	: public Object
	, public DefaultRefCounted
	, public App::IUpdatable
{
protected:
	static HologramScoutMod* sInstance;
	Simulator::cGonzagoTimer deltaTimer;

	bool mbPressedSpace;
	bool mbPressedTab;

public:
	static byte RenderToUse;
	static const uint32_t TYPE = id("HologramScoutMod");
	
	bool isSpecial;
	bool wasActive;
	GameInput input;
	float boundingRadius;
	float mMaxPlayerHealth;

	HologramScoutMod();
	~HologramScoutMod();

	void InitialiseAbilities(bool isSpecial);

	void Update() override;

	void SelectCombatant(cCombatantPtr combatant);
	void DeselectCombatant();

	void GetPlayerInput(cCreatureBasePtr avatar);

	int AddRef() override;
	int Release() override;

	void TriggerSkill(Simulator::cCreatureAbility* ability);
	bool CanUse(byte abilityIndex);

	void OpenUI(bool showAbilities);
	void UpdateUI();
	void CloseUI();

	UILayoutPtr mpLayout;
	cCombatantPtr mpHoveredCombatant;
	cCombatantPtr mpSelectedCombatant;

	Simulator::cCreatureAbility* delayedAbility;

	bool mbAbilityMode;
	map<byte, cCreatureAbilityPtr> mpScanAbilities;
	map<byte, cCreatureAbilityPtr> mpCombatSkills;

	static HologramScoutMod* Get();
	void* Cast(uint32_t type) const override;
};
