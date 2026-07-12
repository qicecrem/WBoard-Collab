1|#include "WBExportPDF.h"
2|
3|#include <QtCore>
4|#include <QtSvg>
5|#include <QPrinter>
6|#include <QPdfWriter>
7|
8|#include "core/WBApplication.h"
9|#include "core/WBSettings.h"
10|#include "core/WBSetting.h"
11|#include "core/WBPersistenceManager.h"
12|
13|#include "domain/WBGraphicsScene.h"
14|#include "domain/WBGraphicsSvgItem.h"
15|#include "domain/WBGraphicsPDFItem.h"
16|
17|#include "document/WBDocumentProxy.h"
18|#include "document/WBDocumentController.h"
19|
20|#include "pdf/GraphicsPDFItem.h"
21|
22|#include "core/memcheck.h"
23|
24|WBExportPDF::WBExportPDF(QObject *parent)
25|    : WBExportAdaptor(parent)
26|{
27|    // NOOP
28|}
29|
30|WBExportPDF::~WBExportPDF()
31|{
32|    // NOOP
33|}
34|
35|void WBExportPDF::persist(WBDocumentProxy* pDocumentProxy)
36|{
37|    persistLocally(pDocumentProxy, tr("Export as PDF File"));
38|}
39|
40|bool WBExportPDF::associatedActionactionAvailableFor(const QModelIndex &selectedIndex)
41|{
42|    const WBDocumentTreeModel *docModel = qobject_cast<const WBDocumentTreeModel*>(selectedIndex.model());
43|    if (!selectedIndex.isValid() || docModel->isCatalog(selectedIndex)) {
44|        return false;
45|    }
46|
47|    return true;
48|}
49|
50|
51|bool WBExportPDF::persistsDocument(WBDocumentProxy* pDocumentProxy, const QString& filename)
52|{
53|    QPdfWriter pdfWriter(filename);
54|
55|    qDebug() << "exporting document to PDF" << filename;
56|
57|    pdfWriter.setResolution(WBSettings::settings()->pdfResolution->get().toInt());
58|    pdfWriter.setPageMargins(QMarginsF());
59|    pdfWriter.setTitle(pDocumentProxy->name());
60|    pdfWriter.setCreator("WBoard PDF export");
61|
62|    //need to calculate screen resolution
63|    QScreen* desktop = QApplication::primaryScreen();
64|    int dpiCommon = (desktop->physicalDpiX() + desktop->physicalDpiY()) / 2;
65|    float scaleFactor = 72.0f / dpiCommon;
66|
67|    QPainter pdfPainter;
68|    bool painterNeedsBegin = true;
69|
70|    int existingPageCount = pDocumentProxy->pageCount();
71|
72|    for(int pageIndex = 0 ; pageIndex < existingPageCount; pageIndex++) {
73|
74|        WBGraphicsScene* scene = WBPersistenceManager::persistenceManager()->loadDocumentScene(pDocumentProxy, pageIndex);
75|        WBApplication::showMessage(tr("Exporting page %1 of %2").arg(pageIndex + 1).arg(existingPageCount));
76|
77|        // set background to white, no crossing for PDF output
78|        bool isDark = scene->isDarkBackground();
79|        WBPageBackground pageBackground = scene->pageBackground();
80|        scene->setBackground(false, WBPageBackground::plain);
81|
82|        // pageSize is the output PDF page size; it is set to equal the scene's boundary size; if the contents
83|        // of the scene overflow from the boundaries, they will be scaled down.
84|        QSize pageSize = scene->sceneSize();
85|
86|        // set high res rendering
87|        scene->setRenderingQuality(WBItem::RenderingQualityHigh);
88|        scene->setRenderingContext(WBGraphicsScene::NonScreen);
89|
90|        // Setting output page size
91|        QPageSize outputPageSize = QPageSize(QSizeF(pageSize.width()*scaleFactor, pageSize.height()*scaleFactor), QPageSize::Point);
92|        pdfWriter.setPageSize(outputPageSize);
93|
94|        // Call begin only once
95|        if(painterNeedsBegin)
96|            painterNeedsBegin = !pdfPainter.begin(&pdfWriter);
97|
98|        else if (pageIndex < existingPageCount)
99|            pdfWriter.newPage();
100|
101|        // Render the scene
102|        scene->render(&pdfPainter, QRectF(), scene->normalizedSceneRect());
103|
104|        // Restore screen rendering quality
105|        scene->setRenderingContext(WBGraphicsScene::Screen);
106|        scene->setRenderingQuality(WBItem::RenderingQualityNormal);
107|
108|        // Restore background state
109|        scene->setBackground(isDark, pageBackground);
110|    }
111|
112|    if(!painterNeedsBegin)
113|        pdfPainter.end();
114|
115|    return true;
116|}
117|
118|QString WBExportPDF::exportExtention()
119|{
120|    return QString(".pdf");
121|}
122|
123|QString WBExportPDF::exportName()
124|{
125|    return tr("Export to PDF");
126|}
127|