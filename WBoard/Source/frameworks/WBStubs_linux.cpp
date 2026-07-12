// Linux stubs — provides implementations for methods missing on Linux
#include "gui/WBKeyboardPalette.h"

void WBKeyboardPalette::onActivated(bool) {}
void WBKeyboardPalette::createCtrlButtons() {}
void WBKeyboardPalette::onLocaleChanged(WBKeyboardLocale*) {}
void WBKeyboardPalette::checkLayout() {}

void WBKeyboardButton::sendUnicodeSymbol(KEYCODE) {}
void WBKeyboardButton::sendControlSymbol(int) {}
