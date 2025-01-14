// xlsxcellrange.cpp

#include <QtGlobal>
#include <QString>
#include <QPoint>
#include <QStringList>

#include "xlsxcellrange.h"
#include "xlsxcellreference.h"

namespace QXlsx {

CellRange::CellRange()
    : top(-1), left(-1), bottom(-2), right(-2)
{
}

CellRange::CellRange(int top, int left, int bottom, int right)
    : top(top), left(left), bottom(bottom), right(right)
{
    fixOrder();
}

CellRange::CellRange(const CellReference &topLeft, const CellReference &bottomRight)
    : top(topLeft.row()), left(topLeft.column())
    , bottom(bottomRight.row()), right(bottomRight.column())
{
    fixOrder();
}

CellRange::CellRange(const CellReference &cell, int rowOffset, int columnOffset)
    : top{cell.row()}, left{cell.column()}
{
    bottom = top + rowOffset;
    right = left + columnOffset;
    fixOrder();
}

CellRange::CellRange(const QString &range)
{
    init(range);
    fixOrder();
}

CellRange::CellRange(const char *range)
{
    init(QString::fromLatin1(range));
    fixOrder();
}

void CellRange::init(const QString &range)
{
    QStringList rs = range.split(QLatin1Char(':'));
    if (rs.size() == 2) {
        CellReference start(rs[0]);
        CellReference end(rs[1]);
        top = start.row();
        left = start.column();
        bottom = end.row();
        right = end.column();
    } else {
        CellReference p(rs[0]);
        top = p.row();
        left = p.column();
        bottom = p.row();
        right = p.column();
    }
}

void CellRange::fixOrder()
{
    if (top > bottom) std::swap(top, bottom);
    if (left > right) std::swap(left, right);
}

bool CellRange::contains(const CellReference &ref) const
{
    return (ref.row() >= top && ref.row() <= bottom &&
            ref.column() >= left && ref.column() <= right);
}

bool CellRange::contains(int row, int column) const
{
    return (row >= top && row <= bottom &&
            column >= left && column <= right);
}

bool CellRange::contains(const CellRange &other) const
{
    if (!other.isValid()) return false;
    return (top <= other.top &&
            bottom >= other.bottom &&
            left <= other.left &&
            right >= other.right);
}

CellRange::CellRange(const CellRange &other)
    : top(other.top), left(other.left), bottom(other.bottom), right(other.right)
{
    fixOrder();
}

CellRange::~CellRange()
{
}

QString CellRange::toString(bool rowFixed, bool colFixed) const
{
    if (!isValid())
        return QString();

    if (left == right && top == bottom) {
        //Single cell
        return CellReference(top, left).toString(rowFixed, colFixed);
    }

    QString cell_1 = CellReference(top, left).toString(rowFixed, colFixed);
    QString cell_2 = CellReference(bottom, right).toString(rowFixed, colFixed);
    return cell_1 + QLatin1String(":") + cell_2;
}

bool CellRange::isValid() const
{
    return left <= right && top <= bottom && left > 0 && top > 0;
}

}

QDebug operator<<(QDebug out, const QXlsx::CellRange &range)
{
    out << range.toString();
    return out;
}
