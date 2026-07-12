// Linux stub for WBWindowCapture
#include "WBWindowCapture.h"

WBWindowCapture::WBWindowCapture(WBDesktopAnnotationController*) {}
WBWindowCapture::~WBWindowCapture() {}
int WBWindowCapture::execute() { return 0; }
const QPixmap WBWindowCapture::getCapturedWindow() { return QPixmap(); }
