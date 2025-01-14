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
#include "xlsxsheetview.h"
#include "xlsxautofilter.h"
#include "xlsxdatavalidation.h"

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
 * @brief The ProtectedRange struct represents the protection parameters for
 * a cell range on a worksheet.
 *
 * A list of cell #ranges to be protected must get a unique non-empty #name.
 * A password protection can be set via #pr attributes. There is a possibility
 * to specify a list of #users who are allowed to edit the protected cells
 * without entering a password each time.
 */
struct QXLSX_EXPORT ProtectedRange
{
    ProtectedRange() {}
    ProtectedRange(const CellRange &range, const QString &name) : name{name}
    {
        ranges << range;
    }
    ProtectedRange(const QList<CellRange> &ranges, const QString &name)
        : name{name}, ranges{ranges}
    {
    }
    PasswordProtection pr; /**< @brief Contains password protection atributes. */
    QStringList users; /**< @brief A list of users who can edit the protected
cells without entering a password.

ECMA-376 recommends `username@domain` format be used. */
    QString name; /**< @brief Required unique protected range name. */
    QList<CellRange> ranges; /**< @brief A list of protected ranges. */

    bool operator==(const ProtectedRange& other) const {
        if (pr != other.pr) return false;
        if (users != other.users) return false;
        if (name != other.name) return false;
        if (ranges != other.ranges) return false;
        return true;
    }
    bool operator!=(const ProtectedRange& other) const {
        return !operator==(other);
    }
    /**
     * @brief sets the password protection to the range.
     * @param password a string that contains the password.
     * @param algorithm a string that describes the hashing algorithm used.
     * See Protection::algorithmName for some reserved values. If no algorithm
     * is specified, 'SHA-512' is used.
     * @param salt a salt string. If no salt is specified, either an empty
     * string or a random 16-byte string is used depending on the
     * Protection::randomizedSalt() value.
     * @param spinCount count of iterations to compute the password hash (more
     * is better, Excel uses value of 100,000). If no spinCount is specified,
     * the value of 100,000 is used.
     *
     * @note 1. This method supports only the following algorithms as they
     * are supported by QCryptographicHash: MD4, MD5, SHA-1, SHA-224, SHA-256,
     * SHA-384, SHA-512, SHA3-224, SHA3-256, SHA3-384, SHA3-512, Keccak-224,
     * Keccak-256, Keccak-384, Keccak-512. Excel uses SHA-512.
     *
     * @note 2. This method computes the hashed password value and stores it in
     * the Protection attributes. You can access this value (in the base-64
     * form) via pr.hashValue. Moreover, the salt value is also stored in the
     * base-64 form in the pr.saltValue attribute. There's no way to know
     * the actual password from xlsx.
     */
    bool setPasswordProtection(const QString &password,
                               const QString &algorithm=QStringLiteral("SHA-512"),
                               const QString &salt = QString(), int spinCount = 100000);
    bool isValid() const;
    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer) const;
};


class WorksheetPrivate;

