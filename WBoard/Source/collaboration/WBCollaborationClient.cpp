#include "WBCollaborationClient.h"
#include "WBCollaborationMessage.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonArray>

WBCollaborationClient::WBCollaborationClient(QObject *parent)
    : QObject(parent)
    , mSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    , mConnected(false)
{
    connect(mSocket, &QWebSocket::connected,
            this, &WBCollaborationClient::onConnected);
    connect(mSocket, &QWebSocket::disconnected,
            this, &WBCollaborationClient::onDisconnected);
    connect(mSocket, &QWebSocket::textMessageReceived,
            this, &WBCollaborationClient::onTextMessageReceived);
    connect(mSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &WBCollaborationClient::onError);
}

WBCollaborationClient::~WBCollaborationClient()
{
    if (mConnected && !mRoomId.isEmpty()) {
        // Send leave message before closing
        QJsonObject obj;
        obj[QStringLiteral("type")] = WBCollaborationMessage::TypeLeave;
        obj[QStringLiteral("user")] = mUserName;
        mSocket->sendTextMessage(
            QString::fromUtf8(WBCollaborationMessage::toJson(obj)));
    }
    mSocket->close();
}

void WBCollaborationClient::connectToServer(const QUrl &url)
{
    qDebug() << "[WBCollaborationClient] Connecting to" << url.toString();
    mSocket->open(url);
}

void WBCollaborationClient::disconnectFromServer()
{
    mSocket->close();
    mConnected = false;
    mRoomId.clear();
}

bool WBCollaborationClient::isConnected() const
{
    return mConnected;
}

void WBCollaborationClient::joinRoom(const QString &roomId, const QString &userName)
{
    if (!mConnected) return;

    mUserName = userName;
    mRoomId = roomId;

    QJsonObject msg = WBCollaborationMessage::buildJoinMessage(roomId, userName);
    mSocket->sendTextMessage(
        QString::fromUtf8(WBCollaborationMessage::toJson(msg)));

    qDebug() << "[WBCollaborationClient] Joining room" << roomId << "as" << userName;
}

void WBCollaborationClient::leaveRoom()
{
    if (!mConnected || mRoomId.isEmpty()) return;

    QJsonObject obj;
    obj[QStringLiteral("type")] = WBCollaborationMessage::TypeLeave;
    obj[QStringLiteral("user")] = mUserName;
    mSocket->sendTextMessage(
        QString::fromUtf8(WBCollaborationMessage::toJson(obj)));

    mRoomId.clear();
}

void WBCollaborationClient::sendMessage(const QJsonObject &message)
{
    if (!mConnected) return;
    mSocket->sendTextMessage(
        QString::fromUtf8(WBCollaborationMessage::toJson(message)));
}

QString WBCollaborationClient::userName() const
{
    return mUserName;
}

QString WBCollaborationClient::roomId() const
{
    return mRoomId;
}

// --- Slots ---

void WBCollaborationClient::onConnected()
{
    mConnected = true;
    qDebug() << "[WBCollaborationClient] Connected";
    emit connected();
}

void WBCollaborationClient::onDisconnected()
{
    mConnected = false;
    qDebug() << "[WBCollaborationClient] Disconnected";
    emit disconnected();
}

void WBCollaborationClient::onTextMessageReceived(const QString &message)
{
    QJsonObject obj = WBCollaborationMessage::fromJson(message);
    if (obj.isEmpty()) {
        qWarning() << "[WBCollaborationClient] Failed to parse message";
        return;
    }
    handleMessage(obj);
}

void WBCollaborationClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    QString errStr = mSocket->errorString();
    qWarning() << "[WBCollaborationClient] Error:" << errStr;
    mConnected = false;
    emit connectionError(errStr);
}

// --- Message dispatch ---

void WBCollaborationClient::handleMessage(const QJsonObject &obj)
{
    QString type = obj[QStringLiteral("type")].toString();

    if (type == WBCollaborationMessage::TypeJoin) {
        emit userJoined(obj[QStringLiteral("user")].toString());
    }
    else if (type == WBCollaborationMessage::TypeLeave) {
        emit userLeft(obj[QStringLiteral("user")].toString());
    }
    else if (type == WBCollaborationMessage::TypeUsers) {
        QStringList users;
        for (const auto &v : obj[QStringLiteral("users")].toArray())
            users.append(v.toString());
        emit userListReceived(users);
    }
    else if (type == WBCollaborationMessage::TypeStroke) {
        emit strokeReceived(obj);
    }
    else if (type == WBCollaborationMessage::TypeErase) {
        emit eraseReceived(obj);
    }
    else if (type == WBCollaborationMessage::TypeCommand) {
        emit commandReceived(obj);
    }
    else if (type == WBCollaborationMessage::TypePageAdd) {
        emit pageAddReceived(obj[QStringLiteral("index")].toInt());
    }
    else if (type == WBCollaborationMessage::TypePageDelete) {
        emit pageDeleteReceived(obj[QStringLiteral("index")].toInt());
    }
    else if (type == WBCollaborationMessage::TypePageSwitch) {
        emit pageSwitchReceived(obj[QStringLiteral("index")].toInt());
    }
    else if (type == WBCollaborationMessage::TypeChat) {
        emit chatReceived(obj[QStringLiteral("user")].toString(),
                           obj[QStringLiteral("text")].toString());
    }
    else if (type == WBCollaborationMessage::TypeCursor) {
        emit cursorReceived(obj[QStringLiteral("user")].toString(), obj);
    }
}
