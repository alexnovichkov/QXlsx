// xlsxcellrange.h

#ifndef QXLSX_XLSXCELLRANGE_H
#define QXLSX_XLSXCELLRANGE_H

#include <QtGlobal>
#include <QObject>
#include <QDebug>

#include "xlsxglobal.h"
#include "xlsxcellreference.h"

namespace QXlsx {

/**
 * @brief The CellRange class represents a range of cells in a worksheet.
 * The CellRange class stores the top left and bottom right rows and columns of a range in a worksheet.
 * @note The numbering of rows and columns starts from 1.
 */
class QXLSX_EXPORT CellRange
{
public:
    CellRange();
    /**
     * @brief creates CellRange from the given @a top, @a left, @a bottom and @a
     * right rows and columns.
     * @param firstRow the first row number.
     * @param firstColumn the first column number.
     * @param lastRow the last row number.
     * @param lastColumn the last column number.
     * @note The numbering of rows and columns starts from 1.
     * @note This constructor checks if first <= last and fixes the order if needed.
     */
    CellRange(int firstRow, int firstColumn, int lastRow, int lastColumn);
    /**
     * @brief creates CellRange from the given @a topLeft and @a bottomRight cells.
     * @param topLeft the top left cell of a range.
     * @param bottomRight the bottom right cell of a range.
     * @note This constructor checks the range for validity and fixes the order if needed.
     */
    CellRange(const CellReference &topLeft, const CellReference &bottomRight);
    /**
     * @brief creates CellRange from the given @a cell and offsets from it.
     * @param cell the starting cell reference.
     * @param rowOffset row offset to be applied to the @a cell row to yield the second corner.
     * @param columnOffset column offset to be applied to the @a cell column to yield the second corner.
     * @note This constructor checks the range for validity and fixes the order if needed.
     * @note The resulting range may still be invalid.
     */
    CellRange(const CellReference &cell, int rowOffset, int columnOffset);
    /**
     * @brief parses @a range and creates CellRange from the result.
     * @param range string representing the range (f.e. "A1:G5").
     * @note This constructor checks the range for validity and fixes the order if needed.
     * @note The resulting range may still be invalid.
     */
    CellRange(const QString &range);
    /**
     * @brief parses @a range and creates CellRange from the result.
     * @param range null-terminated string representing the range (f.e. "A1:G5").
     * @note This constructor checks the range for validity and fixes the order if needed.
     * @note The resulting range may still be invalid.
     */
    CellRange(const char *range);
    CellRange(const CellRange &other);
    ~CellRange();
    /**
     * @brief Converts the range to string notation, such as "A1:B5".
     * @return non-empty string if the range is valid, empty string otherwise.
    */
    QString toString(bool rowFixed = false, bool colFixed = false) const;
    /**
     * @brief returns whether the range is valid, i.e. top row and left column are positive,
     * and bottom row is not less than top row, and right column is not less than left column.
     * @return
     */
    bool isValid() const;
    inline void setFirstRow(int row) { top = row; }
    inline void setLastRow(int row) { bottom = row; }
    inline void setFirstColumn(int column) { left = column; }
    inline void setLastColumn(int column) { right = column; }
    inline int firstRow() const { return top; }
    inline int lastRow() const { return bottom; }
    inline int firstColumn() const { return left; }
    inline int lastColumn() const { return right; }
    inline int rowCount() const { return bottom - top + 1; }
    inline int columnCount() const { return right - left + 1; }
    inline CellReference topLeft() const { return CellReference(top, left); }
    inline CellReference topRight() const { return CellReference(top, right); }
    inline CellReference bottomLeft() const { return CellReference(bottom, left); }
    inline CellReference bottomRight() const { return CellReference(bottom, right); }
    /**
     * @brief Fixes the order of the range corners.
     * @note If the resulting left column or top row are < 1, the range is still invalid.
     */
    void fixOrder();
    /**
     * @brief return whether the range contains the cell defined as CellReference.
     * @param ref cell reference to test.
     * @return `true` if range contains @a ref.
     */
    bool contains(const CellReference &ref) const;
    /**
     * @overload
     * @brief return whether the range contains the cell defined by its @a row
     * and @a column.
     * @param row 1-based row index.
     * @param column 1-based column index.
     * @return `true` if range contains cell (@a row, @a column).
     */
    bool contains(int row, int column) const;
    /**
     * @overload
     * @brief return whether the range contains the @a other range.
     * @param other
     * @return `true` if this range contains the @a other range.
     */
    bool contains(const CellRange &other) const;

    inline void operator =(const CellRange &other)
    {
        top = other.top;
        bottom = other.bottom;
        left = other.left;
        right = other.right;
    }
    inline bool operator ==(const CellRange &other) const
    {
        return top==other.top && bottom==other.bottom
                && left == other.left && right == other.right;
    }
    inline bool operator !=(const CellRange &other) const
    {
        return top!=other.top || bottom!=other.bottom
                || left != other.left || right != other.right;
    }
private:
    void init(const QString &range);

    int top;
    int left;
    int bottom;
    int right;
};

}

QDebug operator<<(QDebug out, const QXlsx::CellRange &range);

Q_DECLARE_TYPEINFO(QXlsx::CellRange, Q_MOVABLE_TYPE);

#endif // QXLSX_XLSXCELLRANGE_H