/**
 * @brief The Worksheet class represents a worksheet in the workbook.
 *
 * ## Sheet Properties
 *
 * Worksheets inherit the common properties of an AbstractSheet:
 * AbstractSheet::codeName(), AbstractSheet::isPublished(),
 * AbstractSheet::tabColor().
 *
 * The following parameters define the behavior and look-and-feel of worksheets:
 *
 * - #isFormatConditionsCalculationEnabled(),
 *   #setFormatConditionsCalculationEnabled() manage the recalculation of the
 *   conditional formatting.
 * - #setSyncedHorizontal(), #isSyncedHorizontal(), #isSyncedVertical(),
 *   #setSyncedVertical(), #topLeftAnchor(), #setTopLeftAnchor() manage the
 *   anchor parameters of the sheet.
 * - #showAutoPageBreaks(), #setShowAutoPageBreaks() manage the auto page breaks
 *   visibility.
 * - #thickBottomBorder(), #setThickBottomBorder(), #thickTopBorder(),
 *   #setThickTopBorder() manage default thickness of rows borders.
 * - #rowsHiddenByDefault(), #setRowsHiddenByDefault() manage the default
 *   visibility of rows.
 *
 * ## Sheet Views
 *
 * See the AbstractSheet documentation on the description of the supported view
 * properties, both inherited from AbstractSheet and specific to Worksheet.
 *
 * ## Sheet printing parameters and page setup parameters.
 *
 * See the AbstractSheet documentation on the description of the supported page
 * and print properties, both inherited from AbstractSheet and specific to
 * Worksheet.
 *
 * ## Rows and columns
 *
 * Row heights are measured in points. The row height can be set separately for
 * each row with #setRowHeight() or for all rows in the sheet with
 * #setDefaultRowHeight(), and tested with #rowHeight().
 * The default row height is 14.4 pt (24 px at 96 dpi).
 *
 * Column widths are measured in widths of the widest digit of the normal
 * style's font. The column width can be set separately for each column with
 * #setColumnWidth() or for all columns in the sheet with
 * #setDefaultColumnWidth(), and tested with #columnWidth(). The default column
 * width is 8 5/7 widths of the widest digit of the normal style's font (Calibri
 * 11 pt).
 *
 * Rows and columns can be set hidden with #setRowHidden(), #setColumnHidden().
 * Also if only some rows need to be visible, then #setRowsHiddenByDefault() can
 * come in handy. Rows and columns visibility can be tested with #isRowHidden()
 * and #isColumnHidden().
 *
 * ## Data manipulation
 *
 * Not written yet.
 *
 * ## Data validation
 *
 * Not written yet.
 *
 * ## Autofiltering and sorting
 *
 * If set, autofilter temporarily hides rows based on a filter criteria, which
 * is applied column by column to a range of data in the worksheet.
 *
 * These methods help you to set up the autofiltering and sorting options for
 * the worksheet:
 *
 * - #autofilter(), #setAutofilter(), #clearAutofilter().
 *
 * See AutoFilter class description on how to set up autofiltering. See also
 * [Autofilter](../../examples/Autofilter/autofilter.cpp) example.
 *
 * ## Conditional formatting
 *
 * A conditional format is a format, such as cell shading or font color, that a
 * spreadsheet application can automatically apply to cells if a specified
 * condition is `true`.
 *
 * These methods help you to set up the conditional formatting options for the worksheet:
 *
 * - #conditionalFormatting(), #addConditionalFormatting(), #clearConditionalformatting().
 *
 * See ConditionalFormatting class on how to set up the conditions. See also
 * [ConditionalFormatting](../../examples/ConditionalFormatting/conditionalformatting.cpp) example.
 *
 * ## Sheet protection
 *
 * Worksheets inherit all the sheet protection methods from AbstractSheet. See
 * docs on Sheet protection there.
 *
 * Additionally worksheets can have cell ranges protection enforced. It allows
 * to block editing of specific cells and cell ranges with a password protection.
 * See #addProtectedRange(), #protectedRanges(), #removeProtectedRange() methods.
 *
 * A protected range must have non-empty unique name, can have a password
 * protection and a list of users who can edit the protected range cells without
 * entering a password. See ProtectedRange documentation. See also
 * [SheetProtection](../../examples/SheetProtection/main.cpp) example of how to
 * set the sheet and range protection.
 *
 */
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

    /**
     * @brief sets formatting for @a range.
     * @param range valid CellRange.
     * @param format If valid, sets new format. If invalid, clears formatting in @a range.
     * @return `true` on success.
     * @sa #format(), #clearFormat().
     */
    bool setFormat(const CellRange &range, const Format &format);
    /**
     * @overload
     * @brief sets formatting for the cell (@a row, @a column).
     * @param row index of the cell row (starting from 1).
     * @param column index of the cell column (starting from 1).
     * @param format If valid, sets new format. If invalid, clears formatting.
     * @return `true` on success.
     */
    bool setFormat(int row, int column, const Format &format);
    /**
     * @brief clears formatting in @a range.
     * @param range valid CellRange.
     * @return `true` on success.
     * @note This method does not remove not used Format from the workbook styles,
     * it simply removes the link between Format and Cell.
     * @sa #setFormat(), #format().
     */
    bool clearFormat(const CellRange &range);
    /**
     * @overload
     * @brief clears formatting in the cell (@a row, @a column).
     * @param row index of the cell row (starting from 1).
     * @param column index of the cell column (starting from 1).
     * @return `true` on success.
     * @note This method does not remove not used Format from the workbook styles,
     * it simply removes the link between Format and Cell.
     * @sa #setFormat(), #format().
     */
    bool clearFormat(int row, int column);
    /**
     * @overload
     * @brief clears formatting on the entire worksheet.
     * @note This method does not remove not used Format from the workbook styles,
     * it simply removes the link between Format and Cell.
     * @sa #setFormat(), #format().
     */
    void clearFormat();
    /**
     * @brief returns format of the @a cell.
     * @param cellRef valid CellReference.
     * @return A copy of the cell Format. May be invalid if no formatting was set
     * for @a cell.
     *
     * You can get the format directly:
     *
     * ```cpp
     * if (auto c = sheet->cell(cellRef))
     *     return c->format();
     * return {};
     * ```
     */
    Format format(const CellReference &cellRef) const;
    /**
     * @overload
     * @brief returns format of the cell (@a row, @a column).
     * @param row index of the cell row (starting from 1).
     * @param column index of the cell column (starting from 1).
     * @return A copy of the cell Format. May be invalid if no formatting was set
     * for the cell.
     *
     * You can get the format directly:
     *
     * ```cpp
     * if (auto c = sheet->cell(row, column))
     *     return c->format();
     * return {};
     * ```
     */
    Format format(int row, int column) const;

    /**
     * @brief Reads the cell data and returns it as a QVariant object.
     * @param cell The cell to read from.
     * @return QVariant object, that may contain a number, a string, a date/time etc.
     * @note If @a cell contains a shared formula, and this cell is not a 'master'
     * formula cell, this method recalculates the formula references to the ones related to
     * @a row and @a column.
     */
    QVariant read(const CellReference &cell) const;
    /**
     * @overload
     * @brief Reads the cell data and returns it as a QVariant object.
     * @param row The cell row (starting from 1).
     * @param column The cell column (starting from 1).
     * @return QVariant object, that may contain a number, a string, a date/time etc.
     * @note If the cell contains a shared formula, and this cell is not a 'master'
     * formula cell, this method recalculates the formula references to the ones related to
     * @a row and @a column.
     */
    QVariant read(int row, int column) const;

    /**
     * @brief writes @a value into the cell @a ref and applies @a format.
     * @param ref the cell reference
     * @param value
     * @param format
     * @return `true` on success.
     *
     * Depending on the @a value type and flags Workbook::isHtmlToRichStringEnabled(),
     * Workbook::isStringsToHyperlinksEnabled(), Workbook::isStringsToNumbersEnabled()
     * etc. this method tries to guess the right Cell::Type to use:
     *
     * value | behaviour | equivalent
     * ----|----|----
     * null variant | creates empty cell | #writeBlank()
     * string starts with '=' | creates a formula string cell | #writeFormula()
     * string contains HTML tags | if Workbook::isHtmlToRichStringEnabled() creates a rich string cell; otherwise creates a shared string cell | #writeString()
     * string resembles a hyperlink | if Workbook::isStringsToHyperlinksEnabled(), creates a hyperlink cell; otherwise creates a shared string cell | #writeHyperlink()
     * plain string | if value is convertible to a number and Workbook::isStringsToNumbersEnabled(), creates a numeric cell; otherwise creates a shared string cell | #writeString()
     * number | creates a numeric cell | #writeNumeric()
     * boolean | creates a boolean cell | #writeBool()
     * date/time | creates a date-time cell | #writeDateTime()
     */
    bool write(const CellReference &ref, const QVariant &value, const Format &format=Format());
    /**
     * @overload
     * @brief writes @a value into the cell (@a row, @a column) and applies @a format.
     * @param row index of the cell row (starting from 1).
     * @param column index of the cell column (starting from 1).
     * @param value value to write.
     * @param format format to apply.
     * @return `true` on success.
     *
     * Depending on the @a value type and flags Workbook::isHtmlToRichStringEnabled(),
     * Workbook::isStringsToHyperlinksEnabled(), Workbook::isStringsToNumbersEnabled()
     * etc. this method tries to guess the right Cell::Type to use:
     *
     * value | behaviour | equivalent
     * ----|----|----
     * null variant | creates empty cell | #writeBlank()
     * string starts with '=' | creates a formula string cell | #writeFormula()
     * string contains HTML tags | if Workbook::isHtmlToRichStringEnabled() creates a rich string cell; otherwise creates a shared string cell | #writeString()
     * string resembles a hyperlink | if Workbook::isStringsToHyperlinksEnabled(), creates a hyperlink cell; otherwise creates a shared string cell | #writeHyperlink()
     * plain string | if value is convertible to a number and Workbook::isStringsToNumbersEnabled(), creates a numeric cell; otherwise creates a shared string cell | #writeString()
     * number | creates a numeric cell | #writeNumeric()
     * boolean | creates a boolean cell | #writeBool()
     * date/time | creates a date-time cell | #writeDateTime()
     */
    bool write(int row, int column, const QVariant &value, const Format &format=Format());

    /**
     * @brief writes @a value into cell @a ref and applies @a format.
     * @param ref cell reference to write data to.
     * @param value data to write.
     * @param format format to apply.
     * @return `true` on success.
     *
     * Depending on the @a value and flag Workbook::isHtmlToRichStringEnabled()
     * this method tries to guess the right Cell::Type to use:
     *
     * value | behaviour
     * ----|----
     * string contains HTML tags and Workbook::isHtmlToRichStringEnabled() | creates shared string cell and writes RichString
     * otherwise | creates shared string cell and writes plain string.
     *
     * See Cell::Type on the inline and shared string types.
     */
    bool writeString(const CellReference &ref, const QString &value, const Format &format=Format());
    /**
     * @overload
     * @brief writes @a value into cell (@a row, @a column) and applies @a format.
     * @param row cell row index (starting from 1).
     * @param column cell column index (starting from 1).
     * @param value data to write.
     * @param format format to apply.
     * @return `true` on success.
     *
     * Depending on the @a value and flag Workbook::isHtmlToRichStringEnabled()
     * this method tries to guess the right Cell::Type to use:
     *
     * value | behaviour
     * ----|----
     * string contains HTML tags and Workbook::isHtmlToRichStringEnabled() | creates shared string cell and writes RichString
     * otherwise | creates shared string cell and writes plain string.
     *
     * See Cell::Type on the inline and shared string types.
     */
    bool writeString(int row, int column, const QString &value, const Format &format=Format());
    /**
     * @overload
     * @brief writes @a value into a shared string cell @a ref and applies @a format.
     * @param ref cell reference to write data to.
     * @param value data to write.
     * @param format format to apply.
     * @return `true` on success.
     *
     * See Cell::Type on the inline and shared string types.
     */
    bool writeString(const CellReference &ref, const RichString &value, const Format &format=Format());
    /**
     * @overload
     * @brief writes @a value into a shared string cell (@a row, @a column) and applies @a format.
     * @param row cell row index (starting from 1).
     * @param column cell column index (starting from 1).
     * @param value data to write.
     * @param format format to apply.
     * @return `true` on success.
     *
     * See Cell::Type on the inline and shared string types.
     */
    bool writeString(int row, int column, const RichString &value, const Format &format=Format());
    /**
     * @brief writes @a value into an inline string cell @a ref and applies @a format.
     * @param ref cell reference to write data to.
     * @param value data to write.
     * @param format format to apply.
     * @return `true` on success.
     *
     * See Cell::Type on the inline and shared string types.
     */
    bool writeInlineString(const CellReference &ref, const QString &value, const Format &format=Format());
    /**
     * @overload
     * @brief writes @a value into an inline string cell (@a row, @a column) and applies @a format.
     * @param row cell row index (starting from 1).
     * @param column cell column index (starting from 1).
     * @param value data to write.
     * @param format format to apply.
     * @return `true` on success.
     *
     * See Cell::Type on the inline and shared string types.
     */
    bool writeInlineString(int row, int column, const QString &value, const Format &format=Format());
    /**
     * @brief writes a numeric @a value into @a ref and applies @a format.
     * @param ref cell reference to write data to.
     * @param value data to write.
     * @param format format to apply.
     * @return `true` on success.
     */
    bool writeNumeric(const CellReference &ref, double value, const Format &format=Format());
    /**
     * @overload
     * @brief writes a numeric @a value into cell (@a row, @a column) and applies @a format.
     * @param row cell row index (starting from 1).
     * @param column cell column index (starting from 1).
     * @param value data to write.
     * @param format format to apply.
     * @return `true` on success.
     */
    bool writeNumeric(int row, int column, double value, const Format &format=Format());
    /**
     * @brief writes a @a formula (with optional computational @a result) into
     * @a ref and applies @a format.
     * @param ref cell reference to write data to.
     * @param formula formula to write.
     * @param format format to apply.
     * @param result optional result of @a formula computation, provided purely
     * as a cashed value (may at any time be overwritten in the nest computational cycle).
     * @return `true` on success.
     */
    bool writeFormula(const CellReference &ref, const CellFormula &formula, const Format &format=Format(), double result=0);
    /**
     * @overload
     * @brief writes a @a formula (with optional computational @a result) into
     * cell (@a row, @a column) and applies @a format.
     * @param row cell row index (starting from 1).
     * @param column cell column index (starting from 1).
     * @param formula formula to write.
     * @param format format to apply.
     * @param result optional result of @a formula computation, provided purely
     * as a cashed value (may at any time be overwritten in the nest computational cycle).
     * @return `true` on success.
     */
    bool writeFormula(int row, int column, const CellFormula &formula, const Format &format=Format(), double result=0);
    /**
     * @brief creates an empty (blank) cell at @a ref and applies @a format to it.
     * @param ref cell reference to apply @a format to.
     * @param format format to apply.
     * @return `true` on success.
     */
    bool writeBlank(const CellReference &ref, const Format &format=Format());
    /**
     * @overload
     * @brief creates an empty (blank) cell (@a row, @a column) and applies @a format to it.
     * @param row row index (starting from 1).
     * @param column column index (starting from 1).
     * @param format to apply.
     * @return `true` on success.
     */
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


    /// Data Validation methods
    /**
     * @brief adds data validation in the sheet.
     * @param validation A valid DataValidation object.
     * @return `true` if @a validation is valid and contains valid range(s),
     * `false` otherwise.
     */
    bool addDataValidation(const DataValidation &validation);
    /**
     * @overload
     * @brief adds data validation in the sheet.
     *
     * This is a convenience method that allows to create a validation from
     * scratch.
     *
     * @param range cells range to apply data validation to.
     * @param type type of data validation
     * @param formula1 the first validation criterion
     * @param predicate operation to combine two validation criteria
     * @param formula2 the second validation criterion.
     * @param strict If `true`, then error message will be shown if the input
     * cell value is not allowable.
     * @return `true` if validation was successfully added, `false` otherwise.
     *
     * You can use overloaded methods to add a specific type of validation.
     */
    bool addDataValidation(const CellRange &range, DataValidation::Type type, const QString &formula1,
                           std::optional<DataValidation::Predicate> predicate = std::nullopt,
                           const QString &formula2 = QString(), bool strict = true);
    /**
     * @overload
     * @brief adds the data validation that allows only a list of specified
     * values.
     * @param range cells range to apply data validation to.
     * @param allowableValues cells range that contains all allowable values.
     * @param strict If `true`, then error message will be shown if the input
     * cell value is not allowable.
     * @return `true` if validation was successfully added, `false` otherwise.
     * @note If you need to customize validation params, f.e. prompt and error
     * messages, use DataValidation constructor.
     */
    bool addDataValidation(const CellRange &range, const CellRange &allowableValues, bool strict = true);
    /**
     * @overload
     * @brief adds the data validation that validates time values.
     * @param range cells range to apply data validation to.
     * @param time1 first time value.
     * @param predicate operation to combine two validation criteria.
     * @param time2 second time value.
     * @param strict If `true`, then error message will be shown if the input
     * cell value is not allowable.
     * @return `true` if validation was successfully added, `false` otherwise.
     * @note If you need to customize validation params, f.e. prompt and error
     * messages, use DataValidation constructor.
     */
    bool addDataValidation(const CellRange &range, const QTime &time1,
                           std::optional<DataValidation::Predicate> predicate = std::nullopt,
                           const QTime &time2 = QTime(), bool strict = true);
    /**
     * @overload
     * @brief adds the data validation that validates date values.
     * @param range cells range to apply data validation to.
     * @param date1 first date value.
     * @param predicate operation to combine two validation criteria.
     * @param date2 second date value.
     * @param strict If `true`, then error message will be shown if the input
     * cell value is not allowable.
     * @return `true` if validation was successfully added, `false` otherwise.
     * @note If you need to customize validation params, f.e. prompt and error
     * messages, use DataValidation constructor.
     */
    bool addDataValidation(const CellRange &range, const QDate &date1,
                           std::optional<DataValidation::Predicate> predicate = std::nullopt,
                           const QDate &date2 = QDate(), bool strict = true);
    /**
     * @overload
     * @brief adds data validation that validates text length.
     * @param range cells range to apply data validation to.
     * @param len1 First text length
     * @param predicate operation to combine two validation criteria.
     * @param len2 Second text length.
     * @param strict If `true`, then error message will be shown if the input
     * cell value is not allowable.
     * @return `true` if validation was successfully added, `false` otherwise.
     * @note If you need to customize validation params, f.e. prompt and error
     * messages, use DataValidation constructor.
     */
    bool addDataValidation(const CellRange &range, int len1,
                           std::optional<DataValidation::Predicate> predicate = std::nullopt,
                           std::optional<int> len2 = std::nullopt, bool strict = true);
    /**
     * @brief returns whether all input prompts from the worksheet are disabled.
     * @return if `true`, then all data validation prompts will not be shown on
     * the worksheet.
     *
     * The default value is `false`.
     */
    std::optional<bool> dataValidationPromptsDisabled() const;
    /**
     * @brief sets whether all input prompts from the worksheet are disabled.
     * @param disabled if `true`, then all data validation prompts will not be
     * shown on the worksheet.
     *
     * If not set, `false` is assumed.
     */
    void setDataValidationPromptsDisabled(bool disabled);
    /**
     * @brief removes all data validation that was added to the worksheet.
     */
    void clearDataValidation();
    /**
     * @brief returns whether the worksheet has any data validation set.
     * @return `true` if the worksheet has any data validation set.
     */
    bool hasDataValidation() const;
    /**
     * @brief returns the list of data validation objects added to the
     * worksheet.
     */
    QList<DataValidation> dataValidationRules() const;
    /**
     * @brief returns the data validation object with @a index.
     * @param index valid index from 0 to #dataValidationsCount()-1.
     * @return A copy of DataValidation object if @a index is valid, a
     * default-constructed (invalid) DataValidation object otherwise.
     */
    DataValidation dataValidation(int index) const;
    /**
     * @brief returns the data validation object with @a index.
     * @param index valid index from 0 to #dataValidationsCount()-1.
     * @return A reference to the DataValidation object.
     * @warning If @a index is invalid, the result is undefined. The reference
     * may become invalidated if you add or remove validation rules.
     */
    DataValidation &dataValidation(int index);
    /**
     * @brief removes the data validation from the worksheet.
     * @param index valid index from 0 to #dataValidationsCount()-1.
     * @return `true` if data validation was successfully removed.
     */
    bool removeDataValidation(int index);
    /**
     * @brief returns the count of data validations added to the worksheet.
     */
    int dataValidationsCount() const;


    /// Conditional formatting

    /**
     * @brief removes all conditional formatting rules from the worksheet.
     */
    void clearConditionalformatting();
    /**
     * @brief returns whether the worksheet has conditional formatting set.
     * @return `true` if #conditionalFormattingCount() > 0.
     */
    bool hasConditionalFormatting() const;
    /**
     * @brief returns the count of conditional formatting rules added to the
     * worksheet.
     */
    int conditionalFormattingCount() const;
    /**
     * @brief returns the list of conditional formatting rules added to the
     * worksheet.
     */
    QList<ConditionalFormatting> conditionalFormattingRules() const;
    /**
     * @brief returns the conditional formatting rule at @a index.
     * @param index valid index from 0 to #conditionalFormattingCount()-1.
     * @return a copy of the ConditionalFormatting object if the index is valid,
     * a default-constructed (invalid) ConditionalFormatting object otherwise.
     */
    ConditionalFormatting conditionalFormatting(int index) const;
    /**
     * @brief returns the conditional formatting rule at @a index.
     * @param index valid index from 0 to #conditionalFormattingCount()-1.
     * @return a reference to the conditional formatting rule at @a index.
     * @warning If @a index is invalid, the behavior isundefined.
     */
    ConditionalFormatting &conditionalFormatting(int index);
    /**
     * @brief removes the conditional formatting rule at @a index.
     * @param index valid index from 0 to #conditionalFormattingCount()-1.
     * @return `true` if @a index is valid and the rule was removed, `false`
     * otherwise.
     */
    bool removeConditionalFormatting(int index);
    /**
     * @brief adds the conditional formatting to the worksheet.
     * @param cf ConditionalFormatting object that contains the conditional
     * formating parameters.
     * @return `true` on success.
     */
    bool addConditionalFormatting(const ConditionalFormatting &cf);

    /// Direct manipulation of cells

    /**
     * @brief returns a cell by its reference.
     * @param ref reference to the cell.
     * @return valid pointer to the cell if the cell was found, `nullptr`
     * otherwise.
     *
     * If no data or format were written into @a ref, this method returns
     * `nullptr`. Use #write() methods to implicitly create a cell.
     */
    Cell *cell(const CellReference &ref) const;
    /**
     * @overload
     * @brief returns cell by its row and column number.
     * @param row 1-based cell row number.
     * @param column 1-based cell column number.
     * @return valid pointer to the cell if the cell was found, `nullptr`
     * otherwise.
     *
     * If no data or format were written into (@a row, @a column), this method
     * returns `nullptr`. Use #write() methods to implicitly create a cell.
     */
    Cell *cell(int row, int column) const;

    /**
     * @brief Inserts an @a image at the position @a ref.
     * @param ref reference to the the image top left corner.
     * @param image Image to be inserted.
     * @return the zero-based index of the newly inserted image on success, -1
     * otherwise.
     */

    /// Images

    int insertImage(const CellReference &ref, const QImage &image);
    /**
     * @overload
     * @brief Inserts an @a image at the position @a row, @a column.
     * @param row the 1-based row index of the image top left corner.
     * @param column the 1-based column index of the image top left corner.
     * @param image Image to be inserted.
     * @return the zero-based index of the newly inserted image on success, -1 otherwise.
     */
    int insertImage(int row, int column, const QImage &image);
    /**
     * @brief returns the image by its index.
     * @param index zero-based image index (0 to #imagesCount()-1).
     * @return non-null QImage if image was found and read.
     */
    QImage image(int index) const;
    /**
     * @overload
     * @brief returns the image by its top left corner.
     * @param row the 1-based row index of the image top left corner.
     * @param column the 1-based column index of the image top left corner.
     * @return non-null QImage if image was found and read.
     */
    QImage image(int row, int column) const;
    /**
     * @brief returns the count of images on this worksheet.
     */
    int imagesCount() const;
    /**
     * @brief removes the image from the worksheet
     * @param row the 1-based row index of the image top left corner.
     * @param column the 1-based column index of the image top left corner.
     * @return `true` if image was found and removed, `false` otherwise.
     */
    bool removeImage(int row, int column);
    /**
     * @overload
     * @brief removes image from the worksheet
     * @param index zero-based index of the image (0 to #imagesCount()-1).
     * @return `true` if image was found and removed, `false` otherwise.
     */
    bool removeImage(int index);
    /**
     * @brief sets the new image content to the image specified by @a index.
     * @param index zero-based index of the image (0 to #imagesCount()-1).
     * @param fileName file name which to load new content from.
     * @param keepSize if `true`, then new image will be resized to the old image size.
     * If `false`, then new image will retain its size.
     * @return `true` if the image was found and file was loaded, `false` otherwise.
     */
    bool changeImage(int index, const QString &fileName, bool keepSize = true);


    /// Charts

    /**
     * @brief creates a new chart and places it inside the current worksheet.
     * @param row the 1-based row index of the chart top left corner.
     * @param column the 1-based column index of the chart top left corner.
     * @param size the chart size in pixels at 96 dpi.
     * @return pointer to the new chart or `nullptr` if no chart was created.
     */
    Chart *insertChart(int row, int column, const QSize &size);
    /**
     * @overload
     * @brief creates a new chart and places it inside the current worksheet.
     * @param row the 1-based row index of the chart top left corner.
     * @param column the 1-based column index of the chart top left corner.
     * @param width width of a chart specified as a Coordinate object. You can use it to set
     * width in pixels, points, millimeters, EMU etc. See Coordinate for help.
     * @param height height of a chart specified as a Coordinate object. You can use it to set
     * height in pixels, points, millimeters, EMU etc. See Coordinate for help.
     * @return pointer to the new chart or `nullptr` if no chart was created.
     */
    Chart *insertChart(int row, int column, Coordinate width, Coordinate height);
    /**
     * @brief returns the chart by its index in the worksheet.
     * @param index zero-based index of the chart (0 to #chartsCount()-1)
     * @return valid pointer to the chart if the chart was found, `nullptr` otherwise.
     */
    Chart *chart(int index) const;
    /**
     * @overload
     * @brief returns the chart by its top left corner.
     * @param row the 1-based row index of the chart top left corner.
     * @param column the 1-based column index of the chart top left corner.
     * @return valid pointer to the chart if the chart was found, `nullptr` otherwise.
     */
    Chart *chart(int row, int column) const;
    /**
     * @brief removes the chart from the worksheet.
     * @param row the 1-based row index of the chart top left corner.
     * @param column the 1-based column index of the chart top left corner.
     * @return `true` if the chart was found and successfully removed, `false` otherwise.
     */
    bool removeChart(int row, int column);
    /**
     * @overload
     * @brief removes the chart from the worksheet.
     * @param index zero-based index of the chart.
     * @return `true` if the chart was found and successfully removed, `false` otherwise.
     */
    bool removeChart(int index);
    /**
     * @brief returns count of charts on the worksheet.
     */
    int chartsCount() const;
    /**
     * @brief Merges a @a range of cells.
     *
     * The first cell will retain its data, other cells will be cleared.
     * All cells will get the same @a format if a valid @a format is given.
     * @return `true` on success.
     */


    /// Rows, columns and cells

    bool mergeCells(const CellRange &range, const Format &format=Format());
    /**
     * @brief Un-merges a @a range of cells.
     * @param range the range to unmerge.
     * @return `true` if @a range was successfully unmerged.
     * @note This method does not check the merged cells range for intersections with
     * other merged cells.
     */
    bool unmergeCells(const CellRange &range);
    /**
     * @brief returns the list of merged ranges.
     * @return non-empty list of merged ranges if any of cells were merged.
     */
    QList<CellRange> mergedCells() const;

    bool setColumnWidth(const CellRange& range, double width);
    bool setColumnFormat(const CellRange& range, const Format &format);
    bool setColumnHidden(const CellRange& range, bool hidden);
    bool setColumnWidth(int columnFirst, int columnLast, double width);
    bool setColumnWidth(int column, double width);
    bool setColumnFormat(int columnFirst, int columnLast, const Format &format);
    bool setColumnFormat(int column, const Format &format);
    bool setColumnHidden(int columnFirst, int columnLast, bool hidden);
    bool setColumnHidden(int column, bool hidden);
    /**
     * @brief returns the width of @a column in characters.
     * @param column column index (1-based).
     * @return the width of @a column in characters of a standard font, if it was set/changed.
     * @note The default column width can be obtained with #defaultColumnWidth().
     */
    std::optional<double> columnWidth(int column) const;
    /**
     * @brief returns the format of @a column
     * @param column column index (1-based).
     * @return A copy of the column format.
     */
    Format columnFormat(int column) const;
    /**
     * @brief returns whether the column is hidden
     * @param column column index (1-based).
     * @return valid bool value if column visibility was changed, `nullopt` otherwise.
     *
     * If the parameter was not set, `false` (not hidden) is assumed.
     */
    std::optional<bool> isColumnHidden(int column) const;

    /**
     * @brief sets rows heights for rows in the range from @a rowFirst to @a rowLast.
     * @param rowFirst the first row index (1-based).
     * @param rowLast the last row index (1-based).
     * @param height row height in points.
     * @return `true` on success.
     */
    bool setRowHeight(int rowFirst, int rowLast, double height);
    /**
     * @overload
     * @brief sets row height for @a row.
     * @param row the row index (1-based).
     * @param height row height in points.
     * @return `true` on success.
     */
    bool setRowHeight(int row, double height);
    /**
     * @brief sets rows format for rows in the range from @a rowFirst to @a rowLast.
     * @param rowFirst  the first row index (1-based).
     * @param rowLast the last row index (1-based).
     * @param format row format.
     * @return `true` on success
     */
    bool setRowFormat(int rowFirst, int rowLast, const Format &format);
    /**
     * @overload
     * @brief sets row format for @a row.
     * @param row the row index (1-based).
     * @param format row format
     * @return `true` on success.
     */
    bool setRowFormat(int row, const Format &format);
    /**
     * @brief sets @a hidden to rows from @a rowFirst to @a rowLast.
     * @param rowFirst The first row index (1-based) of the range to set @a hidden.
     * @param rowLast The last row index (1-based) of the range to set @a hidden.
     * @param hidden visibility of rows.
     * @return `true` if @a rowFirst and @a rowLast are valid and the visibility is successfully changed.
     */
    bool setRowHidden(int rowFirst, int rowLast, bool hidden);
    /**
     * @brief returns the row height in points.
     * @param row the row index (1-based).
     * @return valid double value if the row height was set/changed, `nullopt` otherwise.
     * @note The default row height can be obtained with #defaultRowHeight().
     */
    std::optional<double> rowHeight(int row) const;
    /**
     * @brief returns the row format.
     * @param row the row index (1-based).
     * @return a copy of the row format object.
     */
    Format rowFormat(int row) const;
    /**
     * @brief returns whether the row is hidden.
     * @param row the row index (1-based).
     * @return a valid bool value if the row visibility was changed, `nullopt` otherwise.
     *
     * If the row visibility was not set, `false` (not hidden) is assumed.
     */
    std::optional<bool> isRowHidden(int row) const;

    bool groupRows(int rowFirst, int rowLast, bool collapsed = true);
    /**
     * @brief groups columns from @a colFirst to @a colLast and collapse them as specified.
     * @param colFirst 1-based index of the first column.
     * @param colLast 1-based index of the last column.
     * @param collapsed if `true`, columns will be collapsed.
     * @return `true` if the maximum level of grouping is not exceeded and the grouping is successful,
     * `false` otherwise.
     *
     * The "group/ungroup" button will appear over the column with index colLast+1.
     */
    bool groupColumns(int colFirst, int colLast, bool collapsed = true);
    /**
     * @overload
     * @brief groups columns from @a range and collapse them as specified.
     * @param range range of columns to group.
     * @param collapsed
     * @return `true` if the maximum level of grouping is not exceeded and the grouping is successful,
     * `false` otherwise.
     *
     * The "group/ungroup" button will appear over the column with index colLast+1.
     */
    bool groupColumns(const CellRange &range, bool collapsed = true);
    /**
     * @brief returns the dimension (extent) of the worksheet.
     * @return dimension (extent) of the worksheet as a CellRange object.
     */
    CellRange dimension() const;

    /**
     * @brief autosizes columns widths for all rows in the worksheet.
     * @param firstColumn 1-based index of the first column to autosize.
     * @param lastColumn 1-based index of the last column to autosize.
     * @return `true` on success.
     */
    bool autosizeColumnsWidth(int firstColumn, int lastColumn);
    /**
     * @overload
     * @brief autosizes columns widths for columns specified by range.
     * @param range valid CellRange that specifies the columns range to autosize and
     * the rows range to estimate the maximum column width.
     * @return `true` on success.
     */
    bool autosizeColumnsWidth(const CellRange &range);
    /**
     * @brief autosizes all columns width.
     * @return `true` on success.
     */
    bool autosizeColumnsWidth();
    /**
     * @brief autosizes column width for @a column.
     * @param column 1-based index of the column to autosize.
     * @return `true` on success.
     */
    bool autosizeColumnWidth(int column);

    /**
     * @brief returns  whether the application should do a full calculate on
     * load due to contents on this sheet.
     *
     * After load and successful calc, the application shall set this value to `false`.
     * Set this to `true` when the application should calculate the workbook on load.
     * The default value is `false`.
     */
    std::optional<bool> fullCalculationOnLoad() const;
    /**
     * @brief sets  whether the application should do a full calculate on
     * load due to contents on this sheet.
     *
     * Set this to `true` when the application should calculate the workbook on load.
     * If the parameter is not set, `false` is assumed.
     * @param value If `true`, then the application will calculate the workbook on load.
     */
    void setFullCalculationOnLoad(bool value);


    // Sheet Parameters

    /**
     * @brief returns whether to apply styles in the outline when an outline is
     * applied.
     *
     * The default value is `false`
     */
    std::optional<bool> applyStylesInOutline() const;
    /**
     * @brief sets whether to apply styles in the outline when an outline is
     * applied.
     * @param apply If `true` then cell style will be applied to the outline.
     *
     * If not set, `false` is assumed.
     */
    void setApplyStylesInOutline(bool apply);
    /**
     * @brief returns whether summary rows appear below detail in an outline,
     * when applying an outline.
     *
     * When `true`, a summary row is inserted below the detailed data being
     * summarized and a new outline level is established on that row.
     *
     * When `false`, a summary row is inserted above the detailed data being
     * summarized and a new outline level is established on that row.
     *
     * The default value is `true`.
     */
    std::optional<bool> outlineSummaryBelow();
    /**
     * @brief sets whether summary rows appear below detail in an outline,
     * when applying an outline.
     * @param below  When `true`, a summary row is inserted below the detailed
     * data being summarized and a new outline level is established on that row.
     * When `false`, a summary row is inserted above the detailed data being
     * summarized and a new outline level is established on that row.
     *
     * If not set, `true` is assumed.
     */
    void setOutlineSummaryBelow(bool below);
    /**
     * @brief returns whether summary columns appear to the right of detail in
     * an outline, when applying an outline.
     *
     * When `true`, a summary column is inserted to the right of the detailed
     * data being summarized and a new outline level is established on that
     * column.
     *
     * When `false`, a summary column is inserted to the left of the detailed
     * data being summarized and a new outline level is established on that
     * column.
     *
     * The default value is `true`.
     */
    std::optional<bool> outlineSummaryRight();
    /**
     * @brief sets whether summary columns appear to the right of detail in
     * an outline, when applying an outline.
     * @param right When `true`, a summary column is inserted to the right of
     * the detailed data being summarized and a new outline level is established
     * on that column.
     * When `false`, a summary column is inserted to the left of the detailed
     * data being summarized and a new outline level is established on that
     * column.
     *
     * If not set, `true` is assumed.
     */
    void setOutlineSummaryRight(bool right);
    /**
     * @brief returns whether the conditional formatting calculations shall be
     * evaluated. If set to `false`, then the min/max values of color scales or
     * databars or threshold values in Top N rules shall not be updated.
     *
     * This is useful when conditional formats are being set programmatically at
     * runtime, recalculation of the conditional formatting does not need to be
     * done until the program execution has finished setting all the conditional
     * formatting properties.
     *
     * If not set, the default value is `true`.
     *
     * @return `true` if the recalculation of the conditional formatting is on.
     */
    std::optional<bool> isFormatConditionsCalculationEnabled() const;
    /**
     * @brief sets whether the conditional formatting calculations shall be
     * evaluated.
     * @param enabled If `false`, then the min/max values of color scales or
     * databars or threshold values in Top N rules shall not be updated.
     *
     * This is useful when conditional formats are being set programmatically at
     * runtime, recalculation of the conditional formatting does not need to be
     * done until the program execution has finished setting all the conditional
     * formatting properties.
     *
     * The default value is `true`.
     */
    void setFormatConditionsCalculationEnabled(bool enabled);
    /**
     * @brief returns whether the worksheet is horizontally synced to the #topLeftAnchor().
     * @return If `true`, and scroll location is missing from the window properties,
     * the window view shall be scrolled to the #topLeftAnchor() column.
     *
     * If not set, the default value is `false`.
     */
    std::optional<bool> isSyncedHorizontal() const;
    /**
     * @brief sets whether the worksheet is horizontally synced to the #topLeftAnchor().
     * @param sync If `true`, and scroll location is missing from the window properties,
     * the window view shall be scrolled to the #topLeftAnchor() column.
     *
     * The default value is `false`.
     */
    void setSyncedHorizontal(bool sync);
    /**
     * @brief returns whether the worksheet is vertically synced to the #topLeftAnchor().
     * @return If `true`, and scroll location is missing from the window properties,
     * the window view shall be scrolled to the #topLeftAnchor() row.
     *
     * If not set, the default value is `false`.
     */
    std::optional<bool> isSyncedVertical() const;
    /**
     * @brief sets whether the worksheet is vertically synced to the #topLeftAnchor().
     * @param sync If `true`, and scroll location is missing from the window properties,
     * the window view shall be scrolled to the #topLeftAnchor() row.
     *
     * The default value is `false`.
     */
    void setSyncedVertical(bool sync);
    /**
     * @brief returns the anchor cell the window view shall be scrolled to according
     * to the #isSyncedHorizontal() and #isSyncedVertical() parameters.
     * @return valid CellReference if the anchor cell was set.
     */
    CellReference topLeftAnchor() const;
    /**
     * @brief sets the anchor cell the window view shall be scrolled to according
     * to the #isSyncedHorizontal() and #isSyncedVertical() parameters.
     * @param ref If not valid, clears the anchor cell.
     */
    void setTopLeftAnchor(const CellReference &ref);
    /**
     * @brief returns  whether the sheet displays Automatic Page Breaks.
     * @return automatic page breaks visibility.
     *
     * If not set, the defalult value is `true`.
     */
    std::optional<bool> showAutoPageBreaks() const;
    /**
     * @brief sets whether the sheet displays Automatic Page Breaks.
     * @param show automatic page breaks visibility.
     *
     * The default value is `true`.
     */
    void setShowAutoPageBreaks(bool show);
    /**
     * @brief returns whether the Fit to Page print option is enabled.
     * @return the Fit to Page print option.
     *
     * If not set, the default value is `false`.
     */
    std::optional<bool> fitToPage() const;
    /**
     * @brief sets the Fit to Page print option.
     * @param value If `true`, then the sheet's contents will be scaled to fit the page.
     *
     * The default value is `false`.
     */
    void setFitToPage(bool value);

    // Sheet Format Parameters

    /**
     * @brief returns whether rows have thick bottom borders by default.
     * @return `true` if rows have thick bottom borders by default.
     *
     * The default value is `false`.
     */
    std::optional<bool> thickBottomBorder() const;
    /**
     * @brief sets whether rows have thick bottom borders by default.
     * @param thick if `true`, then rows will have thick bottom borders by default.
     *
     * The default value is `false`.
     */
    void setThickBottomBorder(bool thick);
    /**
     * @brief returns whether rows have thick top borders by default.
     * @return `true` if rows have thick top borders by default.
     *
     * The default value is `false`.
     */
    std::optional<bool> thickTopBorder() const;
    /**
     * @brief sets whether rows have thick top borders by default.
     * @param thick if `true`, then rows will have thick top borders by default.
     *
     * The default value is `false`.
     */
    void setThickTopBorder(bool thick);

    /**
     * @brief returns whether rows are hidden by default.
     * @return `true` if all rows are hidden by default.
     *
     * The default value is `false`.
     *
     * This parameter is useful if it is much shorter to only write out the rows
     * that are not hidden with #setRowHidden().
     */
    std::optional<bool> rowsHiddenByDefault() const;
    /**
     * @brief sets whether rows are hidden by default.
     * @param hidden if `true`, then all rows are hidden by default.
     *
     * The default value is `false`.
     *
     * This parameter is useful if it is much shorter to only write out the rows
     * that are not hidden with #setRowHidden().
     */
    void setRowsHiddenByDefault(bool hidden);
    /**
     * @brief returns the default row height set in the worksheet.
     * @return the default row height in points.
     *
     * The default value is 14.4 (pt).
     */
    double defaultRowHeight() const;
    /**
     * @brief sets the default row height.
     * @param height the default row height in points.
     *
     * The default value is 14.4 (pt).
     *
     * This parameter is useful if most of the rows in a sheet have the same
     * height, but that height isn't the default height (14.4 pt)
     */
    void setDefaultRowHeight(double height);
    /**
     * @brief returns the default column width.
     * @return the default column width in characters of the widest digit of the normal style's font.
     *
     * The default value is 8 5/7.
     */
    double defaultColumnWidth() const;
    /**
     * @brief sets the default column width.
     * @param width value measured in characters of the widest digit of the normal style's font.
     *
     * The default value is 8 5/7.
     */
    void setDefaultColumnWidth(double width);

    ////////////////////////////////////////////////////////////////////////////
    ///  Sheet View parameters                                               ///
    ////////////////////////////////////////////////////////////////////////////

    /**
     * @brief returns whether the panes in the window are locked due to workbook
     * protection.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     *
     * The default value is `false`.
     */
    std::optional<bool> isWindowProtected(int viewIndex = 0) const;
    /**
     * @brief sets whether the panes in the window are locked due to workbook
     * protection.
     *
     * @param protect Sets protected to the view. The default value is `false`.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     */
    void setWindowProtected(bool protect, int viewIndex = 0);
    /**
     * @brief returns whether formulas in the cells are displayed.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return `true` if formulas in the cells are displayed 'as is'. `false`
     * if the computed values are displayed. `nullopt` if the parameter is not
     * set.
     *
     * The default value is `false`.
     */
    std::optional<bool> isFormulasVisible(int viewIndex = 0) const;
    /**
     * @brief sets the visibility of formulas in the cells.
     * @param visible If `true`, then formulas in the cells are displayed 'as
     * is'. If `false`, then the computed values are displayed.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     *
     * If not set, `false` is assumed.
     */
    void setFormulasVisible(bool visible, int viewIndex = 0);
    /**
     * @brief returns whether the sheet grid lines are shown.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return `true` if the grid lines are shown.
     *
     * The default value is 'true'.
     */
    std::optional<bool> isGridLinesVisible(int viewIndex = 0) const;
    /**
     * @brief sets whether the sheet grid lines are shown.
     * @param visible If `true` then grid lines are shown. If `false`, then grid
     * lines are hidden.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     *
     * If not set, `true` is assumed.
     */
    void setGridLinesVisible(bool visible, int viewIndex = 0);
    /**
     * @brief returns whether the row and column headers are shown.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return valid bool if the parameter was set, `nullopt` otherwise.
     *
     * The default value is `true`.
     */
    std::optional<bool> isRowColumnHeadersVisible(int viewIndex = 0) const;
    /**
     * @brief eturns whether the row and column headers are shown.
     * @param visible If `true` then the row and column headers are shown.
     * If `false` then the row and column headers are hidden.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     *
     * If not set, `true` is assumed.
     */
    void setRowColumnHeadersVisible(bool visible, int viewIndex = 0);
    /**
     * @brief returns whether the window should show 0 (zero) in cells
     * containing zero value.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return `true` if zeroes are displayed as is, `false` if cells with zero
     * value appear blank, `nullopt` if the parameter is not set.
     *
     * The default value is `true`.
     */
    std::optional<bool> isZerosVisible(int viewIndex = 0) const;
    /**
     * @brief sets whether the window should show 0 (zero) in cells
     * containing zero value.
     * @param visible if `true`, cells containing zero value shall display 0. If
     * `false`, cells containing zero value shall appear blank.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     *
     * If not set, the default value is `true`.
     */
    void setZerosVisible(bool visible, int viewIndex = 0);
    /**
     * @brief returns whether the sheet is in 'right to left' display mode.
     *
     * When in this mode, Column A is on the far right, Column B is one column
     * left of Column A, and so on. Also, information in cells is displayed in
     * the Right to Left format.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return `true` if the sheet is in 'right to left' display mode.
     *
     * The default value is `false`.
     */
    std::optional<bool> isRightToLeft(int viewIndex = 0) const;
    /**
     * @brief sets whether the sheet is in 'right to left' display mode.
     *
     * @param enable if `true`, Column A is on the far right, Column B is one
     * column left of Column A, and so on. Also, information in cells is
     * displayed in the right-to-left format.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     *
     * If not set, `false` is assumed.
     */
    void setRightToLeft(bool enable, int viewIndex = 0);
    /**
     * @brief returns whether ruler is visible in the worksheet.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return if `true`, then the ruler is visible.
     *
     * The default value is `false`.
     */
    std::optional<bool> isRulerVisible(int viewIndex = 0) const;
    /**
     * @brief sets whether ruler is visible in the worksheet.
     * @param visible if `true`, then the ruler is visible.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     *
     * If not set, `false` is assumed.
     */
    void setRulerVisible(bool visible, int viewIndex = 0);
    /**
     * @brief returns whether the outline symbols are visible.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return if `true`, then the outline symbols are visible.
     *
     * The default value is `true`.
     */
    std::optional<bool> isOutlineSymbolsVisible(int viewIndex = 0) const;
    /**
     * @brief sets whether the outline symbols are visible.
     * @param visible if `true`, then the outline symbols are visible.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     *
     * If not set, `true` is assumed.
     */
    void setOutlineSymbolsVisible(bool visible, int viewIndex = 0);
    /**
     * @brief returns whether page layout view shall display margins.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return `false` means do not display left, right, top (header), and
     * bottom (footer) margins (even when there is data in the header or
     * footer).
     *
     * The default value is `true`.
     */
    std::optional<bool> isPageMarginsVisible(int viewIndex = 0) const;
    /**
     * @brief sets whether page layout view shall display margins.
     * @param visible `false` means do not display left, right, top (header),
     * and bottom (footer) margins (even when there is data in the header or
     * footer).
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     *
     * If not set, `true` is assumed.
     */
    void setPageMarginsVisible(bool visible, int viewIndex = 0);
    /**
     * @brief returns whether the application uses the default grid
     * lines color (system dependent).
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return If `false`, then #viewColorIndex() is used to set the grid color.
     *
     * If not set, the default value is `true`.
     */
    std::optional<bool> isDefaultGridColorUsed(int viewIndex = 0) const;
    /**
     * @brief sets whether the application uses the default grid lines color
     * (system dependent).
     * @param value If `false`, then #viewColorIndex() is used to set the grid
     * color. if `true`, then overrides any color specified in
     * #setViewColorIndex().
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     *
     * If not set, `true` is assumed.
     */
    void setDefaultGridColorUsed(bool value, int viewIndex = 0);
    /**
     * @brief returns the type of a view.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return SheetView::Type enum value.
     *
     * The default value is SheetView::Type::Normal.
     */
    std::optional<SheetView::Type> viewType(int viewIndex = 0) const;
    /**
     * @brief sets the type of the view.
     * @param type One of SheetView::Type enum values.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     *
     * If not set, SheetView::Type::Normal is assumed.
     */
    void setViewType(SheetView::Type type, int viewIndex = 0);
    /**
     * @brief returns the location of the view's top left visible cell.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return Valid location if it was set, invalid one otherwise.
     *
     * The default value is invalid CellReference, that means A1 is used as the
     * top left visible cell.
     */
    CellReference viewTopLeftCell(int viewIndex = 0) const;
    /**
     * @brief sets the location of the view's top left visible cell.
     * @param ref valid CellReference.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     *
     * If not set, invalid CellReference is assumed, that means A1 is used as
     * the top left visible cell.
     */
    void setViewTopLeftCell(const CellReference &ref, int viewIndex = 0);
    /**
     * @brief returns the index to the color value for row/column text headings
     * and gridlines.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return an 'index color value' (ICV) rather than rgb value.
     *
     * If not set, the default value is 64.
     *
     * See #setViewColorIndex() for a list of colors.
     */
    std::optional<int> viewColorIndex(int viewIndex = 0) const;
    /**
     * @brief sets the index to the color value for row/column text headings
     * and gridlines.
     * @param index an 'index color value' from 0 to 64.
     *
     * ICV | color & style
     * ----|----
     * 0 or 8 | black 000000
     * 1 or 9 | white FFFFFF
     * 2 or 10 | red FF0000
     * 3 or 11 | green 00FF00
     * 4 or 12 or 39 | blue 0000FF
     * 5 or 13 or 34 | yellow FFFF00
     * 6 or 14 or 33 | magenta (fuchsia) FF00FF
     * 7 or 15 or 35 | cyan 00FFFF
     * 16 or 37 | dark red 800000
     * 17 | dark green 008000
     * 18 or 32 | dark blue (navy) 000080
     * 19 | dark yellow 808000
     * 20 or 36 | dark magenta (purple) 800080
     * 21 or 38 | dark cyan (teal) 008080
     * 22 | gray 25% C0C0C0
     * 23 | gray 50% 808080
     * 24 | 9999FF
     * 25 | 993366
     * 26 | ivory FFFFCC
     * 27 or 41 | light cyan CCFFFF
     * 28 | 660066
     * 29 | FF8080
     * 30 | 0066CC
     * 31 | CCCCFF
     * 40 | 00CCFF
     * 42 | CCFFCC
     * 43 | FFFF99
     * 44 | 99CCFF
     * 45 | FF99CC
     * 46 | CC99FF
     * 47 | FFCC99
     * 48 | 3366FF
     * 49 | 33CCCC
     * 50 | 99CC00
     * 51 | FFCC00
     * 52 | FF9900
     * 53 | FF6600
     * 54 | 666699
     * 55 | gray 40% 969696
     * 56 | 003366
     * 57 | 339966
     * 58 | 003300
     * 59 | 333300
     * 60 | 993300
     * 61 | 993366
     * 62 | 333399
     * 63 | dark gray 333333
     * 64 | default
     *
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     *
     * If not set, 64 is assumed.
     */
    void setViewColorIndex(int index, int viewIndex = 0);
    /**
     * @brief returns the last defined sheet view's active cell.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return copy of CellReference object.
     */
    CellReference activeCell(int viewIndex = 0) const;
    /**
     * @brief sets active cell to the sheet view.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @param activeCell Valid CellReference object.
     */
    void setActiveCell(const CellReference &activeCell, int viewIndex = 0);
    /**
     * @brief returns a list of cell ranges selected in the view.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return a list of cell ranges.
     */
    QList<CellRange> selectedRanges(int viewIndex = 0) const;
    /**
     * @brief adds @a range to the list of cell ranges selected in the view.
     * @param range valid cell range.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return `true` if @a range was successfully added, `false` if @a range is
     * invalid or already present in the selection.
     */
    bool addSelection(const CellRange &range, int viewIndex = 0);
    /**
     * @brief removes range from the list of cell ranges selected in the view.
     * @param range cell range to remove.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return `true` if @a range was found and removed, `false` otherwise.
     * @note This method does not check @a range for intersections with
     * selection, it simply searches the selection for the whole range and if
     * found, removes it.
     */
    bool removeSelection(const CellRange &range, int viewIndex = 0);
    /**
     * @brief removes all selection from the view.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     */
    void clearSelection(int viewIndex = 0);
    /**
     * @brief returns selection parameters of the view.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return copy of Selection object.
     */
    Selection selection(int viewIndex = 0) const;
    /**
     * @brief returns selection parameters of the view.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return reference to the Selection object.
     */
    Selection &selection(int viewIndex = 0);
    /**
     * @brief sets selection parameters of the view.
     * @param viewIndex zero-based index of the view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @param selection the Selection object.
     */
    void setSelection(const Selection &selection, int viewIndex = 0);
    /**
     * @brief Returns the pane parameters of a view with @a viewIndex.
     * @param viewIndex zero-based index of a sheet view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return Valid ViewPane if there is a sheet view with @a viewIndex and
     * this sheet view has pane parameters set, invalid ViewPane otherwise.
     */
    ViewPane pane(int viewIndex = 0) const;
    /**
     * @brief Returns the pane parameters of a view with @a viewIndex.
     * @param viewIndex zero-based index of a sheet view (from 0 to
     * AbstractSheet::viewsCount()-1).
     * @return The reference to the ViewPane object.
     *
     * If there's no view with @a viewIndex, the behavior is undefined. It is
     * always safe to address the view with index 0.
     */
    ViewPane &pane(int viewIndex = 0);
    /**
     * @brief sets the pane parameters of a view with @a viewIndex.
     * @param pane A ViewPane object.
     * @param viewIndex zero-based index of a sheet view (from 0 to
     * AbstractSheet::viewsCount()-1).
     *
     * This method can be used to clear the panes in the view:
     *
     * @code
     * worksheet(0)->setPane(ViewPane(), 0);
     * @endcode
     */
    void setPane(const ViewPane &pane, int viewIndex = 0);
    /**
     * @brief splits the sheet view with @a viewIndex horizontally making left
     * and right view panes. The left view pane will have the width of
     * @a columnsCount columns.
     * @param columnsCount How many columns to show in the left pane.
     * @param frozen If `true` then after splitting the left pane is frozen.
     * @param viewIndex Index of a view to split.
     *
     * This is a convenience method. To fine-tune the panes use #pane() and
     * #setPane() method.
     */
    void splitViewHorizontally(int columnsCount, bool frozen, int viewIndex = 0);
    /**
     * @brief splits the sheet view with @a viewIndex vertically making top
     * and bottom view panes. The top view pane will have the height of
     * @a rowsCount rows.
     * @param rowsCount How many rows to show in the top pane.
     * @param frozen If `true` then after splitting the top pane is frozen.
     * @param viewIndex Index of a view to split.
     *
     * This is a convenience method. To fine-tune the panes use #pane() and
     * #setPane() method.
     */
    void splitViewVertically(int rowsCount, bool frozen, int viewIndex = 0);
    /**
     * @brief splits the sheet view with @a viewIndex vertically making 4 view
     * panes. The top view panes will have the height of @a rowsCount,
     * the left view panes will have the width of @a columnsCount.
     * @a rowsCount rows.
     * @param rowsCount How many rows to show in the top panes.
     * @param columnsCount How many columns to show in the left panes.
     * @param frozen If `true` then after splitting all panes except the bottom
     * right one are frozen.
     * @param viewIndex Index of a view to split.
     */
    void splitView(int rowsCount, int columnsCount, bool frozen, int viewIndex = 0);



    /// Print and page parameters valid for worksheets only. Rest of parameters see in AbstractSheet.

    /**
     * @brief sets the worksheet's print scale in percents.
     * @param scale value from 10 to 400. The value of 100 equals 100% scaling.
     *
     * If not set, value of 100 is assumed.
     * @note If #fitToWidth() and/or #fitToHeight() are specified, #printScale() is ignored.
     */
    void setPrintScale(int scale);
    /**
     * @brief returns the worksheet's print scale in percents.
     * @return value from 10 to 400.
     *
     * If not set, value of 100 is assumed.
     */
    std::optional<int> printScale() const;
    /**
     * @brief returns the order of printing the worksheet pages.
     * @return one of PageSetup::PageOrder enum values.
     *
     * The default value is PageSetup::PageOrder::DownThenOver.
     */
    std::optional<PageSetup::PageOrder> pageOrder() const;
    /**
     * @brief sets the order of printing the worksheet pages.
     * @param pageOrder one of PageSetup::PageOrder enum values.
     *
     * If not set, PageSetup::PageOrder::DownThenOver is assumed.
     */
    void setPageOrder(PageSetup::PageOrder pageOrder);
    /**
     * @brief returns the number of horizontal pages to fit on when printing.
     * @return number of pages.
     *
     * The default value is 1.
     */
    std::optional<int> fitToWidth() const;
    /**
     * @brief sets the number of horizontal pages to fit on when printing.
     * @param pages number of pages starting from 1.
     *
     * If not set, 1 is assumed.
     */
    void setFitToWidth(int pages);
    /**
     * @brief returns the number of vertical pages to fit on when printing.
     * @return number of pages.
     *
     * The default value is 1.
     */
    std::optional<int> fitToHeight() const;
    /**
     * @brief sets the number of vertical pages to fit on when printing.
     * @param pages number of pages starting from 1.
     *
     * If not set, 1 is assumed.
     */
    void setFitToHeight(int pages);
    /**
     * @brief returns how to display cells with errors when printing the worksheet.
     * @return A PageSetup::PrintError enum value or `nullopt` if the parameter was not set.
     *
     * The default value is PageSetup::PrintError::Displayed.
     */
    std::optional<PageSetup::PrintError> printErrors() const;
    /**
     * @brief sets how to display cells with errors when printing the worksheet.
     * @param mode A PageSetup::PrintError enum value.
     *
     * If not set, PageSetup::PrintError::Displayed is assumed.
     */
    void setPrintErrors(PageSetup::PrintError mode);
    /**
     * @brief returns how cell comments shall be printed.
     * @return
     *
     * The default value is PageSetup::CellComments::DoNotPrint.
     */
    std::optional<PageSetup::CellComments> printCellComments() const;
    /**
     * @brief sets how cell comments shall be printed.
     * @param mode A PageSetup::CellComments enum value.
     *
     * If not set, PageSetup::CellComments::DoNotPrint is assumed.
     */
    void setPrintCellComments(PageSetup::CellComments mode);
    /**
     * @brief returns whether to print grid lines.
     *
     * The default value is `false` (do not print grid lines).
     */
    std::optional<bool> printGridLines() const;
    /**
     * @brief sets whether to print grid lines.
     * @param printGridLines if `true`, then grid lines are printed.
     *
     * If the parameter is not set, `false` is assumed (do not print grid lines).
     */
    void setPrintGridLines(bool printGridLines);
    /**
     * @brief returns whether to print row and column headings.
     *
     * The default value is `false` (do not print headings).
     */
    std::optional<bool> printHeadings() const;
    /**
     * @brief sets whether to print row and column headings.
     * @param printHeadings if `true`, then headings are printed.
     *
     * If the parameter is not set, `false` is assumed (do not print headings).
     */
    void setPrintHeadings(bool printHeadings);
    /**
     * @brief returns whether to center data on page horizontally when printing.
     *
     * The default value is `false` (do not center data).
     */
    std::optional<bool> printHorizontalCentered() const;
    /**
     * @brief sets whether to center data on page horizontally when printing.
     * @param printHorizontalCentered if `true`, then data is centered on page
     * when printing.
     *
     * If the parameter is not set, `false` is assumed (do not center data).
     */
    void setPrintHorizontalCentered(bool printHorizontalCentered);
    /**
     * @brief returns whether to center data on page vertically when printing.
     *
     * The default value is `false` (do not center data).
     */
    std::optional<bool> printVerticalCentered() const;
    /**
     * @brief sets whether to center data on page vertically when printing.
     * @param printVerticalCentered if `true`, then data is centered on page
     * when printing.
     *
     * If the parameter is not set, `false` is assumed (do not center data).
     */
    void setPrintVerticalCentered(bool printVerticalCentered);

    // Autofilter parameters

    /**
     * @brief returns the autofilter parameters.
     * @return Reference to the AutoFilter class.
     */
    AutoFilter &autofilter();
    /**
     * @brief returns the autofilter parameters.
     * @return copy of the AutoFilter class.
     */
    AutoFilter autofilter() const;
    /**
     * @brief clears all autofilter parameters, effectively removing autofiltering
     * from the sheet. Equivalent to `setAutofilter(Autofilter());`
     */
    void clearAutofilter();
    /**
     * @brief sets the autofilter parameters.
     * @param autofilter the AutoFilter object.
     *
     * If @a autofilter is invalid, setting it is equivalent to `clearAutofilter();`.
     */
    void setAutofilter(const AutoFilter &autofilter);

    /// Sheet protection

    /**
     * @brief adds a protected range object to the list of protected ranges.
     * @param range CellRange to protect.
     * @param name A distinctive name of the protected range.
     * @return `true` on success, `false` if @a range or @a name is invalid.
     *
     * To set additional parameters (password, users) use #protectedRange() method.
     */
    bool addProtectedRange(const CellRange &range, const QString &name);
    /**
     * @brief adds a protected range object to the list of protected ranges.
     * @param ranges A list of ranges to protect.
     * @param name A distinctive name of the protected ranges.
     * @return `true` on success, `false` if @a range or @a name is empty.
     *
     * To set additional parameters (password, users) use #protectedRange() method.
     *
     * @note Unlike #addProtectedRange(), this method allows to protect  not
     * one range, but a list or ranges and give it a distinctive name.
     */
    bool addProtectedRanges(const QList<CellRange> &ranges, const QString &name);
    /**
     * @brief returns a list of all protected cell ranges.
     */
    QList<CellRange> protectedRanges() const;
    /**
     * @brief returns a ProtectedRange object from the list of the worksheet's
     * protected ranges.
     * @param rangeIndex zero-based index of a range (0 to #protectedRangesCount()-1).
     * @return A copy of the ProtectedRange object.
     */
    ProtectedRange protectedRange(int rangeIndex) const;
    /**
     * @brief returns a ProtectedRange object from the list of the worksheet's
     * protected ranges.
     * @param rangeIndex zero-based index of a range (0 to #protectedRangesCount()-1).
     * If @a rangeIndex is invalid, the behaviour is undefined.
     * @return A reference to the ProtectedRange object.
     */
    ProtectedRange &protectedRange(int rangeIndex);
    /**
     * @brief removes the ProtectedRange object from the list of the worksheet's
     * protected ranges.
     * @param rangeIndex zero-based index of a range (0 to #protectedRangesCount()-1).
     * @return `true` on success.
     */
    bool removeProtectedRange(int rangeIndex);
    /**
     * @brief searches @a range in the list of the worksheet's protected ranges
     * and deletes it. If the protected range becomes empty, deletes it as well.
     * @param range A cell range to remove.
     * @return `true` on success.
     */
    bool removeProtectedRange(const CellRange &range);
    /**
     * @brief returns the count of the worksheet's protected ranges.
     */
    int protectedRangesCount() const;
    /**
     * @brief removes all protected ranges defined in the worksheet.
     */
    void clearProtectedRanges();

private:
    QMap<int, double> getMaximumColumnWidths(int firstRow = 1, int lastRow = INT_MAX);
    void saveToXmlFile(QIODevice *device) const override;
    bool loadFromXmlFile(QIODevice *device) override;
};

}
#endif // XLSXWORKSHEET_H
