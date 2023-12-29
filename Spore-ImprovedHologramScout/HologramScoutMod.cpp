#include "stdafx.h"
#include "HologramScoutMod.h"

HologramScoutMod* HologramScoutMod::sInstance;
byte HologramScoutMod::RenderToUse;

HologramScoutMod::HologramScoutMod()
{
	RenderToUse = 0;
	sInstance = this;
}


HologramScoutMod::~HologramScoutMod()
{
}

// For internal use, do not modify.
int HologramScoutMod::AddRef()
{
	return DefaultRefCounted::AddRef();
}

// For internal use, do not modify.
int HologramScoutMod::Release()
{
	return DefaultRefCounted::Release();
}

HologramScoutMod* HologramScoutMod::Get()
{
	return sInstance;
}

void HologramScoutMod::Update()
{
	if (isSpecial == 1 && Simulator::IsSpaceGame() && GameNounManager.GetAvatar())
	{
		//field_101C
		//field_166C
		//field_1034
		//field_1124
		auto avatar = GameNounManager.GetAvatar();
		//avatar->mStandardSpeed = 10;
		//avatar->mTurnRate = 10;
		//avatar->mDesiredSpeed = 10;
		//avatar->GetModel()->GetModelWorld()->SetLightingWorld(Simulator::GetPlayerUFO()->GetModel()->GetModelWorld()->GetLightingWorld(0),0,1);
		//avatar->field_101C = 0;
		//avatar->field_166C = 0;
		//avatar->GetModel()->field_5C = 0
		
		/*if (avatar->mGeneralFlags |= 0x200)
		{
			avatar->mGeneralFlags -= 0x200;
		}*/
		
		//avatar->mFlags = 512;
		avatar->mbIsGhost = 0;
		avatar->mbIsTangible = 0;
		avatar->mbKeepPinnedToPlanet = 0;
		avatar->mbEnabled = 1;
		avatar->mCurrentLoudness = 0;
		//object_cast<Simulator::cCombatant>(avatar)->field_80 = 0;
	}
	else
	{
		isSpecial = 0;
	}
}

// You can extend this function to return any other types your class implements.
void* HologramScoutMod::Cast(uint32_t type) const
{
	CLASS_CAST(Object);
	CLASS_CAST(HologramScoutMod);
	return nullptr;
}
