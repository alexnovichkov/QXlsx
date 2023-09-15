// xlsxworksheet.h

#ifndef XLSXWORKSHEET_H
#define XLSXWORKSHEET_H

#include <memory>
#include <QtGlobal>
#include <QObject>
#include <QStringList>
#include <QMap>
#include <QVariant>
#include <QPointF>
#include <QIODevice>
#include <QDateTime>
#include <QUrl>
#include <QImage>

#include "xlsxabstractsheet.h"
#include "xlsxcell.h"
#include "xlsxcellrange.h"
#include "xlsxcellreference.h"

class WorksheetTest;

namespace QXlsx {

class DocumentPrivate;
class Workbook;
class Format;
class Drawing;
class DataValidation;
class ConditionalFormatting;
class CellRange;
class RichString;
class Relationships;
class Chart;

/**
 * @brief The SheetView struct represents one of the 'views' into the worksheet.
 */
struct QXLSX_EXPORT SheetView
        //TODO: convert to implicitly shareable class
{
    /**
     * @brief The Type enum defines the kinds of view available to an application
     * when rendering a SpreadsheetML document.
     */
    enum class Type
    {
        Normal, /**< Specifies that the worksheet should be displayed without regard to pagination.*/
        PageBreakPreview, /**<  Specifies that the worksheet should be displayed
showing where pages would break if it were printed. */
        PageLayout /**< Specifies that the worksheet should be displayed
mimicking how it would look if printed. */
    };

    /**
     * @brief Flag indicating whether the panes in the window are locked due to
     * workbook protection.
     *
     * This is an option when the workbook structure is protected.
     *
     * If not set, the default value is false.
     */
    std::optional<bool> windowProtection;
    /**
     * @brief Flag indicating whether this sheet should display formulas.
     *
     * If not set, the default value is false.
     */
    std::optional<bool> showFormulas;
    /**
     * @brief Flag indicating whether this sheet should display gridlines.
     *
     * If not set, the default value is true.
     */
    std::optional<bool> showGridLines;
    /**
     * @brief Flag indicating whether the sheet should display row and column headings.
     *
     * If not set, the default value is true.
     */
    std::optional<bool> showRowColHeaders;
    /**
     * @brief Flag indicating whether the window should show 0 (zero) in cells
     * containing zero value.
     *
     * When false, cells with zero value appear blank instead of showing the number zero.
     *
     * If not set, the default value is true.
     */
    std::optional<bool> showZeros;
    /**
     * @brief Flag indicating whether the sheet is in 'right to left' display mode.
     *
     * When in this mode, Column A is on the far right, Column B ;is one column
     * left of Column A, and so on. Also, information in cells is displayed in
     * the Right to Left format.
     *
     * If not set, the default value is false.
     */
    std::optional<bool> rightToLeft;
    /**
     * @brief Flag indicating whether this sheet is selected.
     *
     * When only 1 sheet is selected and active, this value should be in synch
     * with the activeTab value.
     *
     * If not set, the default value is false.
     */
    std::optional<bool> tabSelected;
    /**
     * @brief Show the ruler in page layout view.
     *
     * If not set, the default value is false.
     */
    std::optional<bool> showRuler;
    /**
     * @brief Flag indicating whether the sheet has outline symbols visible.
     *
     * If not set, the default value is true.
     */
    std::optional<bool> showOutlineSymbols;
    /**
     * @brief Flag indicating whether page layout view shall display margins.
     *
     * False means do not display left, right, top (header), and bottom (footer)
     * margins (even when there is data in the header or footer).
     *
     * If not set, the default value is true.
     */
    std::optional<bool> showWhiteSpace;
    /**
     * @brief Flag indicating that the application should use the default grid
     * lines color (system dependent).
     *
     * Overrides any color specified in colorId.
     *
     * If not set, the default value is true.
     */
    std::optional<bool> defaultGridColor;
    /**
     * @brief View type.
     *
     * If not set, the default value is Type::Normal.
     */
    std::optional<Type> type;
    /**
     * @brief Location of the top left visible cell.
     */
    CellReference topLeftCell;
    /**
     * @brief  Index to the color value for row/column text headings and gridlines.
     * This is an 'index color value' (ICV) rather than rgb value.
     *
     * If not set, the default value is 64.
     */
    std::optional<int> colorId;
    /**
     * @brief Window zoom magnification for current view representing percent values.
     *
     * This parameter is restricted to values ranging from 10 to 400.
     *
     * If not set, the default value is 100.
     */
    std::optional<int> zoomScale;
    /**
     * @brief Zoom magnification to use when in normal view, representing percent values.
     *
     * This attribute is restricted to values ranging from 10 to 400. Zero value implies
     * automatic setting. Horizontal & Vertical scale together.
     *
     * If not set, the default value is 0 (auto zoom scale in normal view).
     */
    std::optional<int> zoomScaleNormal;
    /**
     * @brief Zoom magnification to use when in page break view, representing percent values.
     *
     * This attribute is restricted to values ranging from 10 to 400. Zero value implies
     * automatic setting. Horizontal & Vertical scale together.
     *
     * If not set, the default value is 0 (auto zoom scale in page break view).
     */
    std::optional<int> zoomScalePageBreakView;
    /**
     * @brief Zoom magnification to use when in page layout view, representing percent values.
     *
     * This attribute is restricted to values ranging from 10 to 400. Zero value implies
     * automatic setting. Horizontal & Vertical scale together.
     *
     * If not set, the default value is 0 (auto zoom scale in page layout view).
     */
    std::optional<int> zoomScalePageLayoutView;
    /**
     * @brief Zero-based index of this workbook view, pointing to a specific workbookView
     * element in the bookViews collection.
     */
    int workbookViewId = 0; //required

    //TODO:
//    std::optional<CT_Pane> pane;
//    std::optional<CT_Selection> selection;
//    std::optional<CT_PivotSelection> pivotSelection;

    ExtensionList extLst;

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer, const QLatin1String &name) const;
};

