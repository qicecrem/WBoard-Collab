// Linux stub for WBClockPalette
#include "WBClockPalette.h"

WBClockPalette::WBClockPalette(QWidget* parent) : WBFloatingPalette(Qt::TopLeftCorner, parent) {}
WBClockPalette::~WBClockPalette() {}
void WBClockPalette::timerEvent(QTimerEvent*) {}
void WBClockPalette::showEvent(QShowEvent*) {}
void WBClockPalette::hideEvent(QShowEvent*) {}
int WBClockPalette::radius() { return 0; }
