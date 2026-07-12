#ifndef WBCOLLABORATIONPALETTE_H
#define WBCOLLABORATIONPALETTE_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtCore/QStringList>

class WBCollaborationManager;

class WBCollaborationPalette : public QWidget
{
    Q_OBJECT

public:
    explicit WBCollaborationPalette(WBCollaborationManager *manager,
                                     QWidget *parent = nullptr);
    ~WBCollaborationPalette();

private slots:
    void onStartHost();
    void onStopHost();
    void onJoinByAddress();
    void onJoinRoom();
    void onLeaveRoom();

    void onHostDiscovered(const QString &roomId, const QString &hostAddress,
                          quint16 port);
    void onUserJoined(const QString &userName);
    void onUserLeft(const QString &userName);
    void onUserListUpdated(const QStringList &users);
    void onConnectionStatusChanged(const QString &status);
    void onErrorOccurred(const QString &error);

    void updateUI();

private:
    void setupUI();
    void setStatus(const QString &text, bool isError = false);

    WBCollaborationManager *mManager;

    // Host controls
    QLineEdit *mHostRoomName;
    QLineEdit *mHostUserName;
    QLineEdit *mHostPort;
    QPushButton *mHostStartBtn;
    QPushButton *mHostStopBtn;

    // Client controls (manual join)
    QLineEdit *mClientAddress;
    QLineEdit *mClientPort;
    QLineEdit *mClientUserName;
    QPushButton *mClientJoinBtn;

    // Discovery list
    QListWidget *mDiscoveryList;
    QPushButton *mDiscoveryJoinBtn;

    // Shared
    QPushButton *mLeaveBtn;
    QLabel *mStatusLabel;
    QListWidget *mUserList;
    QLabel *mRoomLabel;

    QStackedWidget *mStack;
};

#endif // WBCOLLABORATIONPALETTE_H
