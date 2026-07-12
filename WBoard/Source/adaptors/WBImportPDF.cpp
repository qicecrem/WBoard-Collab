1|#include "WBImportPDF.h"
2|
3|#include "document/WBDocumentProxy.h"
4|
5|#include "core/WBApplication.h"
6|#include "core/WBPersistenceManager.h"
7|
8|#include "domain/WBGraphicsPDFItem.h"
9|
10|#include "pdf/PDFRenderer.h"
11|
12|#include "core/memcheck.h"
13|
14|WBImportPDF::WBImportPDF(QObject *parent)
15|    : WBPageBasedImportAdaptor(parent)
16|{
17|    QScreen* desktop = QApplication::primaryScreen();
18|    this->dpi = (desktop->physicalDpiX() + desktop->physicalDpiY()) / 2;
19|}
20|
21|
22|WBImportPDF::~WBImportPDF()
23|{
24|    // NOOP
25|}
26|
27|
28|QStringList WBImportPDF::supportedExtentions()
29|{
30|    return QStringList("pdf");
31|}
32|
33|
34|QString WBImportPDF::importFileFilter()
35|{
36|    return tr("Portable Document Format (*.pdf)");
37|}
38|
39|
40|QList<WBGraphicsItem*> WBImportPDF::import(const QUuid& uuid, const QString& filePath)
41|{
42|    QList<WBGraphicsItem*> result;
43|
44|    PDFRenderer *pdfRenderer = PDFRenderer::rendererForUuid(uuid, filePath, true); // renderer is automatically deleted when not used anymore
45|
46|    if (!pdfRenderer->isValid())
47|    {
48|        WBApplication::showMessage(tr("PDF import failed."));
49|        return result;
50|    }
51|    pdfRenderer->setDPI(this->dpi);
52|
53|    int pdfPageCount = pdfRenderer->pageCount();
54|
55|    for(int pdfPageNumber = 1; pdfPageNumber <= pdfPageCount; pdfPageNumber++)
56|    {
57|        WBApplication::showMessage(tr("Importing page %1 of %2").arg(pdfPageNumber).arg(pdfPageCount), true);
58|        result << new WBGraphicsPDFItem(pdfRenderer, pdfPageNumber); // deleted by the scene
59|    }
60|    return result;
61|}
62|
63|void WBImportPDF::placeImportedItemToScene(WBGraphicsScene* scene, WBGraphicsItem* item)
64|{
65|    WBGraphicsPDFItem *pdfItem = (WBGraphicsPDFItem*)item;
66|
67|    pdfItem->setPos(-pdfItem->boundingRect().width() / 2, -pdfItem->boundingRect().height() / 2);
68|
69|    scene->setAsBackgroundObject(pdfItem, false, false);
70|
71|    scene->setNominalSize(pdfItem->boundingRect().width(), pdfItem->boundingRect().height());
72|}
73|
74|const QString& WBImportPDF::folderToCopy()
75|{
76|    return WBPersistenceManager::objectDirectory;
77|}
78|