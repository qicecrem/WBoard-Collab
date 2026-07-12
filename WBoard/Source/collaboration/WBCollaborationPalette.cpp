#include "WBCollaborationPalette.h"
#include "WBCollaborationManager.h"
#include "WBCollaborationDiscovery.h"
#include "WBCollaborationMessage.h"

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QMessageBox>
#include <QtCore/QDebug>

WBCollaborationPalette::WBCollaborationPalette(WBCollaborationManager *manager,
                                                 QWidget *parent)
    : QWidget(parent)
    , mManager(manager)
    , mHostRoomName(nullptr)
    , mHostUserName(nullptr)
    , mHostPort(nullptr)
    , mHostStartBtn(nullptr)
    , mHostStopBtn(nullptr)
    , mClientAddress(nullptr)
    , mClientPort(nullptr)
    , mClientUserName(nullptr)
    , mClientJoinBtn(nullptr)
    , mDiscoveryList(nullptr)
    , mDiscoveryJoinBtn(nullptr)
    , mLeaveBtn(nullptr)
    , mStatusLabel(nullptr)
    , mUserList(nullptr)
    , mRoomLabel(nullptr)
    , mStack(nullptr)
{
    setupUI();

    // Connect manager signals
    connect(mManager, &WBCollaborationManager::userJoined,
            this, &WBCollaborationPalette::onUserJoined);
    connect(mManager, &WBCollaborationManager::userLeft,
            this, &WBCollaborationPalette::onUserLeft);
    connect(mManager, &WBCollaborationManager::userListUpdated,
            this, &WBCollaborationPalette::onUserListUpdated);
    connect(mManager, &WBCollaborationManager::connectionStatusChanged,
            this, &WBCollaborationPalette::onConnectionStatusChanged);
    connect(mManager, &WBCollaborationManager::errorOccurred,
            this, &WBCollaborationPalette::onErrorOccurred);

    updateUI();
}

WBCollaborationPalette::~WBCollaborationPalette()
{
}

