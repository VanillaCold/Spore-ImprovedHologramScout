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
public:
	static byte RenderToUse;
	static const uint32_t TYPE = id("HologramScoutMod");
	
	bool isSpecial;
	bool wasActive;
	GameInput input;
	float boundingRadius;

	HologramScoutMod();
	~HologramScoutMod();

	void Update() override;

	int AddRef() override;
	int Release() override;

	void OpenUI(bool showAbilities);

	UILayoutPtr mpLayout;

	static HologramScoutMod* Get();
	void* Cast(uint32_t type) const override;
};
