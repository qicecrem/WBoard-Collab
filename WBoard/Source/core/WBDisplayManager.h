#ifndef WBDISPLAYMANAGER_H_
#define WBDISPLAYMANAGER_H_

#include <QtWidgets>
#include <QScreen>

class WBBlackoutWidget;
class WBBoardView;

class WBDisplayManager : public QObject
11|{
12|    Q_OBJECT
13|
14|    public:
15|        WBDisplayManager(QObject *parent = 0);
16|        virtual ~WBDisplayManager();
17|
18|        int numScreens();
19|
20|        int numPreviousViews();
21|
22|        void setControlWidget(QWidget* pControlWidget);
23|
24|        void setDisplayWidget(QWidget* pDisplayWidget);
25|
26|        void setDesktopWidget(QWidget* pControlWidget);
27|
28|        void setPreviousDisplaysWidgets(QList<WBBoardView*> pPreviousViews);
29|
30|        bool hasControl()
31|        {
32|            return mControlScreenIndex > -1;
33|        }
34|
35|        bool hasDisplay()
36|        {
37|            return mDisplayScreenIndex > -1;
38|        }
39|
40|        bool hasPrevious()
41|        {
42|            return !mPreviousScreenIndexes.isEmpty();
43|        }
44|
45|        enum DisplayRole
46|        {
47|            None = 0, Control, Display, Previous1, Previous2, Previous3, Previous4, Previous5
48|        };
49|
50|        bool useMultiScreen() { return mUseMultiScreen; }
51|
52|        void setUseMultiScreen(bool pUse);
53|
54|        int controleScreenIndex()
55|        {
56|            return mControlScreenIndex;
57|        }
58|
59|        QRect controlGeometry();
60|        QRect displayGeometry();
61|
62|   signals:
63|        void screenLayoutChanged();
64|        void adjustDisplayViewsRequired();
65|
66|   public slots:
67|        void reinitScreens(bool bswap);
68|
69|        void adjustScreens(int screen);
70|
71|        void blackout();
72|
73|        void unBlackout();
74|
75|        void setRoleToScreen(DisplayRole role, int screenIndex);
76|
77|        void swapDisplayScreens(bool swap);
78|
private:
    void positionScreens();

    void initScreenIndexes();

    QRect screenGeometry(int screenIndex);
83|
84|        int mControlScreenIndex;
85|
86|        int mDisplayScreenIndex;
87|
88|        QList<int> mPreviousScreenIndexes;
89|
90|        QScreen* mDesktop;
91|
92|        QWidget* mControlWidget;
93|
94|        QWidget* mDisplayWidget;
95|
96|        QWidget *mDesktopWidget;
97|
98|        QList<WBBoardView*> mPreviousDisplayWidgets;
99|
100|        QList<WBBlackoutWidget*> mBlackoutWidgets;
101|
102|        QList<DisplayRole> mScreenIndexesRoles;
103|
104|        bool mUseMultiScreen;
105|
106|};
107|
108|#endif /* WBDISPLAYMANAGER_H_ */
109|