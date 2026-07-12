#include "WBSignalingServer.h"
#include "WBCollaborationMessage.h"

#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QDebug>

WBSignalingServer::WBSignalingServer(quint16 port, QObject *parent)
    : QObject(parent)
{
    mServer = new QWebSocketServer(QStringLiteral("WBoard-Collaboration"),
                                    QWebSocketServer::NonSecureMode, this);

    if (!mServer->listen(QHostAddress::Any, port)) {
        qWarning() << "[WBSignalingServer] Failed to listen on port" << port
                    << ":" << mServer->errorString();
        return;
    }

    connect(mServer, &QWebSocketServer::newConnection,
            this, &WBSignalingServer::onNewConnection);

    qDebug() << "[WBSignalingServer] Listening on port" << port;
}

WBSignalingServer::~WBSignalingServer()
{
    mServer->close();
    // QWebSocket objects are children of mServer, auto-deleted
}

bool WBSignalingServer::isListening() const
{
    return mServer->isListening();
}

quint16 WBSignalingServer::serverPort() const
{
    return mServer->serverPort();
}

QList<QString> WBSignalingServer::rooms() const
{
    QSet<QString> roomSet;
    for (auto &info : mClients)
        roomSet.insert(info.roomId);
    return QList<QString>(roomSet.begin(), roomSet.end());
}

QList<QString> WBSignalingServer::usersInRoom(const QString &roomId) const
{
    QList<QString> users;
    for (auto &info : mClients) {
        if (info.roomId == roomId)
            users.append(info.userName);
    }
    return users;
}

void WBSignalingServer::onNewConnection()
{
    QWebSocket *socket = mServer->nextPendingConnection();
    if (!socket) return;

    connect(socket, &QWebSocket::textMessageReceived,
            this, &WBSignalingServer::onTextMessageReceived);
    connect(socket, &QWebSocket::disconnected,
            this, &WBSignalingServer::onSocketDisconnected);

    // Client info populated on join
    mClients[socket] = {socket, QString(), QString()};

    qDebug() << "[WBSignalingServer] New connection from"
             << socket->peerAddress().toString();
    emit clientConnected(socket->peerAddress().toString());
}

void WBSignalingServer::onTextMessageReceived(const QString &message)
{
    QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
    if (!socket || !mClients.contains(socket)) return;

    QJsonObject obj = WBCollaborationMessage::fromJson(message);
    QString type = obj[QStringLiteral("type")].toString();

    if (type == WBCollaborationMessage::TypeJoin) {
        QString roomId = obj[QStringLiteral("room")].toString();
        QString userName = obj[QStringLiteral("user")].toString();

        if (roomId.isEmpty() || userName.isEmpty()) {
            sendError(socket, QStringLiteral("room and user required"));
            return;
        }

        // Leave previous room if any
        QString oldRoom = mClients[socket].roomId;
        if (!oldRoom.isEmpty()) {
            // broadcast leave to old room (excluding self)
            QJsonObject leaveObj;
            leaveObj[QStringLiteral("type")] = WBCollaborationMessage::TypeLeave;
            leaveObj[QStringLiteral("user")] = mClients[socket].userName;
            broadcastToRoom(oldRoom, WBCollaborationMessage::toJson(leaveObj), socket);
        }

        mClients[socket].roomId = roomId;
        mClients[socket].userName = userName;

        // Notify other clients in room
        QJsonObject joinObj;
        joinObj[QStringLiteral("type")] = WBCollaborationMessage::TypeJoin;
        joinObj[QStringLiteral("user")] = userName;
        broadcastToRoom(roomId, WBCollaborationMessage::toJson(joinObj), socket);

        // Send current user list to the new client
        QJsonObject usersObj;
        usersObj[QStringLiteral("type")] = WBCollaborationMessage::TypeUsers;
        QJsonArray userArray;
        for (auto &user : usersInRoom(roomId))
            userArray.append(user);
        usersObj[QStringLiteral("users")] = userArray;
        sendTo(socket, WBCollaborationMessage::toJson(usersObj));

        qDebug() << "[WBSignalingServer]" << userName << "joined room" << roomId;
        emit userJoinedRoom(roomId, userName);
    }
    else {
        // Forward all other messages to room (excluding sender)
        ClientInfo &info = mClients[socket];
        if (!info.roomId.isEmpty()) {
            broadcastToRoom(info.roomId, message.toUtf8(), socket);
            emit messageForwarded(info.userName, info.roomId, message.toUtf8());
        }
    }
}

void WBSignalingServer::onSocketDisconnected()
{
    QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
    if (!socket || !mClients.contains(socket)) return;

    ClientInfo &info = mClients[socket];
    qDebug() << "[WBSignalingServer]" << info.userName << "disconnected";

    if (!info.roomId.isEmpty()) {
        QJsonObject leaveObj;
        leaveObj[QStringLiteral("type")] = WBCollaborationMessage::TypeLeave;
        leaveObj[QStringLiteral("user")] = info.userName;
        broadcastToRoom(info.roomId, WBCollaborationMessage::toJson(leaveObj), socket);
        emit userLeftRoom(info.roomId, info.userName);
    }

    emit clientDisconnected(socket->peerAddress().toString());
    mClients.remove(socket);
    socket->deleteLater();
}

void WBSignalingServer::broadcastToRoom(const QString &roomId,
                                         const QByteArray &message,
                                         QWebSocket *exclude)
{
    for (auto it = mClients.begin(); it != mClients.end(); ++it) {
        if (it->roomId == roomId && it->socket != exclude) {
            it->socket->sendTextMessage(QString::fromUtf8(message));
        }
    }
}

void WBSignalingServer::sendTo(QWebSocket *socket, const QByteArray &message)
{
    socket->sendTextMessage(QString::fromUtf8(message));
}

void WBSignalingServer::sendError(QWebSocket *socket, const QString &reason)
{
    QJsonObject obj;
    obj[QStringLiteral("type")]  = WBCollaborationMessage::TypeError;
    obj[QStringLiteral("reason")] = reason;
    sendTo(socket, WBCollaborationMessage::toJson(obj));
}
