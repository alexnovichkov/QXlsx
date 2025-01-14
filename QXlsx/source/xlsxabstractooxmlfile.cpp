// xlsxabstractooxmlfile.cpp

#include <QtGlobal>
#include <QBuffer>
#include <QByteArray>
#include <QDebug>

#include "xlsxabstractooxmlfile.h"
#include "xlsxabstractooxmlfile_p.h"

namespace QXlsx {

AbstractOOXmlFilePrivate::AbstractOOXmlFilePrivate(AbstractOOXmlFile *q, AbstractOOXmlFile::CreateFlag flag=AbstractOOXmlFile::F_NewFromScratch)
    : relationships(new Relationships), flag(flag), q_ptr(q)
{

}

AbstractOOXmlFilePrivate::~AbstractOOXmlFilePrivate()
{
    delete relationships;
}

AbstractOOXmlFile::AbstractOOXmlFile(CreateFlag flag)
    :d_ptr(new AbstractOOXmlFilePrivate(this, flag))
{
}

AbstractOOXmlFile::AbstractOOXmlFile(AbstractOOXmlFilePrivate *d)
    :d_ptr(d)
{

}

AbstractOOXmlFile::~AbstractOOXmlFile()
{
    delete d_ptr;
}

QByteArray AbstractOOXmlFile::saveToXmlData() const
{
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    saveToXmlFile(&buffer);

    return data;
}

bool AbstractOOXmlFile::loadFromXmlData(const QByteArray &data)
{
    QBuffer buffer;
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);

    return loadFromXmlFile(&buffer);
}

void AbstractOOXmlFile::setFilePath(const QString path)
{
    Q_D(AbstractOOXmlFile);
    d->filePathInPackage = path;
}

QString AbstractOOXmlFile::filePath() const
{
    Q_D(const AbstractOOXmlFile);
    return d->filePathInPackage;
}

Relationships *AbstractOOXmlFile::relationships() const
{
    Q_D(const AbstractOOXmlFile);
    return d->relationships;
}


}
