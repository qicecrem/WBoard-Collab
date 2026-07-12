// Linux stub for WBPodcastController — complete implementation
#include "WBPodcastController.h"
#include <QAction>
#include <QEvent>
#include <QTimerEvent>

WBPodcastController* WBPodcastController::sInstance = nullptr;

WBPodcastController::WBPodcastController(QObject* parent) : QObject(parent) { sInstance = this; }
WBPodcastController::~WBPodcastController() { sInstance = nullptr; }
WBPodcastController* WBPodcastController::instance() { return sInstance; }

void WBPodcastController::start() {}
void WBPodcastController::stop() {}
void WBPodcastController::pause() {}
void WBPodcastController::unpause() {}
void WBPodcastController::toggleRecordingPalette(bool) {}
void WBPodcastController::recordToggled(bool) {}
void WBPodcastController::pauseToggled(bool) {}
void WBPodcastController::setSourceWidget(QWidget*) {}
void WBPodcastController::timerEvent(QTimerEvent*) {}
void WBPodcastController::processWidgetPaintEvent() {}
void WBPodcastController::processScenePaintEvent() {}
void WBPodcastController::sceneChanged(const QList<QRectF>&) {}
void WBPodcastController::sceneBackgroundChanged() {}
void WBPodcastController::activeSceneChanged() {}
void WBPodcastController::applicationMainModeChanged(WBApplicationController::MainMode) {}
void WBPodcastController::applicationDesktopMode(bool) {}
void WBPodcastController::webActiveWebPageChanged(WBWebView*) {}
void WBPodcastController::encodingStatus(const QString&) {}
void WBPodcastController::encodingFinished(bool) {}
void WBPodcastController::applicationAboutToQuit() {}
void WBPodcastController::groupActionTriggered(QAction*) {}
void WBPodcastController::actionToggled(bool) {}
void WBPodcastController::updateActionState() {}
bool WBPodcastController::eventFilter(QObject*, QEvent*) { return false; }
QStringList WBPodcastController::audioRecordingDevices() { return {}; }
QList<QAction*> WBPodcastController::audioRecordingDevicesActions() { return {}; }
QList<QAction*> WBPodcastController::videoSizeActions() { return {}; }
QList<QAction*> WBPodcastController::podcastPublicationActions() { return {}; }

#include "moc_WBPodcastController.cpp"
