// xlsxabstractooxmlfile.h

#ifndef QXLSX_XLSXABSTRACTOOXMLFILE_H
#define QXLSX_XLSXABSTRACTOOXMLFILE_H

#include "xlsxglobal.h"

namespace QXlsx {

class Relationships;
class AbstractOOXmlFilePrivate;

class QXLSX_EXPORT AbstractOOXmlFile
{
    Q_DECLARE_PRIVATE(AbstractOOXmlFile)

public:
    enum CreateFlag
    {
        F_NewFromScratch,
        F_LoadFromExists
    };

public:
    virtual ~AbstractOOXmlFile();

    virtual void saveToXmlFile(QIODevice *device) const = 0;
    virtual bool loadFromXmlFile(QIODevice *device) = 0;

    virtual QByteArray saveToXmlData() const;
    virtual bool loadFromXmlData(const QByteArray &data);

    Relationships *relationships() const;

    void setFilePath(const QString path);
    QString filePath() const;

protected:
    AbstractOOXmlFile(CreateFlag flag);
    AbstractOOXmlFile(AbstractOOXmlFilePrivate *d);

    AbstractOOXmlFilePrivate *d_ptr;
};

}

#endif // QXLSX_XLSXABSTRACTOOXMLFILE_H