void WBCollaborationPalette::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // Title
    QLabel *title = new QLabel(QStringLiteral("Collaboration"), this);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 2);
    title->setFont(titleFont);
    mainLayout->addWidget(title);

    // Status
    mStatusLabel = new QLabel(this);
    mStatusLabel->setWordWrap(true);
    mainLayout->addWidget(mStatusLabel);

    mRoomLabel = new QLabel(this);
    mRoomLabel->setWordWrap(true);
    mainLayout->addWidget(mRoomLabel);

    // Stacked widget for different modes
    mStack = new QStackedWidget(this);

    // --- Page 0: Disconnected / Setup ---
    QWidget *setupPage = new QWidget();
    QVBoxLayout *setupLayout = new QVBoxLayout(setupPage);

    // Host section
    QGroupBox *hostGroup = new QGroupBox(QStringLiteral("Host a Session"), setupPage);
    QFormLayout *hostForm = new QFormLayout(hostGroup);

    mHostRoomName = new QLineEdit(QStringLiteral("My Whiteboard"), hostGroup);
    mHostUserName = new QLineEdit(QStringLiteral("Teacher"), hostGroup);
    mHostPort = new QLineEdit(QString::number(WBCollaborationMessage::DefaultWsPort), hostGroup);
    mHostPort->setMaximumWidth(80);

    hostForm->addRow(QStringLiteral("Room:"), mHostRoomName);
    hostForm->addRow(QStringLiteral("Name:"), mHostUserName);
    hostForm->addRow(QStringLiteral("Port:"), mHostPort);

    mHostStartBtn = new QPushButton(QStringLiteral("Start Hosting"), hostGroup);
    mHostStopBtn = new QPushButton(QStringLiteral("Stop"), hostGroup);
    mHostStopBtn->setVisible(false);

    QHBoxLayout *hostBtnLayout = new QHBoxLayout();
    hostBtnLayout->addWidget(mHostStartBtn);
    hostBtnLayout->addWidget(mHostStopBtn);
    hostForm->addRow(hostBtnLayout);

    setupLayout->addWidget(hostGroup);

    // Client: Manual join section
    QGroupBox *clientGroup = new QGroupBox(QStringLiteral("Join a Session"), setupPage);
    QFormLayout *clientForm = new QFormLayout(clientGroup);

    mClientAddress = new QLineEdit(clientGroup);
    mClientAddress->setPlaceholderText(QStringLiteral("192.168.1.5"));
    mClientPort = new QLineEdit(QString::number(WBCollaborationMessage::DefaultWsPort), clientGroup);
    mClientPort->setMaximumWidth(80);
    mClientUserName = new QLineEdit(QStringLiteral("Student"), clientGroup);

    clientForm->addRow(QStringLiteral("Host IP:"), mClientAddress);
    clientForm->addRow(QStringLiteral("Port:"), mClientPort);
    clientForm->addRow(QStringLiteral("Name:"), mClientUserName);

    mClientJoinBtn = new QPushButton(QStringLiteral("Join"), clientGroup);
    clientForm->addRow(mClientJoinBtn);

    setupLayout->addWidget(clientGroup);

    // Discovery section
    QGroupBox *discoveryGroup = new QGroupBox(QStringLiteral("Discovered Rooms"), setupPage);
    QVBoxLayout *discoveryLayout = new QVBoxLayout(discoveryGroup);

    mDiscoveryList = new QListWidget(discoveryGroup);
    mDiscoveryList->setMaximumHeight(120);
    mDiscoveryJoinBtn = new QPushButton(QStringLiteral("Join Selected Room"), discoveryGroup);

    discoveryLayout->addWidget(mDiscoveryList);
    discoveryLayout->addWidget(mDiscoveryJoinBtn);

    setupLayout->addWidget(discoveryGroup);
    setupLayout->addStretch();

    mStack->addWidget(setupPage);

    // --- Page 1: Connected ---
    QWidget *connectedPage = new QWidget();
    QVBoxLayout *connectedLayout = new QVBoxLayout(connectedPage);

    QLabel *userListLabel = new QLabel(QStringLiteral("Participants:"), connectedPage);
    connectedLayout->addWidget(userListLabel);

    mUserList = new QListWidget(connectedPage);
    connectedLayout->addWidget(mUserList);

    mLeaveBtn = new QPushButton(QStringLiteral("Leave Session"), connectedPage);
    QPalette leavePal = mLeaveBtn->palette();
    leavePal.setColor(QPalette::Button, QColor(220, 60, 60));
    mLeaveBtn->setPalette(leavePal);
    connectedLayout->addWidget(mLeaveBtn);

    mStack->addWidget(connectedPage);

    mainLayout->addWidget(mStack);

    // --- Connect buttons ---
    connect(mHostStartBtn, &QPushButton::clicked,
            this, &WBCollaborationPalette::onStartHost);
    connect(mHostStopBtn, &QPushButton::clicked,
            this, &WBCollaborationPalette::onStopHost);
    connect(mClientJoinBtn, &QPushButton::clicked,
            this, &WBCollaborationPalette::onJoinByAddress);
    connect(mDiscoveryJoinBtn, &QPushButton::clicked,
            this, &WBCollaborationPalette::onJoinRoom);
    connect(mLeaveBtn, &QPushButton::clicked,
            this, &WBCollaborationPalette::onLeaveRoom);
}

void WBCollaborationPalette::onStartHost()
{
    QString room = mHostRoomName->text().trimmed();
    QString user = mHostUserName->text().trimmed();
    quint16 port = static_cast<quint16>(mHostPort->text().toUInt());

    if (room.isEmpty() || user.isEmpty()) {
        setStatus(QStringLiteral("Room name and user name are required."), true);
        return;
    }

    setStatus(QStringLiteral("Starting..."));
    mManager->startHosting(room, user, port);
}

void WBCollaborationPalette::onStopHost()
{
    mManager->stopHosting();
}

