#ifndef WBSIGNALINGSERVER_H
#define WBSIGNALINGSERVER_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QWebSocketServer>
#include <QWebSocket>

class WBSignalingServer : public QObject
{
    Q_OBJECT

public:
    explicit WBSignalingServer(quint16 port = 19877, QObject *parent = nullptr);
    ~WBSignalingServer();

    bool isListening() const;
    quint16 serverPort() const;
    QList<QString> rooms() const;
    QList<QString> usersInRoom(const QString &roomId) const;

signals:
    void clientConnected(const QString &peerInfo);
    void clientDisconnected(const QString &peerInfo);
    void userJoinedRoom(const QString &roomId, const QString &userName);
    void userLeftRoom(const QString &roomId, const QString &userName);
    void messageForwarded(const QString &from, const QString &to, const QByteArray &payload);

private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString &message);
    void onSocketDisconnected();

private:
    void broadcastToRoom(const QString &roomId, const QByteArray &message,
                          QWebSocket *exclude = nullptr);
    void sendTo(QWebSocket *socket, const QByteArray &message);
    void sendError(QWebSocket *socket, const QString &reason);

    struct ClientInfo {
        QWebSocket *socket;
        QString userName;
        QString roomId;
    };

    QWebSocketServer *mServer;
    QMap<QWebSocket*, ClientInfo> mClients;
};

#endif // WBSIGNALINGSERVER_H
