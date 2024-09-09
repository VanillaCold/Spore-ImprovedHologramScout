//###########################
//# Trigger config file for Walk around #
//##########################

[all]
//############################
//### Keyboard controls ###

//# move forward

// See also TriggerConfigWASD

//#jump
any space = 0x00000007

//#sprint
any 0x10 = 0x00000008

//#sneak = Shift
any 0x12 = 0x00000009

///
//#rotate camera left
any , = 0x00000010

//#rotate camera right
any . = 0x00000011

//#zoom camera in
any = = 0x00000012
any + = 0x00000012
any numpad+ = 0x00000012

//#zoom camera out
any - = 0x00000013
any numpad- = 0x00000013

//#reset camera
any / = 0x00000014

//#untarget
norepeat escape = 0x00000015

//#block
any b = 0x00000019

//############################
//### mouse controls ###

// action
any mouse_button_left = 0x00000016

// control camera pitch / pan
any mouse_button_middle = 0x00000018

// set avatar movement goal
any mouse_button_right = 0x00000020

//############################
//### custom controls ###

// switch one
any tab = 0x00000031

// numbers 1 through 4, for ability usage
any 1 = 0x10000001
any 2 = 0x10000002
any 3 = 0x10000003
any 4 = 0x10000004

// ##### debug cheats ######
// debug locomotion
control l = 0x99999992
        
