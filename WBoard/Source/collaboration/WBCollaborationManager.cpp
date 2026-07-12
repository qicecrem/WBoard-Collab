#include "WBCollaborationManager.h"
#include "WBCollaborationClient.h"
#include "WBCollaborationDiscovery.h"
#include "WBSignalingServer.h"
#include "WBCollaborationMessage.h"

#include "domain/WBGraphicsScene.h"
#include "domain/WBGraphicsStroke.h"
#include "board/WBDrawingController.h"
#include "core/WBSettings.h"
#include "core/WB.h"
#include "board/WBBoardController.h"
#include "core/WBApplication.h"

#include <QtCore/QDebug>

WBCollaborationManager::WBCollaborationManager(QObject *parent)
    : QObject(parent)
    , mMode(Off)
    , mServer(nullptr)
    , mClient(nullptr)
    , mDiscovery(nullptr)
    , mScene(nullptr)
{
}

WBCollaborationManager::~WBCollaborationManager()
{
    stopHosting();
    leaveRoom();
}

// ===== Host =====

void WBCollaborationManager::startHosting(const QString &roomId,
                                           const QString &userName,
                                           quint16 port)
{
    stopHosting();
    leaveRoom();

    mMode = Host;
    mUserName = userName;
    mRoomId = roomId;

    // 1. Start WebSocket server
    mServer = new WBSignalingServer(port, this);
    if (!mServer->isListening()) {
        emit errorOccurred(QStringLiteral("Failed to start server on port %1").arg(port));
        stopHosting();
        return;
    }

    // 2. Connect self as first client (loopback)
    mClient = new WBCollaborationClient(this);
    connect(mClient, &WBCollaborationClient::connected,
            this, &WBCollaborationManager::onClientConnected);
    connect(mClient, &WBCollaborationClient::disconnected,
            this, &WBCollaborationManager::onClientDisconnected);
    connect(mClient, &WBCollaborationClient::connectionError,
            this, &WBCollaborationManager::onClientError);

    connect(mClient, &WBCollaborationClient::userJoined,
            this, &WBCollaborationManager::onClientUserJoined);
    connect(mClient, &WBCollaborationClient::userLeft,
            this, &WBCollaborationManager::onClientUserLeft);
    connect(mClient, &WBCollaborationClient::userListReceived,
            this, &WBCollaborationManager::onClientUserListReceived);

    connect(mClient, &WBCollaborationClient::strokeReceived,
            this, &WBCollaborationManager::onRemoteStrokeReceived);
    connect(mClient, &WBCollaborationClient::cursorReceived,
            this, &WBCollaborationManager::onRemoteCursorReceived);

    mClient->connectToServer(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(port)));

    // 3. Start UDP discovery
    if (!mDiscovery) {
        mDiscovery = new WBCollaborationDiscovery(this);
    }
    mDiscovery->startAdvertising(roomId, userName, port);

    emit modeChanged(Host);
    emit connectionStatusChanged(QStringLiteral("Hosting on port %1").arg(port));

    qDebug() << "[Collaboration] Started hosting room" << roomId
             << "as" << userName << "on port" << port;
}

void WBCollaborationManager::stopHosting()
{
    if (mMode != Host) return;

    if (mDiscovery)
        mDiscovery->stopAdvertising();

    if (mClient) {
        mClient->leaveRoom();
        mClient->disconnectFromServer();
        mClient->deleteLater();
        mClient = nullptr;
    }

    if (mServer) {
        mServer->deleteLater();
        mServer = nullptr;
    }

    mMode = Off;
    mRemoteUsers.clear();
    emit modeChanged(Off);
    emit connectionStatusChanged(QStringLiteral("Disconnected"));

    qDebug() << "[Collaboration] Stopped hosting";
}

// ===== Client =====

