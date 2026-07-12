#ifndef WBCOLLABORATIONCLIENT_H
#define WBCOLLABORATIONCLIENT_H

#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtCore/QJsonObject>
#include <QWebSocket>

class WBCollaborationClient : public QObject
{
    Q_OBJECT

public:
    explicit WBCollaborationClient(QObject *parent = nullptr);
    ~WBCollaborationClient();

    void connectToServer(const QUrl &url);
    void disconnectFromServer();
    bool isConnected() const;

    void joinRoom(const QString &roomId, const QString &userName);
    void leaveRoom();
    void sendMessage(const QJsonObject &message);

    QString userName() const;
    QString roomId() const;

signals:
    void connected();
    void disconnected();
    void connectionError(const QString &errorString);

    void userJoined(const QString &userName);
    void userLeft(const QString &userName);
    void userListReceived(const QStringList &users);

    void strokeReceived(const QJsonObject &data);
    void eraseReceived(const QJsonObject &data);
    void commandReceived(const QJsonObject &data);
    void pageAddReceived(int index);
    void pageDeleteReceived(int index);
    void pageSwitchReceived(int index);
    void chatReceived(const QString &userName, const QString &message);
    void cursorReceived(const QString &userName, const QJsonObject &data);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error);

private:
    void handleMessage(const QJsonObject &obj);

    QWebSocket *mSocket;
    QString mUserName;
    QString mRoomId;
    bool mConnected;
};

#endif // WBCOLLABORATIONCLIENT_H
