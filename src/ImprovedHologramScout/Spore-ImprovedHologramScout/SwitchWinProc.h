#pragma once

#include <Spore\BasicIncludes.h>

#define SwitchWinProcPtr intrusive_ptr<SwitchWinProc>

// To avoid repeating UTFWin:: all the time.
using namespace UTFWin;

class SwitchWinProc 
	: public IWinProc
	, public DefaultRefCounted
{
public:
	static const uint32_t TYPE = id("SwitchWinProc");
	
	SwitchWinProc(bool type);
	~SwitchWinProc();

	int AddRef() override;
	int Release() override;
	void* Cast(uint32_t type) const override;
	
	int GetEventFlags() const override;
	bool buttonType;
	// This is the function you have to implement, called when a window you added this winproc to received an event
	bool HandleUIMessage(IWindow* pWindow, const Message& message) override;
	
};