void WBCollaborationPalette::onJoinByAddress()
{
    QString hostAddr = mClientAddress->text().trimmed();
    quint16 port = static_cast<quint16>(mClientPort->text().toUInt());
    QString user = mClientUserName->text().trimmed();

    if (hostAddr.isEmpty() || user.isEmpty()) {
        setStatus(QStringLiteral("Host address and name are required."), true);
        return;
    }

    setStatus(QStringLiteral("Connecting..."));
    mManager->joinRoom(hostAddr, port, QString(), user);
}

void WBCollaborationPalette::onJoinRoom()
{
    QListWidgetItem *item = mDiscoveryList->currentItem();
    if (!item) {
        setStatus(QStringLiteral("Select a room first."), true);
        return;
    }

    QString user = mClientUserName->text().trimmed();
    if (user.isEmpty()) {
        setStatus(QStringLiteral("Enter your name first."), true);
        return;
    }

    // The item data stores key: "address:port:roomId"
    QStringList parts = item->data(Qt::UserRole).toString().split(':');
    if (parts.size() < 3) return;

    QString address = parts[0];
    quint16 port = static_cast<quint16>(parts[1].toUInt());
    QString roomId = parts.mid(2).join(':');

    setStatus(QStringLiteral("Connecting..."));
    mManager->joinRoom(address, port, roomId, user);
}

void WBCollaborationPalette::onLeaveRoom()
{
    mManager->leaveRoom();
}

void WBCollaborationPalette::onHostDiscovered(const QString &roomId,
                                                const QString &hostAddress,
                                                quint16 port)
{
    QString label = QStringLiteral("%1  (%2:%3)")
                        .arg(roomId, hostAddress)
                        .arg(port);
    QString key = hostAddress + ":" + QString::number(port) + ":" + roomId;

    // Deduplicate
    for (int i = 0; i < mDiscoveryList->count(); ++i) {
        if (mDiscoveryList->item(i)->data(Qt::UserRole).toString() == key)
            return;
    }

    QListWidgetItem *item = new QListWidgetItem(label);
    item->setData(Qt::UserRole, key);
    mDiscoveryList->addItem(item);
}

void WBCollaborationPalette::onUserJoined(const QString &userName)
{
    mUserList->addItem(userName);
}

void WBCollaborationPalette::onUserLeft(const QString &userName)
{
    for (int i = 0; i < mUserList->count(); ++i) {
        if (mUserList->item(i)->text() == userName) {
            delete mUserList->takeItem(i);
            break;
        }
    }
}

void WBCollaborationPalette::onUserListUpdated(const QStringList &users)
{
    mUserList->clear();
    mUserList->addItems(users);
}

void WBCollaborationPalette::onConnectionStatusChanged(const QString &status)
{
    setStatus(status);
    updateUI();
}

void WBCollaborationPalette::onErrorOccurred(const QString &error)
{
    setStatus(error, true);
}

void WBCollaborationPalette::updateUI()
{
    bool active = mManager->isActive();
    bool isHost = (mManager->mode() == WBCollaborationManager::Host);

    mStack->setCurrentIndex(active ? 1 : 0);

    mHostStartBtn->setVisible(!active);
    mHostStopBtn->setVisible(active && isHost);

    mHostRoomName->setEnabled(!active);
    mHostUserName->setEnabled(!active);
    mHostPort->setEnabled(!active);

    mClientAddress->setEnabled(!active);
    mClientPort->setEnabled(!active);
    mClientUserName->setEnabled(!active);
    mClientJoinBtn->setEnabled(!active);

    mDiscoveryJoinBtn->setEnabled(!active && mDiscoveryList->currentItem() != nullptr);

    if (active) {
        mRoomLabel->setText(QStringLiteral("Room: %1  |  You: %2")
                                .arg(mManager->roomId(), mManager->userName()));
    } else {
        mRoomLabel->setText(QString());
    }
}

void WBCollaborationPalette::setStatus(const QString &text, bool isError)
{
    mStatusLabel->setText(text);
    if (isError) {
        mStatusLabel->setStyleSheet(QStringLiteral("color: #cc3333;"));
    } else {
        mStatusLabel->setStyleSheet(QString());
    }
}
