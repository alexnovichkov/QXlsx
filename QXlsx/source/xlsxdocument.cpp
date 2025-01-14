// xlsxdocument.cpp

#include <QtGlobal>
#include <QFile>
#include <QPointF>
#include <QBuffer>
#include <QDir>
#include <QTemporaryFile>
#include <QFile>
#include <QSharedPointer>
#include <QDebug>

#include "xlsxdocument.h"
#include "xlsxworkbook.h"
#include "xlsxworksheet.h"
#include "xlsxchartsheet.h"
#include "xlsxcontenttypes_p.h"
#include "xlsxrelationships_p.h"
#include "xlsxstyles_p.h"
#include "xlsxtheme_p.h"
#include "xlsxdocpropsapp_p.h"
#include "xlsxdocpropscore_p.h"
#include "xlsxsharedstrings_p.h"
#include "xlsxutility_p.h"
#include "xlsxworkbook_p.h"
#include "xlsxdrawing_p.h"
#include "xlsxmediafile_p.h"
#include "xlsxchart.h"
#include "xlsxzipreader_p.h"
#include "xlsxzipwriter_p.h"

/*
    From Wikipedia: The Open Packaging Conventions (OPC) is a
    container-file technology initially created by Microsoft to store
    a combination of XML and non-XML files that together form a single
    entity such as an Open XML Paper Specification (OpenXPS)
    document. http://en.wikipedia.org/wiki/Open_Packaging_Conventions.

    At its simplest an Excel XLSX file contains the following elements:

         ____ [Content_Types].xml
        |
        |____ docProps
        | |____ app.xml
        | |____ core.xml
        |
        |____ xl
        | |____ workbook.xml
        | |____ worksheets
        | | |____ sheet1.xml
        | |
        | |____ styles.xml
        | |
        | |____ theme
        | | |____ theme1.xml
        | |
        | |_____rels
        | |____ workbook.xml.rels
        |
        |_____rels
          |____ .rels

    The Packager class coordinates the classes that represent the
    elements of the package and writes them into the XLSX file.
*/

namespace QXlsx {

class DocumentPrivate
{
    Q_DECLARE_PUBLIC(Document)
public:
    DocumentPrivate(Document *p);
    void init();

    bool loadPackage(QIODevice *device);
    bool savePackage(QIODevice *device) const;

    // copy style from one xlsx file to other
    //    static bool copyStyle(const QString &from, const QString &to);

    Document *q_ptr;
    const QString defaultPackageName; //default name when package name not specified
    QString packageName; //name of the .xlsx file

    QMap<QString, QString> documentProperties; //core, app and custom properties

    QMap<Document::Metadata, QVariant> metadata; //core and app properties

