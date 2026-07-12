1|#include <QScreen>
2|
3|#include "WBScreenMirror.h"
4|
5|#include "core/WBSettings.h"
6|#include "core/WBSetting.h"
7|#include "core/WBApplication.h"
8|#include "board/WBBoardController.h"
9|
10|#if defined(Q_OS_OSX)
11|#include <ApplicationServices/ApplicationServices.h>
12|#endif
13|
14|#include "core/memcheck.h"
15|
16|
17|WBScreenMirror::WBScreenMirror(QWidget* parent)
18|    : QWidget(parent)
19|    , mScreenIndex(0)
20|    , mSourceWidget(0)
21|    , mTimerID(0)
22|{
23|    // NOOP
24|}
25|
26|
27|WBScreenMirror::~WBScreenMirror()
28|{
29|    // NOOP
30|}
31|
32|
33|void WBScreenMirror::paintEvent(QPaintEvent *event)
34|{
35|    Q_UNUSED(event);
36|
37|    QPainter painter(this);
38|
39|    painter.fillRect(0, 0, width(), height(), QBrush(Qt::black));
40|
41|    if (!mLastPixmap.isNull())
42|    {
43|        int x = (width() - mLastPixmap.width()) / 2;
44|        int y = (height() - mLastPixmap.height()) / 2;
45|
46|        painter.drawPixmap(x, y, mLastPixmap);
47|    }
48|}
49|
50|
51|void WBScreenMirror::timerEvent(QTimerEvent *event)
52|{
53|    Q_UNUSED(event);
54|
55|    grabPixmap();
56|
57|    update();
58|}
59|
60|void WBScreenMirror::grabPixmap()
61|{
62|    if (mSourceWidget)
63|    {
64|        QPoint topLeft = mSourceWidget->mapToGlobal(mSourceWidget->geometry().topLeft());
65|        QPoint bottomRight = mSourceWidget->mapToGlobal(mSourceWidget->geometry().bottomRight());
66|
67|        mRect.setTopLeft(topLeft);
68|        mRect.setBottomRight(bottomRight);
69|        mLastPixmap = mSourceWidget->grab();
70|    }
71|    else{
72|        // WHY HERE?
73|        // this is the case we are showing the desktop but the is no widget and we use the last widget rectagle to know
74|        // what we have to grab. Not very good way of doing
75|        QScreen * desktop = QApplication::primaryScreen();
76|        QScreen * screen = WBApplication::controlScreen();
77|        mLastPixmap = screen->grabWindow(desktop->effectiveWinId(), mRect.x(), mRect.y(), mRect.width(), mRect.height());
78|    }
79|
80|    if (!mLastPixmap.isNull())
81|        mLastPixmap = mLastPixmap.scaled(width(), height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
82|}
83|
84|
85|void WBScreenMirror::setSourceWidget(QWidget *sourceWidget)
86|{
87|    mSourceWidget = sourceWidget;
88|
89|    mScreenIndex = QApplication::primaryScreen()->screenNumber(sourceWidget);
90|
91|    grabPixmap();
92|
93|    update();
94|}
95|
96|
97|void WBScreenMirror::start()
98|{
99|    qDebug() << "mirroring START";
100|    WBApplication::boardController->freezeW3CWidgets(true);
101|    if (mTimerID == 0)
102|    {
103|        int ms = 125;
104|
105|        bool success;
106|        int fps = WBSettings::settings()->mirroringRefreshRateInFps->get().toInt(&success);
107|
108|        if (success && fps > 0)
109|        {
110|            ms = 1000 / fps;
111|        }
112|
113|        mTimerID = startTimer(ms);
114|    }
115|    else
116|    {
117|        qDebug() << "WBScreenMirror::start() : Timer already running ...";
118|    }
119|}
120|
121|
122|void WBScreenMirror::stop()
123|{
124|    qDebug() << "mirroring STOP";
125|    WBApplication::boardController->freezeW3CWidgets(false);
126|    if (mTimerID != 0)
127|    {
128|        killTimer(mTimerID);
129|        mTimerID = 0;
130|    }
131|}
132|