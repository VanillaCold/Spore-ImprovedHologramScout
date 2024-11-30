#include "stdafx.h"
#include "SpawnObject.h"
#include <Spore/Simulator/cGameTerrainCursor.h>

SpawnObject::SpawnObject()
{
}


SpawnObject::~SpawnObject()
{
}


void SpawnObject::ParseLine(const ArgScript::Line& line)
{
	Vector3 position = { 0, 0, 0 };
	if (auto cursor = Simulator::cGameTerrainCursor::GetTerrainCursor()) {
		position = cursor->GetPosition();
	}
	else
	{
		position = GameNounManager.GetAvatar()->GetPosition();
	}


	//HoloScout_TestProp
	auto test = simulator_new<Simulator::cInteractiveOrnament>();
	ResourceKey thingy = ResourceKey(id("HoloScout_TestProp"), TypeIDs::prop, 0x02E9C426);

	struct DefinitionStruct //We define a struct here, as the actual one isn't in the SDK yet.
	{
		uint32_t nounID;
		uint32_t definitionID;
		const Vector3& position = position;
		PropertyListPtr propList;
	}definition;

	definition.definitionID = thingy.instanceID;
	definition.nounID = Simulator::GameNounIDs::kInteractiveOrnament;

	PropManager.GetPropertyList(thingy.instanceID, thingy.groupID, definition.propList);

	test->SetDefinitionID((int)&definition, 0, 0);

	test->SetScale(test->mScale);
	test->mbFixed = true;

	test->mbFixed = true;
	test->mbPickable = false;
	test->mbKeepPinnedToPlanet = false;
	test->mbIsTangible = true;
	test->mbIsGhost = false;
	test->mbTransformDirty = false;

	//CALL(Address(0x00c3fde0), int, Args(Simulator::cGameData*, int, size_t, App::Property*), Args(object_cast<Simulator::cGameData>(test), (int)&definition, 0, nullptr));


	test->Teleport(position,Math::Quaternion());
}

const char* SpawnObject::GetDescription(ArgScript::DescriptionMode mode) const
{
	if (mode == ArgScript::DescriptionMode::Basic) {
		return "This cheat does something.";
	}
	else {
		return "SpawnObject: Elaborate description of what this cheat does.";
	}
}
