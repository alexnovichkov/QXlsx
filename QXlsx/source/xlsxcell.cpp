// xlsxcell.cpp

#include <cmath>

#include <QtGlobal>
#include <QDebug>
#include <QDateTime>
#include <QDate>
#include <QTime>

#include "xlsxcell.h"
#include "xlsxformat.h"
#include "xlsxformat_p.h"
#include "xlsxutility_p.h"
#include "xlsxworksheet.h"
#include "xlsxworkbook.h"
#include "xlsxsharedstrings_p.h"

namespace QXlsx {

class CellPrivate
{
    Q_DECLARE_PUBLIC(Cell)
public:
    CellPrivate(Cell *p);
    CellPrivate(const CellPrivate * const cp);
public:
    Worksheet *parent;
    Cell *q_ptr;
public:
    Cell::Type cellType;
    QVariant value;
    CellFormula formula;
    Format format;
    RichString richString;
    qint32 styleNumber;
};

CellPrivate::CellPrivate(Cell *p) :
    q_ptr(p)
{

}

CellPrivate::CellPrivate(const CellPrivate * const cp)
    : parent(cp->parent)
    , cellType(cp->cellType)
    , value(cp->value)
    , formula(cp->formula)
    , format(cp->format)
    , richString(cp->richString)
    , styleNumber(cp->styleNumber)
{

}

Cell::Cell(const QVariant &data,
           Type type,
           const Format &format,
           Worksheet *parent,
           qint32 styleIndex ,
           const RichString &richString) :
    d_ptr(new CellPrivate(this))
{
    d_ptr->value = data;
    d_ptr->cellType = type;
    d_ptr->format = format;
    d_ptr->parent = parent;
    d_ptr->styleNumber = styleIndex;
    d_ptr->richString = richString;
}

Cell::Cell(const Cell * const cell, Worksheet *parent):
    d_ptr(new CellPrivate(cell->d_ptr))
{
    d_ptr->q_ptr = this;
    d_ptr->parent = parent;
    if (d_ptr->cellType == Cell::Type::SharedString)
        d_ptr->parent->workbook()->sharedStrings()->addSharedString(d_ptr->richString);
}

Cell::~Cell()
{
    if ( NULL != d_ptr )
        delete d_ptr;
}

Cell::Type Cell::type() const
{
    Q_D(const Cell);

    return d->cellType;
}

QVariant Cell::value() const
{
    Q_D(const Cell);

    return d->value;
}

QVariant Cell::readValue() const
{
    Q_D(const Cell);

    QVariant ret; // return value
    ret = d->value;

    Format fmt = this->format();

    if (isDateTime())
    {
        QVariant vDT = dateTime();
        if ( vDT.isNull() )
        {
            return QVariant();
        }

        // https://github.com/QtExcel/QXlsx/issues/171
        // https://www.qt.io/blog/whats-new-in-qmetatype-qvariant
        #if QT_VERSION >= 0x060000 // Qt 6.0 or over
                if ( vDT.metaType().id() == QMetaType::QDateTime )
                {
                    ret = vDT;
                }
                else if ( vDT.metaType().id() == QMetaType::QDate )
                {
                    ret = vDT;
                }
                else if ( vDT.metaType().id() == QMetaType::QTime )
                {
                    ret = vDT;
                }
                else
                {
                    return QVariant();
                }
        #else
                if ( vDT.type() == QVariant::DateTime )
                {
                    ret = vDT;
                }
                else if ( vDT.type() == QVariant::Date )
                {
                    ret = vDT;
                }
                else if ( vDT.type() == QVariant::Time )
                {
                    ret = vDT;
                }
                else
                {
                    return QVariant();
                }
        #endif

        // QDateTime dt = dateTime();
        // ret = dt;

        // QString strFormat = fmt.numberFormat();
        // if (!strFormat.isEmpty())
        // {
        //     // TODO: use number format
        // }

        // qint32 styleNo = d->styleNumber;

        // if (styleNo == 10)
        // {
        // }

        // if (styleNo == 11)
        // {
            // QTime timeValue = dt.time(); // only time. (HH:mm:ss)
            // ret = timeValue;
            // return ret;
        // }

        // if (styleNo == 12)
        // {
        // }

        // if (styleNo == 13) // (HH:mm:ss)
        // {
            // double dValue = d->value.toDouble();
            // int day = int(dValue); // unit is day.
            // double deciamlPointValue1 = dValue - double(day);

            // double dHour = deciamlPointValue1 * (double(1.0) / double(24.0));
            // int hour = int(dHour);

            // double deciamlPointValue2 = deciamlPointValue1 - (double(hour) * (double(1.0) / double(24.0)));
            // double dMin = deciamlPointValue2 * (double(1.0) / double(60.0));
            // int min = int(dMin);

            // double deciamlPointValue3 = deciamlPointValue2 - (double(min) * (double(1.0) / double(60.0)));
            // double dSec = deciamlPointValue3 * (double(1.0) / double(60.0));
            // int sec = int(dSec);

            // int totalHour = hour + (day * 24);

            // QString strTime;
            // strTime = QString("%1:%2:%3").arg(totalHour).arg(min).arg(sec);
            // ret = strTime;

            // return ret;
        // }

        // return ret;
        // */
    }

    if (hasFormula())
        ret = this->formula().text();

    return ret;
}

void Cell::setValue(const QVariant &value)
{
    Q_D(Cell);
    d->value = value;
}

Format Cell::format() const
{
    Q_D(const Cell);

    return d->format;
}

void Cell::setFormat(const Format &format)
{
    Q_D(Cell);
    d->format = format;
}

bool Cell::hasFormula() const
{
    Q_D(const Cell);

    return d->formula.isValid();
}

CellFormula Cell::formula() const
{
    Q_D(const Cell);

    return d->formula;
}

void Cell::setFormula(const CellFormula &formula)
{
    Q_D(Cell);
    d->formula = formula;
}

bool Cell::isDateTime() const
{
    Q_D(const Cell);

    Type cellType = d->cellType;
    double dValue = d->value.toDouble(); // number
//    QString strValue = d->value.toString().toUtf8();
    bool isValidFormat = d->format.isValid();
    bool isDateTimeFormat = d->format.isDateTimeFormat(); // datetime format

    // dev67
    if ( cellType == Type::Number ||
         cellType == Type::Date ||
         cellType == Type::Custom )
    {
        if ( dValue >= 0 &&
             isValidFormat &&
             isDateTimeFormat )
        {
            return true;
        }
    }

    return false;
}

QVariant Cell::dateTime() const
{
    Q_D(const Cell);

    if (!isDateTime())
    {
        return QVariant();
    }

    // dev57

    QVariant ret;
    double dValue = d->value.toDouble();
    bool isDate1904 = d->parent->workbook()->date1904().value_or(false);
    ret = datetimeFromNumber(dValue, isDate1904);
    return ret;
}

bool Cell::isRichString() const
{
    Q_D(const Cell);

    if ( d->cellType != Type::SharedString &&
            d->cellType != Type::InlineString &&
            d->cellType != Type::Formula )
    {
        return false;
    }

    return d->richString.isRichString();
}

RichString Cell::richString() const
{
    Q_D(const Cell);
    return d->richString;
}

void Cell::setRichString(const RichString &richString)
{
    Q_D(Cell);
    d->richString = richString;
}

qint32 Cell::styleNumber() const 
{
    Q_D(const Cell);

    qint32 ret = d->styleNumber;
    return ret;
}

bool Cell::isDateType(Type cellType, const Format &format)
{
    if ( cellType == Type::Number || cellType == Type::Date || cellType == Type::Custom)
        return format.isValid() && format.isDateTimeFormat();
    return false;
}

}