    QSharedPointer<Workbook> workbook;
    std::shared_ptr<ContentTypes> contentTypes;
    bool isLoad;
};

namespace xlsxDocumentCpp {
    std::string copyTag(const std::string &sFrom, const std::string &sTo, const std::string &tag) {
        const std::string tagToFindStart = "<" + tag;
        const std::string tagToFindEnd = "</" + tag;
        const std::string tagEnd = "</" + tag + ">";

        // search all occurrences of tag in 'sFrom'
        std::string sFromData = "";
        size_t startIndex = 0;
        while (true) {
            std::size_t startPos = sFrom.find(tagToFindStart, startIndex);
            if (startPos != std::string::npos) {
                std::size_t endPos = sFrom.find(tagToFindEnd, startPos);
                std::string tagEndTmp = tagEnd;
                if (endPos == std::string::npos) {    // second try to find the ending, maybe it is "/>" 
                    endPos = sFrom.find("/>", startPos);
                    tagEndTmp = "/>";
                }
                if (endPos != std::string::npos) {
                    sFromData += sFrom.substr(startPos, endPos - startPos) + tagEndTmp;
                    startIndex = endPos + strlen(tagEndTmp.c_str());
                }
                else {
                    break;
                }
            }
            else {
                break;
            }
        }

        std::string sOut = sTo; // copy 'sTo' in the output string

        if (!sFromData.empty()) { // tag found in 'from'?
                                  // search all occurrences of tag in 'sOut' and delete them
            int firstPosTag = -1;
            while (true) {
                std::size_t startPos = sOut.find(tagToFindStart);
                if (startPos != std::string::npos) {
                    std::size_t endPos = sOut.find(tagToFindEnd);
                    std::string tagEndTmp = tagEnd;
                    if (endPos == std::string::npos) {    // second try to find the ending, maybe it is "/>" 
                        endPos = sOut.find("/>", startPos);
                        tagEndTmp = "/>";
                    }
                    if (endPos != std::string::npos) {
                        if (firstPosTag < 0)
                            firstPosTag = startPos;
                        std::string stringBefore = sOut.substr(0, startPos);
                        endPos += strlen(tagEndTmp.c_str());
                        std::string stringAfter = sOut.substr(endPos, strlen(sOut.c_str()) - endPos);
                        sOut = stringBefore + stringAfter;
                    }
                    else {
                        break;
                    }
                }
                else {
                    break;
                }
            }

            if (firstPosTag == -1) {
                // tag not found in 'sTo' file
                // try to find a default pos using standard tags
                std::vector<std::string> defaultPos{ "</styleSheet>", "<pageMargins", "</workbook>" };
                for (unsigned int i = 0; i < defaultPos.size(); ++i) {
                    std::size_t iDefaultPos = sOut.find(defaultPos[i]);
                    if (iDefaultPos != std::string::npos) {
                        firstPosTag = iDefaultPos;
                        break;
                    }
                }
            }

            // add the tag extracted from 'sFrom' in 'sOut'
            // add in the position of the first tag found in 'sOut' ('firstPosTag')
            if (firstPosTag >= 0) {
                std::string stringBefore = sOut.substr(0, firstPosTag);
                std::string stringAfter = sOut.substr(firstPosTag, strlen(sOut.c_str()) - firstPosTag);
                sOut = stringBefore + sFromData + stringAfter;
            }
        }

        return sOut;
    }
}

DocumentPrivate::DocumentPrivate(Document *p) :
    q_ptr(p), defaultPackageName(QStringLiteral("Book1.xlsx")),
    isLoad(false)
{
}

void DocumentPrivate::init()
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    qRegisterMetaTypeStreamOperators<QXlsx::Color>("XlsxColor");
#endif
    if (!contentTypes)
        contentTypes = std::make_shared<ContentTypes>(ContentTypes::F_NewFromScratch);

    if (workbook.isNull())
        workbook = QSharedPointer<Workbook>(new Workbook(Workbook::F_NewFromScratch));
}

