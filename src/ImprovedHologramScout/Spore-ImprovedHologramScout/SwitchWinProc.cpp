#include "stdafx.h"
#include "SwitchWinProc.h"
#include "HologramScoutMod.h";

SwitchWinProc::SwitchWinProc(bool type)
{
	buttonType = type;
}


SwitchWinProc::~SwitchWinProc()
{
}

// For internal use, do not modify.
int SwitchWinProc::AddRef()
{
	return DefaultRefCounted::AddRef();
}

// For internal use, do not modify.
int SwitchWinProc::Release()
{
	return DefaultRefCounted::Release();
}

// You can extend this function to return any other types your class implements.
void* SwitchWinProc::Cast(uint32_t type) const
{
	CLASS_CAST(Object);
	CLASS_CAST(IWinProc);
	CLASS_CAST(SwitchWinProc);
	return nullptr;
}

// This method returns a combinations of values in UTFWin::EventFlags.
// The combination determines what types of events (messages) this window procedure receives.
// By default, it receives mouse/keyboard input and advanced messages.
int SwitchWinProc::GetEventFlags() const
{
	return kEventFlagBasicInput | kEventFlagAdvanced;
}

// The method that receives the message. The first thing you should do is probably
// checking what kind of message was sent...
bool SwitchWinProc::HandleUIMessage(IWindow* window, const Message& message)
{
	if (message.IsType(MessageType::kMsgButtonSelect))
	{
		HologramScoutMod::Get()->mbAbilityMode = buttonType;
	}
	// Return true if the message was handled, and therefore no other window procedure should receive it.
	return false;
}
