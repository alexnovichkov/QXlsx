// xlsxdatetype.h

#ifndef QXLSX_XLSXDATETYPE_H
#define QXLSX_XLSXDATETYPE_H

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QDate>
#include <QTime>

#include "xlsxglobal.h"

namespace QXlsx {

class QXLSX_EXPORT DateType
{
public:
    DateType();
/*
    DateType(bool is1904 = false);
    DateType(double d, bool is1904 = false);
    DateType(QDateTime qdt, bool is1904 = false);
    DateType(QDate qd, bool is1904 = false);
    DateType(QTime qt, bool is1904 = false);
public:
    enum currentDateType { DateAndTimeType, OnlyDateType, OnlyTimeType };
public:
    currentDateType getType();
    bool getValue(QDateTime* pQdt);
    bool getValue(QDate* pQd);
    bool getValue(QTime* pQt);
    bool getValue(double* pD);

protected:

protected:
    bool isSet;
    double dValue;
    bool is1904Type;
    currentDateType dType;
*/
};

}
#endif 