bool DocumentPrivate::loadPackage(QIODevice *device)
{
    Q_Q(Document);
    ZipReader zipReader(device);
    QStringList filePaths = zipReader.filePaths();

    //Load the Content_Types file
    if (!filePaths.contains(QLatin1String("[Content_Types].xml")))
        return false;
    contentTypes = std::make_shared<ContentTypes>(ContentTypes::F_LoadFromExists);
    contentTypes->loadFromXmlData(zipReader.fileData(QStringLiteral("[Content_Types].xml")));

    //Load root rels file
    if (!filePaths.contains(QLatin1String("_rels/.rels")))
        return false;
    Relationships rootRels;
    rootRels.loadFromXmlData(zipReader.fileData(QStringLiteral("_rels/.rels")));

    //load core properties
    QList<XlsxRelationship> rels_core = rootRels.packageRelationships(QStringLiteral("/metadata/core-properties"));
    if (!rels_core.isEmpty()) {
        //Get the core property file name if it exists.
        //In normal case, this should be "docProps/core.xml"
        QString docPropsCore_Name = rels_core[0].target;

        DocPropsCore props(DocPropsCore::F_LoadFromExists);
        props.loadFromXmlData(zipReader.fileData(docPropsCore_Name));
        metadata.insert(props.properties());
    }

    //load extended properties
    QList<XlsxRelationship> rels_app = rootRels.documentRelationships(QStringLiteral("/extended-properties"));
    if (!rels_app.isEmpty()) {
        //Get the app property file name if it exists.
        //In normal case, this should be "docProps/app.xml"
        QString docPropsApp_Name = rels_app[0].target;

        DocPropsApp docPropsApp(DocPropsApp::F_LoadFromExists);
        docPropsApp.loadFromXmlData(zipReader.fileData(docPropsApp_Name));
        metadata.insert(docPropsApp.properties());
    }

    //load workbook now, Get the workbook file path from the root rels file
    //In normal case, this should be "xl/workbook.xml"
    workbook = QSharedPointer<Workbook>(new Workbook(Workbook::F_LoadFromExists));
    QList<XlsxRelationship> rels_xl = rootRels.documentRelationships(QStringLiteral("/officeDocument"));
    if (rels_xl.isEmpty())
        return false;
    const QString xlworkbook_Path = rels_xl[0].target;
    const auto parts = splitPath(xlworkbook_Path);
    const QString xlworkbook_Dir = parts.first();
    const QString relFilePath = getRelFilePath(xlworkbook_Path);

    workbook->relationships()->loadFromXmlData( zipReader.fileData(relFilePath) );
    workbook->setFilePath(xlworkbook_Path);
    workbook->loadFromXmlData(zipReader.fileData(xlworkbook_Path));

    //load styles
    QList<XlsxRelationship> rels_styles = workbook->relationships()->documentRelationships(QStringLiteral("/styles"));
    if (!rels_styles.isEmpty()) {
        //In normal case this should be styles.xml which in xl
        QString name = rels_styles[0].target;

        // dev34
        QString path;
        if ( xlworkbook_Dir == QLatin1String(".") ) // root
        {
            path = name;
        }
        else
        {
            path = xlworkbook_Dir + QLatin1String("/") + name;
        }

        QSharedPointer<Styles> styles (new Styles(Styles::F_LoadFromExists));
        styles->loadFromXmlData(zipReader.fileData(path));
        workbook->d_func()->styles = styles;
    }

    //load sharedStrings
    QList<XlsxRelationship> rels_sharedStrings = workbook->relationships()->documentRelationships(QStringLiteral("/sharedStrings"));
    if (!rels_sharedStrings.isEmpty()) {
        //In normal case this should be sharedStrings.xml which in xl
        QString name = rels_sharedStrings[0].target;
        QString path = xlworkbook_Dir + QLatin1String("/") + name;
        workbook->d_func()->sharedStrings->loadFromXmlData(zipReader.fileData(path));
    }

    //load theme
    QList<XlsxRelationship> rels_theme = workbook->relationships()->documentRelationships(QStringLiteral("/theme"));
    if (!rels_theme.isEmpty()) {
        //In normal case this should be theme/theme1.xml which in xl
        QString name = rels_theme[0].target;
        QString path = xlworkbook_Dir + QLatin1String("/") + name;
        workbook->theme()->loadFromXmlData(zipReader.fileData(path));
    }

    //load sheets
    for (int i=0; i<workbook->sheetsCount(); ++i) {
        AbstractSheet *sheet = workbook->sheet(i);
        QString strFilePath = sheet->filePath();
        QString rel_path = getRelFilePath(strFilePath);
        //If the .rel file exists, load it.
        if (zipReader.filePaths().contains(rel_path))
            sheet->relationships()->loadFromXmlData(zipReader.fileData(rel_path));
        sheet->loadFromXmlData(zipReader.fileData(sheet->filePath()));
    }

    //load external links
    for (int i=0; i<workbook->d_func()->externalLinks.count(); ++i) {
        SimpleOOXmlFile *link = workbook->d_func()->externalLinks[i].data();
        QString rel_path = getRelFilePath(link->filePath());
        //If the .rel file exists, load it.
        if (zipReader.filePaths().contains(rel_path))
            link->relationships()->loadFromXmlData(zipReader.fileData(rel_path));
        link->loadFromXmlData(zipReader.fileData(link->filePath()));
    }

    //load drawings
    for (int i=0; i<workbook->drawings().size(); ++i) {
        Drawing *drawing = workbook->drawings()[i];
        QString rel_path = getRelFilePath(drawing->filePath());
        if (zipReader.filePaths().contains(rel_path))
            drawing->relationships()->loadFromXmlData(zipReader.fileData(rel_path));
        drawing->loadFromXmlData(zipReader.fileData(drawing->filePath()));
    }

    //load charts
    auto chartFileToLoad = workbook->chartFiles();
    for (int i=0; i<chartFileToLoad.size(); ++i) {
        QSharedPointer<Chart> cf = chartFileToLoad[i].lock();
        QString rel_path = getRelFilePath(cf->filePath());
        if (zipReader.filePaths().contains(rel_path))
            cf->relationships()->loadFromXmlData(zipReader.fileData(rel_path));
        cf->loadFromXmlData(zipReader.fileData(cf->filePath()));

        //relations
        cf->loadMediaFiles(workbook.get());
    }

    //load media files
    const auto mediaFileToLoad = workbook->mediaFiles();
    for (const auto &mf : mediaFileToLoad) {
        if (auto media = mf.lock()) {
            const QString path = media->fileName();
            const QString suffix = path.mid(path.lastIndexOf(QLatin1Char('.'))+1);
            media->set(zipReader.fileData(path), suffix);
        }
    }

    isLoad = true; 
    return true;
}

