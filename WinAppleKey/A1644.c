#include "driver.h"

BOOLEAN g_FnKeyActive = 0;

typedef struct KbdBuf KbdBuf;
struct KbdBuf {
	BYTE modifiers;
	BYTE _;
	BYTE keys[6]; // The keyboard will only recognise six normal keys simultaneously; any further pressed keys are ignored.
	BYTE special_key;
};

void ProcessA1644Buffer(BYTE* buf_, ULONG size)
{
	KbdBuf* buf = buf_;

	// SwapFnCtrl mode
	//if (g_dwSwapFnCtrl)
	//{
	//	// Physical LCtrl pressed
	//	if (buf->modifiers & HidLCtrlMask)
	//	{
	//		if(!buf->keys[0]) // And is it pressed alone?
	//			g_FnKeyActive = TRUE;

	//		buf->modifiers &= ~HidLCtrlMask; // Clear LCtrl modifier
	//	}
	//	else // Physical LCtrl not pressed
	//	{
	//		if (!buf->keys[0]) // Only unset g_FnKeyActive when there is no other key still being pressed
	//			g_FnKeyActive = FALSE;
	//	}

	//	// Physical Fn pressed?
	//	if (buf->special_key & 0x2)
	//		buf->modifiers |= HidLCtrlMask; // Set LCtrl modifier
	//	else
	//		buf->modifiers &= ~HidLCtrlMask; // Clear LCtrl modifier
	//}
	//else // Not SwapFnCtrl mode
	BOOLEAN prev_fn = g_FnKeyActive;
	g_FnKeyActive = buf->special_key & 0x2; // Set FakeFnActive state based on physical Fn key state

	// Fn has just been released, so prevent immediate accidental keystokes by clearing the buffer.
	if ((prev_fn != g_FnKeyActive) && !g_FnKeyActive) {
		buf->keys[0] = 0;
		buf->keys[1] = 0;
		buf->keys[2] = 0;
		buf->keys[3] = 0;
		buf->keys[4] = 0;
		buf->keys[5] = 0;
		buf->special_key = 0;
	}

	// Eject Pressed?
	if (buf->special_key & 0x1 && g_dwEjectScanCode <= 0xff) {
		buf->keys[5] = HidDel; // Avoid overriding key like caps lock by using last (6th) slot.
	}

	buf->special_key = 0;

	// Alt-Cmd swap

	if (buf->modifiers & HidLAltMask) {
		buf->modifiers &= ~HidLAltMask;
		buf->modifiers |= HidLCmdMask;
	}
	else if (buf->modifiers & HidLCmdMask) {
		buf->modifiers &= ~HidLCmdMask;
		buf->modifiers |= HidLAltMask;
	}

	if (buf->modifiers & HidRAltMask) {
		buf->modifiers &= ~HidRAltMask;
		buf->modifiers |= HidRCtrlMask;
	}
	else if (buf->modifiers & HidRCmdMask) {
		buf->modifiers &= ~HidRCmdMask;
		buf->modifiers |= HidRAltMask;
	}
	
	// Process Fn key combinations
	if (g_FnKeyActive) {
		for (int idx = 0; idx < 6; idx++) {
			BYTE key2;
			switch (buf->keys[idx]) {
			case HidLeft: key2 = HidHome; break;
			case HidRight: key2 = HidEnd; break;
			case HidUp: key2 = HidPgUp; break;
			case HidDown: key2 = HidPgDown; break;
			case HidEnter: key2 = HidInsert; break;
			case HidF1: key2 = HidF13; break;
			case HidF2: key2 = HidF14; break;
			case HidF3: key2 = HidF15; break;
			case HidF4: key2 = HidF16; break;
			case HidF5: key2 = HidF17; break;
			case HidF6: key2 = HidF18; break;
			case HidF7: key2 = HidF19; break;
			case HidF8: key2 = HidF20; break;
			case HidF9: key2 = HidF21; break;
			case HidF10: key2 = HidF22; break;
			case HidF11: key2 = HidF23; break;
			case HidF12: key2 = HidF24; break;
			case HidKeyP: key2 = HidPrtScr; break;
			case HidKeyB: key2 = HidPauseBreak; break;
			//case HidKeyS: key2 = HidScrLck; break;
			case HidCapsLock: key2 = HidCapsLock; break;
			default:
				key2 = 0; // Ignore this key
				break;
			}
			buf->keys[idx] = key2;
		}
	}
}