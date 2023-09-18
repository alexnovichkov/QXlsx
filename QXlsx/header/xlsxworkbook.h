// xlsxworkbook.h

#ifndef XLSXWORKBOOK_H
#define XLSXWORKBOOK_H

#include <QtGlobal>
#include <QList>
#include <QImage>
#include <QSharedPointer>
#include <QIODevice>

#include <memory>

#include "xlsxglobal.h"
#include "xlsxabstractooxmlfile.h"
#include "xlsxabstractsheet.h"

namespace QXlsx {

class SharedStrings;
class Styles;
class Drawing;
class Document;
class Theme;
class Relationships;
class DocumentPrivate;
class MediaFile;
class Chart;
class Chartsheet;
class Worksheet;
class WorkbookPrivate;

class QXLSX_EXPORT Workbook : public AbstractOOXmlFile
{
    Q_DECLARE_PRIVATE(Workbook)
public:
    ~Workbook();

    int sheetCount() const;
    AbstractSheet *sheet(int index) const;

    AbstractSheet *addSheet(const QString &name = QString(), AbstractSheet::Type type = AbstractSheet::Type::Worksheet);
    AbstractSheet *insertSheet(int index, const QString &name = QString(), AbstractSheet::Type type = AbstractSheet::Type::Worksheet);
    bool renameSheet(int index, const QString &name);
    bool deleteSheet(int index);
    bool copySheet(int index, const QString &newName=QString());
    bool moveSheet(int srcIndex, int distIndex);

    AbstractSheet *activeSheet() const;
    bool setActiveSheet(int index);

//    void addChart();
    bool defineName(const QString &name, const QString &formula, const QString &comment=QString(), const QString &scope=QString());
    bool isDate1904() const;
    /*!
    Excel for Windows uses a default epoch of 1900 and Excel
    for Mac uses an epoch of 1904. However, Excel on either
    platform will convert automatically between one system
    and the other. Qt Xlsx stores dates in the 1900 format
    by default.

    \note This function should be called before any date/time
    has been written.
    */
    void setDate1904(bool date1904);
    bool isStringsToNumbersEnabled() const;
    /*
    Enable the worksheet.write() method to convert strings
    to numbers, where possible, using float() in order to avoid
    an Excel warning about "Numbers Stored as Text".

    The default is false
    */
    void setStringsToNumbersEnabled(bool enable=true);
    bool isStringsToHyperlinksEnabled() const;
    void setStringsToHyperlinksEnabled(bool enable=true);
    bool isHtmlToRichStringEnabled() const;
    void setHtmlToRichStringEnabled(bool enable=true);
    QString defaultDateFormat() const;
    void setDefaultDateFormat(const QString &format);

    //internal used member
    bool addMediaFile(QSharedPointer<MediaFile> media, bool force=false);
    void removeMediaFile(QSharedPointer<MediaFile> media);
    QList<QWeakPointer<MediaFile> > mediaFiles() const;
    void addChartFile(const QSharedPointer<Chart> &chartFile);
    void removeChartFile(const QSharedPointer<Chart> &chart);
    QList<QWeakPointer<Chart> > chartFiles() const;

private:
    friend class Worksheet;
    friend class Chartsheet;
    friend class WorksheetPrivate;
    friend class Document;
    friend class DocumentPrivate;

    Workbook(Workbook::CreateFlag flag);

    void saveToXmlFile(QIODevice *device) const override;
    bool loadFromXmlFile(QIODevice *device) override;

    SharedStrings *sharedStrings() const;
    Styles *styles();
    Theme *theme();
    QList<QImage> images();
    QList<Drawing *> drawings();
    QList<QSharedPointer<AbstractSheet> > getSheetsByTypes(AbstractSheet::Type type) const;
    QStringList worksheetNames() const;
    AbstractSheet *addSheet(const QString &name, int sheetId, AbstractSheet::Type type);
};

}

#endif // XLSXWORKBOOK_H