bool DocumentPrivate::savePackage(QIODevice *device) const
{
    Q_Q(const Document);

    ZipWriter zipWriter(device);
    if (zipWriter.error())
        return false;

    contentTypes->clearOverrides();

    DocPropsApp docPropsApp(DocPropsApp::F_NewFromScratch);
    DocPropsCore docPropsCore(DocPropsCore::F_NewFromScratch);

    // save worksheet xml files
    QList<QSharedPointer<AbstractSheet> > worksheets = workbook->getSheetsByType(AbstractSheet::Type::Worksheet);
    if (!worksheets.isEmpty())
        docPropsApp.addHeadingPair(QStringLiteral("Worksheets"), worksheets.size());

    for (int i = 0 ; i < worksheets.size(); ++i)
    {
        QSharedPointer<AbstractSheet> sheet = worksheets[i];
        contentTypes->addWorksheetName(QStringLiteral("sheet%1").arg(i+1));
        docPropsApp.addPartTitle(sheet->name());

        zipWriter.addFile(QStringLiteral("xl/worksheets/sheet%1.xml").arg(i+1), sheet->saveToXmlData());

        Relationships *rel = sheet->relationships();
        if (!rel->isEmpty())
            zipWriter.addFile(QStringLiteral("xl/worksheets/_rels/sheet%1.xml.rels").arg(i+1), rel->saveToXmlData());
    }

    //save chartsheet xml files
    QList<QSharedPointer<AbstractSheet> > chartsheets = workbook->getSheetsByType(AbstractSheet::Type::Chartsheet);
    if (!chartsheets.isEmpty())
        docPropsApp.addHeadingPair(QStringLiteral("Chartsheets"), chartsheets.size());
    for (int i=0; i<chartsheets.size(); ++i)
    {
        QSharedPointer<AbstractSheet> sheet = chartsheets[i];
        contentTypes->addChartsheetName(QStringLiteral("sheet%1").arg(i+1));
        docPropsApp.addPartTitle(sheet->name());

        zipWriter.addFile(QStringLiteral("xl/chartsheets/sheet%1.xml").arg(i+1), sheet->saveToXmlData());
        Relationships *rel = sheet->relationships();
        if (!rel->isEmpty())
            zipWriter.addFile(QStringLiteral("xl/chartsheets/_rels/sheet%1.xml.rels").arg(i+1), rel->saveToXmlData());
    }

    // save external links xml files
    for (int i=0; i<workbook->d_func()->externalLinks.count(); ++i)
    {
        SimpleOOXmlFile *link = workbook->d_func()->externalLinks[i].data();
        contentTypes->addExternalLinkName(QStringLiteral("externalLink%1").arg(i+1));

        zipWriter.addFile(QStringLiteral("xl/externalLinks/externalLink%1.xml").arg(i+1), link->saveToXmlData());
        Relationships *rel = link->relationships();
        if (!rel->isEmpty())
            zipWriter.addFile(QStringLiteral("xl/externalLinks/_rels/externalLink%1.xml.rels").arg(i+1), rel->saveToXmlData());
    }

    // save workbook xml file
    contentTypes->addWorkbook();
    zipWriter.addFile(QStringLiteral("xl/workbook.xml"), workbook->saveToXmlData());
    zipWriter.addFile(QStringLiteral("xl/_rels/workbook.xml.rels"), workbook->relationships()->saveToXmlData());

    // save drawing xml files
    for (int i=0; i<workbook->drawings().size(); ++i)
    {
        contentTypes->addDrawingName(QStringLiteral("drawing%1").arg(i+1));

        Drawing *drawing = workbook->drawings()[i];
        zipWriter.addFile(QStringLiteral("xl/drawings/drawing%1.xml").arg(i+1), drawing->saveToXmlData());
        if (!drawing->relationships()->isEmpty())
            zipWriter.addFile(QStringLiteral("xl/drawings/_rels/drawing%1.xml.rels").arg(i+1), drawing->relationships()->saveToXmlData());
    }

    // save docProps app/core xml file
    const auto docProps = q->allMetadata();
    for (auto name : docProps.keys()) {
        docPropsApp.setProperty(name, q->metadata(name));
        docPropsCore.setProperty(name, q->metadata(name));
    }
    contentTypes->addDocPropApp();
    contentTypes->addDocPropCore();
    zipWriter.addFile(QStringLiteral("docProps/app.xml"), docPropsApp.saveToXmlData());
    zipWriter.addFile(QStringLiteral("docProps/core.xml"), docPropsCore.saveToXmlData());

    // save sharedStrings xml file
    if (!workbook->sharedStrings()->isEmpty()) {
        contentTypes->addSharedString();
        zipWriter.addFile(QStringLiteral("xl/sharedStrings.xml"), workbook->sharedStrings()->saveToXmlData());
    }

    // save calc chain [dev16]
    contentTypes->addCalcChain();
    zipWriter.addFile(QStringLiteral("xl/calcChain.xml"), workbook->styles()->saveToXmlData());

    // save styles xml file
    contentTypes->addStyles();
    zipWriter.addFile(QStringLiteral("xl/styles.xml"), workbook->styles()->saveToXmlData());

    // save theme xml file
    contentTypes->addTheme();
    zipWriter.addFile(QStringLiteral("xl/theme/theme1.xml"), workbook->theme()->saveToXmlData());

    // save chart xml files
    auto chartFiles = workbook->chartFiles();
    for (int i=0; i<chartFiles.size(); ++i)
    {
        contentTypes->addChartName(QStringLiteral("chart%1").arg(i+1));
        QSharedPointer<Chart> cf = chartFiles[i];
        cf->saveMediaFiles(workbook.get());
        zipWriter.addFile(QStringLiteral("xl/charts/chart%1.xml").arg(i+1), cf->saveToXmlData());

        if (auto rel = cf->relationships(); rel && !rel->isEmpty())
            zipWriter.addFile(QStringLiteral("xl/charts/_rels/chart%1.xml.rels").arg(i+1), rel->saveToXmlData());
    }

    // save media files
    const auto mfs = workbook->mediaFiles();
    for (int i=0; i < mfs.size(); ++i) {
        if (auto mf = mfs[i].lock()) {
            if (!mf->mimeType().isEmpty())
                contentTypes->addDefault(mf->suffix(), mf->mimeType());

            zipWriter.addFile(QStringLiteral("xl/media/image%1.%2").arg(i+1).arg(mf->suffix()), mf->contents());
        }
    }

    // save root .rels xml file
    Relationships rootrels;
    rootrels.addDocumentRelationship(QStringLiteral("/officeDocument"), QStringLiteral("xl/workbook.xml"));
    rootrels.addPackageRelationship(QStringLiteral("/metadata/core-properties"), QStringLiteral("docProps/core.xml"));
    rootrels.addDocumentRelationship(QStringLiteral("/extended-properties"), QStringLiteral("docProps/app.xml"));
    zipWriter.addFile(QStringLiteral("_rels/.rels"), rootrels.saveToXmlData());

    // save content types xml file
    zipWriter.addFile(QStringLiteral("[Content_Types].xml"), contentTypes->saveToXmlData());
    zipWriter.close();

    return true;
}

