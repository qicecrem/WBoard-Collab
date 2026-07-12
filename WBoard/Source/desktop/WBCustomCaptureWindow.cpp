1|#include <QApplication>
2|#include <QScreen>
3|#include <QStyle>
4|
5|#include "WBCustomCaptureWindow.h"
6|
7|#include "frameworks/WBPlatformUtils.h"
8|#include "gui/WBRubberBand.h"
9|
10|#include "core/memcheck.h"
11|
12|WBCustomCaptureWindow::WBCustomCaptureWindow(QWidget *parent)
13|    : QDialog(parent, Qt::FramelessWindowHint  | Qt::Window)
14|    , mSelectionBand(0)
15|    , mRubberBandStyle(0)
16|    , mOrigin(0,0)
17|    , mIsSelecting(false)
18|{
19|    setCursor(Qt::CrossCursor);
20|    setWindowOpacity(0.0);
21|}
22|
23|
24|WBCustomCaptureWindow::~WBCustomCaptureWindow()
25|{
26|    delete mSelectionBand;
27|    delete mRubberBandStyle;
28|}
29|
30|
31|QPixmap WBCustomCaptureWindow::getSelectedPixmap()
32|{
33|    if (mSelectionBand)
34|    {
35|        return mWholeScreenPixmap.copy(mSelectionBand->geometry());
36|    }
37|    else
38|    {
39|        return QPixmap();
40|    }
41|}
42|
43|
44|int WBCustomCaptureWindow::execute(const QPixmap &pScreenPixmap)
45|{
46|    mWholeScreenPixmap = pScreenPixmap;
47|
48|    QScreen *desktop = QApplication::primaryScreen();
49|    int currentScreen = desktop->screenNumber(QCursor::pos());
50|    setGeometry(desktop->geometry(currentScreen));
51|    this->show();
52|    setWindowOpacity(1.0);
53|
54|    return exec();
55|}
56|
57|
58|void WBCustomCaptureWindow::mouseMoveEvent ( QMouseEvent * event )
59|{
60|    if (mIsSelecting)
61|    {
62|        mSelectionBand->setGeometry(QRect(mOrigin, event->pos()).normalized());
63|    }
64|
65|    event->accept();
66|}
67|
68|
69|void WBCustomCaptureWindow::mousePressEvent ( QMouseEvent * event )
70|{
71|    if (!mIsSelecting)
72|    {
73|        mIsSelecting = true;
74|        mOrigin = event->pos();
75|
76|        if (!mSelectionBand)
77|        {
78|            mSelectionBand = new WBRubberBand(QRubberBand::Rectangle, this);
79|        }
80|
81|        mSelectionBand->setGeometry(QRect(mOrigin, QSize()));
82|        mSelectionBand->show();
83|        event->accept();
84|    }
85|}
86|
87|
88|void WBCustomCaptureWindow::mouseReleaseEvent ( QMouseEvent * event )
89|{
90|    mIsSelecting = false;
91|
92|    if (mSelectionBand)
93|    {
94|        mSelectionBand->hide();
95|    }
96|
97|    event->accept();
98|
99|    // do not accept very small selection
100|    if (!(mSelectionBand->geometry().width() < 6 && mSelectionBand->geometry().height() < 6))
101|    {
102|        accept();
103|    }
104|
105|}
106|
107|
108|void WBCustomCaptureWindow::keyPressEvent ( QKeyEvent * event )
109|{
110|    if (event->key() == Qt::Key_Escape)
111|    {
112|        mIsSelecting = false;
113|
114|        if (mSelectionBand)
115|        {
116|            mSelectionBand->hide();
117|        }
118|
119|        event->accept();
120|        reject();
121|    }
122|}
123|
124|
125|void WBCustomCaptureWindow::showEvent ( QShowEvent * event )
126|{
127|    Q_UNUSED(event);
128|}
129|
130|void WBCustomCaptureWindow::paintEvent(QPaintEvent *event)
131|{
132|    Q_UNUSED(event);
133|    QPainter painter(this);
134|    painter.drawPixmap(0,0, mWholeScreenPixmap);
135|}
136|