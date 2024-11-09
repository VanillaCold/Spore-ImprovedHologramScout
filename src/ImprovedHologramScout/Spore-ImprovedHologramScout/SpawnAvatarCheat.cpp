#include "stdafx.h"
#include "SpawnAvatarCheat.h"
#include "HologramScoutMod.h"

SpawnAvatarCheat::SpawnAvatarCheat()
{
	
}


SpawnAvatarCheat::~SpawnAvatarCheat()
{
}


void SpawnAvatarCheat::ParseLine(const ArgScript::Line& line)
{
	if (line.HasFlag("c"))
	{
		float a = 9999.0;
		uint32_t& b = (uint32_t&)a;
		SporeDebugPrint("0x%X", b);
		return;
	}

	if (line.HasFlag("b"))
	{
		auto hover = GameViewManager.GetHoveredObject();
		auto plant = object_cast<Simulator::cGamePlant>(hover);

		uint32_t classType = 0;
		if (hover != nullptr)
		{
			classType = hover->GetNounID();
		}

		SporeDebugPrint("plant flag! %b", GamePlantManager.field_20 & 1);

		SporeDebugPrint("hover? %b\nplant? %b\nclass noun ID: 0x%x", hover != nullptr, plant != nullptr, classType);
		return;
	}


	if (line.HasFlag("a"))
	{
		SporeDebugPrint("%x", GameModeManager.GetActiveMode());
		return;
	}

	auto avatar = GameNounManager.GetAvatar();
	auto creaturebase = object_cast<Simulator::cCreatureBase>(avatar);


	//auto prof = avatar->mpSpeciesProfile;
	//SporeDebugPrint("%i", prof->field_510);

	avatar->mbKeepPinnedToPlanet = 0;
	//avatar->mbTeleport = 1;

	//avatar->Teleport(avatar->mPosition + avatar->mPosition.Normalized(), avatar->mOrientation);

	//avatar->mVelocity += 5* avatar->mPosition.Normalized();
	if (line.HasFlag("flags"))
	{
		avatar->mpAnimatedCreature->mpModel->mCollisionMode = Graphics::CollisionMode::MeshTriangle;
		avatar->mpAnimatedCreature->mpModel->AddGroup(3);

		//char grp2Flags[32]{ " " };
		//auto thing2 = _itoa_s(f2, grp2Flags, 2);
		
		//SporeDebugPrint(genFlags);
		//SporeDebugPrint(grp2Flags);
		return;
	}

	if (line.HasFlag("attk"))
	{
		if (!avatar->mpCombatantTarget)
		{
			Vector3 pos = avatar->mPosition;
			float closestDist = 99999999999999;
			for (auto combatant : Simulator::GetData<Simulator::cCreatureAnimal>())
			{
				float dist = Math::distance(combatant->ToSpatialObject()->GetPosition(), pos);
				if (dist < closestDist && combatant != avatar && dist != 0.0)
				{
					closestDist = dist;
					avatar->mpCombatantTarget = combatant.get();
					avatar->mpTarget = combatant.get();
				}
			}
		}

		auto count = avatar->GetAbilitiesCount();
		int index = 6;
		for (int i = 0; i < count; i++)
		{
			auto ability = avatar->GetAbility(i);
			if (ability && ability->mCategory == 1)
			{
				index = i;
				break;
			}
		}
		size_t argCount;
		auto args = line.GetArgumentsRange(&argCount, 0, 1);
		
		if (argCount != 0)
		{
			index = mpFormatParser->ParseInt(args[0]);
		}

		//auto ability = avatar->GetAbility(index);
		//SporeDebugPrint("%i, %ls, 0x%x", index,
		//	ability->nName.GetText(), ability->mpPropList->GetResourceKey().instanceID );

		avatar->PlayAbility(index);
		//CALL(Address(0x00c1e5c0), int, Args(Simulator::cCreatureBase*, int, Anim::AnimIndex), Args(avatar, index, 0));
	}
	else
	{
		avatar->DoJump(0);
		//CALL(Address(0x00c18ca0), int, Args(Simulator::cCreatureBase*, int), Args(avatar, 50));
	}
}

void SpawnAvatarCheat::OnShopperAccept(const ResourceKey& selection)
{
	using namespace Simulator;
	if (selection != ResourceKey(0, 0, 0))
	{
		Vector3 position = { 0,0,0 };
		if (IsSpaceGame())
		{
			position = GetPlayerUFO()->mPosition;
		}
		auto creature = Simulator::cCreatureAnimal::Create(position, SpeciesManager.GetSpeciesProfile(selection), 1, nullptr, 0);
		SporeDebugPrint(" % i, % i", creature->mFlags, creature->mGeneralFlags);
		
		//creature->field_BE0 = Vector3(0.0F, 50.0F, 0.0F);
		//creature->mVelocity = Vector3(0.0F, 50.0F, 0.0F);
	}
}

const char* SpawnAvatarCheat::GetDescription(ArgScript::DescriptionMode mode) const
{
	if (mode == ArgScript::DescriptionMode::Basic) {
		return "This cheat does something.";
	}
	else {
		return "SpawnAvatarCheat: Elaborate description of what this cheat does.";
	}
}