//bool DocumentPrivate::copyStyle(const QString &from, const QString &to)
//{
//    // create a temp file because the zip writer cannot modify already existing zips
//    QTemporaryFile tempFile;
//    tempFile.open();
//    tempFile.close();
//    QString temFilePath = QFileInfo(tempFile).absoluteFilePath();

//    ZipWriter temporalZip(temFilePath);

//    ZipReader zipReader(from);
//    QStringList filePaths = zipReader.filePaths();

//    QSharedPointer<ZipReader> toReader = QSharedPointer<ZipReader>(new ZipReader(to));

//    QStringList toFilePaths = toReader->filePaths();

//    // copy all files from "to" zip except those related to style
//    for (int i = 0; i < toFilePaths.size(); i++) {
//        if (toFilePaths[i].contains(QLatin1String("xl/styles"))) {
//            if (filePaths.contains(toFilePaths[i])) {    // style file exist in 'from' as well
//                // modify style file
//                std::string fromData = QString::fromUtf8(zipReader.fileData(toFilePaths[i])).toStdString();
//                std::string toData = QString::fromUtf8(toReader->fileData(toFilePaths[i])).toStdString();
//                // copy default theme style from 'from' to 'to'
//                toData = xlsxDocumentCpp::copyTag(fromData, toData, "dxfs");
//                temporalZip.addFile(toFilePaths.at(i), QString::fromUtf8(toData.c_str()).toUtf8());

