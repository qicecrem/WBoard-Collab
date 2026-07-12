// Linux stub for WBWebController — complete implementation
#include "WBWebController.h"
#include <QAction>

WBWebController::WBWebController(WBMainWindow*) {}
WBWebController::~WBWebController() {}
void WBWebController::loadUrl(const QUrl&) {}
void WBWebController::showTabAtTop(bool) {}
void WBWebController::closing() {}
void WBWebController::adaptToolBar() {}
void WBWebController::show() {}
void WBWebController::cut() {}
void WBWebController::copy() {}
void WBWebController::paste() {}
void WBWebController::screenLayoutChanged() {}
void WBWebController::setSourceWidget(QWidget*) {}
void WBWebController::captureWindow() {}
void WBWebController::customCapture() {}
void WBWebController::toogleMirroring(bool) {}
void WBWebController::captureoEmbed() {}
void WBWebController::captureEduMedia() {}
bool WBWebController::isOEmbedable(const QUrl&) { return false; }
bool WBWebController::hasEmbeddedContent() { return false; }
void WBWebController::getEmbeddableContent() {}
bool WBWebController::isEduMedia(const QUrl&) { return false; }
void WBWebController::triggerWebTools(bool) {}
void WBWebController::activePageChanged() {}
void WBWebController::toggleWebTrap(bool) {}
void WBWebController::onOEmbedParsed(QList<sOEmbedContent>) {}

#include "moc_WBWebController.cpp"