void WBCollaborationManager::joinRoom(const QString &hostAddress,
                                       quint16 port,
                                       const QString &roomId,
                                       const QString &userName)
{
    stopHosting();
    leaveRoom();

    mMode = Client;
    mUserName = userName;
    mRoomId = roomId;

    mClient = new WBCollaborationClient(this);
    connect(mClient, &WBCollaborationClient::connected,
            this, &WBCollaborationManager::onClientConnected);
    connect(mClient, &WBCollaborationClient::disconnected,
            this, &WBCollaborationManager::onClientDisconnected);
    connect(mClient, &WBCollaborationClient::connectionError,
            this, &WBCollaborationManager::onClientError);

    connect(mClient, &WBCollaborationClient::userJoined,
            this, &WBCollaborationManager::onClientUserJoined);
    connect(mClient, &WBCollaborationClient::userLeft,
            this, &WBCollaborationManager::onClientUserLeft);
    connect(mClient, &WBCollaborationClient::userListReceived,
            this, &WBCollaborationManager::onClientUserListReceived);

    connect(mClient, &WBCollaborationClient::strokeReceived,
            this, &WBCollaborationManager::onRemoteStrokeReceived);
    connect(mClient, &WBCollaborationClient::cursorReceived,
            this, &WBCollaborationManager::onRemoteCursorReceived);

    // If hostAddress is a hostname or "localhost", use ws://
    QString urlStr;
    if (hostAddress == QStringLiteral("localhost") ||
        hostAddress == QStringLiteral("127.0.0.1")) {
        urlStr = QStringLiteral("ws://127.0.0.1:%1").arg(port);
    } else {
        urlStr = QStringLiteral("ws://%1:%2").arg(hostAddress, QString::number(port));
    }

    mClient->connectToServer(QUrl(urlStr));

    emit modeChanged(Client);
    emit connectionStatusChanged(QStringLiteral("Connecting to %1").arg(urlStr));
}

void WBCollaborationManager::leaveRoom()
{
    if (mMode == Off) return;

    if (mDiscovery)
        mDiscovery->stopBrowsing();

    if (mClient) {
        mClient->leaveRoom();
        mClient->disconnectFromServer();
        mClient->deleteLater();
        mClient = nullptr;
    }

    mMode = Off;
    mRemoteUsers.clear();
    emit modeChanged(Off);
}

// ===== Status =====

WBCollaborationManager::Mode WBCollaborationManager::mode() const
{
    return mMode;
}

bool WBCollaborationManager::isActive() const
{
    return mMode != Off;
}

QString WBCollaborationManager::userName() const
{
    return mUserName;
}

QString WBCollaborationManager::roomId() const
{
    return mRoomId;
}

QStringList WBCollaborationManager::remoteUsers() const
{
    return mRemoteUsers;
}

// ===== Scene Hook =====

void WBCollaborationManager::installOnScene(WBGraphicsScene *scene)
{
    mScene = scene;
    // The scene will emit strokeCompleted; we connect in the caller
}

// ===== Outgoing: local stroke → broadcast =====

void WBCollaborationManager::onStrokeCompleted(WBGraphicsStroke *stroke, int tool)
{
    if (!mClient || !mClient->isConnected() || !stroke) return;

    auto points = stroke->points();
    if (points.isEmpty()) return;

    QJsonArray ptsArray;
    for (const auto &pt : points) {
        QJsonArray p;
        p.append(pt.first.x());
        p.append(pt.first.y());
        p.append(pt.second); // pressure
        ptsArray.append(p);
    }

    WBDrawingController *dc = WBDrawingController::drawingController();
    QString toolName;
    if (tool == WBStylusTool::Pen)
        toolName = WBCollaborationMessage::ToolPen;
    else if (tool == WBStylusTool::Marker)
        toolName = WBCollaborationMessage::ToolMarker;
    else if (tool == WBStylusTool::Line)
        toolName = WBCollaborationMessage::ToolLine;
    else
        toolName = WBCollaborationMessage::ToolPen;

    // Determine page index
    int pageIndex = 0;
    if (WBApplication::boardController) {
        pageIndex = WBApplication::boardController->activeSceneIndex();
    }

    QJsonObject msg = WBCollaborationMessage::buildStrokeMessage(
        toolName,
        dc->toolColor(false).name(),
        dc->toolColor(true).name(),
        dc->currentToolWidthIndex(),
        pageIndex,
        ptsArray);

    mClient->sendMessage(msg);

    qDebug() << "[Collaboration] Sent stroke:" << ptsArray.size() << "points, tool:" << toolName;
}

