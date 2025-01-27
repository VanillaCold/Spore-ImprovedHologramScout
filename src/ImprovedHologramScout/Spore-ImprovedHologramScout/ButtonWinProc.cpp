#include "stdafx.h"
#include "ButtonWinProc.h"
#include "HologramCombatManager.h"
//#include "HologramScoutMod.h"

ButtonWinProc::ButtonWinProc(int attkID)
{
	this->attkID = attkID;
}


ButtonWinProc::~ButtonWinProc()
{
}

// For internal use, do not modify.
int ButtonWinProc::AddRef()
{
	return DefaultRefCounted::AddRef();
}

// For internal use, do not modify.
int ButtonWinProc::Release()
{
	return DefaultRefCounted::Release();
}

// You can extend this function to return any other types your class implements.
void* ButtonWinProc::Cast(uint32_t type) const
{
	CLASS_CAST(Object);
	CLASS_CAST(IWinProc);
	CLASS_CAST(ButtonWinProc);
	return nullptr;
}

// This method returns a combinations of values in UTFWin::EventFlags.
// The combination determines what types of events (messages) this window procedure receives.
// By default, it receives mouse/keyboard input and advanced messages.
int ButtonWinProc::GetEventFlags() const
{
	return kEventFlagBasicInput | kEventFlagAdvanced | kEventFlagUpdate;
}

// The method that receives the message. The first thing you should do is probably
// checking what kind of message was sent...
bool ButtonWinProc::HandleUIMessage(IWindow* window, const Message& message)
{
	if (!GameNounManager.GetAvatar())
	{
		return false;
	}

	auto ability = GameNounManager.GetAvatar()->GetAbility(attkID);

	if (message.IsType(MessageType::kMsgButtonSelect))
	{
		HologramCombatManager::Get()->TriggerSkill(ability);
		return true;
	}

	if (message.IsType(MessageType::kMsgUpdate))
	{
		auto button = object_cast<UTFWin::IButton>(window);
		string str;
		
		//char flags[32]{" "};
		//auto thing = _itoa_s(button->GetButtonStateFlags(), flags, 2);
		//SporeDebugPrint(flags);
		
		if (HologramCombatManager::Get()->CanUse(attkID))
		{
			button->SetButtonStateFlag(4, false);
		}
		else
		{
			button->SetButtonStateFlag(4, true);
		}

		auto val = 255 * !(button->GetButtonStateFlags() & 4);
		if (window->FindWindowByID(0x0))
			window->FindWindowByID(0x0)->SetShadeColor(Math::Color(val, val, val, 240));
	}


	// Return true if the message was handled, and therefore no other window procedure should receive it.
	return false;
}
