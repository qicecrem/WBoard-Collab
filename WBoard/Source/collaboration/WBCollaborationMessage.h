#ifndef WBCOLLABORATIONMESSAGE_H
#define WBCOLLABORATIONMESSAGE_H

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QString>

namespace WBCollaborationMessage {

    // Message types
    const QString TypeJoin       = QStringLiteral("join");
    const QString TypeLeave      = QStringLiteral("leave");
    const QString TypeUsers      = QStringLiteral("users");
    const QString TypeStroke     = QStringLiteral("stroke");
    const QString TypeErase      = QStringLiteral("erase");
    const QString TypeCommand    = QStringLiteral("command");
    const QString TypePageAdd    = QStringLiteral("pageAdd");
    const QString TypePageDelete = QStringLiteral("pageDelete");
    const QString TypePageSwitch = QStringLiteral("pageSwitch");
    const QString TypeCursor     = QStringLiteral("cursor");
    const QString TypeChat       = QStringLiteral("chat");
    const QString TypeError      = QStringLiteral("error");

    // Tool names for serialization
    const QString ToolPen    = QStringLiteral("pen");
    const QString ToolMarker = QStringLiteral("marker");
    const QString ToolLine   = QStringLiteral("line");
    const QString ToolEraser = QStringLiteral("eraser");

    // UDP discovery
    constexpr int    DiscoveryPort    = 19876;
    constexpr int    DiscoveryIntervalMs = 2000;
    const QString    DiscoveryPrefix  = QStringLiteral("WBOARD_DISCOVER");

    // Default WebSocket server port
    constexpr int    DefaultWsPort    = 19877;

    // Build helpers
    inline QJsonObject buildJoinMessage(const QString &roomId, const QString &userName) {
        QJsonObject obj;
        obj[QStringLiteral("type")] = TypeJoin;
        obj[QStringLiteral("room")] = roomId;
        obj[QStringLiteral("user")] = userName;
        return obj;
    }

    inline QJsonObject buildStrokeMessage(const QString &tool,
                                           const QString &colorOnLight,
                                           const QString &colorOnDark,
                                           int widthIndex,
                                           int pageIndex,
                                           const QJsonArray &points) {
        QJsonObject obj;
        obj[QStringLiteral("type")]          = TypeStroke;
        obj[QStringLiteral("tool")]          = tool;
        obj[QStringLiteral("colorOnLight")]  = colorOnLight;
        obj[QStringLiteral("colorOnDark")]   = colorOnDark;
        obj[QStringLiteral("widthIndex")]    = widthIndex;
        obj[QStringLiteral("pageIndex")]     = pageIndex;
        obj[QStringLiteral("points")]        = points;
        return obj;
    }

    inline QByteArray toJson(const QJsonObject &obj) {
        return QJsonDocument(obj).toJson(QJsonDocument::Compact);
    }

    inline QJsonObject fromJson(const QByteArray &data) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError)
            return QJsonObject();
        return doc.object();
    }

    inline QJsonObject fromJson(const QString &text) {
        return fromJson(text.toUtf8());
    }

} // namespace WBCollaborationMessage

#endif // WBCOLLABORATIONMESSAGE_H
