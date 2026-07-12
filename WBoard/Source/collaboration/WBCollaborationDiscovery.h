#ifndef WBCOLLABORATIONDISCOVERY_H
#define WBCOLLABORATIONDISCOVERY_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QHostAddress>

struct WBCollaborationHost {
    QString roomId;
    QString hostName;
    QHostAddress address;
    quint16 wsPort;
    QString displayText() const {
        return QStringLiteral("%1 (%2:%3)").arg(roomId, address.toString())
               .arg(wsPort);
    }
};

class WBCollaborationDiscovery : public QObject
{
    Q_OBJECT

public:
    explicit WBCollaborationDiscovery(QObject *parent = nullptr);
    ~WBCollaborationDiscovery();

    // --- Advertiser mode (the host) ---
    void startAdvertising(const QString &roomId, const QString &hostName,
                           quint16 wsPort);
    void stopAdvertising();

    // --- Browser mode (the client) ---
    void startBrowsing();
    void stopBrowsing();

    QList<WBCollaborationHost> discoveredHosts() const;

signals:
    void hostDiscovered(const WBCollaborationHost &host);
    void hostRemoved(const WBCollaborationHost &host);

private slots:
    void onBroadcastReadyRead();
    void onAdvertiseTimer();

private:
    void parseBroadcastDatagram(const QByteArray &data,
                                 const QHostAddress &sender,
                                 quint16 senderPort);

    QUdpSocket *mBroadcastSocket;
    QTimer *mAdvertiseTimer;

    QString mAdvertisedRoomId;
    QString mAdvertisedHostName;
    quint16 mAdvertisedPort;
    bool mIsAdvertising;
    bool mIsBrowsing;

    // Track hosts with TTL (10s = 5 broadcast intervals)
    QMap<QString, QPair<WBCollaborationHost, int>> mHosts;
    static constexpr int HostTtlIntervals = 5;
};

#endif // WBCOLLABORATIONDISCOVERY_H