// ===== Incoming: remote stroke → apply locally =====

void WBCollaborationManager::onRemoteStrokeReceived(const QJsonObject &data)
{
    if (!mScene) return;
    applyRemoteStroke(data);
}

void WBCollaborationManager::applyRemoteStroke(const QJsonObject &data)
{
    QString tool = data[QStringLiteral("tool")].toString();
    int widthIndex = data[QStringLiteral("widthIndex")].toInt();
    QJsonArray ptsArray = data[QStringLiteral("points")].toArray();

    if (ptsArray.isEmpty()) return;

    // Parse points
    QList<QPair<QPointF, qreal>> remotePoints;
    for (const auto &v : ptsArray) {
        QJsonArray pt = v.toArray();
        if (pt.size() < 2) continue;
        qreal px = pt[0].toDouble();
        qreal py = pt[1].toDouble();
        qreal pressure = (pt.size() >= 3) ? pt[2].toDouble() : 1.0;
        remotePoints.append({QPointF(px, py), pressure});
    }

    WBDrawingController *dc = WBDrawingController::drawingController();
    if (!dc) return;

    // Save state
    int savedTool = dc->stylusTool();
    int savedWidth = dc->currentToolWidthIndex();
    int savedColor = dc->currentToolColorIndex();

    // Set remote tool
    int remoteTool = WBStylusTool::Pen;
    if (tool == WBCollaborationMessage::ToolPen)    remoteTool = WBStylusTool::Pen;
    else if (tool == WBCollaborationMessage::ToolMarker) remoteTool = WBStylusTool::Marker;
    else if (tool == WBCollaborationMessage::ToolLine)   remoteTool = WBStylusTool::Line;

    dc->setStylusTool(remoteTool);
    dc->setLineWidthIndex(widthIndex);

    // Replay stroke on scene
    QPointF firstPoint = remotePoints.first().first;
    qreal firstPressure = remotePoints.first().second;

    mScene->inputDevicePress(firstPoint, firstPressure);

    for (int i = 1; i < remotePoints.size(); ++i) {
        mScene->inputDeviceMove(remotePoints[i].first, remotePoints[i].second);
    }

    mScene->inputDeviceRelease();

    // Restore state
    dc->setStylusTool(savedTool);
    dc->setLineWidthIndex(savedWidth);
    dc->setColorIndex(savedColor);

    qDebug() << "[Collaboration] Applied remote stroke:" << remotePoints.size()
             << "points, tool:" << tool;
}

// ===== Client slot handlers =====

void WBCollaborationManager::onClientConnected()
{
    // Now join the room
    if (mClient && !mRoomId.isEmpty()) {
        mClient->joinRoom(mRoomId, mUserName);
    }

    emit connectionStatusChanged(QStringLiteral("Connected"));
    qDebug() << "[Collaboration] Client connected, joining room" << mRoomId;
}

void WBCollaborationManager::onClientDisconnected()
{
    emit connectionStatusChanged(QStringLiteral("Disconnected"));
    qDebug() << "[Collaboration] Client disconnected";
}

void WBCollaborationManager::onClientError(const QString &error)
{
    emit errorOccurred(error);
    qWarning() << "[Collaboration] Client error:" << error;
}

void WBCollaborationManager::onClientUserJoined(const QString &userName)
{
    if (userName == mUserName) return; // skip self
    if (!mRemoteUsers.contains(userName))
        mRemoteUsers.append(userName);
    emit userJoined(userName);
    emit userListUpdated(mRemoteUsers);
}

void WBCollaborationManager::onClientUserLeft(const QString &userName)
{
    mRemoteUsers.removeAll(userName);
    emit userLeft(userName);
    emit userListUpdated(mRemoteUsers);
}

void WBCollaborationManager::onClientUserListReceived(const QStringList &users)
{
    mRemoteUsers = users;
    mRemoteUsers.removeAll(mUserName); // remove self
    emit userListUpdated(mRemoteUsers);
}

void WBCollaborationManager::onRemoteCursorReceived(const QString &userName,
                                                      const QJsonObject &data)
{
    Q_UNUSED(userName);
    Q_UNUSED(data);
    // TODO: display remote cursors (v2)
}