class WorksheetPrivate;
class QXLSX_EXPORT Worksheet : public AbstractSheet
{
    Q_DECLARE_PRIVATE(Worksheet)

private:
    friend class DocumentPrivate;
    friend class Workbook;
    friend class ::WorksheetTest;
    Worksheet(const QString &sheetName, int sheetId, Workbook *book, CreateFlag flag);
    Worksheet *copy(const QString &distName, int distId) const override;

public:
    ~Worksheet();

public:
    bool write(const CellReference &row_column, const QVariant &value, const Format &format=Format());
    bool write(int row, int column, const QVariant &value, const Format &format=Format());

    QVariant read(const CellReference &row_column) const;
    QVariant read(int row, int column) const;

    bool writeString(const CellReference &row_column, const QString &value, const Format &format=Format());
    bool writeString(int row, int column, const QString &value, const Format &format=Format());
    bool writeString(const CellReference &row_column, const RichString &value, const Format &format=Format());
    bool writeString(int row, int column, const RichString &value, const Format &format=Format());

    bool writeInlineString(const CellReference &row_column, const QString &value, const Format &format=Format());
    bool writeInlineString(int row, int column, const QString &value, const Format &format=Format());

    bool writeNumeric(const CellReference &row_column, double value, const Format &format=Format());
    bool writeNumeric(int row, int column, double value, const Format &format=Format());

    bool writeFormula(const CellReference &row_column, const CellFormula &formula, const Format &format=Format(), double result=0);
    bool writeFormula(int row, int column, const CellFormula &formula, const Format &format=Format(), double result=0);

    bool writeBlank(const CellReference &row_column, const Format &format=Format());
    bool writeBlank(int row, int column, const Format &format=Format());

    bool writeBool(const CellReference &row_column, bool value, const Format &format=Format());
    bool writeBool(int row, int column, bool value, const Format &format=Format());

    bool writeDateTime(const CellReference &row_column, const QDateTime& dt, const Format &format=Format());
    bool writeDateTime(int row, int column, const QDateTime& dt, const Format &format=Format());

    // dev67
    bool writeDate(const CellReference &row_column, const QDate& dt, const Format &format=Format());
    bool writeDate(int row, int column, const QDate& dt, const Format &format=Format());

    bool writeTime(const CellReference &row_column, const QTime& t, const Format &format=Format());
    bool writeTime(int row, int column, const QTime& t, const Format &format=Format());

    bool writeHyperlink(const CellReference &row_column, const QUrl &url, const Format &format=Format(), const QString &display=QString(), const QString &tip=QString());
    bool writeHyperlink(int row, int column, const QUrl &url, const Format &format=Format(), const QString &display=QString(), const QString &tip=QString());

