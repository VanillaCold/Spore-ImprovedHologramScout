#pragma once

#include <Spore\BasicIncludes.h>

#define HologramUIManagerPtr intrusive_ptr<HologramUIManager>

class HologramUIManager 
	: public Object
	, public DefaultRefCounted
	, public App::IUpdatable
{
protected:
	static HologramUIManager* sInstance;
public:
	static const uint32_t TYPE = id("HologramUIManager");
	
	HologramUIManager();
	~HologramUIManager();

	void Update() override;

	void OpenUI(bool showAbilities);
	void UpdateUI();
	void CloseUI();

	void DeselectUI();
	static HologramUIManager* Get();

	UILayoutPtr mpLayout;


	int AddRef() override;
	int Release() override;
	void* Cast(uint32_t type) const override;


};