//                continue;
//            }
//        }

//        if (toFilePaths[i].contains(QLatin1String("xl/workbook"))) {
//            if (filePaths.contains(toFilePaths[i])) {    // workbook file exist in 'from' as well
//                // modify workbook file
//                std::string fromData = QString::fromUtf8(zipReader.fileData(toFilePaths[i])).toStdString();
//                std::string toData = QString::fromUtf8(toReader->fileData(toFilePaths[i])).toStdString();
//                // copy default theme style from 'from' to 'to'
//                toData = xlsxDocumentCpp::copyTag(fromData, toData, "workbookPr");
//                temporalZip.addFile(toFilePaths.at(i), QString::fromUtf8(toData.c_str()).toUtf8());
//                continue;
//            }
//        }

//        if (toFilePaths[i].contains(QLatin1String("xl/worksheets/sheet"))) {
//            if (filePaths.contains(toFilePaths[i])) {    // sheet file exist in 'from' as well
//                // modify sheet file
//                std::string fromData = QString::fromUtf8(zipReader.fileData(toFilePaths[i])).toStdString();
//                std::string toData = QString::fromUtf8(toReader->fileData(toFilePaths[i])).toStdString();
//                // copy "conditionalFormatting" from 'from' to 'to'
//                toData = xlsxDocumentCpp::copyTag(fromData, toData, "conditionalFormatting");
//                temporalZip.addFile(toFilePaths.at(i), QString::fromUtf8(toData.c_str()).toUtf8());
//                continue;
//            }
//        }

//        QByteArray data = toReader->fileData(toFilePaths.at(i));
//        temporalZip.addFile(toFilePaths.at(i), data);
//    }

//    temporalZip.close();

//    toReader.clear();

//    tempFile.close();

//    QFile::remove(to);
//    tempFile.copy(to);

//    return true;
//}

Document::Document(QObject *parent) :
    QObject(parent), d_ptr(new DocumentPrivate(this))
{
    d_ptr->init();
}

Document::Document(const QString &name, bool loadImmediately, QObject *parent) :
    QObject(parent), d_ptr(new DocumentPrivate(this))
{
    d_ptr->packageName = name; 

    if (loadImmediately) {
        load();
    }

    d_ptr->init();
}

Document::Document(QIODevice *device, QObject *parent) :
    QObject(parent), d_ptr(new DocumentPrivate(this))
{
    if (device && device->isReadable()) {
        if (!d_ptr->loadPackage(device)) {
            // NOTICE: failed to load package 
        }
    }
    d_ptr->init();
}

bool Document::write(const CellReference &row_column, const QVariant &value, const Format &format)
{
    if (Worksheet *sheet = activeWorksheet())
        return sheet->write(row_column, value, format);
    return false;
}

bool Document::write(int row, int col, const QVariant &value, const Format &format)
{
    if (Worksheet *sheet = activeWorksheet())
        return sheet->write(row, col, value, format);
    return false;
}

QVariant Document::read(const CellReference &cell) const
{
    if (Worksheet *sheet = activeWorksheet())
        return sheet->read(cell);
    return QVariant();
}

QVariant Document::read(int row, int col) const
{
    if (Worksheet *sheet = activeWorksheet())
        return sheet->read(row, col);
    return QVariant();
}

QVariant Document::metadata(Metadata property) const
{
    Q_D(const Document);
    return d->metadata.value(property);
}

void Document::setMetadata(Metadata property, const QVariant &value)
{
    Q_D(Document);
    d->metadata[property] = value;
}