    bool addDataValidation(const DataValidation &validation);
    bool addConditionalFormatting(const ConditionalFormatting &cf);
    /**
     * @brief returns cell by its reference.
     * @param row_column reference to the cell.
     * @return valid pointer to the cell if the cell was found, nullptr otherwise.
     */
    Cell *cell(const CellReference &row_column) const;
    /**
     * @overload
     * @brief returns cell by its row and column number.
     * @param row zero-based cell row number.
     * @param column zero-based cell column number.
     * @return valid pointer to the cell if the cell was found, nullptr otherwise.
     */
    Cell *cell(int row, int column) const;

    /*!
     * @brief Inserts an \a image at the position \a row, \a column.
     * @param row the zero-based row index of the image top left corner.
     * @param column the zero-based column index of the image top left corner.
     * @return image index (zero-based) on success, -1 otherwise.
     */
    int insertImage(int row, int column, const QImage &image);
    /**
     * @brief returns an image.
     * @param index zero-based image index (0 to #imageCount()-1).
     * @return non-null QImage if image was found and read.
     */
    QImage image(int index) const;
    /**
     * @overload
     * @brief returns image.
     * @param row the zero-based row index of the image top left corner.
     * @param column the zero-based column index of the image top left corner.
     * @return non-null QImage if image was found and read.
     */
    QImage image(int row, int column) const;
    /**
     * @brief returns the count of images on this worksheet.
     */
    int imageCount() const;
    /**
     * @brief removes image from the worksheet
     * @param row the zero-based row index of the image top left corner.
     * @param column the zero-based column index of the image top left corner.
     * @return true if image was found and removed, false otherwise.
     */
    bool removeImage(int row, int column);
    /**
     * @brief removes image from the worksheet
     * @param index zero-based index of the image (0 to #imageCount()-1).
     * @return true if image was found and removed, false otherwise.
     */
    bool removeImage(int index);

    /**
     * @brief creates a new chart and places it inside the current worksheet.
     * @param row the zero-based row index of the chart top left corner.
     * @param column the zero-based column index of the chart top left corner.
     * @param size the chart size in pixels.
     * @return pointer to the new chart or nullptr if no chart was created.
     */
    Chart *insertChart(int row, int column, const QSize &size);
    //TODO: add insertImage(int row, int column, const Size &size); to specify size in mm, pt etc.
    /**
     * @brief returns chart
     * @param index zero-based index of the chart (0 to #chartCount()-1)
     * @return valid pointer to the chart if the chart was found, nullptr otherwise.
     */
    Chart *chart(int index) const;
    /**
     * @overload
     * @brief returns chart.
     * @param row the zero-based row index of the chart top left corner.
     * @param column the zero-based column index of the chart top left corner.
     * @return valid pointer to the chart if the chart was found, nullptr otherwise.
     */
    Chart *chart(int row, int column);
    /**
     * @brief removes chart from the worksheet.
     * @param row the zero-based row index of the chart top left corner.
     * @param column the zero-based column index of the chart top left corner.
     * @return true if the chart was found and successfully removed, false otherwise.
     */
    bool removeChart(int row, int column);
    /**
     * @overload
     * @brief removes chart from the worksheet.
     * @param index zero-based index of the chart.
     * @return true if the chart was found and successfully removed, false otherwise.
     */
    bool removeChart(int index);
    /**
     * @brief returns count of charts on the worksheet.
     */
    int chartCount() const;

    bool mergeCells(const CellRange &range, const Format &format=Format());
    bool unmergeCells(const CellRange &range);
    QList<CellRange> mergedCells() const;

    bool setColumnWidth(const CellRange& range, double width);
    bool setColumnFormat(const CellRange& range, const Format &format);
    bool setColumnHidden(const CellRange& range, bool hidden);
    bool setColumnWidth(int colFirst, int colLast, double width);
    bool setColumnFormat(int colFirst, int colLast, const Format &format);
    bool setColumnHidden(int colFirst, int colLast, bool hidden);

    double columnWidth(int column) const;
    Format columnFormat(int column) const;
    bool isColumnHidden(int column) const;

    /**
     * @brief sets rows heights for rows in the range from rowFirst to rowLast.
     * @param rowFirst the first row index (1-based).
     * @param rowLast the last row index (1-based).
     * @param height row height in points.
     * @return true on success.
     */
    bool setRowHeight(int rowFirst,int rowLast, double height);
    bool setRowFormat(int rowFirst,int rowLast, const Format &format);
    bool setRowHidden(int rowFirst,int rowLast, bool hidden);

    double rowHeight(int row) const;
    Format rowFormat(int row) const;
    bool isRowHidden(int row) const;

