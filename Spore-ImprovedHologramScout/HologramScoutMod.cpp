#include "stdafx.h"
#include "HologramScoutMod.h"

HologramScoutMod* HologramScoutMod::sInstance;
byte HologramScoutMod::RenderToUse;

HologramScoutMod::HologramScoutMod()
{
	RenderToUse = 0;
	mpLayout = nullptr;
	sInstance = this;

	isSpecial = 0;
	wasActive = 0;
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
			}
		}

		if (isSpecial)
		{

			//avatar->mFlags = 0x220;
			avatar->mbIsGhost = 0;
			avatar->mbIsTangible = 0;
			avatar->mbKeepPinnedToPlanet = 0;
			avatar->mbEnabled = 1;
			avatar->mbFixed = 0;
			avatar->mbIsInvalid = 0;
			avatar->mbSelfPowered = true;
			avatar->mCurrentLoudness = 0;
			//object_cast<Simulator::cCombatant>(avatar)->field_80 = 0;
		}
	}
	else
	{
		if (wasActive && Simulator::IsSpaceGame())
		{
			for (auto creature : Simulator::GetData<Simulator::cCreatureAnimal>())
			{
				creature->SetScale(creature->mScale);
			}
		}
		wasActive = 0;
		isSpecial = 0;
	}

	if (!(Simulator::IsSpaceGame() && GameNounManager.GetAvatar()) && mpLayout)
	{
		mpLayout = nullptr;
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



// You can extend this function to return any other types your class implements.
void* HologramScoutMod::Cast(uint32_t type) const
{
	CLASS_CAST(Object);
	CLASS_CAST(HologramScoutMod);
	return nullptr;
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