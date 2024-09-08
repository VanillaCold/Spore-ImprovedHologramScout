#include "stdafx.h"
#include "HologramScoutMod.h"
#include <Spore/Simulator/cCreatureGameData.h>

HologramScoutMod* HologramScoutMod::sInstance;
byte HologramScoutMod::RenderToUse;

HologramScoutMod::HologramScoutMod()
{
	RenderToUse = 0;
	mpLayout = nullptr;
	sInstance = this;

	isSpecial = 0;
	wasActive = 0;
	mbPressedSpace = 0;
}


HologramScoutMod::~HologramScoutMod()
{
}

HologramScoutMod* HologramScoutMod::Get()
{
	return sInstance;
}

void HologramScoutMod::Update()
{
	if (Simulator::IsSpaceGame() && GameNounManager.GetAvatar())
	{
		wasActive = 1;
		auto avatar = GameNounManager.GetAvatar();
		for (auto creature : Simulator::GetData<Simulator::cCreatureAnimal>())
		{
			if (creature != avatar && creature->mbEnabled)
			{
				creature->SetScale(creature->mScale * 0.25f);
				creature->mScale /= (0.25f);
				creature->mMaxHealthPoints = max(creature->mHealthPoints, creature->mMaxHealthPoints);
			}
		}

		//Selection code
		Vector3 cameraPosition, mouseDir;
		App::GetViewer()->GetCameraToMouse(cameraPosition, mouseDir);
		auto viewDir = CameraManager.GetViewer()->GetViewTransform().GetRotation().Row(1);
		vector<cSpatialObjectPtr> objects;
		Vector3 intersection;

		Graphics::FilterSettings filter;
		filter.collisionMode = Graphics::CollisionMode::MeshCluster;

		if (GameViewManager.RaycastAll(cameraPosition, cameraPosition + mouseDir * 1000.0f, objects, true))
		{
			cCombatantPtr comb;
			for each (cSpatialObjectPtr obj in objects)
			{
				if (object_cast<Simulator::cCombatant>(obj))
				{
					comb = object_cast<Simulator::cCombatant>(obj);
					break;
				}
			}

			if (comb != nullptr)
			{
				if (PlanetModel.mpTerrain->Raycast(cameraPosition, comb->ToSpatialObject()->mPosition) == Vector3(0, 0, 0))
				{
					SporeDebugPrint("raycasted!");

					SporeDebugPrint("%f, %f", comb->mHealthPoints, comb->mMaxHealthPoints);
					if (mpHoveredCombatant != comb)
					{
						comb->ToSpatialObject()->SetIsRolledOver(true);
						if (mpHoveredCombatant)
						{
							mpHoveredCombatant->ToSpatialObject()->SetIsRolledOver(false);
						}
						mpHoveredCombatant = comb;
					}
				}
				else
				{
					comb = nullptr;
					mpHoveredCombatant = nullptr;
				}
			}
			else
			{
				if (mpHoveredCombatant != nullptr)
				{
					mpHoveredCombatant->ToSpatialObject()->SetIsRolledOver(false);
				}
			}
		}

		GetPlayerInput(avatar);
		UpdateUI();
			
		if (isSpecial)
		{
			

			//avatar->mFlags = 0x220;
			avatar->mbIsGhost = 0;
			avatar->mbIsTangible = 1;
			avatar->mbKeepPinnedToPlanet = 0;
			avatar->mbEnabled = 1;
			avatar->mbFixed = 0;
			avatar->mbIsInvalid = 0;
			avatar->mbSelfPowered = true;
			avatar->mCurrentLoudness = 0;
			//object_cast<Simulator::cCombatant>(avatar)->field_80 = 0;
		}
		else
		{
			object_cast<Simulator::cCombatant>(avatar)->field_80 = true;
		}
	}
	else
	{
		if (wasActive && Simulator::IsSpaceGame())
		{
			for (auto creature : Simulator::GetData<Simulator::cCreatureAnimal>())
			{
				creature->SetScale(creature->mScale);
				creature->mMaxHealthPoints = 1;
			}
		}
		wasActive = 0;
		isSpecial = 0;
	}

	if (!(Simulator::IsSpaceGame() && GameNounManager.GetAvatar()) && mpLayout)
	{
		if (Simulator::IsSpaceGame())
		{
			CloseUI();
		}
		mpLayout = nullptr;
	}
}




void HologramScoutMod::GetPlayerInput(cCreatureBasePtr avatar)
{
	if (GameInputManager.IsTriggered(0x00000007)) //Spacebar
	{
		if (!mbPressedSpace)
		{
			mbPressedSpace = 1;
			if (avatar->mbSupported || CreatureGameData.mNumFlapsAllowed > 0)
			{
				if (avatar->mbSupported)
				{
					auto flight = avatar->GetAbility(avatar->GetAbilityIndexByType(18));
					if (flight)
					{
						float gliding;
						App::Property::GetFloat(flight->mpPropList.get(), 0x02CC974A, gliding);
						CreatureGameData.mNumFlapsAllowed = round(gliding) * 2;
					}
				}
				else
				{
					CreatureGameData.mNumFlapsAllowed -= 1;
				}

				avatar->DoJump(50 * (!isSpecial));
			}
			else
			{
				CreatureGameData.mNumFlapsAllowed = 0;
			}
		}
	}
	else
	{
		mbPressedSpace = false;
	}


	bool leftClick = GameInputManager.IsTriggered(0x00000016);
	if (leftClick) //Left click
	{
		SporeDebugPrint("click!");
		if (mpHoveredCombatant)
		{
			SelectCombatant(mpHoveredCombatant);
		}
	}
}


