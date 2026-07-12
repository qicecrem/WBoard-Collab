1|#ifndef WBPREFERENCESCONTROLLER_H_
2|#define WBPREFERENCESCONTROLLER_H_
3|
4|#include <QtWidgets>
5|#include <QDialog>
6|#include "ui_brushProperties.h"
7|
8|class WBColorPicker;
9|class WBApplication;
10|class WBSettings;
11|class WBPreferencesController;
12|class WBBrushPropertiesFrame;
13|
14|namespace Ui
15|{
16|	class preferencesDialog;
17|}
18|
19|class WBPreferencesDialog : public QDialog
20|{
21|    Q_OBJECT;
22|
23|public:
24|    WBPreferencesDialog(WBPreferencesController* prefController, QWidget* parent = 0,Qt::WindowFlags f = 0 );
25|    ~WBPreferencesDialog();
26|
27|protected:
28|    void closeEvent(QCloseEvent* e);
29|    WBPreferencesController *mPreferencesController;
30|};
31|
32|class WBPreferencesController : public QObject
33|{
34|    Q_OBJECT
35|
36|public:
37|    WBPreferencesController(QWidget *parent);
38|    virtual ~WBPreferencesController();
39|
40|public slots:
41|    void show();
42|
43|protected:
44|    void wire();
45|    void init();
46|
47|    WBPreferencesDialog* mPreferencesWindow;
48|    Ui::preferencesDialog* mPreferencesUI;
49|    WBBrushPropertiesFrame* mPenProperties;
50|    WBBrushPropertiesFrame* mMarkerProperties;
51|    WBColorPicker* mDarkBackgroundGridColorPicker;
52|    WBColorPicker* mLightBackgroundGridColorPicker;
53|
54|protected slots:
55|    void close();
56|    void defaultSettings();
57|    void penPreviewFromSizeChanged(int value);
58|    void darkBackgroundCrossOpacityValueChanged(int value);
59|    void lightBackgroundCrossOpacityValueChanged(int value);
60|    void widthSliderChanged(int value);
61|    void opacitySliderChanged(int value);
62|    void colorSelected(const QColor&);
63|    void setCrossColorOnDarkBackground(const QColor& color);
64|    void setCrossColorOnLightBackground(const QColor& color);
65|    void toolbarPositionChanged(bool checked);
66|    void toolbarOrientationVertical(bool checked);
67|    void toolbarOrientationHorizontal(bool checked);
68|    void systemOSKCheckBoxToggled(bool checked);
69|
70|private slots:
71|    void adjustScreens(int screen);
72|
73|private:
74|    static qreal sSliderRatio;
75|    static qreal sMinPenWidth;
76|    static qreal sMaxPenWidth;
77|    QScreen* mDesktop;
78|
79|};
80|
81|class WBBrushPropertiesFrame : public Ui::brushProperties
82|{
83|public:
84|    WBBrushPropertiesFrame(QFrame* owner, const QList<QColor>& lightBackgroundColors,const QList<QColor>& darkBackgroundColors, const QList<QColor>& lightBackgroundSelectedColors,const QList<QColor>& darkBackgroundSelectedColors, WBPreferencesController* controller);
85|
86|    virtual ~WBBrushPropertiesFrame(){}
87|
88|    QList<WBColorPicker*> lightBackgroundColorPickers;
89|    QList<WBColorPicker*> darkBackgroundColorPickers;
90|
91|};
92|
93|#endif /* WBPREFERENCESCONTROLLER_H_ */
94|