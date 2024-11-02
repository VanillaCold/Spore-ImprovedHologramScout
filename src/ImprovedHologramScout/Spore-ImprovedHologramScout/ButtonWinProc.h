#pragma once

#include <Spore\BasicIncludes.h>

#define ButtonWinProcPtr intrusive_ptr<ButtonWinProc>

// To avoid repeating UTFWin:: all the time.
using namespace UTFWin;

class ButtonWinProc 
	: public IWinProc
	, public DefaultRefCounted
{
public:
	static const uint32_t TYPE = id("ButtonWinProc");

	byte attkID;
	
	ButtonWinProc(int attkID);
	~ButtonWinProc();

	int AddRef() override;
	int Release() override;
	void* Cast(uint32_t type) const override;
	
	int GetEventFlags() const override;
	// This is the function you have to implement, called when a window you added this winproc to received an event
	bool HandleUIMessage(IWindow* pWindow, const Message& message) override;
	
};