void HologramScoutMod::OpenUI(bool useAbilities)
{
	if (mpLayout == nullptr)
	{
		mpLayout = new UTFWin::UILayout();
	}
	mpLayout->LoadByID(id("HoloScoutUI"));
	mpLayout->SetParentWindow(WindowManager.GetMainWindow()->FindWindowByID(0x05E66E88));
	mpLayout->SetVisible(true);
	mpLayout->GetContainerWindow()->AddWinProc(new UTFWin::SimpleLayout());
	mpLayout->GetContainerWindow()->SendToBack(mpLayout->GetContainerWindow());
	
	if (!useAbilities)
	{
		mpLayout->FindWindowByID(id("CrtSpecialAbilities"))->SetVisible(false);
		mpLayout->FindWindowByID(id("CrtBaseAbilities"))->SetVisible(false);
		mpLayout->FindWindowByID(id("InfHealthDisplay"))->SetVisible(true);
	}
	auto avatar = GameNounManager.GetAvatar();
	auto imageKey = ResourceKey(avatar->mSpeciesKey.instanceID, TypeIDs::png, avatar->mSpeciesKey.groupID);

	auto imageDraw = new UTFWin::ImageDrawable();
	ImagePtr image;
	UTFWin::Image::GetImage(imageKey, image);
	imageDraw->SetImage(image.get());
	mpLayout->FindWindowByID(id("CreatureIcon"))->SetDrawable(imageDraw);

	WindowManager.GetMainWindow()->FindWindowByID(0x065E40F0)->GetParent()->SetVisible(false);
}

void HologramScoutMod::UpdateUI()
{
	//doing this so that we won't need to recursively search all windows repeatedly
	auto statDisplay = mpLayout->FindWindowByID(id("StatDisplay"));
	auto avatar = GameNounManager.GetAvatar();
	if (!avatar || !statDisplay)
	{
		CloseUI();
		return;
	}

	auto healthText = statDisplay->FindWindowByID(id("CrtHealthText"));
	
	string16 text;
	text.sprintf(u"%i", int(avatar->mHealthPoints));

	healthText->SetCaption(text.c_str());


	float healthPercent = avatar->mHealthPoints / avatar->mMaxHealthPoints;
	auto healthBar = statDisplay->FindWindowByID(id("CrtHealthBar"));
	healthBar->SetArea(Math::Rectangle(0, 0, 68 * healthPercent, 13));
	healthBar->SetFlag(UTFWin::WindowFlags::kWinFlagClip, true);
	healthBar->GetParent()->SetFlag(UTFWin::WindowFlags::kWinFlagClip, true);

}

void HologramScoutMod::CloseUI()
{
	mpLayout = nullptr;
	WindowManager.GetMainWindow()->FindWindowByID(0x065E40F0)->GetParent()->SetVisible(true);
}



// You can extend this function to return any other types your class implements.
void* HologramScoutMod::Cast(uint32_t type) const
{
	CLASS_CAST(Object);
	CLASS_CAST(HologramScoutMod);
	return nullptr;
}

void HologramScoutMod::SelectCombatant(cCombatantPtr combatant)
{
	GameNounManager.GetAvatar()->mpCombatantTarget = combatant.get();
	mpLayout->FindWindowByID(id("TargetCreatureUI"))->SetVisible(true);
	mpSelectedCombatant = combatant;
	SporeDebugPrint("Selected combatant!");

	if (object_cast<Simulator::cSpatialObject>(combatant))
	{
		Simulator::cSpatialObject* spatial = object_cast<Simulator::cSpatialObject>(combatant);
		auto& key = spatial->GetModelKey();

		SporeDebugPrint("(0x%x!0x%x.0x%x)", key.groupID, key.instanceID, key.typeID);
		auto imageKey = ResourceKey(key.instanceID, TypeIDs::png, key.groupID);

		if (ResourceManager.GetResource(imageKey, nullptr))
		{
			auto imageDraw = new UTFWin::ImageDrawable();
			ImagePtr image;
			UTFWin::Image::GetImage(imageKey, image);
			imageDraw->SetImage(image.get());
			mpLayout->FindWindowByID(id("TargetCreatureImage"))->SetDrawable(imageDraw);
		}
	}
}

void HologramScoutMod::DeselectCombatant()
{
	mpLayout->FindWindowByID(id("TargetCreatureUI"))->SetVisible(false);
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