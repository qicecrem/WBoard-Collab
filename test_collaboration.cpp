// Collaboration module standalone test
// Build: cd build && g++ -std=c++17 -I../WBoard/Source -I../WBoard/Source/collaboration \
//   $(pkg-config --cflags Qt6Core Qt6WebSockets Qt6Network) \
//   ../WBoard/Source/collaboration/WBSignalingServer.cpp \
//   ../WBoard/Source/collaboration/WBCollaborationClient.cpp \
//   test_collaboration.cpp \
//   $(pkg-config --libs Qt6Core Qt6WebSockets Qt6Network) -fPIC -o test_collaboration

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>

#include "WBSignalingServer.h"
#include "WBCollaborationClient.h"
#include "WBCollaborationMessage.h"

static int g_passed = 0;
static int g_failed = 0;

#define TEST(name) qDebug() << "\n=== TEST:" << name << "==="
#define CHECK(cond, msg) do { \
    if (cond) { qDebug() << "  PASS:" << msg; g_passed++; } \
    else { qDebug() << "  FAIL:" << msg; g_failed++; } \
} while(0)

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== Collaboration Module Test Suite ===\n";

    // ===== Test 1: Signaling Server =====
    TEST("SignalingServer");
    {
        WBSignalingServer server(19990);
        CHECK(server.isListening(), "Server listens on port 19990");
        CHECK(server.serverPort() == 19990, "Server port is 19990");
        CHECK(server.rooms().isEmpty(), "No rooms initially");
        CHECK(server.usersInRoom("test").isEmpty(), "No users in empty room");
    }
    qDebug() << "  Server auto-closed (RAII)";

    // ===== Test 2: Client Connection =====
    TEST("Client Connection");
    {
        WBSignalingServer server(19991);
        CHECK(server.isListening(), "Server started");

        WBCollaborationClient client1;
        client1.connectToServer(QUrl("ws://127.0.0.1:19991"));

        // Wait for connection
        QEventLoop loop;
        QTimer::singleShot(2000, &loop, &QEventLoop::quit);
        QObject::connect(&client1, &WBCollaborationClient::connected, [&]() {
            qDebug() << "  Client1 connected";
            loop.quit();
        });
        loop.exec();

        CHECK(client1.isConnected(), "Client connected to server");
    }
    qDebug() << "  Server + client auto-closed";

    // ===== Test 3: Room Join =====
    TEST("Room Join");
    {
        WBSignalingServer server(19992);
        CHECK(server.isListening(), "Server started");

        WBCollaborationClient client1;
        client1.connectToServer(QUrl("ws://127.0.0.1:19992"));

        QEventLoop loop;
        bool joined = false;
        QStringList userList;

        QObject::connect(&client1, &WBCollaborationClient::connected, [&]() {
            client1.joinRoom("room-abc", "Alice");
        });
        QObject::connect(&client1, &WBCollaborationClient::userListReceived, [&](const QStringList &users) {
            userList = users;
            joined = true;
            loop.quit();
        });
        QTimer::singleShot(3000, &loop, &QEventLoop::quit);
        loop.exec();

        CHECK(joined, "Client1 joined room and received user list");
        CHECK(userList.contains("Alice"), "Alice is in the user list");
    }

    // ===== Test 4: Two Clients in Same Room =====
    TEST("Two Clients in Same Room");
    {
        WBSignalingServer server(19993);
        CHECK(server.isListening(), "Server started");

        WBCollaborationClient client1;
        WBCollaborationClient client2;

        client1.connectToServer(QUrl("ws://127.0.0.1:19993"));
        client2.connectToServer(QUrl("ws://127.0.0.1:19993"));

        bool c1Joined = false, c2Joined = false, c1SeesC2 = false;

        QObject::connect(&client1, &WBCollaborationClient::connected, [&]() {
            client1.joinRoom("room-xyz", "Alice");
        });
        QObject::connect(&client1, &WBCollaborationClient::userJoined, [&](const QString &user) {
            if (user == "Bob") c1SeesC2 = true;
        });

        QObject::connect(&client2, &WBCollaborationClient::connected, [&]() {
            client2.joinRoom("room-xyz", "Bob");
        });
        QObject::connect(&client2, &WBCollaborationClient::userListReceived, [&](const QStringList &users) {
            c2Joined = (users.size() >= 2);
        });
        QObject::connect(&client1, &WBCollaborationClient::userListReceived, [&](const QStringList &users) {
            c1Joined = (users.size() >= 1);
        });

        QEventLoop loop;
        QTimer::singleShot(3000, &loop, &QEventLoop::quit);
        loop.exec();

        CHECK(c1Joined, "Client1 joined room");
        CHECK(c2Joined, "Client2 joined room and sees both users");
        CHECK(c1SeesC2, "Client1 received Bob's join notification");
        CHECK(server.usersInRoom("room-xyz").size() == 2, "Server reports 2 users in room");
    }

    // ===== Test 5: Stroke Message Round-Trip =====
    TEST("Stroke Message Round-Trip");
    {
        WBSignalingServer server(19994);
        CHECK(server.isListening(), "Server started");

        WBCollaborationClient client1;
        WBCollaborationClient client2;

        client1.connectToServer(QUrl("ws://127.0.0.1:19994"));
        client2.connectToServer(QUrl("ws://127.0.0.1:19994"));

        bool strokeReceived = false;
        QJsonObject receivedStroke;

        QObject::connect(&client1, &WBCollaborationClient::connected, [&]() {
            client1.joinRoom("stroke-room", "Alice");
        });
        QObject::connect(&client2, &WBCollaborationClient::connected, [&]() {
            client2.joinRoom("stroke-room", "Bob");
        });
        QObject::connect(&client2, &WBCollaborationClient::strokeReceived, [&](const QJsonObject &data) {
            receivedStroke = data;
            strokeReceived = true;
        });

        // Wait for both to join, then send stroke
        QEventLoop loop;
        QTimer *sendTimer = new QTimer();
        sendTimer->setSingleShot(true);
        QObject::connect(sendTimer, &QTimer::timeout, [&]() {
            QJsonArray pts;
            pts.append(QJsonArray{100.0, 200.0, 1.0});
            pts.append(QJsonArray{105.0, 205.0, 0.95});
            pts.append(QJsonArray{110.0, 210.0, 0.90});

            QJsonObject msg = WBCollaborationMessage::buildStrokeMessage(
                "pen", "#FF0000", "#FF4444", 2, 0, pts);
            client1.sendMessage(msg);
        });
        sendTimer->start(1500);

        QTimer::singleShot(3000, &loop, &QEventLoop::quit);
        loop.exec();

        CHECK(strokeReceived, "Client2 received stroke message");
        CHECK(receivedStroke[QStringLiteral("tool")].toString() == "pen", "Tool is pen");
        CHECK(receivedStroke[QStringLiteral("colorOnLight")].toString() == "#FF0000", "Color is correct");
        CHECK(receivedStroke[QStringLiteral("widthIndex")].toInt() == 2, "Width index is correct");
        CHECK(receivedStroke[QStringLiteral("points")].toArray().size() == 3, "Has 3 points");
    }

    // ===== Test 6: Erase Message Round-Trip =====
    TEST("Erase Message Round-Trip");
    {
        WBSignalingServer server(19995);
        CHECK(server.isListening(), "Server started");

        WBCollaborationClient client1;
        WBCollaborationClient client2;

        client1.connectToServer(QUrl("ws://127.0.0.1:19995"));
        client2.connectToServer(QUrl("ws://127.0.0.1:19995"));

        bool eraseReceived = false;
        QJsonObject receivedErase;

        QObject::connect(&client1, &WBCollaborationClient::connected, [&]() {
            client1.joinRoom("erase-room", "Alice");
        });
        QObject::connect(&client2, &WBCollaborationClient::connected, [&]() {
            client2.joinRoom("erase-room", "Bob");
        });
        QObject::connect(&client2, &WBCollaborationClient::eraseReceived, [&](const QJsonObject &data) {
            receivedErase = data;
            eraseReceived = true;
        });

        QEventLoop loop;
        QTimer *sendTimer = new QTimer();
        sendTimer->setSingleShot(true);
        QObject::connect(sendTimer, &QTimer::timeout, [&]() {
            QJsonObject msg;
            msg[QStringLiteral("type")] = WBCollaborationMessage::TypeErase;
            msg[QStringLiteral("pageIndex")] = 0;
            QJsonArray uuids;
            uuids.append(QStringLiteral("550e8400-e29b-41d4-a716-446655440000"));
            msg[QStringLiteral("removedUuids")] = uuids;
            msg[QStringLiteral("addedItems")] = QJsonArray();
            client1.sendMessage(msg);
        });
        sendTimer->start(1500);

        QTimer::singleShot(3000, &loop, &QEventLoop::quit);
        loop.exec();

        CHECK(eraseReceived, "Client2 received erase message");
        CHECK(receivedErase[QStringLiteral("removedUuids")].toArray().size() == 1, "Has 1 removed UUID");
    }

    // ===== Test 7: Page Operation Messages =====
    TEST("Page Operations");
    {
        WBSignalingServer server(19996);
        CHECK(server.isListening(), "Server started");

        WBCollaborationClient client1;
        WBCollaborationClient client2;

        client1.connectToServer(QUrl("ws://127.0.0.1:19996"));
        client2.connectToServer(QUrl("ws://127.0.0.1:19996"));

        bool addRcvd = false, delRcvd = false, switchRcvd = false;
        int addIdx = -1, delIdx = -1, switchIdx = -1;

        QObject::connect(&client1, &WBCollaborationClient::connected, [&]() {
            client1.joinRoom("page-room", "Alice");
        });
        QObject::connect(&client2, &WBCollaborationClient::connected, [&]() {
            client2.joinRoom("page-room", "Bob");
        });
        QObject::connect(&client2, &WBCollaborationClient::pageAddReceived, [&](int idx) {
            addRcvd = true; addIdx = idx;
        });
        QObject::connect(&client2, &WBCollaborationClient::pageDeleteReceived, [&](int idx) {
            delRcvd = true; delIdx = idx;
        });
        QObject::connect(&client2, &WBCollaborationClient::pageSwitchReceived, [&](int idx) {
            switchRcvd = true; switchIdx = idx;
        });

        QEventLoop loop;
        QTimer *sendTimer = new QTimer();
        sendTimer->setSingleShot(true);
        QObject::connect(sendTimer, &QTimer::timeout, [&]() {
            // Send page operations
            QJsonObject msg;
            msg[QStringLiteral("type")] = WBCollaborationMessage::TypePageAdd;
            msg[QStringLiteral("index")] = 3;
            client1.sendMessage(msg);

            msg[QStringLiteral("type")] = WBCollaborationMessage::TypePageDelete;
            msg[QStringLiteral("index")] = 2;
            client1.sendMessage(msg);

            msg[QStringLiteral("type")] = WBCollaborationMessage::TypePageSwitch;
            msg[QStringLiteral("index")] = 1;
            client1.sendMessage(msg);
        });
        sendTimer->start(1500);

        QTimer::singleShot(3000, &loop, &QEventLoop::quit);
        loop.exec();

        CHECK(addRcvd && addIdx == 3, "Page add received with index 3");
        CHECK(delRcvd && delIdx == 2, "Page delete received with index 2");
        CHECK(switchRcvd && switchIdx == 1, "Page switch received with index 1");
    }

    // ===== Test 8: Leave and Disconnect =====
    TEST("Leave and Disconnect");
    {
        WBSignalingServer server(19997);
        CHECK(server.isListening(), "Server started");

        WBCollaborationClient client1;
        WBCollaborationClient client2;
        bool c1GotLeave = false;

        client1.connectToServer(QUrl("ws://127.0.0.1:19997"));
        client2.connectToServer(QUrl("ws://127.0.0.1:19997"));

        QObject::connect(&client1, &WBCollaborationClient::connected, [&]() {
            client1.joinRoom("leave-room", "Alice");
        });
        QObject::connect(&client2, &WBCollaborationClient::connected, [&]() {
            client2.joinRoom("leave-room", "Bob");
        });
        QObject::connect(&client1, &WBCollaborationClient::userLeft, [&](const QString &user) {
            if (user == "Bob") c1GotLeave = true;
        });

        QEventLoop loop;
        QTimer *leaveTimer = new QTimer();
        leaveTimer->setSingleShot(true);
        QObject::connect(leaveTimer, &QTimer::timeout, [&]() {
            client2.disconnectFromServer();
            QTimer::singleShot(500, &loop, &QEventLoop::quit);
        });
        leaveTimer->start(1500);

        QTimer::singleShot(4000, &loop, &QEventLoop::quit);
        loop.exec();

        CHECK(c1GotLeave, "Client1 received Bob's leave notification");
        CHECK(!client2.isConnected(), "Client2 is disconnected");
    }

    // ===== Results =====
    qDebug() << "\n========================================";
    qDebug() << "RESULTS:" << g_passed << "passed," << g_failed << "failed";
    qDebug() << "========================================";

    return g_failed > 0 ? 1 : 0;
}
