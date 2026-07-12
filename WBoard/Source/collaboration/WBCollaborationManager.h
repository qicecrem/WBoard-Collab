#ifndef WBCOLLABORATIONMANAGER_H
#define WBCOLLABORATIONMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QStringList>
#include <QtCore/QPointF>

class WBCollaborationClient;
class WBCollaborationDiscovery;
class WBSignalingServer;
class WBGraphicsScene;
class WBGraphicsStroke;

class WBCollaborationManager : public QObject
{
    Q_OBJECT

public:
    enum Mode {
        Off,
        Host,
        Client
    };

    explicit WBCollaborationManager(QObject *parent = nullptr);
    ~WBCollaborationManager();

    // --- Host ---
    void startHosting(const QString &roomId, const QString &userName, quint16 port);
    void stopHosting();

    // --- Client ---
    void joinRoom(const QString &hostAddress, quint16 port,
                  const QString &roomId, const QString &userName);
    void leaveRoom();

    // --- Status ---
    Mode mode() const;
    bool isActive() const;
    QString userName() const;
    QString roomId() const;
    QStringList remoteUsers() const;

    // --- Hook into WBGraphicsScene ---
    void installOnScene(WBGraphicsScene *scene);

    // --- Internal guard ---
    bool isApplyingRemote() const { return mApplyingRemote; }
    void setApplyingRemote(bool v) { mApplyingRemote = v; }

signals:
    void modeChanged(Mode mode);
    void userJoined(const QString &userName);
    void userLeft(const QString &userName);
    void userListUpdated(const QStringList &users);
    void connectionStatusChanged(const QString &status);
    void errorOccurred(const QString &error);

    // Discovery signals (for clients)
    void hostDiscovered(const QString &roomId, const QString &hostAddress,
                        quint16 port);

public slots:
    void onStrokeCompleted(WBGraphicsStroke *stroke, int tool);
    void onItemsRemoved(const QJsonArray &removedUuids, const QJsonArray &addedData);

    // --- Page operation notification (called by WBBoardController) ---
    void notifyPageAdded(int index);
    void notifyPageDeleted(int index);
    void notifyPageDuplicated(int sourceIndex, int targetIndex);
    void notifyPageSwitched(int index);

private slots:
    void onRemoteStrokeReceived(const QJsonObject &data);
    void onRemoteEraseReceived(const QJsonObject &data);
    void onRemoteCommandReceived(const QJsonObject &data);
    void onRemoteCursorReceived(const QString &userName, const QJsonObject &data);
    void onRemotePageAddReceived(int index);
    void onRemotePageDeleteReceived(int index);
    void onRemotePageSwitchReceived(int index);
    void onClientUserJoined(const QString &userName);
    void onClientUserLeft(const QString &userName);
    void onClientUserListReceived(const QStringList &users);
    void onClientConnected();
    void onClientDisconnected();
    void onClientError(const QString &error);
    void onUndoStackIndexChanged(int index);

private:
    void applyRemoteStroke(const QJsonObject &data);
    void applyRemoteErase(const QJsonObject &data);

    Mode mMode;
    QString mUserName;
    QString mRoomId;

    WBSignalingServer *mServer;
    WBCollaborationClient *mClient;
    WBCollaborationDiscovery *mDiscovery;
    WBGraphicsScene *mScene;

    QStringList mRemoteUsers;
    int mUndoIndex;
    bool mApplyingRemote;
};

#endif // WBCOLLABORATIONMANAGER_H
