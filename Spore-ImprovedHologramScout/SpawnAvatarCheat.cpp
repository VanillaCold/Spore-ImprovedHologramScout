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

	auto avatar = GameNounManager.GetAvatar();
	auto creaturebase = object_cast<Simulator::cCreatureBase>(avatar);

	avatar->mbKeepPinnedToPlanet = 0;
	//avatar->mbTeleport = 1;

	//avatar->Teleport(avatar->mPosition + avatar->mPosition.Normalized(), avatar->mOrientation);

	//avatar->mVelocity += 5* avatar->mPosition.Normalized();
	if (line.HasFlag("flags"))
	{
		SporeDebugPrint("%x", avatar->mFlags);
		return;
	}

	if (line.HasFlag("attk"))
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
		SporeDebugPrint("%f", closestDist);
		CALL(Address(0x00c1e5c0), int, Args(Simulator::cCreatureBase*, int, Anim::AnimIndex), Args(avatar, 6, 0));
	}
	else
	{
		CALL(Address(0x00c18ca0), int, Args(Simulator::cCreatureBase*, int), Args(avatar, 2));
	}
	//auto a = line.GetArguments(1);
	//byte b = mpFormatParser->ParseUInt(a[0]);

	//HologramScoutMod::RenderToUse = b;

	return;
	if (line.HasFlag("test"))
	{
		for (auto creature : Simulator::GetData<Simulator::cCreatureAnimal>())
		{
			//if (creature->m)
			//{
				App::ConsolePrintF("%i, %i, %b, %b", creature->mFlags, creature->mGeneralFlags, creature->mbIsTangible, creature->mbIsGhost);
			//}
		}

		for (auto city : Simulator::GetData<Simulator::cCityWalls>())
		{
			//city->field_25C = 0;
			city->mWallSize = 0;
			city->GetModel()->mDefaultBBox.upper = Vector3(0,0,0);
			city->GetModel()->mDefaultBBox.lower = Vector3(0, 0, 0);
			city->GetModel()->mDefaultBoundingRadius = 0;
			SporeDebugPrint("%b, %b, %b, %b", city->field_A4, city->field_A5, city->field_A7, city->field_20);
			city->GetModel()->mCollisionMode = Graphics::CollisionMode::BoundingBox;
			city->mBoundingRadius = 0;
			city->GetModel()->GetModelWorld()->UpdatePickMesh(city->GetModel());
			city->mpCity->SetHasModelChanged(1);
			city->SetHasModelChanged(1);

			SporeDebugPrint("aaaa");
		}

		return;
	}

	if (!GameNounManager.GetAvatar())
	{
		Sporepedia::ShopperRequest request = Sporepedia::ShopperRequest(this);
		request.Show(request);
	}
	else
	{
		auto avatar = GameNounManager.GetAvatar();
		auto creaturebase = object_cast<Simulator::cCreatureBase>(avatar);
		
		avatar->mbKeepPinnedToPlanet = 0;
		avatar->mbTeleport = 1;
		avatar->mVelocity += 5 * avatar->mPosition.Normalized();
		
		//avatar->SetVelocity(vel);
		
		
		auto loco = avatar->ToLocomotiveObject();
		creaturebase->field_0C = avatar->field_0C;
		//loco->mStandardSpeed = avatar->mStandardSpeed;
		SporeDebugPrint("%f, %f, %f", avatar->mStandardSpeed, loco->mStandardSpeed, creaturebase->mStandardSpeed);
		//SporeDebugPrint("Flags (model): %i, Flags (creature): %i, Speed state: %i, Unknown model bool: %b, Group model flags: %i, %i", avatar->mFlags, avatar->mGeneralFlags, avatar->mSpeedState, avatar->GetModel()->field_5C, avatar->GetModel()->mGroupFlags[0], avatar->GetModel()->mGroupFlags[1]);
	}
	// This method is called when your cheat is invoked.
	// Put your cheat code here.
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
