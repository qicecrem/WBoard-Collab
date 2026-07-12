1|#include <QFile>
2|#include <QScreen>
3|
4|#include "PDFRenderer.h"
5|
6|#include "XPDFRenderer.h"
7|
8|#include "core/WBApplication.h"
9|#include "core/memcheck.h"
10|
11|
12|QMap< QUuid, QPointer<PDFRenderer> > PDFRenderer::sRenderers;
13|
14|PDFRenderer::PDFRenderer() : dpiForRendering(96)
15|{
16|}
17|
18|PDFRenderer::~PDFRenderer()
19|{
20|    // NOOP
21|}
22|
23|PDFRenderer* PDFRenderer::rendererForUuid(const QUuid &uuid, const QString &filename, bool importingFile)
24|{
25|    if (sRenderers.contains(uuid))
26|    {
27|        return sRenderers.value(uuid);
28|    }
29|    else
30|    {
31|		PDFRenderer *newRenderer = new XPDFRenderer(filename, importingFile);
32|
33|        newRenderer->setRefCount(0);
34|        newRenderer->setFileUuid(uuid);
35|
36|        QFile file(filename);
37|        file.open(QIODevice::ReadOnly);
38|        newRenderer->setFileData(file.readAll());
39|        file.close();
40|
41|        sRenderers.insert(newRenderer->fileUuid(), newRenderer);
42|
43|        QScreen* desktop = QApplication::primaryScreen();
44|        int dpiCommon = (desktop->physicalDpiX() + desktop->physicalDpiY()) / 2;
45|        newRenderer->setDPI(dpiCommon);
46|
47|        return newRenderer;
48|    }
49|}
50|
51|void PDFRenderer::setRefCount(const QAtomicInt &refCount)
52|{
53|    mRefCount = refCount;
54|}
55|
56|void PDFRenderer::setFileData(const QByteArray &fileData)
57|{
58|    mFileData = fileData;
59|}
60|
61|void PDFRenderer::setFileUuid(const QUuid &fileUuid)
62|{
63|    mFileUuid = fileUuid;
64|}
65|
66|void PDFRenderer::attach()
67|{
68|    mRefCount.ref();
69|}
70|
71|void PDFRenderer::detach()
72|{
73|    mRefCount.deref();
74|    if (mRefCount.loadAcquire() == 0)
75|    {
76|        sRenderers.remove(mFileUuid);
77|        delete this;
78|    }
79|}
80|