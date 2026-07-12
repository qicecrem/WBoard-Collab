#include <QRegExp>
#include "WBPlatformUtils.h"

#include <QtWidgets>
#include <QProcess>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QScreen>
#include <QHostInfo>

#include "frameworks/WBFileSystemUtils.h"
#include "core/memcheck.h"

void WBPlatformUtils::init()
{
    initializeKeyboardLayouts();
}

void WBPlatformUtils::destroy()
{
    destroyKeyboardLayouts();
}

QString WBPlatformUtils::applicationResourcesDirectory()
{
    return QApplication::applicationDirPath();
}

void WBPlatformUtils::hideFile(const QString &filePath)
{
    Q_UNUSED(filePath);
}

void WBPlatformUtils::setFileType(const QString &filePath, unsigned long fileType)
{
    Q_UNUSED(filePath);
    Q_UNUSED(fileType);
}

void WBPlatformUtils::fadeDisplayOut()
{
    // NOOP on Linux
}

void WBPlatformUtils::fadeDisplayIn()
{
    // NOOP on Linux
}

QStringList WBPlatformUtils::availableTranslations()
{
    QString translationsPath = applicationResourcesDirectory() + "/" + "Language" + "/";
    QStringList translationsList = WBFileSystemUtils::allFiles(translationsPath);
    return translationsList;
}

QString WBPlatformUtils::translationPath(QString pFilePrefix, QString pLanguage)
{
    QString path = applicationResourcesDirectory() + "/" + "Language" + "/" + pFilePrefix + "_" + pLanguage + ".qm";
    return path;
}

QString WBPlatformUtils::systemLanguage()
{
    return QLocale::system().name().left(2);
}

bool WBPlatformUtils::hasVirtualKeyboard()
{
    return false;
}

void WBPlatformUtils::bringPreviousProcessToFront()
{
    // NOOP on Linux
}

QString WBPlatformUtils::osUserLoginName()
{
    return qEnvironmentVariable("USER", qEnvironmentVariable("USERNAME", "unknown"));
}

void WBPlatformUtils::setDesktopMode(bool desktop)
{
    Q_UNUSED(desktop);
    // NOOP on Linux
}

void WBPlatformUtils::setWindowNonActivableFlag(QWidget* widget, bool nonAcivable)
{
    Q_UNUSED(widget);
    Q_UNUSED(nonAcivable);
    // NOOP on Linux
}

QString WBPlatformUtils::computerName()
{
    return QHostInfo::localHostName();
}

WBKeyboardLocale** WBPlatformUtils::getKeyboardLayouts(int& nCount)
{
    nCount = 0;
    return nullptr;
}

QString WBPlatformUtils::urlFromClipboard()
{
    return QApplication::clipboard()->text();
}

void WBPlatformUtils::setFrontProcess()
{
    // NOOP on Linux
}

void WBPlatformUtils::showFullScreen(QWidget * pWidget)
{
    if (pWidget)
        pWidget->showFullScreen();
}

void WBPlatformUtils::showOSK(bool show)
{
    Q_UNUSED(show);
    // NOOP on Linux
}

void WBPlatformUtils::initializeKeyboardLayouts()
{
    nKeyboardLayouts = 0;
    keyboardLayouts = nullptr;
}

void WBPlatformUtils::destroyKeyboardLayouts()
{
}

int WBPlatformUtils::nKeyboardLayouts = 0;
WBKeyboardLocale** WBPlatformUtils::keyboardLayouts = nullptr;

WBPlatformUtils::WBPlatformUtils() {}
WBPlatformUtils::~WBPlatformUtils() {}
WBKeyboardLocale::~WBKeyboardLocale() {}