bool Document::hasMetadata(Metadata property) const
{
    Q_D(const Document);
    return d->metadata.contains(property);
}

QMap<Document::Metadata, QVariant> Document::allMetadata() const
{
    Q_D(const Document);
    return d->metadata;
}

Workbook *Document::workbook() const
{
    Q_D(const Document);
    return d->workbook.data();
}

AbstractSheet *Document::sheet(const QString &sheetName) const
{
    Q_D(const Document);
    return d->workbook->sheet(sheetName);
}

bool Document::addSheet(const QString &name, AbstractSheet::Type type)
{
    Q_D(Document);
    return d->workbook->addSheet(name, type);
}

Chartsheet *Document::addChartsheet(const QString &name)
{
    Q_D(Document);
    return dynamic_cast<Chartsheet*>(d->workbook->addSheet(name, AbstractSheet::Type::Chartsheet));
}

Worksheet *Document::addWorksheet(const QString &name)
{
    Q_D(Document);
    return dynamic_cast<Worksheet*>(d->workbook->addSheet(name));
}

bool Document::insertSheet(int index, const QString &name, AbstractSheet::Type type)
{
    Q_D(Document);
    return d->workbook->insertSheet(index, name, type);
}

int Document::sheetsCount() const
{
    Q_D(const Document);
    return d->workbook->sheetsCount();
}

bool Document::renameSheet(const QString &oldName, const QString &newName)
{
    Q_D(Document);
    if (oldName == newName)
        return false;
    return d->workbook->renameSheet(sheetNames().indexOf(oldName), newName);
}

bool Document::copySheet(const QString &srcName, const QString &dstName)
{
    Q_D(Document);
    if (srcName == dstName)
        return false;
    return d->workbook->copySheet(sheetNames().indexOf(srcName), dstName);
}

bool Document::moveSheet(const QString &sheetName, int dstIndex)
{
    Q_D(Document);
    return d->workbook->moveSheet(sheetName, dstIndex);
}

bool Document::moveSheet(int srcIndex, int dstIndex)
{
    Q_D(Document);
    return d->workbook->moveSheet(srcIndex, dstIndex);
}

bool Document::deleteSheet(const QString &name)
{
    Q_D(Document);
    return d->workbook->deleteSheet(sheetNames().indexOf(name));
}

bool Document::deleteSheet(int index)
{
    Q_D(Document);
    return d->workbook->deleteSheet(index);
}

AbstractSheet *Document::activeSheet() const
{
    Q_D(const Document);

    return d->workbook->activeSheet();
}

Worksheet *Document::activeWorksheet() const
{
    Q_D(const Document);
    return d->workbook->activeWorksheet();
}

Chartsheet *Document::activeChartsheet() const
{
    Q_D(const Document);
    return d->workbook->activeChartsheet();
}

bool Document::setActiveSheet(const QString &name)
{
    Q_D(Document);
    return d->workbook->setActiveSheet(sheetNames().indexOf(name));
}

bool Document::setActiveSheet(int index)
{
    Q_D(Document);
    return d->workbook->setActiveSheet(index);
}

QStringList Document::sheetNames() const
{
    Q_D(const Document);
    return d->workbook->sheetNames();
}

bool Document::save() const
{
    Q_D(const Document);
    QString name = d->packageName.isEmpty() ? d->defaultPackageName : d->packageName;

    return saveAs(name);
}

bool Document::saveAs(const QString &name) const
{
    QFile file(name);
    if (file.open(QIODevice::WriteOnly))
        return saveAs(&file);
    return false;
}

bool Document::saveAs(QIODevice *device) const
{
    Q_D(const Document);
    return d->savePackage(device);
}

bool Document::isLoaded() const
{
    Q_D(const Document);
    return d->isLoad; 
}

bool Document::load()
{
    if (QFile::exists(d_ptr->packageName)) {
        QFile xlsx(d_ptr->packageName);
        if (xlsx.open(QFile::ReadOnly)) {
            return d_ptr->loadPackage(&xlsx);
        }
    }
    return false;
}

//bool Document::copyStyle(const QString &from, const QString &to) {
//    return DocumentPrivate::copyStyle(from, to);
//}

Document::~Document()
{
    delete d_ptr;
}

}
