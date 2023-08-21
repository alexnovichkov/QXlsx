// xlsxdocpropscore_p.h

#ifndef XLSXDOCPROPSCORE_H
#define XLSXDOCPROPSCORE_H

#include "xlsxglobal.h"
#include "xlsxabstractooxmlfile.h"

#include <QMap>
#include <QStringList>

class QIODevice;

namespace QXlsx {

class DocPropsCore : public AbstractOOXmlFile
{
public:
    explicit DocPropsCore(CreateFlag flag);

    bool setProperty(const QString &name, const QString &value);
    QString property(const QString &name) const;
    QStringList propertyNames() const;
        
    void saveToXmlFile(QIODevice *device) const override;
    bool loadFromXmlFile(QIODevice *device) override;

private:
    QMap<QString, QString> m_properties;
};

}

#endif // XLSXDOCPROPSCORE_H
