1|#include "WBMetadataDcSubsetAdaptor.h"
2|
3|#include <QtWidgets>
4|#include <QtXml>
5|#include <QScreen>
6|
7|#include "core/WBSettings.h"
8|#include "core/WBApplication.h"
9|#include "board/WBBoardController.h"
10|
11|#include "document/WBDocumentProxy.h"
12|
13|#include "core/memcheck.h"
14|
15|const QString WBMetadataDcSubsetAdaptor::nsRdf = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
16|const QString WBMetadataDcSubsetAdaptor::nsDc = "http://purl.org/dc/elements/1.1/";
17|const QString WBMetadataDcSubsetAdaptor::metadataFilename = "metadata.rdf";
18|
19|
20|WBMetadataDcSubsetAdaptor::WBMetadataDcSubsetAdaptor()
21|{
22|
23|}
24|
25|
26|WBMetadataDcSubsetAdaptor::~WBMetadataDcSubsetAdaptor()
27|{
28|    // NOOP
29|}
30|
31|
32|void WBMetadataDcSubsetAdaptor::persist(WBDocumentProxy* proxy)
33|{
34|    if(!QDir(proxy->persistencePath()).exists()){
35|        //In this case the a document is an empty document so we do not persist it
36|        return;
37|    }
38|    QString fileName = proxy->persistencePath() + "/" + metadataFilename;
39|    qWarning() << "Persisting document; path is" << fileName;
40|    QFile file(fileName);
41|    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
42|    {
43|        qCritical() << "cannot open " << fileName << " for writing ...";
44|        qCritical() << "error : "  << file.errorString();
45|        return;
46|    }
47|
48|    QXmlStreamWriter xmlWriter(&file);
49|    xmlWriter.setAutoFormatting(true);
50|
51|    xmlWriter.writeStartDocument();
52|    xmlWriter.writeDefaultNamespace(nsRdf);
53|    xmlWriter.writeNamespace(nsDc, "dc");
54|    xmlWriter.writeNamespace(WBSettings::uniboardDocumentNamespaceUri, "ub");
55|
56|    xmlWriter.writeStartElement("RDF");
57|
58|    xmlWriter.writeStartElement("Description");
59|    xmlWriter.writeAttribute("about", proxy->metaData(WBSettings::documentIdentifer).toString());
60|
61|    xmlWriter.writeTextElement(nsDc, "title", proxy->metaData(WBSettings::documentName).toString());
62|    xmlWriter.writeTextElement(nsDc, "type", proxy->metaData(WBSettings::documentGroupName).toString());
63|    xmlWriter.writeTextElement(nsDc, "date", proxy->metaData(WBSettings::documentDate).toString());
64|    xmlWriter.writeTextElement(nsDc, "format", "image/svg+xml");
65|
66|    xmlWriter.writeTextElement(nsDc, "identifier", proxy->metaData(WBSettings::documentIdentifer).toString());
67|    xmlWriter.writeTextElement(WBSettings::uniboardDocumentNamespaceUri, "version", WBSettings::currentFileVersion);
68|    QString width = QString::number(proxy->defaultDocumentSize().width());
69|    QString height = QString::number(proxy->defaultDocumentSize().height());
70|    xmlWriter.writeTextElement(WBSettings::uniboardDocumentNamespaceUri, "size", QString("%1x%2").arg(width).arg(height));
71|
72|    xmlWriter.writeTextElement(WBSettings::uniboardDocumentNamespaceUri, "updated-at", WBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTimeUtc()));
73|
74|    xmlWriter.writeTextElement(WBSettings::uniboardDocumentNamespaceUri, "page-count", QString::number(proxy->pageCount()));
75|
76|    xmlWriter.writeEndElement(); 
77|    xmlWriter.writeEndElement();
78|
79|    xmlWriter.writeEndDocument();
80|
81|    file.flush();
82|    file.close();
83|}
84|
85|
86|QMap<QString, QVariant> WBMetadataDcSubsetAdaptor::load(QString pPath)
87|{
88|
89|    QMap<QString, QVariant> metadata;
90|
91|    QString fileName = pPath + "/" + metadataFilename;
92|
93|    QFile file(fileName);
94|
95|    bool sizeFound = false;
96|    bool updatedAtFound = false;
97|    QString date;
98|
99|    if (file.exists())
100|    {
101|        if (!file.open(QIODevice::ReadOnly))
102|        {
103|            qWarning() << "Cannot open file " << fileName << " for reading ...";
104|            return metadata;
105|        }
106|
107|        QXmlStreamReader xml(&file);
108|
109|        while (!xml.atEnd())
110|        {
111|            xml.readNext();
112|
113|            if (xml.isStartElement())
114|            {
115|                QString docVersion = "4.1"; // untagged doc version 4.1
116|
117|                if (xml.name() == "title")
118|                {
119|                    metadata.insert(WBSettings::documentName, xml.readElementText());
120|                }
121|                else if (xml.name() == "type")
122|                {
123|                    metadata.insert(WBSettings::documentGroupName, xml.readElementText());
124|                }
125|                else if (xml.name() == "date")
126|                {
127|                    date = xml.readElementText();
128|                }
129|                else if (xml.name() == "identifier")
130|                {
131|                        metadata.insert(WBSettings::documentIdentifer, xml.readElementText());
132|                }
133|                else if (xml.name() == "version"
134|                        && xml.namespaceUri() == WBSettings::uniboardDocumentNamespaceUri)
135|                {
136|                        docVersion = xml.readElementText();
137|                }
138|                else if (xml.name() == "size"
139|                        && xml.namespaceUri() == WBSettings::uniboardDocumentNamespaceUri)
140|                {
141|                    QString size = xml.readElementText();
142|                    QStringList sizeParts = size.split("x");
143|                    bool ok = false;
144|                    int width, height;
145|                    if (sizeParts.count() >= 2)
146|                    {
147|                        bool widthOK, heightOK;
148|                        width = sizeParts.at(0).toInt(&widthOK);
149|                        height = sizeParts.at(1).toInt(&heightOK);
150|                        ok = widthOK && heightOK;
151|
152|                        QSize docSize(width, height);
153|
154|                        if (width == 1024 && height == 768) // move from 1024/768 to 1280/960
155|                        {
156|                            docSize = WBSettings::settings()->pageSize->get().toSize();
157|                        }
158|
159|                        metadata.insert(WBSettings::documentSize, QVariant(docSize));
160|                    }
161|                    if (!ok)
162|                    {
163|                        qWarning() << "Invalid document size:" << size;
164|                    }
165|
166|                    sizeFound = true;
167|
168|                }
169|                else if (xml.name() == "updated-at"
170|                        && xml.namespaceUri() == WBSettings::uniboardDocumentNamespaceUri)
171|                {
172|                    metadata.insert(WBSettings::documentUpdatedAt, xml.readElementText());
173|                    updatedAtFound = true;
174|                }
175|                else if (xml.name() == "page-count"
176|                        && xml.namespaceUri() == WBSettings::uniboardDocumentNamespaceUri)
177|                {
178|                    metadata.insert(WBSettings::documentPageCount, xml.readElementText());
179|                }
180|                metadata.insert(WBSettings::documentVersion, docVersion);
181|            }
182|
183|            if (xml.hasError())
184|            {
185|                qWarning() << "error parsing metadata.rdf file " << xml.errorString();
186|            }
187|        }
188|
189|        file.close();
190|    }
191|
192|    if (!sizeFound)
193|    {
194|        QScreen* dw = QApplication::primaryScreen();
195|        int controlScreenIndex = dw->primaryScreen();
196|
197|        QSize docSize = dw->geometry(controlScreenIndex).size();
198|        docSize.setHeight(docSize.height() - 70); // 70 = toolbar height
199|
200|        qWarning() << "Document size not found, using default view size" << docSize;
201|
202|        metadata.insert(WBSettings::documentSize, QVariant(docSize));
203|    }
204|
205|    // this is necessary to update the old files date
206|    QString dateString = metadata.value(WBSettings::documentDate).toString();
207|    if(dateString.length() < 10){
208|        metadata.remove(WBSettings::documentDate);
209|        metadata.insert(WBSettings::documentDate,dateString+"T00:00:00Z");
210|    }
211|
212|    if (!updatedAtFound) {
213|        metadata.insert(WBSettings::documentUpdatedAt, dateString);
214|    }
215|
216|    metadata.insert(WBSettings::documentDate, QVariant(date));
217|
218|
219|    return metadata;
220|}
221|
222|