#include "stdafx.h"
#include "Detours.h"

void HologramAudioDetour::DETOUR(uint32_t id, Audio::AudioTrack track, Vector3 pos)
{
	if (id == 0x73ecda8a && Simulator::IsSpaceGame() && HologramScoutManager::Get()->mbHasBeamedDown)
	{
		return;
	}
	else
	{
		return original_function(id, track, pos);;
	}
}

void SetRenderType__detour::DETOUR(int renderType, bool arg2) 
{
	if (renderType == 15 && Simulator::IsSpaceGame() && HologramScoutManager::Get()->mbHasBeamedDown == 1)
	{
		renderType = 0;
	}
	original_function(this, renderType, arg2);
}