    bool groupRows(int rowFirst, int rowLast, bool collapsed = true);
    /**
     * @brief groups columns from @a colFirst to @a colLast and collapse them as specified.
     * @param colFirst 1-based index of the first column.
     * @param colLast 1-based index of the last column.
     * @param collapsed if true, columns will be collapsed.
     * @return true if the maximum level of grouping is not exceeded and the grouping is successful,
     * false otherwise.
     *
     * The "group/ungroup" button will appear over the column with index colLast+1.
     */
    bool groupColumns(int colFirst, int colLast, bool collapsed = true);
    /**
     * @overload
     * @brief groups columns from @a range and collapse them as specified.
     * @param range range of columns to group.
     * @param collapsed
     * @return true if the maximum level of grouping is not exceeded and the grouping is successful,
     * false otherwise.
     *
     * The "group/ungroup" button will appear over the column with index colLast+1.
     */
    bool groupColumns(const CellRange &range, bool collapsed = true);
    CellRange dimension() const;
    /**
     * @brief returns whether the panes in the window are locked due to workbook protection.
     *
     * The default value is false.
     * @note The worksheet can have more than one view. This method returns the
     * property of the last one. To get the specific view use #view() method.
     */
    bool isWindowProtected() const;
    /**
     * @brief sets whether the panes in the window are locked due to workbook protection.
     *
     * @param protect Sets protected to the view. The default value is false.
     * @note The worksheet can have more than one view. This method sets the
     * property of the last one. If no sheet views were added, this method
     * adds the default one. To get the specific view use #view() method.
     */
    void setWindowProtected(bool protect);
    //TODO: doc
    bool isFormulasVisible() const;
    void setFormulasVisible(bool visible);//TODO: doc
    bool isGridLinesVisible() const;//TODO: doc
    void setGridLinesVisible(bool visible);//TODO: doc
    bool isRowColumnHeadersVisible() const;//TODO: doc
    void setRowColumnHeadersVisible(bool visible);//TODO: doc
    bool isZerosVisible() const;//TODO: doc
    void setZerosVisible(bool visible);//TODO: doc
    bool isRightToLeft() const;//TODO: doc
    void setRightToLeft(bool enable);//TODO: doc
    bool isSelected() const;//TODO: doc
    void setSelected(bool select);//TODO: doc
    bool isRulerVisible() const;//TODO: doc
    void setRulerVisible(bool visible);//TODO: doc
    bool isOutlineSymbolsVisible() const;//TODO: doc
    void setOutlineSymbolsVisible(bool visible);//TODO: doc
    bool isWhiteSpaceVisible() const;//TODO: doc
    void setWhiteSpaceVisible(bool visible);//TODO: doc
    /**
     * @brief returns the sheet view with the (zero-based) #index.
     * @param index index of the sheet view.
     * @return the sheet view with the (zero-based) #index. If no such view is
     * found, returns the default-constructed one.
     */
    SheetView view(int index) const;
    /**
     * @brief returns the sheet view with the (zero-based) #index.
     * @param index non-negative index of the view. If the index is negative,
     * throws std::out_of_range exception.
     * @return reference to the sheet view with the (zero-based) #index.
     * If no such view was found, inserts a new view at #index.
     * @note The newly inserted view will point to the workbook view with
     * index 0. See SheetView::workbookViewId.
     *
     */
    SheetView &view(int index);
    /**
     * @brief returns the count of sheet views defined in the worksheet.
     * @return
     */
    int viewsCount() const;
    /**
     * @brief adds new default-constructed sheet view.
     * @return reference to the added view.
     */
    SheetView & addView();
    /**
     * @brief removes the sheet view with #index.
     * @param index non-negative index of the view.
     * @return true if the view was found and removed, false otherwise.
     */
    bool removeView(int index);


    /**
     * @brief autosizes columns widths for all rows in the worksheet.
     * @param firstColumn 1-based index of the first column to autosize.
     * @param lastColumn 1-based index of the last column to autosize.
     * @return true on success.
     */
    bool autosizeColumnWidths(int firstColumn, int lastColumn);
    /**
     * @overload
     * @brief autosizes columns widths for columns specified by range.
     * @param range valid CellRange that specifies the columns range to autosize and
     * the rows range to estimate the maximum column width.
     * @return true on success.
     */
    bool autosizeColumnWidths(const CellRange &range);

private:
    QMap<int, int> getMaximumColumnWidths(int firstRow = 1, int lastRow = INT_MAX);
    void saveToXmlFile(QIODevice *device) const override;
    bool loadFromXmlFile(QIODevice *device) override;
};

}
#endif // XLSXWORKSHEET_H
