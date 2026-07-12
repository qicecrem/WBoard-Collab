1|#include "WBExportFullPDF.h"
2|
3|#include <QtCore>
4|#include <QtSvg>
5|#include <QPrinter>
6|
7|#include "core/WBApplication.h"
8|#include "core/WBSettings.h"
9|#include "core/WBSetting.h"
10|#include "core/WBPersistenceManager.h"
11|
12|#include "domain/WBGraphicsScene.h"
13|#include "domain/WBGraphicsSvgItem.h"
14|#include "domain/WBGraphicsPDFItem.h"
15|
16|#include "document/WBDocumentProxy.h"
17|#include "document/WBDocumentController.h"
18|
19|#include "pdf/GraphicsPDFItem.h"
20|
21|#include "WBExportPDF.h"
22|
23|//#include <Merger.h>
24|//#include <Exception.h>
25|//#include <Transformation.h>
26|
27|#include "core/memcheck.h"
28|
29|
30|//using namespace merge_lib;
31|
32|
33|WBExportFullPDF::WBExportFullPDF(QObject *parent)
34|    : WBExportAdaptor(parent)
35|{
36|    //need to calculate screen resolution
37|    QScreen* desktop = QApplication::primaryScreen();
38|    int dpiCommon = (desktop->physicalDpiX() + desktop->physicalDpiY()) / 2;
39|    mScaleFactor = 72.0f / dpiCommon; // 1pt = 1/72 inch
40|
41|    mSimpleExporter = new WBExportPDF();
42|}
43|
44|
45|WBExportFullPDF::~WBExportFullPDF()
46|{
47|    // NOOP
48|}
49|
50|
51|void WBExportFullPDF::saveOverlayPdf(WBDocumentProxy* pDocumentProxy, const QString& filename)
52|{
53|    if (!pDocumentProxy || filename.length() == 0 || pDocumentProxy->pageCount() == 0)
54|        return;
55|
56|    mSimpleExporter->persistsDocument(pDocumentProxy, filename);
57|}
58|
59|
60|void WBExportFullPDF::persist(WBDocumentProxy* pDocumentProxy)
61|{
62|    persistLocally(pDocumentProxy, tr("Export as PDF File"));
63|}
64|
65|
66|bool WBExportFullPDF::persistsDocument(WBDocumentProxy* pDocumentProxy, const QString& filename)
67|{
68|    QFile file(filename);
69|    if (file.exists()) file.remove();
70|
71|    QString overlayName = filename;
72|    overlayName.replace(".pdf", "_overlay.pdf");
73|
74|    QFile previousOverlay(overlayName);
75|    if (previousOverlay.exists())
76|        previousOverlay.remove();
77|
78|    mHasPDFBackgrounds = false;
79|
80|    saveOverlayPdf(pDocumentProxy, overlayName);
81|
82|    if (!mHasPDFBackgrounds)
83|    {
84|        QFile f(overlayName);
85|        f.rename(filename);
86|    }
87|    else
88|    {
89|        //Merger merger;
90|        //try
91|        //{
92|        //    merger.addOverlayDocument(QFile::encodeName(overlayName).constData());
93|
94|        //    MergeDescription mergeInfo;
95|
96|        //    int existingPageCount = pDocumentProxy->pageCount();
97|
98|        //    for(int pageIndex = 0 ; pageIndex < existingPageCount; pageIndex++)
99|        //    {
100|        //        WBGraphicsScene* scene = WBPersistenceManager::persistenceManager()->loadDocumentScene(pDocumentProxy, pageIndex);
101|        //        WBGraphicsPDFItem *pdfItem = qgraphicsitem_cast<WBGraphicsPDFItem*>(scene->backgroundObject());
102|
103|        //        QSize pageSize = scene->nominalSize();
104|        //        
105|        //        if (pdfItem)
106|        //        {
107|        //            QString pdfName = WBPersistenceManager::objectDirectory + "/" + pdfItem->fileUuid().toString() + ".pdf";
108|        //            QString backgroundPath = pDocumentProxy->persistencePath() + "/" + pdfName;
109|        //            QRectF annotationsRect = scene->annotationsBoundingRect();
110|
111|        //            // Original datas
112|        //            double xAnnotation = qRound(annotationsRect.x());
113|        //            double yAnnotation = qRound(annotationsRect.y());
114|        //            double xPdf = qRound(pdfItem->sceneBoundingRect().x());
115|        //            double yPdf = qRound(pdfItem->sceneBoundingRect().y());
116|        //            double hPdf = qRound(pdfItem->sceneBoundingRect().height());
117|
118|        //            // Exportation-transformed datas
119|        //            double hScaleFactor = pageSize.width()/annotationsRect.width();
120|        //            double vScaleFactor = pageSize.height()/annotationsRect.height();
121|        //            double scaleFactor = qMin(hScaleFactor, vScaleFactor);
122|
123|        //            double xAnnotationsOffset = 0;
124|        //            double yAnnotationsOffset = 0;
125|        //            double hPdfTransformed = qRound(hPdf * scaleFactor);
126|
127|        //            // Here, we force the PDF page to be on the topleft corner of the page
128|        //            double xPdfOffset = 0;
129|        //            double yPdfOffset = (hPdf - hPdfTransformed) * mScaleFactor;
130|
131|        //            // Now we align the items
132|        //            xPdfOffset += (xPdf - xAnnotation) * scaleFactor * mScaleFactor;
133|        //            yPdfOffset -= (yPdf - yAnnotation) * scaleFactor * mScaleFactor;
134|
135|        //            // If the PDF was scaled when added to the scene (e.g if it was loaded from a document with a different DPI
136|        //            // than the current one), it should also be scaled here.
137|        //            qreal pdfScale = pdfItem->scale();
138|
139|        //            TransformationDescription pdfTransform(xPdfOffset, yPdfOffset, scaleFactor * pdfScale, 0);
140|        //            TransformationDescription annotationTransform(xAnnotationsOffset, yAnnotationsOffset, 1, 0);
141|
142|        //            MergePageDescription pageDescription(pageSize.width() * mScaleFactor,
143|        //                                                 pageSize.height() * mScaleFactor,
144|        //                                                 pdfItem->pageNumber(),
145|        //                                                 QFile::encodeName(backgroundPath).constData(),
146|        //                                                 pdfTransform,
147|        //                                                 pageIndex + 1,
148|        //                                                 annotationTransform,
149|        //                                                 false, false);
150|
151|        //            mergeInfo.push_back(pageDescription);
152|
153|        //            merger.addBaseDocument(QFile::encodeName(backgroundPath).constData());
154|        //        }
155|        //        else
156|        //        {
157|        //            MergePageDescription pageDescription(pageSize.width() * mScaleFactor,
158|        //                     pageSize.height() * mScaleFactor,
159|        //                     0,
160|        //                     "",
161|        //                     TransformationDescription(),
162|        //                     pageIndex + 1,
163|        //                     TransformationDescription(),
164|        //                     false, true);
165|
166|        //            mergeInfo.push_back(pageDescription);
167|        //        }
168|        //    }
169|
170|        //    merger.merge(QFile::encodeName(overlayName).constData(), mergeInfo);
171|
172|        //    merger.saveMergedDocumentsAs(QFile::encodeName(filename).constData());
173|
174|        //}
175|        //catch(Exception e)
176|        //{
177|        //    qDebug() << "PdfMerger failed to merge documents to " << filename << " - Exception : " << e.what();
178|
179|        //    // default to raster export
180|        //    mSimpleExporter->persistsDocument(pDocumentProxy, filename);
181|        //}
182|
183|        if (!WBApplication::app()->isVerbose())
184|        {
185|            QFile::remove(overlayName);
186|        }
187|    }
188|
189|    return true;
190|}
191|
192|bool WBExportFullPDF::associatedActionactionAvailableFor(const QModelIndex &selectedIndex)
193|{
194|    const WBDocumentTreeModel *docModel = qobject_cast<const WBDocumentTreeModel*>(selectedIndex.model());
195|    if (!selectedIndex.isValid() || docModel->isCatalog(selectedIndex)) {
196|        return false;
197|    }
198|
199|    return true;
200|}
201|
202|
203|QString WBExportFullPDF::exportExtention()
204|{
205|    return QString(".pdf");
206|}
207|
208|QString WBExportFullPDF::exportName()
209|{
210|    return tr("Export to PDF");
211|}
212|