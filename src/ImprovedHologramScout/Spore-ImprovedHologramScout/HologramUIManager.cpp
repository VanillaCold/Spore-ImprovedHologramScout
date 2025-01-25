#include "stdafx.h"
#include "HologramUIManager.h"

HologramUIManager::HologramUIManager()
{
}


HologramUIManager::~HologramUIManager()
{
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
