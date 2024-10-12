#pragma once
#include "stdafx.h"
#include <Spore/BasicIncludes.h>
#include "HologramScoutMod.h"
#include <Spore\Simulator\cGetOutOfUFOToolStrategy.h>
using namespace Simulator;

//Detouring the OnUse function of cGetOutOfUFOToolStrategy.
virtual_detour(Chocice75_ImprovedHologramScout_OnUseDetour, cGetOutOfUFOToolStrategy, cToolStrategy, bool(cSpaceToolData*)) {};


//Detour of the Update() function
virtual_detour(Chocice75_ImprovedHologramScout_UpdateDetour, cGetOutOfUFOToolStrategy, cToolStrategy, bool(cSpaceToolData*, bool, const char16_t**)){};

//This is needed to ensure that creatures in the space stage don't always deal 9999 damage like they do in vanilla. 
virtual_detour(OverrideCreatureDamageDetour, Simulator::cCreatureBase, Simulator::cCombatant, int(float, uint32_t, int, const Vector3&, cCombatant*)){};

//Stop the hologram sound playing when not using the hologram scout, but instead using the non-hologram scout.
static_detour(HologramAudioDetour, void(uint32_t, Audio::AudioTrack, Vector3)){};

//Stop the scout from looking like a hlogram when it's not a hologram.
member_detour(SetRenderType__detour, App::cViewer, void(int, bool)) {};

member_detour(PlayAbilityDetour, Simulator::cCreatureBase, void(int abilityIndex, Anim::AnimIndex* dstAnimIndex)) {};