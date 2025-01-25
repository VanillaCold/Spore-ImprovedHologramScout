#pragma once

#include <Spore\BasicIncludes.h>

#define HologramUIManagerPtr intrusive_ptr<HologramUIManager>

class HologramUIManager 
	: public Object
	, public DefaultRefCounted
{
public:
	static const uint32_t TYPE = id("HologramUIManager");
	
	HologramUIManager();
	~HologramUIManager();

	int AddRef() override;
	int Release() override;
	void* Cast(uint32_t type) const override;
};
