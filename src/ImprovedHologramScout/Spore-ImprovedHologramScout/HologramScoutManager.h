#pragma once

#include <Spore\BasicIncludes.h>

#define HologramScoutManagerPtr intrusive_ptr<HologramScoutManager>

///
/// In your dllmain Initialize method, add the system like this:
/// ModAPI::AddSimulatorStrategy(new HologramScoutManager(), HologramScoutManager::NOUN_ID);
///

class HologramScoutManager
	: public Simulator::cStrategy
{
public:
	static const uint32_t TYPE = id("Spore_ImprovedHologramScout::HologramScoutManager");
	static const uint32_t NOUN_ID = TYPE;

	int AddRef() override;
	int Release() override;
	void Initialize() override;
	void Dispose() override;
	const char* GetName() const override;
	bool Write(Simulator::ISerializerStream* stream) override;
	bool Read(Simulator::ISerializerStream* stream) override;
	void Update(int deltaTime, int deltaGameTime) override;

	//
	// You can add more methods here
	//

	static Simulator::Attribute ATTRIBUTES[];

	void RescaleCreatures();
	Simulator::cCombatant* GetHoveredCombatant();
	void GetPlayerInput();


private:
	//
	// You can add members here
	//
	bool mbHasBeamedDown;
	bool mbWasActive;

	
	float mMaxPlayerHealth;
};