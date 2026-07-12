#include "WBCollaborationDiscovery.h"
#include "WBCollaborationMessage.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

WBCollaborationDiscovery::WBCollaborationDiscovery(QObject *parent)
    : QObject(parent)
    , mBroadcastSocket(new QUdpSocket(this))
    , mAdvertiseTimer(new QTimer(this))
    , mAdvertisedPort(0)
    , mIsAdvertising(false)
    , mIsBrowsing(false)
{
    connect(mAdvertiseTimer, &QTimer::timeout,
            this, &WBCollaborationDiscovery::onAdvertiseTimer);
}

WBCollaborationDiscovery::~WBCollaborationDiscovery()
{
    stopAdvertising();
    stopBrowsing();
}

// ===== Advertiser =====

void WBCollaborationDiscovery::startAdvertising(const QString &roomId,
                                                  const QString &hostName,
                                                  quint16 wsPort)
{
    stopAdvertising();
    mAdvertisedRoomId = roomId;
    mAdvertisedHostName = hostName;
    mAdvertisedPort = wsPort;
    mIsAdvertising = true;

    // Send first broadcast immediately, then on timer
    onAdvertiseTimer();
    mAdvertiseTimer->start(WBCollaborationMessage::DiscoveryIntervalMs);

    qDebug() << "[Discovery] Started advertising room" << roomId << "on port" << wsPort;
}

void WBCollaborationDiscovery::stopAdvertising()
{
    if (mIsAdvertising) {
        mAdvertiseTimer->stop();
        mIsAdvertising = false;
        qDebug() << "[Discovery] Stopped advertising";
    }
}

void WBCollaborationDiscovery::onAdvertiseTimer()
{
    if (!mIsAdvertising) return;

    QJsonObject obj;
    obj[QStringLiteral("type")] = WBCollaborationMessage::DiscoveryPrefix;
    obj[QStringLiteral("room")] = mAdvertisedRoomId;
    obj[QStringLiteral("host")] = mAdvertisedHostName;
    obj[QStringLiteral("port")] = static_cast<int>(mAdvertisedPort);

    QByteArray datagram = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    mBroadcastSocket->writeDatagram(
        datagram, QHostAddress::Broadcast,
        WBCollaborationMessage::DiscoveryPort);
}

// ===== Browser =====

void WBCollaborationDiscovery::startBrowsing()
{
    stopBrowsing();
    mIsBrowsing = true;

    // Bind to discovery port
    if (!mBroadcastSocket->bind(WBCollaborationMessage::DiscoveryPort,
                                 QUdpSocket::ShareAddress |
                                 QUdpSocket::ReuseAddressHint)) {
        qWarning() << "[Discovery] Failed to bind to port"
                    << WBCollaborationMessage::DiscoveryPort
                    << ":" << mBroadcastSocket->errorString();
        mIsBrowsing = false;
        return;
    }

    connect(mBroadcastSocket, &QUdpSocket::readyRead,
            this, &WBCollaborationDiscovery::onBroadcastReadyRead);

    qDebug() << "[Discovery] Started browsing for collaboration rooms";
}

void WBCollaborationDiscovery::stopBrowsing()
{
    if (mIsBrowsing) {
        disconnect(mBroadcastSocket, &QUdpSocket::readyRead,
                   this, &WBCollaborationDiscovery::onBroadcastReadyRead);
        mBroadcastSocket->close();
        mIsBrowsing = false;
        mHosts.clear();
        qDebug() << "[Discovery] Stopped browsing";
    }
}

void WBCollaborationDiscovery::onBroadcastReadyRead()
{
    while (mBroadcastSocket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(static_cast<int>(mBroadcastSocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort = 0;

        mBroadcastSocket->readDatagram(data.data(), data.size(),
                                         &sender, &senderPort);
        parseBroadcastDatagram(data, sender, senderPort);
    }

    // Decrement TTLs and remove expired hosts
    QList<QString> toRemove;
    for (auto it = mHosts.begin(); it != mHosts.end(); ++it) {
        it.value().second--;
        if (it.value().second <= 0)
            toRemove.append(it.key());
    }
    for (const auto &key : toRemove) {
        WBCollaborationHost host = mHosts.value(key).first;
        mHosts.remove(key);
        emit hostRemoved(host);
    }
}

void WBCollaborationDiscovery::parseBroadcastDatagram(const QByteArray &data,
                                                        const QHostAddress &sender,
                                                        quint16 senderPort)
{
    Q_UNUSED(senderPort);

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) return;

    QJsonObject obj = doc.object();
    if (obj[QStringLiteral("type")].toString() != WBCollaborationMessage::DiscoveryPrefix)
        return;

    WBCollaborationHost host;
    host.roomId   = obj[QStringLiteral("room")].toString();
    host.hostName = obj[QStringLiteral("host")].toString();
    host.address  = sender;
    host.wsPort   = static_cast<quint16>(obj[QStringLiteral("port")].toInt());

    // Deduplicate by address+port
    QString key = sender.toString() + ":" + QString::number(host.wsPort);
    bool isNew = !mHosts.contains(key);

    mHosts[key] = {host, HostTtlIntervals};

    if (isNew)
        emit hostDiscovered(host);
}

QList<WBCollaborationHost> WBCollaborationDiscovery::discoveredHosts() const
{
    QList<WBCollaborationHost> result;
    for (auto &p : mHosts)
        result.append(p.first);
    return result;
}
