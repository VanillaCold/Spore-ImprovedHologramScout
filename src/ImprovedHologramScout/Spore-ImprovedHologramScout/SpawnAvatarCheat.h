#pragma once

#include <Spore\BasicIncludes.h>

class SpawnAvatarCheat
	: public ArgScript::ICommand
	, public Sporepedia::IShopperListener
{
public:
	SpawnAvatarCheat();
	~SpawnAvatarCheat();

	// Called when the cheat is invoked
	void ParseLine(const ArgScript::Line& line) override;
	virtual void OnShopperAccept(const ResourceKey& selection);
	
	// Returns a string containing the description. If mode != DescriptionMode::Basic, return a more elaborated description
	const char* GetDescription(ArgScript::DescriptionMode mode) const override;
};

