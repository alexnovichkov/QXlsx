// xlsxworksheet.cpp

#include <QtGlobal>
#include <QVariant>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QPoint>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QBuffer>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QTextDocument>
#include <QDir>
#include <QMapIterator>
#include <QMap>

#include <cmath>

#include "xlsxrichstring.h"
#include "xlsxcellreference.h"
#include "xlsxworksheet.h"
#include "xlsxworksheet_p.h"
#include "xlsxworkbook.h"
#include "xlsxformat.h"
#include "xlsxformat_p.h"
#include "xlsxutility_p.h"
#include "xlsxsharedstrings_p.h"
#include "xlsxdrawing_p.h"
#include "xlsxstyles_p.h"
#include "xlsxcell.h"
#include "xlsxcell_p.h"
#include "xlsxcellrange.h"
#include "xlsxconditionalformatting_p.h"
#include "xlsxdrawinganchor_p.h"
#include "xlsxchart.h"
#include "xlsxcellformula.h"
#include "xlsxcellformula_p.h"

namespace QXlsx {

WorksheetPrivate::WorksheetPrivate(Worksheet *p, Worksheet::CreateFlag flag) : AbstractSheetPrivate(p, flag)

{
}

WorksheetPrivate::~WorksheetPrivate()
{
}

/*
  Calculate the "spans" attribute of the <row> tag. This is an
  XLSX optimisation and isn't strictly required. However, it
  makes comparing files easier. The span is the same for each
  block of 16 rows.
 */
void WorksheetPrivate::calculateSpans() const
{
    rowSpans.clear();
    int span_min = XLSX_COLUMN_MAX+1;
    int span_max = -1;

    for (int row_num = dimension.firstRow(); row_num <= dimension.lastRow(); row_num++) {
        auto it = cellTable.constFind(row_num);
        if (it != cellTable.constEnd()) {
            for (int col_num = dimension.firstColumn(); col_num <= dimension.lastColumn(); col_num++) {
                if (it->contains(col_num)) {
                    if (span_max == -1) {
                        span_min = col_num;
                        span_max = col_num;
                    } else {
                        if (col_num < span_min)
                            span_min = col_num;
                        else if (col_num > span_max)
                            span_max = col_num;
                    }
                }
            }
        }
        auto cIt = comments.constFind(row_num);
        if (cIt != comments.constEnd()) {
            for (int col_num = dimension.firstColumn(); col_num <= dimension.lastColumn(); col_num++) {
                if (cIt->contains(col_num)) {
                    if (span_max == -1) {
                        span_min = col_num;
                        span_max = col_num;
                    } else {
                        if (col_num < span_min)
                            span_min = col_num;
                        else if (col_num > span_max)
                            span_max = col_num;
                    }
                }
            }
        }

        if (row_num%16 == 0 || row_num == dimension.lastRow()) {
            if (span_max != -1) {
                rowSpans[row_num / 16] = QStringLiteral("%1:%2").arg(span_min).arg(span_max);
                span_min = XLSX_COLUMN_MAX+1;
                span_max = -1;
            }
        }
    }
}


QString WorksheetPrivate::generateDimensionString() const
{
    if (!dimension.isValid())
        return QStringLiteral("A1");
    else
        return dimension.toString();
}

/*
  Check that row and col are valid and store the max and min
  values for use in other methods/elements. The ignore_row /
  ignore_col flags is used to indicate that we wish to perform
  the dimension check without storing the value. The ignore
  flags are use by setRow() and dataValidate.
*/
int WorksheetPrivate::checkDimensions(int row, int col, bool ignore_row, bool ignore_col)
{
    Q_ASSERT_X(row!=0, "checkDimensions", "row should start from 1 instead of 0");
    Q_ASSERT_X(col!=0, "checkDimensions", "column should start from 1 instead of 0");

    if (row > XLSX_ROW_MAX || row < 1 || col > XLSX_COLUMN_MAX || col < 1)
        return -1;

    if (!ignore_row) {
        if (row < dimension.firstRow() || dimension.firstRow() == -1) dimension.setFirstRow(row);
        if (row > dimension.lastRow()) dimension.setLastRow(row);
    }
    if (!ignore_col) {
        if (col < dimension.firstColumn() || dimension.firstColumn() == -1) dimension.setFirstColumn(col);
        if (col > dimension.lastColumn()) dimension.setLastColumn(col);
    }

    return 0;
}

/*!
 * \internal
 */
Worksheet::Worksheet(const QString &name, int id, Workbook *workbook, CreateFlag flag)
    :AbstractSheet(name, id, workbook, new WorksheetPrivate(this, flag))
{
    if (!workbook) //For unit test propose only. Ignore the memery leak.
        d_func()->workbook = new Workbook(flag);
}

/*!
 * \internal
 *
 * Make a copy of this sheet.
 */

Worksheet *Worksheet::copy(const QString &distName, int distId) const
{
    Q_D(const Worksheet);
    Worksheet *sheet = new Worksheet(distName, distId, d->workbook, F_NewFromScratch);
    WorksheetPrivate *sheet_d = sheet->d_func();

    sheet_d->dimension = d->dimension;
    if (d->drawing) {
        sheet_d->drawing = std::make_shared<Drawing>(sheet, F_NewFromScratch);
        for (auto anchor: qAsConst(d->drawing->anchors)) {
            anchor->copyTo(sheet_d->drawing.get());
        }
    }
    sheet_d->sheetState = d->sheetState;
    sheet_d->type = d->type;
    sheet_d->headerFooter = d->headerFooter;
    sheet_d->pageMargins = d->pageMargins;
    sheet_d->pageSetup = d->pageSetup;
    sheet_d->pictureFile = d->pictureFile;
    sheet_d->sheetProtection = d->sheetProtection;

    QMapIterator<int, QMap<int, std::shared_ptr<Cell> > > it(d->cellTable);
    while (it.hasNext())
    {
        it.next();
        int row = it.key();
        QMapIterator<int, std::shared_ptr<Cell> > it2(it.value());
        while (it2.hasNext())
        {
            it2.next();
            int col = it2.key();

            auto cell = std::make_shared<Cell>(it2.value().get());
            cell->d_ptr->parent = sheet;

            if (cell->cellType() == Cell::Type::SharedString)
                d->workbook->sharedStrings()->addSharedString(cell->d_ptr->richString);

            sheet_d->cellTable[row][col] = cell;
        }
    }

    sheet_d->merges = d->merges;
//    sheet_d->rowsInfo = d->rowsInfo;
//    sheet_d->colsInfo = d->colsInfo;
//    sheet_d->colsInfoHelper = d->colsInfoHelper;
//    sheet_d->dataValidationsList = d->dataValidationsList;
//    sheet_d->conditionalFormattingList = d->conditionalFormattingList;

    return sheet;
}

Worksheet::~Worksheet()
{
}

bool Worksheet::isWindowProtected() const
{
    Q_D(const Worksheet);
    return d->windowProtection;
}

void Worksheet::setWindowProtected(bool protect)
{
    Q_D(Worksheet);
    d->windowProtection = protect;
}

bool Worksheet::isFormulasVisible() const
{
    Q_D(const Worksheet);
    return d->showFormulas;
}

void Worksheet::setFormulasVisible(bool visible)
{
    Q_D(Worksheet);
    d->showFormulas = visible;
}

bool Worksheet::isGridLinesVisible() const
{
    Q_D(const Worksheet);
    return d->showGridLines;
}

void Worksheet::setGridLinesVisible(bool visible)
{
    Q_D(Worksheet);
    d->showGridLines = visible;
}

bool Worksheet::isRowColumnHeadersVisible() const
{
    Q_D(const Worksheet);
    return d->showRowColHeaders;
}

void Worksheet::setRowColumnHeadersVisible(bool visible)
{
    Q_D(Worksheet);
    d->showRowColHeaders = visible;
}

bool Worksheet::isRightToLeft() const
{
    Q_D(const Worksheet);
    return d->rightToLeft;
}

void Worksheet::setRightToLeft(bool enable)
{
    Q_D(Worksheet);
    d->rightToLeft = enable;
}

bool Worksheet::isZerosVisible() const
{
    Q_D(const Worksheet);
    return d->showZeros;
}

void Worksheet::setZerosVisible(bool visible)
{
    Q_D(Worksheet);
    d->showZeros = visible;
}

bool Worksheet::isSelected() const
{
    Q_D(const Worksheet);
    return d->tabSelected;
}

void Worksheet::setSelected(bool select)
{
    Q_D(Worksheet);
    d->tabSelected = select;
}

bool Worksheet::isRulerVisible() const
{
    Q_D(const Worksheet);
    return d->showRuler;

}

void Worksheet::setRulerVisible(bool visible)
{
    Q_D(Worksheet);
    d->showRuler = visible;

}

bool Worksheet::isOutlineSymbolsVisible() const
{
    Q_D(const Worksheet);
    return d->showOutlineSymbols;
}

void Worksheet::setOutlineSymbolsVisible(bool visible)
{
    Q_D(Worksheet);
    d->showOutlineSymbols = visible;
}

bool Worksheet::isWhiteSpaceVisible() const
{
    Q_D(const Worksheet);
    return d->showWhiteSpace;
}

void Worksheet::setWhiteSpaceVisible(bool visible)
{
    Q_D(Worksheet);
    d->showWhiteSpace = visible;
}

bool Worksheet::write(int row, int column, const QVariant &value, const Format &format)
{
    Q_D(Worksheet);

    if (d->checkDimensions(row, column))
        return false;

    bool ret = true;
    if (value.isNull())
    {
        //Blank
        ret = writeBlank(row, column, format);
    }
    else if (value.userType() == QMetaType::QString)
    {
        //String
        QString token = value.toString();
        bool ok;

        if (token.startsWith(QLatin1String("=")))
        {
            //convert to formula
            ret = writeFormula(row, column, CellFormula(token), format);
        }
        else if (d->workbook->isStringsToHyperlinksEnabled() && token.contains(d->urlPattern))
        {
            //convert to url
            ret = writeHyperlink(row, column, QUrl(token));
        }
        else if (d->workbook->isStringsToNumbersEnabled() && (value.toDouble(&ok), ok))
        {
            //Try convert string to number if the flag enabled.
            ret = writeNumeric(row, column, value.toDouble(), format);
        }
        else
        {
            //normal string now
            ret = writeString(row, column, token, format);
        }
    }
    else if (value.userType() == qMetaTypeId<RichString>())
    {
        ret = writeString(row, column, value.value<RichString>(), format);
    }
    else if (value.userType() == QMetaType::Int || value.userType() == QMetaType::UInt
               || value.userType() == QMetaType::LongLong || value.userType() == QMetaType::ULongLong
               || value.userType() == QMetaType::Double || value.userType() == QMetaType::Float)
    {
        //Number

        ret = writeNumeric(row, column, value.toDouble(), format);
    }
    else if (value.userType() == QMetaType::Bool)
    {
        //Bool
        ret = writeBool(row,column, value.toBool(), format);
    }
    else if (value.userType() == QMetaType::QDateTime ) // dev67
    {
        //DateTime, Date
        //  note that, QTime cann't convert to QDateTime
        ret = writeDateTime(row, column, value.toDateTime(), format);
    }
    else if ( value.userType() == QMetaType::QDate ) // dev67
    {
        ret = writeDate(row, column, value.toDate(), format);
    }
    else if (value.userType() == QMetaType::QTime)
    {
        //Time
        ret = writeTime(row, column, value.toTime(), format);
    }
    else if (value.userType() == QMetaType::QUrl)
    {
        //Url
        ret = writeHyperlink(row, column, value.toUrl(), format);
    }
    else
    {
        //Wrong type
        return false;
    }

    return ret;
}

bool Worksheet::write(const CellReference &row_column, const QVariant &value, const Format &format)
{
    if (!row_column.isValid())
        return false;

    return write(row_column.row(), row_column.column(), value, format);
}

QVariant Worksheet::read(const CellReference &row_column) const
{
    if (!row_column.isValid())
        return QVariant();

    return read(row_column.row(), row_column.column());
}

QVariant Worksheet::read(int row, int column) const
{
    Q_D(const Worksheet);

    Cell *c = cell(row, column);
    if (!c)
        return QVariant();

    if (c->hasFormula()) {
        if (c->formula().formulaType() == CellFormula::Type::Normal)
            return QVariant(QLatin1String("=")+c->formula().formulaText());

        if (c->formula().formulaType() == CellFormula::Type::Shared) {
            if (!c->formula().formulaText().isEmpty())
                return QVariant(QLatin1String("=")+c->formula().formulaText());

            int si = c->formula().sharedIndex();
            const CellFormula &rootFormula = d->sharedFormulaMap[ si ];
            CellReference rootCellRef = rootFormula.reference().topLeft();
            QString rootFormulaText = rootFormula.formulaText();
            QString newFormulaText = convertSharedFormula(rootFormulaText, rootCellRef, CellReference(row, column));
            return QVariant(QLatin1String("=")+newFormulaText);
        }
    }

    if (c->isDateTime())
    {
        QVariant vDateTime = c->dateTime();
        return vDateTime;
    }

    return c->value();
}

Cell *Worksheet::cell(const CellReference &row_column) const
{
    if (!row_column.isValid())
        return 0;

    return cell(row_column.row(), row_column.column());
}

Cell *Worksheet::cell(int row, int col) const
{
    Q_D(const Worksheet);
    auto it = d->cellTable.constFind(row);
    if (it == d->cellTable.constEnd())
        return 0;
    if (!it->contains(col))
        return 0;

    return (*it)[col].get();
}

Format WorksheetPrivate::cellFormat(int row, int col) const
{
    auto it = cellTable.constFind(row);
    if (it == cellTable.constEnd())
        return Format();
    if (!it->contains(col))
        return Format();
    return (*it)[col]->format();
}

bool Worksheet::writeString(const CellReference &row_column, const RichString &value, const Format &format)
{
    if (!row_column.isValid())
        return false;

    return writeString(row_column.row(), row_column.column(), value, format);
}

bool Worksheet::writeString(int row, int column, const RichString &value, const Format &format)
{
    Q_D(Worksheet);
//    QString content = value.toPlainString();
    if (d->checkDimensions(row, column))
        return false;

//    if (content.size() > d->xls_strmax) {
//        content = content.left(d->xls_strmax);
//        error = -2;
//    }

    d->sharedStrings()->addSharedString(value);
    Format fmt = format.isValid() ? format : d->cellFormat(row, column);
    if (value.fragmentCount() == 1 && value.fragmentFormat(0).isValid())
        fmt.mergeFormat(value.fragmentFormat(0));
    d->workbook->styles()->addXfFormat(fmt);
    auto cell = std::make_shared<Cell>(value.toPlainString(), Cell::Type::SharedString, fmt, this);
    cell->d_ptr->richString = value;
    d->cellTable[row][column] = cell;
    return true;
}

bool Worksheet::writeString(const CellReference &row_column, const QString &value, const Format &format)
{
    if (!row_column.isValid())
        return false;

    return writeString(row_column.row(), row_column.column(), value, format);
}

bool Worksheet::writeString(int row, int column, const QString &value, const Format &format)
{
    Q_D(Worksheet);
    if (d->checkDimensions(row, column))
        return false;

    RichString rs;
    if (d->workbook->isHtmlToRichStringEnabled() && Qt::mightBeRichText(value))
        rs.setHtml(value);
    else
        rs.addFragment(value, Format());

    return writeString(row, column, rs, format);
}

bool Worksheet::writeInlineString(const CellReference &row_column, const QString &value, const Format &format)
{
    if (!row_column.isValid())
        return false;

    return writeInlineString(row_column.row(), row_column.column(), value, format);
}

bool Worksheet::writeInlineString(int row, int column, const QString &value, const Format &format)
{
    Q_D(Worksheet);
    //int error = 0;
    QString content = value;
    if (d->checkDimensions(row, column))
        return false;

    if (value.size() > XLSX_STRING_MAX) {
        content = value.left(XLSX_STRING_MAX);
        //error = -2;
    }

    Format fmt = format.isValid() ? format : d->cellFormat(row, column);
    d->workbook->styles()->addXfFormat(fmt);
    d->cellTable[row][column] = std::make_shared<Cell>(value, Cell::Type::InlineString, fmt, this);
    return true;
}

bool Worksheet::writeNumeric(const CellReference &row_column, double value, const Format &format)
{
    if (!row_column.isValid())
        return false;

    return writeNumeric(row_column.row(), row_column.column(), value, format);
}

bool Worksheet::writeNumeric(int row, int column, double value, const Format &format)
{
    Q_D(Worksheet);
    if (d->checkDimensions(row, column))
        return false;

    Format fmt = format.isValid() ? format : d->cellFormat(row, column);
    d->workbook->styles()->addXfFormat(fmt);
    d->cellTable[row][column] = std::make_shared<Cell>(value, Cell::Type::Number, fmt, this);
    return true;
}


bool Worksheet::writeFormula(const CellReference &row_column, const CellFormula &formula, const Format &format, double result)
{
    if (!row_column.isValid())
        return false;

    return writeFormula(row_column.row(), row_column.column(), formula, format, result);
}

bool Worksheet::writeFormula(int row, int column, const CellFormula &formula_, const Format &format, double result)
{
    Q_D(Worksheet);

    if (d->checkDimensions(row, column))
        return false;

    Format fmt = format.isValid() ? format : d->cellFormat(row, column);
    d->workbook->styles()->addXfFormat(fmt);

    CellFormula formula = formula_;
    formula.d->ca = true;
    if (formula.formulaType() == CellFormula::Type::Shared)
    {
        //Assign proper shared index for shared formula
        int si = 0;
        while ( d->sharedFormulaMap.contains(si) )
        {
            ++si;
        }
        formula.d->si = si;
        d->sharedFormulaMap[si] = formula;
    }

    auto data = std::make_shared<Cell>(result, Cell::Type::Number, fmt, this);
    data->d_ptr->formula = formula;
    d->cellTable[row][column] = data;

    CellRange range = formula.reference();
    if (formula.formulaType() == CellFormula::Type::Shared) {
        CellFormula sf(QString(), CellFormula::Type::Shared);
        sf.d->si = formula.sharedIndex();
        for (int r=range.firstRow(); r<=range.lastRow(); ++r) {
            for (int c=range.firstColumn(); c<=range.lastColumn(); ++c) {
                if (!(r==row && c==column)) {
                    if (Cell *ce = cell(r, c)) {
                        ce->d_ptr->formula = sf;
                    } else {
                        auto newCell = std::make_shared<Cell>(result, Cell::Type::Number, fmt, this);
                        newCell->d_ptr->formula = sf;
                        d->cellTable[r][c] = newCell;
                    }
                }
            }
        }
    }

    return true;
}

bool Worksheet::writeBlank(const CellReference &row_column, const Format &format)
{
    if (!row_column.isValid())
        return false;

    return writeBlank(row_column.row(), row_column.column(), format);
}

bool Worksheet::writeBlank(int row, int column, const Format &format)
{
    Q_D(Worksheet);
    if (d->checkDimensions(row, column))
        return false;

    Format fmt = format.isValid() ? format : d->cellFormat(row, column);
    d->workbook->styles()->addXfFormat(fmt);

    //Note: NumberType with an invalid QVariant value means blank.
    d->cellTable[row][column] = std::make_shared<Cell>(QVariant{}, Cell::Type::Number, fmt, this);

    return true;
}

bool Worksheet::writeBool(const CellReference &row_column, bool value, const Format &format)
{
    if (!row_column.isValid())
        return false;

    return writeBool(row_column.row(), row_column.column(), value, format);
}

bool Worksheet::writeBool(int row, int column, bool value, const Format &format)
{
    Q_D(Worksheet);
    if (d->checkDimensions(row, column))
        return false;

    Format fmt = format.isValid() ? format : d->cellFormat(row, column);
    d->workbook->styles()->addXfFormat(fmt);
    d->cellTable[row][column] = std::make_shared<Cell>(value, Cell::Type::Boolean, fmt, this);

    return true;
}

bool Worksheet::writeDateTime(const CellReference &row_column, const QDateTime &dt, const Format &format)
{
    if (!row_column.isValid())
        return false;

    return writeDateTime(row_column.row(), row_column.column(), dt, format);
}

bool Worksheet::writeDateTime(int row, int column, const QDateTime &dt, const Format &format)
{
    Q_D(Worksheet);
    if (d->checkDimensions(row, column))
        return false;

    Format fmt = format.isValid() ? format : d->cellFormat(row, column);
    if (!fmt.isValid() || !fmt.isDateTimeFormat())
        fmt.setNumberFormat(d->workbook->defaultDateFormat());
    d->workbook->styles()->addXfFormat(fmt);

    double value = datetimeToNumber(dt, d->workbook->isDate1904());

    d->cellTable[row][column] = std::make_shared<Cell>(value, Cell::Type::Number, fmt, this);

    return true;
}

// dev67
bool Worksheet::writeDate(const CellReference &row_column, const QDate &dt, const Format &format)
{
    if (!row_column.isValid())
        return false;

    return writeDate(row_column.row(), row_column.column(), dt, format);
}

// dev67
bool Worksheet::writeDate(int row, int column, const QDate &dt, const Format &format)
{
    Q_D(Worksheet);
    if (d->checkDimensions(row, column))
        return false;

    Format fmt = format.isValid() ? format : d->cellFormat(row, column);

    if (!fmt.isValid() || !fmt.isDateTimeFormat())
        fmt.setNumberFormat(d->workbook->defaultDateFormat());

    d->workbook->styles()->addXfFormat(fmt);

    double value = datetimeToNumber(QDateTime(dt, QTime(0,0,0)), d->workbook->isDate1904());

    d->cellTable[row][column] = std::make_shared<Cell>(value, Cell::Type::Number, fmt, this);

    return true;
}

bool Worksheet::writeTime(const CellReference &row_column, const QTime &t, const Format &format)
{
    if (!row_column.isValid())
        return false;

    return writeTime(row_column.row(), row_column.column(), t, format);
}

bool Worksheet::writeTime(int row, int column, const QTime &t, const Format &format)
{
    Q_D(Worksheet);
    if (d->checkDimensions(row, column))
        return false;

    Format fmt = format.isValid() ? format : d->cellFormat(row, column);
    if (!fmt.isValid() || !fmt.isDateTimeFormat())
        fmt.setNumberFormat(QStringLiteral("hh:mm:ss"));
    d->workbook->styles()->addXfFormat(fmt);

    d->cellTable[row][column] = std::make_shared<Cell>(timeToNumber(t), Cell::Type::Number, fmt, this);

    return true;
}

bool Worksheet::writeHyperlink(const CellReference &row_column, const QUrl &url, const Format &format, const QString &display, const QString &tip)
{
    if (!row_column.isValid())
        return false;

    return writeHyperlink(row_column.row(), row_column.column(), url, format, display, tip);
}

bool Worksheet::writeHyperlink(int row, int column, const QUrl &url, const Format &format, const QString &display, const QString &tip)
{
    Q_D(Worksheet);
    if (d->checkDimensions(row, column))
        return false;

    //int error = 0;

    QString urlString = url.toString();

    //Generate proper display string
    QString displayString = display.isEmpty() ? urlString : display;
    if (displayString.startsWith(QLatin1String("mailto:")))
        displayString.replace(QLatin1String("mailto:"), QString());
    if (displayString.size() > XLSX_STRING_MAX) {
        displayString = displayString.left(XLSX_STRING_MAX);
        //error = -2;
    }

    /*
      Location within target. If target is a workbook (or this workbook)
      this shall refer to a sheet and cell or a defined name. Can also
      be an HTML anchor if target is HTML file.

      c:\temp\file.xlsx#Sheet!A1
      http://a.com/aaa.html#aaaaa
    */
    QString locationString;
    if (url.hasFragment()) {
        locationString = url.fragment();
        urlString = url.toString(QUrl::RemoveFragment);
    }

    Format fmt = format.isValid() ? format : d->cellFormat(row, column);
    //Given a default style for hyperlink
    if (!fmt.isValid()) {
        fmt.setVerticalAlignment(Format::AlignVCenter);
        fmt.setFontColor(Qt::blue);
        fmt.setFontUnderline(Format::FontUnderlineSingle);
    }
    d->workbook->styles()->addXfFormat(fmt);

    //Write the hyperlink string as normal string.
    d->sharedStrings()->addSharedString(displayString);
    d->cellTable[row][column] = std::make_shared<Cell>(displayString, Cell::Type::SharedString, fmt, this);

    //Store the hyperlink data in a separate table
    d->urlTable[row][column] = QSharedPointer<XlsxHyperlinkData>(new XlsxHyperlinkData(XlsxHyperlinkData::External, urlString, locationString, QString(), tip));

    return true;
}

bool Worksheet::addDataValidation(const DataValidation &validation)
{
    Q_D(Worksheet);
    if (validation.ranges().isEmpty() || validation.validationType()==DataValidation::None)
        return false;

    d->dataValidationsList.append(validation);
    return true;
}

bool Worksheet::addConditionalFormatting(const ConditionalFormatting &cf)
{
    Q_D(Worksheet);
    if (cf.ranges().isEmpty())
        return false;

    for (int i=0; i<cf.d->cfRules.size(); ++i) {
        const std::shared_ptr<XlsxCfRuleData> &rule = cf.d->cfRules[i];
        if (!rule->dxfFormat.isEmpty())
            d->workbook->styles()->addDxfFormat(rule->dxfFormat);
        rule->priority = 1;
    }
    d->conditionalFormattingList.append(cf);
    return true;
}

int Worksheet::insertImage(int row, int column, const QImage &image)
{
    Q_D(Worksheet);

    if (!image.isNull()) {
        if (!d->drawing)
            d->drawing = std::make_shared<Drawing>(this, F_NewFromScratch);

        DrawingOneCellAnchor* anchor = new DrawingOneCellAnchor(d->drawing.get(), DrawingAnchor::Picture);

        /*
        The size are expressed as English Metric Units (EMUs).
        EMU is 1/360 000 of centimiter.
        */
        anchor->from = XlsxMarker(row, column, 0, 0);
        float scaleX = 36e6f / std::max(1,image.dotsPerMeterX());
        float scaleY = 36e6f / std::max(1,image.dotsPerMeterY());
        anchor->ext = QSize( int(image.width() * scaleX), int(image.height() * scaleY) );

        anchor->setObjectPicture(image);
        return d->drawing->anchors.size()-1;
    }
    return -1;
}

QImage Worksheet::image(int imageIndex) const
{
    Q_D(const Worksheet);

    if (imageIndex < 0 || !d->drawing)
        return {};

   DrawingAnchor* danchor = d->drawing->anchors.value(imageIndex);
   // QSharedPointer<Drawing> // for multithread
   if (danchor) {
       QImage img;
       danchor->getObjectPicture(img);
       return img;
   }

   return {};
}

QImage Worksheet::image(int row, int column) const
{
    Q_D(const Worksheet);

    if (d->drawing) {
        for (auto anchor: qAsConst(d->drawing->anchors)) {
            if (anchor && anchor->row() == row && anchor->col() == column) {
                QImage img;
                anchor->getObjectPicture(img);
                return img;
            }
        }
    }
    return {};
}

int Worksheet::imageCount() const
{
    Q_D(const Worksheet);
    int count = 0;
    if (d->drawing) {
        for (auto anchor: qAsConst(d->drawing->anchors)) {
            if (anchor && anchor->isPicture())
                count++;
        }
    }
    return count;
}


Chart *Worksheet::insertChart(int row, int column, const QSize &size)
{
    Q_D(Worksheet);

    if (!d->drawing)
        d->drawing = std::make_shared<Drawing>(this, F_NewFromScratch);

    DrawingOneCellAnchor *anchor = new DrawingOneCellAnchor(d->drawing.get());

    /*
        The size are expressed as English Metric Units (EMUs). There are
        12,700 EMUs per point. Therefore, 12,700 * 3 /4 = 9,525 EMUs per
        pixel
    */
    anchor->from = XlsxMarker(row, column, 0, 0);
    anchor->ext = size * 9525;

    QSharedPointer<Chart> chart = QSharedPointer<Chart>(new Chart(this, F_NewFromScratch));
    anchor->setObjectGraphicFrame(chart);

    return chart.data();
}

Chart *Worksheet::chart(int index) const
{
    Q_D(const Worksheet);

    if (index < 0) return nullptr;

    if (d->drawing) {
        int currentIndex = -1;
        for (auto anchor: qAsConst(d->drawing->anchors)) {
            if (anchor && anchor->chart()) {
                currentIndex++;
                if (currentIndex == index)
                    return anchor->chart().get();
            }
        }
    }

    return nullptr;
}

Chart *Worksheet::chart(int row, int column)
{
    Q_D(const Worksheet);
    if (d->drawing) {
        for (auto anchor: qAsConst(d->drawing->anchors)) {
            if (anchor && anchor->row() == row && anchor->col() == column) {
                return anchor->chart().get();
            }
        }
    }
    return nullptr;
}

bool Worksheet::removeChart(int row, int column)
{
    Q_D(Worksheet);
    if (d->drawing) {
        auto anchor = std::find_if(std::begin(d->drawing->anchors),
                                   std::end(d->drawing->anchors),
                                   [row, column](auto anchor)
        {return anchor && anchor->row() == row && anchor->col() == column;});
        if (anchor != std::end(d->drawing->anchors)) {
            bool result = (*anchor)->removeObjectGraphicFrame();
            delete *anchor;
            d->drawing->anchors.erase(anchor);

            return result;
        }
    }
    return false;
}

bool Worksheet::removeChart(int index)
{
    Q_D(Worksheet);

    if (index < 0) return false;
    if (d->drawing) {
        int currentIndex = -1;
        for (auto anchor: qAsConst(d->drawing->anchors)) {
            if (anchor && anchor->chart()) {
                currentIndex++;
                if (currentIndex == index) {
                    bool result = anchor->removeObjectGraphicFrame();
                    delete anchor;
                    d->drawing->anchors.removeOne(anchor);
                    return result;
                }
            }
        }
    }

    return false;
}

int Worksheet::chartCount() const
{
    Q_D(const Worksheet);
    int count = 0;

    if (d->drawing) {
        for (auto anchor: qAsConst(d->drawing->anchors)) {
            if (anchor && anchor->isChart())
                count++;
        }
    }
    return count;
}

/*!
    Merge a \a range of cells. The first cell should contain the data and the others should
    be blank. All cells will be applied the same style if a valid \a format is given.
    Returns true on success.

    \note All cells except the top-left one will be cleared.
 */
bool Worksheet::mergeCells(const CellRange &range, const Format &format)
{
    Q_D(Worksheet);
    if (range.rowCount() < 2 && range.columnCount() < 2)
        return false;

    if (d->checkDimensions(range.firstRow(), range.firstColumn()))
        return false;

    if (format.isValid())
    {
        d->workbook->styles()->addXfFormat(format);
    }

    for (int row = range.firstRow(); row <= range.lastRow(); ++row) {
        for (int col = range.firstColumn(); col <= range.lastColumn(); ++col) {
            if (row == range.firstRow() && col == range.firstColumn()) {
                if (Cell *c = cell(row, col)) {
                    if (format.isValid())
                        c->d_ptr->format = format;
                }
                else
                    writeBlank(row, col, format);
            }
            else
                writeBlank(row, col, format);
        }
    }

    d->merges.append(range);
    return true;
}

/*!
    Unmerge the cells in the \a range. Returns true on success.

*/
bool Worksheet::unmergeCells(const CellRange &range)
{
    Q_D(Worksheet);
    return d->merges.removeOne(range);
}

/*!
  Returns all the merged cells.
*/
QList<CellRange> Worksheet::mergedCells() const
{
    Q_D(const Worksheet);

    // dev57

    QList<CellRange> emptyList;

    if ( d->type == AbstractSheet::Type::Worksheet )
    {
        return d->merges;
    }
    else if ( d->type == AbstractSheet::Type::Chartsheet )
    {
    }
    else if ( d->type == AbstractSheet::Type::Dialogsheet )
    {
    }
    else if ( d->type == AbstractSheet::Type::Macrosheet )
    {
    }
    else
    { // undefined
    }

    return emptyList;
}

/*!
 * \internal
 */
void Worksheet::saveToXmlFile(QIODevice *device) const
{
    Q_D(const Worksheet);
    d->relationships->clear();

    QXmlStreamWriter writer(device);

    writer.writeStartDocument(QStringLiteral("1.0"), true);
    writer.writeStartElement(QStringLiteral("worksheet"));
    writer.writeAttribute(QStringLiteral("xmlns"), QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main"));
    writer.writeAttribute(QStringLiteral("xmlns:r"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships"));

    //for Excel 2010
    //    writer.writeAttribute("xmlns:mc", "http://schemas.openxmlformats.org/markup-compatibility/2006");
    //    writer.writeAttribute("xmlns:x14ac", "http://schemas.microsoft.com/office/spreadsheetml/2009/9/ac");
    //    writer.writeAttribute("mc:Ignorable", "x14ac");

    writer.writeStartElement(QStringLiteral("dimension"));
    writer.writeAttribute(QStringLiteral("ref"), d->generateDimensionString());
    writer.writeEndElement();//dimension

    writer.writeStartElement(QStringLiteral("sheetViews"));
    writer.writeStartElement(QStringLiteral("sheetView"));
    if (d->windowProtection)
        writer.writeAttribute(QStringLiteral("windowProtection"), QStringLiteral("1"));
    if (d->showFormulas)
        writer.writeAttribute(QStringLiteral("showFormulas"), QStringLiteral("1"));
    if (!d->showGridLines)
        writer.writeAttribute(QStringLiteral("showGridLines"), QStringLiteral("0"));
    if (!d->showRowColHeaders)
        writer.writeAttribute(QStringLiteral("showRowColHeaders"), QStringLiteral("0"));
    if (!d->showZeros)
        writer.writeAttribute(QStringLiteral("showZeros"), QStringLiteral("0"));
    if (d->rightToLeft)
        writer.writeAttribute(QStringLiteral("rightToLeft"), QStringLiteral("1"));
    if (d->tabSelected)
        writer.writeAttribute(QStringLiteral("tabSelected"), QStringLiteral("1"));
    if (!d->showRuler)
        writer.writeAttribute(QStringLiteral("showRuler"), QStringLiteral("0"));
    if (!d->showOutlineSymbols)
        writer.writeAttribute(QStringLiteral("showOutlineSymbols"), QStringLiteral("0"));
    if (!d->showWhiteSpace)
        writer.writeAttribute(QStringLiteral("showWhiteSpace"), QStringLiteral("0"));
    writer.writeAttribute(QStringLiteral("workbookViewId"), QStringLiteral("0"));
    writer.writeEndElement();//sheetView
    writer.writeEndElement();//sheetViews

    writer.writeStartElement(QStringLiteral("sheetFormatPr"));
    writer.writeAttribute(QStringLiteral("defaultRowHeight"), QString::number(d->defaultRowHeight));
    if (d->defaultRowHeight != 15)
        writer.writeAttribute(QStringLiteral("customHeight"), QStringLiteral("1"));
    if (d->defaultRowZeroed)
        writer.writeAttribute(QStringLiteral("zeroHeight"), QStringLiteral("1"));
    if (d->outlineRowLevel)
        writer.writeAttribute(QStringLiteral("outlineLevelRow"), QString::number(d->outlineRowLevel));
    if (d->outlineColLevel)
        writer.writeAttribute(QStringLiteral("outlineLevelCol"), QString::number(d->outlineColLevel));

    //for Excel 2010
    //    writer.writeAttribute("x14ac:dyDescent", "0.25");
    writer.writeEndElement();//sheetFormatPr

    if (!d->colsInfo.isEmpty())
    {
        writer.writeStartElement(QStringLiteral("cols"));
        QMapIterator<int, QSharedPointer<XlsxColumnInfo> > it(d->colsInfo);
        while (it.hasNext())
        {
            it.next();
            QSharedPointer<XlsxColumnInfo> col_info = it.value();
            writer.writeStartElement(QStringLiteral("col"));
            writer.writeAttribute(QStringLiteral("min"), QString::number(col_info->firstColumn));
            writer.writeAttribute(QStringLiteral("max"), QString::number(col_info->lastColumn));
            if (col_info->width)
                writer.writeAttribute(QStringLiteral("width"), QString::number(col_info->width, 'g', 15));
            if (!col_info->format.isEmpty())
                writer.writeAttribute(QStringLiteral("style"), QString::number(col_info->format.xfIndex()));
            if (col_info->hidden)
                writer.writeAttribute(QStringLiteral("hidden"), QStringLiteral("1"));
            if (col_info->width)
                writer.writeAttribute(QStringLiteral("customWidth"), QStringLiteral("1"));
            if (col_info->outlineLevel)
                writer.writeAttribute(QStringLiteral("outlineLevel"), QString::number(col_info->outlineLevel));
            if (col_info->collapsed)
                writer.writeAttribute(QStringLiteral("collapsed"), QStringLiteral("1"));
            writer.writeEndElement();//col
        }
        writer.writeEndElement();//cols
    }

    writer.writeStartElement(QStringLiteral("sheetData"));
    if (d->dimension.isValid())
        d->saveXmlSheetData(writer);
    writer.writeEndElement();//sheetData

    //sheet protection
    if (d->sheetProtection.has_value()) d->sheetProtection->write(writer);

    d->saveXmlMergeCells(writer);
    for (const ConditionalFormatting &cf : qAsConst(d->conditionalFormattingList))
        cf.saveToXml(writer);
    d->saveXmlDataValidations(writer);

    d->pageMargins.write(writer);
    d->pageSetup.write(writer);
    d->headerFooter.write(writer);

    d->saveXmlHyperlinks(writer);
    d->saveXmlDrawings(writer);

    if (d->pictureFile) {
        d->relationships->addDocumentRelationship(QStringLiteral("/image"), QStringLiteral("../media/image%1.%2")
                                                   .arg(d->pictureFile->index()+1)
                                                   .arg(d->pictureFile->suffix()));
        writer.writeEmptyElement(QStringLiteral("picture"));
        writer.writeAttribute(QStringLiteral("r:id"), QStringLiteral("rId%1").arg(d->relationships->count()));
    }

    if (d->extLst.isValid()) d->extLst.write(writer, "extLst");

    writer.writeEndElement(); // worksheet
    writer.writeEndDocument();
}

void WorksheetPrivate::saveXmlSheetData(QXmlStreamWriter &writer) const
{
    calculateSpans();
    for (int row_num = dimension.firstRow(); row_num <= dimension.lastRow(); row_num++)
    {
        auto ctIt = cellTable.constFind(row_num);
        auto riIt = rowsInfo.constFind(row_num);
        if (ctIt == cellTable.constEnd() && riIt == rowsInfo.constEnd() && !comments.contains(row_num))
        {
            //Only process rows with cell data / comments / formatting
            continue;
        }

        int span_index = (row_num-1) / 16;
        QString span;
        auto rsIt = rowSpans.constFind(span_index);
        if (rsIt != rowSpans.constEnd())
            span = rsIt.value();

        writer.writeStartElement(QStringLiteral("row"));
        writer.writeAttribute(QStringLiteral("r"), QString::number(row_num));

        if (!span.isEmpty())
            writer.writeAttribute(QStringLiteral("spans"), span);

        if (riIt != rowsInfo.constEnd())
        {
            QSharedPointer<XlsxRowInfo> rowInfo = riIt.value();
            if (!rowInfo->format.isEmpty())
            {
                writer.writeAttribute(QStringLiteral("s"), QString::number(rowInfo->format.xfIndex()));
                writer.writeAttribute(QStringLiteral("customFormat"), QStringLiteral("1"));
            }

            //!Todo: support customHeight from info struct
            //!Todo: where does this magic number '15' come from?
            if (rowInfo->customHeight) {
                writer.writeAttribute(QStringLiteral("ht"), QString::number(rowInfo->height));
                writer.writeAttribute(QStringLiteral("customHeight"), QStringLiteral("1"));
            } else {
                writer.writeAttribute(QStringLiteral("customHeight"), QStringLiteral("0"));
            }

            if (rowInfo->hidden)
                writer.writeAttribute(QStringLiteral("hidden"), QStringLiteral("1"));
            if (rowInfo->outlineLevel > 0)
                writer.writeAttribute(QStringLiteral("outlineLevel"), QString::number(rowInfo->outlineLevel));
            if (rowInfo->collapsed)
                writer.writeAttribute(QStringLiteral("collapsed"), QStringLiteral("1"));
        }

        //Write cell data if row contains filled cells
        if (ctIt != cellTable.constEnd())
        {
            for (int col_num = dimension.firstColumn(); col_num <= dimension.lastColumn(); col_num++)
            {
                if (ctIt->contains(col_num))
                {
                    saveXmlCellData(writer, row_num, col_num, (*ctIt)[col_num]);
                }
            }
        }
        writer.writeEndElement(); //row
    }
}

void WorksheetPrivate::saveXmlCellData(QXmlStreamWriter &writer, int row, int col, std::shared_ptr<Cell> cell) const
{
    Q_Q(const Worksheet);

    //This is the innermost loop so efficiency is important.
    QString cell_pos = CellReference(row, col).toString();

    writer.writeStartElement(QStringLiteral("c"));
    writer.writeAttribute(QStringLiteral("r"), cell_pos);

    QMap<int, QSharedPointer<XlsxRowInfo> >::ConstIterator rIt;
    QMap<int, QSharedPointer<XlsxColumnInfo> >::ConstIterator cIt;

    //Style used by the cell, row or col
    if (!cell->format().isEmpty())
        writer.writeAttribute(QStringLiteral("s"), QString::number(cell->format().xfIndex()));
    else if ((rIt = rowsInfo.constFind(row)) != rowsInfo.constEnd() && !(*rIt)->format.isEmpty())
        writer.writeAttribute(QStringLiteral("s"), QString::number((*rIt)->format.xfIndex()));
    else if ((cIt = colsInfoHelper.constFind(col)) != colsInfoHelper.constEnd() && !(*cIt)->format.isEmpty())
        writer.writeAttribute(QStringLiteral("s"), QString::number((*cIt)->format.xfIndex()));

    if (cell->cellType() == Cell::Type::SharedString) // 's'
    {
        int sst_idx;
        if (cell->isRichString())
            sst_idx = sharedStrings()->getSharedStringIndex(cell->d_ptr->richString);
        else
            sst_idx = sharedStrings()->getSharedStringIndex(cell->value().toString());

        writer.writeAttribute(QStringLiteral("t"), QStringLiteral("s"));
        writer.writeTextElement(QStringLiteral("v"), QString::number(sst_idx));
    }
    else if (cell->cellType() == Cell::Type::InlineString) // 'inlineStr'
    {
        writer.writeAttribute(QStringLiteral("t"), QStringLiteral("inlineStr"));
        writer.writeStartElement(QStringLiteral("is"));
        if (cell->isRichString())
        {
            //Rich text string
            RichString string = cell->d_ptr->richString;
            for (int i=0; i<string.fragmentCount(); ++i)
            {
                writer.writeStartElement(QStringLiteral("r"));
                if (string.fragmentFormat(i).hasFontData())
                {
                    writer.writeStartElement(QStringLiteral("rPr"));
                    //Todo
                    writer.writeEndElement();// rPr
                }
                writer.writeStartElement(QStringLiteral("t"));
                if (isSpaceReserveNeeded(string.fragmentText(i)))
                    writer.writeAttribute(QStringLiteral("xml:space"), QStringLiteral("preserve"));
                writer.writeCharacters(string.fragmentText(i));
                writer.writeEndElement();// t
                writer.writeEndElement(); // r
            }
        }
        else
        {
            writer.writeStartElement(QStringLiteral("t"));
            QString string = cell->value().toString();
            if (isSpaceReserveNeeded(string))
                writer.writeAttribute(QStringLiteral("xml:space"), QStringLiteral("preserve"));
            writer.writeCharacters(string);
            writer.writeEndElement(); // t
        }
        writer.writeEndElement();//is
    }
    else if (cell->cellType() == Cell::Type::Number) // 'n'
    {
        writer.writeAttribute(QStringLiteral("t"), QStringLiteral("n")); // dev67

        if (cell->hasFormula())
        {
            QString strFormula = cell->formula().d->formula;
            Q_UNUSED(strFormula);
            cell->formula().saveToXml(writer);
        }

        if (cell->value().isValid())
        {   //note that, invalid value means 'v' is blank
            double value = cell->value().toDouble();
            writer.writeTextElement(QStringLiteral("v"), QString::number(value, 'g', 15));
        }
    }
    else if (cell->cellType() == Cell::Type::String) // 'str'
    {
        writer.writeAttribute(QStringLiteral("t"), QStringLiteral("str"));
        if (cell->hasFormula())
            cell->formula().saveToXml(writer);

        writer.writeTextElement(QStringLiteral("v"), cell->value().toString());
    }
    else if (cell->cellType() == Cell::Type::Boolean) // 'b'
    {
        writer.writeAttribute(QStringLiteral("t"), QStringLiteral("b"));

        // dev34

        if (cell->hasFormula())
        {
            QString strFormula = cell->formula().d->formula;
            Q_UNUSED(strFormula);
            cell->formula().saveToXml(writer);
        }

        writer.writeTextElement(QStringLiteral("v"), cell->value().toBool() ? QStringLiteral("1") : QStringLiteral("0"));
    }
    else if (cell->cellType() == Cell::Type::Date) // 'd'
    {
        // dev67

         double num = cell->value().toDouble();
         bool is1904 = q->workbook()->isDate1904();
         if (!is1904 && num > 60) // for mac os excel
         {
             num = num - 1;
         }

         // number type. see for 18.18.11 ST_CellType (Cell Type) more information.
         writer.writeAttribute(QStringLiteral("t"), QStringLiteral("n"));
         writer.writeTextElement(QStringLiteral("v"), cell->value().toString() );

    }
    else if (cell->cellType() == Cell::Type::Error) // 'e'
    {
        writer.writeAttribute(QStringLiteral("t"), QStringLiteral("e"));
        writer.writeTextElement(QStringLiteral("v"), cell->value().toString() );
    }
    else // if (cell->cellType() == Cell::CustomType)
    {
        // custom type

        if (cell->hasFormula())
        {
            QString strFormula = cell->formula().d->formula;
            Q_UNUSED(strFormula);
            cell->formula().saveToXml(writer);
        }

        if (cell->value().isValid())
        {   //note that, invalid value means 'v' is blank
            double value = cell->value().toDouble();
            writer.writeTextElement(QStringLiteral("v"), QString::number(value, 'g', 15));
        }
    }

    writer.writeEndElement(); // c
}

void WorksheetPrivate::saveXmlMergeCells(QXmlStreamWriter &writer) const
{
    if (merges.isEmpty())
        return;

    writer.writeStartElement(QStringLiteral("mergeCells"));
    writer.writeAttribute(QStringLiteral("count"), QString::number(merges.size()));

    for (const CellRange &range : qAsConst(merges))
    {
        writer.writeEmptyElement(QStringLiteral("mergeCell"));
        writer.writeAttribute(QStringLiteral("ref"), range.toString());
    }

    writer.writeEndElement(); //mergeCells
}

void WorksheetPrivate::saveXmlDataValidations(QXmlStreamWriter &writer) const
{
    if (dataValidationsList.isEmpty())
        return;

    writer.writeStartElement(QStringLiteral("dataValidations"));
    writer.writeAttribute(QStringLiteral("count"), QString::number(dataValidationsList.size()));

    for (const DataValidation &validation : qAsConst(dataValidationsList))
        validation.saveToXml(writer);

    writer.writeEndElement(); //dataValidations
}

void WorksheetPrivate::saveXmlHyperlinks(QXmlStreamWriter &writer) const
{
    if (urlTable.isEmpty())
        return;

    writer.writeStartElement(QStringLiteral("hyperlinks"));
    QMapIterator<int, QMap< int, QSharedPointer<XlsxHyperlinkData> > > it(urlTable);

    while (it.hasNext())
    {
        it.next();
        int row = it.key();
        QMapIterator< int, QSharedPointer<XlsxHyperlinkData> > it2(it.value());

        while (it2.hasNext())
        {
            it2.next();
            int col = it2.key();
            QSharedPointer<XlsxHyperlinkData> data = it2.value();
            QString ref = CellReference(row, col).toString();

            // dev57
            // writer.writeEmptyElement(QStringLiteral("hyperlink"));
            writer.writeStartElement(QStringLiteral("hyperlink"));

            writer.writeAttribute(QStringLiteral("ref"), ref); // required field

            if ( data->linkType == XlsxHyperlinkData::External )
            {
                // Update relationships
                relationships->addWorksheetRelationship(QStringLiteral("/hyperlink"), data->target, QStringLiteral("External"));

                writer.writeAttribute(QStringLiteral("r:id"), QStringLiteral("rId%1").arg(relationships->count()));
            }

            if (!data->location.isEmpty())
            {
                writer.writeAttribute(QStringLiteral("location"), data->location);
            }

            if (!data->display.isEmpty())
            {
                writer.writeAttribute(QStringLiteral("display"), data->display);
            }

            if (!data->tooltip.isEmpty())
            {
                writer.writeAttribute(QStringLiteral("tooltip"), data->tooltip);
            }

            // dev57
            writer.writeEndElement(); // hyperlink
        }
    }

    writer.writeEndElement(); // hyperlinks
}

void WorksheetPrivate::saveXmlDrawings(QXmlStreamWriter &writer) const
{
    if (!drawing)
        return;

    int idx = workbook->drawings().indexOf(drawing.get());
    relationships->addWorksheetRelationship(QStringLiteral("/drawing"), QStringLiteral("../drawings/drawing%1.xml").arg(idx+1));

    writer.writeEmptyElement(QStringLiteral("drawing"));
    writer.writeAttribute(QStringLiteral("r:id"), QStringLiteral("rId%1").arg(relationships->count()));
}

void WorksheetPrivate::splitColsInfo(int colFirst, int colLast)
{
    // Split current columnInfo, for example, if "A:H" has been set,
    // we are trying to set "B:D", there should be "A", "B:D", "E:H".
    // This will be more complex if we try to set "C:F" after "B:D".
    {
        QMapIterator<int, QSharedPointer<XlsxColumnInfo> > it(colsInfo);
        while (it.hasNext()) {
            it.next();
            QSharedPointer<XlsxColumnInfo> info = it.value();
            if (colFirst > info->firstColumn && colFirst <= info->lastColumn) {
                //split the range,
                QSharedPointer<XlsxColumnInfo> info2(new XlsxColumnInfo(*info));
                info->lastColumn = colFirst - 1;
                info2->firstColumn = colFirst;
                colsInfo.insert(colFirst, info2);
                for (int c = info2->firstColumn; c <= info2->lastColumn; ++c)
                    colsInfoHelper[c] = info2;

                break;
            }
        }
    }
    {
        QMapIterator<int, QSharedPointer<XlsxColumnInfo> > it(colsInfo);
        while (it.hasNext()) {
            it.next();
            QSharedPointer<XlsxColumnInfo> info = it.value();
            if (colLast >= info->firstColumn && colLast < info->lastColumn) {
                QSharedPointer<XlsxColumnInfo> info2(new XlsxColumnInfo(*info));
                info->lastColumn = colLast;
                info2->firstColumn = colLast + 1;
                colsInfo.insert(colLast + 1, info2);
                for (int c = info2->firstColumn; c <= info2->lastColumn; ++c)
                    colsInfoHelper[c] = info2;

                break;
            }
        }
    }
}

bool WorksheetPrivate::isColumnRangeValid(int colFirst, int colLast)
{
    bool ignore_row = true;
    bool ignore_col = false;

    if (colFirst > colLast)
        return false;

    if (checkDimensions(1, colLast, ignore_row, ignore_col))
        return false;
    if (checkDimensions(1, colFirst, ignore_row, ignore_col))
        return false;

    return true;
}

QList<int> WorksheetPrivate ::getColumnIndexes(int colFirst, int colLast)
{
    splitColsInfo(colFirst, colLast);

    QList<int> nodes;
    nodes.append(colFirst);
    for (int col = colFirst; col <= colLast; ++col)
    {
        auto it = colsInfo.constFind(col);
        if (it != colsInfo.constEnd())
        {
            if (nodes.last() != col)
                nodes.append(col);

            int nextCol = (*it)->lastColumn + 1;
            if (nextCol <= colLast)
                nodes.append(nextCol);
        }
    }

    return nodes;
}

/*!
  Sets width in characters of a \a range of columns to \a width.
  Returns true on success.
 */
bool Worksheet::setColumnWidth(const CellRange &range, double width)
{
    if (!range.isValid())
        return false;

    return setColumnWidth(range.firstColumn(), range.lastColumn(), width);
}

/*!
  Sets format property of a \a range of columns to \a format. Columns are 1-indexed.
  Returns true on success.
 */
bool Worksheet::setColumnFormat(const CellRange& range, const Format &format)
{
    if (!range.isValid())
        return false;

    return setColumnFormat(range.firstColumn(), range.lastColumn(), format);
}

/*!
  Sets hidden property of a \a range of columns to \a hidden. Columns are 1-indexed.
  Hidden columns are not visible.
  Returns true on success.
 */
bool Worksheet::setColumnHidden(const CellRange &range, bool hidden)
{
    if (!range.isValid())
        return false;

    return setColumnHidden(range.firstColumn(), range.lastColumn(), hidden);
}

/*!
  Sets width in characters for columns [\a colFirst, \a colLast] to \a width.
  Columns are 1-indexed.
  Returns true on success.
 */
bool Worksheet::setColumnWidth(int colFirst, int colLast, double width)
{
    Q_D(Worksheet);

    const auto columnInfoList = d->getColumnInfoList(colFirst, colLast);
    for (const auto &columnInfo : columnInfoList)
       columnInfo->width = width;

    return (columnInfoList.count() > 0);
}

/*!
  Sets format property of a range of columns [\a colFirst, \a colLast] to \a format.
  Columns are 1-indexed.
  Returns true on success.
 */
bool Worksheet::setColumnFormat(int colFirst, int colLast, const Format &format)
{
    Q_D(Worksheet);

    const QList <QSharedPointer<XlsxColumnInfo> > columnInfoList = d->getColumnInfoList(colFirst, colLast);
    for (const QSharedPointer<XlsxColumnInfo> &columnInfo : columnInfoList)
       columnInfo->format = format;

    if(columnInfoList.count() > 0) {
       d->workbook->styles()->addXfFormat(format);
       return true;
    }

    return false;
}

/*!
  Sets hidden property of a range of columns [\a colFirst, \a colLast] to \a hidden.
  Columns are 1-indexed. Returns true on success.
 */
bool Worksheet::setColumnHidden(int colFirst, int colLast, bool hidden)
{
    Q_D(Worksheet);

    const QList <QSharedPointer<XlsxColumnInfo> > columnInfoList = d->getColumnInfoList(colFirst, colLast);
    for (const QSharedPointer<XlsxColumnInfo> &columnInfo : columnInfoList)
       columnInfo->hidden = hidden;

    return (columnInfoList.count() > 0);
}

/*!
  Returns width of the \a column in characters of the normal font. Columns are 1-indexed.
 */
double Worksheet::columnWidth(int column)
{
    Q_D(Worksheet);

    QList<QSharedPointer<XlsxColumnInfo> > columnInfoList = d->getColumnInfoList(column, column);

    if (columnInfoList.count() == 1) {
        // column information is found
        // qDebug() << "[debug]" << __FUNCTION__ <<  "column (info) is found. " << column << oneColWidth;
        if (columnInfoList.at(0)->isSetWidth)
            return columnInfoList.at(0)->width;
    }

    // use default width
    // column information is not found
    // qDebug() << "[debug]" << __FUNCTION__ <<  "column (info) is not found. " << column;
    double defaultColWidth = d->sheetFormatProps.defaultColWidth;
    return defaultColWidth;
}

/*!
  Returns formatting of the \a column. Columns are 1-indexed.
 */
Format Worksheet::columnFormat(int column)
{
    Q_D(Worksheet);

    QList <QSharedPointer<XlsxColumnInfo> > columnInfoList = d->getColumnInfoList(column, column);
    if (columnInfoList.count() == 1)
       return columnInfoList.at(0)->format;

    return Format();
}

/*!
  Returns true if \a column is hidden. Columns are 1-indexed.
 */
bool Worksheet::isColumnHidden(int column)
{
    Q_D(Worksheet);

    QList <QSharedPointer<XlsxColumnInfo> > columnInfoList = d->getColumnInfoList(column, column);
    if (columnInfoList.count() == 1)
       return columnInfoList.at(0)->hidden;

    return false;
}

/*!
  Sets the \a height of the rows including and between \a rowFirst and \a rowLast.
  Row height measured in point size.
  Rows are 1-indexed.

  Returns true if success.
*/
bool Worksheet::setRowHeight(int rowFirst,int rowLast, double height)
{
    Q_D(Worksheet);

    const QList <QSharedPointer<XlsxRowInfo> > rowInfoList = d->getRowInfoList(rowFirst,rowLast);
    for (const QSharedPointer<XlsxRowInfo> &rowInfo : rowInfoList) {
        rowInfo->height = height;
        rowInfo->customHeight = true;
    }

    return rowInfoList.count() > 0;
}

/*!
  Sets the \a format of the rows including and between \a rowFirst and \a rowLast.
  Rows are 1-indexed.

  Returns true if success.
*/
bool Worksheet::setRowFormat(int rowFirst,int rowLast, const Format &format)
{
    Q_D(Worksheet);

    const QList <QSharedPointer<XlsxRowInfo> > rowInfoList = d->getRowInfoList(rowFirst,rowLast);
    for (const QSharedPointer<XlsxRowInfo> &rowInfo : rowInfoList)
        rowInfo->format = format;

    d->workbook->styles()->addXfFormat(format);
    return rowInfoList.count() > 0;
}

/*!
  Sets the \a hidden proeprty of the rows including and between \a rowFirst and \a rowLast.
  Rows are 1-indexed. If hidden is true rows will not be visible.

  Returns true if success.
*/
bool Worksheet::setRowHidden(int rowFirst,int rowLast, bool hidden)
{
    Q_D(Worksheet);

    const QList <QSharedPointer<XlsxRowInfo> > rowInfoList = d->getRowInfoList(rowFirst,rowLast);
    for (const QSharedPointer<XlsxRowInfo> &rowInfo : rowInfoList)
        rowInfo->hidden = hidden;

    return rowInfoList.count() > 0;
}

/*!
 Returns height of \a row in points.
*/
double Worksheet::rowHeight(int row)
{
    Q_D(Worksheet);
    const int min_col = d->dimension.isValid() ? d->dimension.firstColumn() : 1;

    auto it = d->rowsInfo.constFind(row);
    if (d->checkDimensions(row, min_col, false, true) || it == d->rowsInfo.constEnd())
    {
        return d->sheetFormatProps.defaultRowHeight; //return default on invalid row
    }

    return (*it)->height;
}

/*!
 Returns format of \a row.
*/
Format Worksheet::rowFormat(int row)
{
    Q_D(Worksheet);
    const int min_col = d->dimension.isValid() ? d->dimension.firstColumn() : 1;
    auto it = d->rowsInfo.constFind(row);
    if (d->checkDimensions(row, min_col, false, true) || it == d->rowsInfo.constEnd())
        return Format(); //return default on invalid row

    return (*it)->format;
}

/*!
 Returns true if \a row is hidden.
*/
bool Worksheet::isRowHidden(int row)
{
    Q_D(Worksheet);
    const int min_col = d->dimension.isValid() ? d->dimension.firstColumn() : 1;
    auto it = d->rowsInfo.constFind(row);
    if (d->checkDimensions(row, min_col, false, true) || it == d->rowsInfo.constEnd())
        return false; //return default on invalid row

    return (*it)->hidden;
}

/*!
   Groups rows from \a rowFirst to \a rowLast with the given \a collapsed.

   Returns false if error occurs.
 */
bool Worksheet::groupRows(int rowFirst, int rowLast, bool collapsed)
{
    Q_D(Worksheet);

    for (int row=rowFirst; row<=rowLast; ++row) {
        auto it = d->rowsInfo.find(row);
        if (it != d->rowsInfo.end()) {
            (*it)->outlineLevel += 1;
        } else {
            QSharedPointer<XlsxRowInfo> info(new XlsxRowInfo);
            info->outlineLevel += 1;
            it = d->rowsInfo.insert(row, info);
        }
        if (collapsed)
            (*it)->hidden = true;
    }
    if (collapsed) {
        auto it = d->rowsInfo.find(rowLast+1);
        if (it == d->rowsInfo.end())
            it = d->rowsInfo.insert(rowLast+1, QSharedPointer<XlsxRowInfo>(new XlsxRowInfo));
        (*it)->collapsed = true;
    }
    return true;
}

/*!
    \overload

    Groups columns with the given \a range and \a collapsed.
 */
bool Worksheet::groupColumns(const CellRange &range, bool collapsed)
{
    if (!range.isValid())
        return false;

    return groupColumns(range.firstColumn(), range.lastColumn(), collapsed);
}

/*!
   Groups columns from \a colFirst to \a colLast with the given \a collapsed.
   Returns false if error occurs.
*/
bool Worksheet::groupColumns(int colFirst, int colLast, bool collapsed)
{
    Q_D(Worksheet);

    d->splitColsInfo(colFirst, colLast);

    QList<int> nodes;
    nodes.append(colFirst);
    for (int col = colFirst; col <= colLast; ++col) {
        auto it = d->colsInfo.constFind(col);
        if (it != d->colsInfo.constEnd()) {
            if (nodes.last() != col)
                nodes.append(col);
            int nextCol = (*it)->lastColumn + 1;
            if (nextCol <= colLast)
                nodes.append(nextCol);
        }
    }

    for (int idx = 0; idx < nodes.size(); ++idx)
    {
        int colStart = nodes[idx];
        auto it = d->colsInfo.constFind(colStart);
        if (it != d->colsInfo.constEnd())
        {
            (*it)->outlineLevel += 1;
            if (collapsed)
                (*it)->hidden = true;
        }
        else
        {
            int colEnd = (idx == nodes.size() - 1) ? colLast : nodes[idx+1] - 1;
            QSharedPointer<XlsxColumnInfo> info(new XlsxColumnInfo(colStart, colEnd, false));
            info->outlineLevel += 1;
            d->colsInfo.insert(colFirst, info);
            if (collapsed)
                info->hidden = true;
            for (int c = colStart; c <= colEnd; ++c)
                d->colsInfoHelper[c] = info;
        }
    }

    if (collapsed) {
        int col = colLast+1;
        d->splitColsInfo(col, col);
        auto it = d->colsInfo.constFind(col);
        if (it != d->colsInfo.constEnd())
            (*it)->collapsed = true;
        else {
            QSharedPointer<XlsxColumnInfo> info(new XlsxColumnInfo(col, col, false));
            info->collapsed = true;
            d->colsInfo.insert(col, info);
            d->colsInfoHelper[col] = info;
        }
    }

    return false;
}

/*!
    Return the range that contains cell data.
 */
CellRange Worksheet::dimension() const
{
    Q_D(const Worksheet);
    return d->dimension;
}

/*
 Convert the height of a cell from user's units to pixels. If the
 height hasn't been set by the user we use the default value. If
 the row is hidden it has a value of zero.
*/
int WorksheetPrivate::rowPixelsSize(int row) const
{
    double height;
    auto it = rowSizes.constFind(row);
    if (it != rowSizes.constEnd())
        height = it.value();
    else
        height = defaultRowHeight;
    return static_cast<int>(4.0 / 3.0 *height);
}

/*
 Convert the width of a cell from user's units to pixels. Excel rounds
 the column width to the nearest pixel. If the width hasn't been set
 by the user we use the default value. If the column is hidden it
 has a value of zero.
*/
int WorksheetPrivate::colPixelsSize(int col) const
{
    double max_digit_width = 7.0; //For Calabri 11
    double padding = 5.0;
    int pixels = 0;

    auto it = colSizes.constFind(col);
    if (it != colSizes.constEnd()) {
        double width = it.value();
        if (width < 1)
            pixels = static_cast<int>(width * (max_digit_width + padding) + 0.5);
        else
            pixels = static_cast<int>(width * max_digit_width + 0.5) + padding;
    } else {
        pixels = 64;
    }
    return pixels;
}

void WorksheetPrivate::loadXmlSheetData(QXmlStreamReader &reader)
{
    Q_ASSERT(reader.name() == QLatin1String("sheetData"));

    const auto &name = reader.name();

    //since row numbers are optional, we need to track the current row
    int currentRow = 0;
    while (!reader.atEnd())    {
        auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            const auto &a = reader.attributes();

            if (reader.name() == QLatin1String("row")) {
                currentRow++;
                if (a.hasAttribute(QLatin1String("customFormat"))
                        || a.hasAttribute(QLatin1String("customHeight"))
                        || a.hasAttribute(QLatin1String("hidden"))
                        || a.hasAttribute(QLatin1String("outlineLevel"))
                        || a.hasAttribute(QLatin1String("collapsed")))    {

                    QSharedPointer<XlsxRowInfo> info(new XlsxRowInfo);
                    if (a.hasAttribute(QLatin1String("customFormat")) &&
                            a.hasAttribute(QLatin1String("s"))) {
                        int idx = a.value(QLatin1String("s")).toInt();
                        info->format = workbook->styles()->xfFormat(idx);
                    }
                    parseAttributeBool(a, QLatin1String("customHeight"), info->customHeight);
                    parseAttributeDouble(a, QLatin1String("ht"), info->height);
                    parseAttributeBool(a, QLatin1String("hidden"), info->hidden);
                    parseAttributeBool(a, QLatin1String("collapsed"), info->collapsed);
                    parseAttributeInt(a, QLatin1String("outlineLevel"), info->outlineLevel);

                    //"r" is optional too.
                    if (a.hasAttribute(QLatin1String("r"))) {
                        int row = a.value(QLatin1String("r")).toInt();
                        rowsInfo[row] = info;
                    }
                    else rowsInfo[currentRow] = info;
                }
            }
            else if (reader.name() == QLatin1String("c"))  //Cell
                loadXmlCell(reader);
        }
        else if (token == QXmlStreamReader::EndElement && reader.name() == name)
            break;
    }
}

void WorksheetPrivate::loadXmlCell(QXmlStreamReader &reader)
{
    Q_Q(Worksheet);

    const auto &name = reader.name();
    const auto &a = reader.attributes();

    QString r = a.value(QLatin1String("r")).toString();
    CellReference pos(r);

    //get format
    Format format;
    qint32 styleIndex = -1;
    if (a.hasAttribute(QLatin1String("s"))) {// Style (defined in the styles.xml file)
        //"s" == style index
        int idx = a.value(QLatin1String("s")).toInt();
        format = workbook->styles()->xfFormat(idx);
        styleIndex = idx;
    }

    auto cellType = Cell::Type::Custom;

    if (a.hasAttribute(QLatin1String("t"))) {// Type
        auto typeString = a.value(QLatin1String("t")).toString();
        Cell::fromString(typeString, cellType);
    }

    if (Cell::isDateType(cellType, format)) cellType = Cell::Type::Date;

    // create a heap of new cell
    auto cell = std::make_shared<Cell>(QVariant{}, cellType, format, q, styleIndex);

    while (!reader.atEnd())    {
        auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("f")) {// formula
                CellFormula &formula = cell->d_func()->formula;
                formula.loadFromXml(reader);
                if (formula.formulaType() == CellFormula::Type::Shared &&
                    !formula.formulaText().isEmpty()) {
                    int si = formula.sharedIndex();
                    sharedFormulaMap[si] = formula;
                }
            }
            else if (reader.name() == QLatin1String("v")) {// Value
                QString value = reader.readElementText();
                if (cellType == Cell::Type::SharedString) {
                    int sst_idx = value.toInt();
                    sharedStrings()->incRefByStringIndex(sst_idx);
                    RichString rs = sharedStrings()->getSharedString(sst_idx);
                    QString strPlainString = rs.toPlainString();
                    cell->d_func()->value = strPlainString;
                    if (rs.isRichString())
                        cell->d_func()->richString = rs;
                }
                else if (cellType == Cell::Type::Number) {
                    cell->d_func()->value = value.toDouble();
                }
                else if (cellType == Cell::Type::Boolean) {
                    cell->d_func()->value = fromST_Boolean(value);
                }
                else  if (cellType == Cell::Type::Date) {
                    // [dev54] DateType

                    double dValue = value.toDouble(); // days from 1900(or 1904)
                    bool bIsDate1904 = q->workbook()->isDate1904();

                    QVariant vDatetimeValue = datetimeFromNumber( dValue, bIsDate1904 );
                    Q_UNUSED(vDatetimeValue);
                    // cell->d_func()->value = vDatetimeValue;
                    cell->d_func()->value = dValue; // dev67
                }
                else {
                    // ELSE type
                    cell->d_func()->value = value;
                }
            }
            else if (reader.name() == QLatin1String("is")) {
                //TODO: add rich text read support
                cell->d_func()->value = reader.readElementText();
            }
            else if (reader.name() == QLatin1String("extLst"))
                reader.skipCurrentElement();
        }
        else if (token == QXmlStreamReader::EndElement && reader.name() == name)
            break;
    }
    cellTable[pos.row()][pos.column()] = cell;
}

void WorksheetPrivate::loadXmlColumnsInfo(QXmlStreamReader &reader)
{
    Q_ASSERT(reader.name() == QLatin1String("cols"));

    while (!reader.atEnd() &&
           !(reader.name() == QLatin1String("cols") &&
             reader.tokenType() == QXmlStreamReader::EndElement))
    {
        reader.readNextStartElement();
        if (reader.tokenType() == QXmlStreamReader::StartElement)
        {
            if (reader.name() == QLatin1String("col"))
            {
                QSharedPointer<XlsxColumnInfo> info(new XlsxColumnInfo(0, 1, false));

                QXmlStreamAttributes colAttrs = reader.attributes();
                int min = colAttrs.value(QLatin1String("min")).toInt();
                int max = colAttrs.value(QLatin1String("max")).toInt();
                info->firstColumn = min;
                info->lastColumn = max;

                //Flag indicating that the column width for the affected column(s) is different from the
                // default or has been manually set
                if(colAttrs.hasAttribute(QLatin1String("customWidth")))
                {
                    info->customWidth = colAttrs.value(QLatin1String("customWidth")) == QLatin1String("1");
                }

                //Note, node may have "width" without "customWidth"
                // [dev54]
                if (colAttrs.hasAttribute(QLatin1String("width")))
                {
                    double width = colAttrs.value(QLatin1String("width")).toDouble();
                    info->width = width;
                    info->isSetWidth = true; // [dev54]
                }

                info->hidden = colAttrs.value(QLatin1String("hidden")) == QLatin1String("1");
                info->collapsed = colAttrs.value(QLatin1String("collapsed")) == QLatin1String("1");

                if (colAttrs.hasAttribute(QLatin1String("style")))
                {
                    int idx = colAttrs.value(QLatin1String("style")).toInt();
                    info->format = workbook->styles()->xfFormat(idx);
                }

                if (colAttrs.hasAttribute(QLatin1String("outlineLevel")))
                {
                    info->outlineLevel = colAttrs.value(QLatin1String("outlineLevel")).toInt();
                }

                // qDebug() << "[debug] " << __FUNCTION__ << min << max << info->width << hasWidth;

                colsInfo.insert(min, info);
                for (int col = min ; col <= max ; ++col)
                {
                    colsInfoHelper[col] = info;
                }
            }
        }
    }
}

void WorksheetPrivate::loadXmlMergeCells(QXmlStreamReader &reader)
{
    // issue #173 https://github.com/QtExcel/QXlsx/issues/173

    Q_ASSERT(reader.name() == QLatin1String("mergeCells"));

    QXmlStreamAttributes attributes = reader.attributes();

    bool isCount = attributes.hasAttribute(QLatin1String("count"));
    int count = 0;
    if ( !isCount )
    {
        qWarning("no count");
    }
    else
    {
        count = attributes.value(QLatin1String("count")).toInt();
    }

    while ( !reader.atEnd() &&
            !(reader.name() == QLatin1String("mergeCells") &&
            reader.tokenType() == QXmlStreamReader::EndElement) )
    {
        reader.readNextStartElement();
        if (reader.tokenType() == QXmlStreamReader::StartElement)
        {
            if (reader.name() == QLatin1String("mergeCell"))
            {
                QXmlStreamAttributes attrs = reader.attributes();
                QString rangeStr = attrs.value(QLatin1String("ref")).toString();
                merges.append(CellRange(rangeStr));
            }
        }
    }

    if (isCount)
    {
        int mergesSize = merges.size();
        if ( mergesSize != count )
        {
            qWarning("read merge cells error");
        }
    }

}

void WorksheetPrivate::loadXmlDataValidations(QXmlStreamReader &reader)
{
    Q_ASSERT(reader.name() == QLatin1String("dataValidations"));
    QXmlStreamAttributes attributes = reader.attributes();
    int count = attributes.value(QLatin1String("count")).toInt();

    while (!reader.atEnd() && !(reader.name() == QLatin1String("dataValidations")
            && reader.tokenType() == QXmlStreamReader::EndElement)) {
        reader.readNextStartElement();
        if (reader.tokenType() == QXmlStreamReader::StartElement
                && reader.name() == QLatin1String("dataValidation")) {
            dataValidationsList.append(DataValidation::loadFromXml(reader));
        }
    }

    if (dataValidationsList.size() != count)
        qDebug("read data validation error");
}

void WorksheetPrivate::loadXmlSheetViews(QXmlStreamReader &reader)
{
    Q_ASSERT(reader.name() == QLatin1String("sheetViews"));

    while (!reader.atEnd() && !(reader.name() == QLatin1String("sheetViews")
            && reader.tokenType() == QXmlStreamReader::EndElement)) {
        reader.readNextStartElement();
        if (reader.tokenType() == QXmlStreamReader::StartElement && reader.name() == QLatin1String("sheetView")) {
            QXmlStreamAttributes attrs = reader.attributes();
            //default false
            windowProtection = attrs.value(QLatin1String("windowProtection")) == QLatin1String("1");
            showFormulas = attrs.value(QLatin1String("showFormulas")) == QLatin1String("1");
            rightToLeft = attrs.value(QLatin1String("rightToLeft")) == QLatin1String("1");
            tabSelected = attrs.value(QLatin1String("tabSelected")) == QLatin1String("1");
            //default true
            showGridLines = attrs.value(QLatin1String("showGridLines")) != QLatin1String("0");
            showRowColHeaders = attrs.value(QLatin1String("showRowColHeaders")) != QLatin1String("0");
            showZeros = attrs.value(QLatin1String("showZeros")) != QLatin1String("0");
            showRuler = attrs.value(QLatin1String("showRuler")) != QLatin1String("0");
            showOutlineSymbols = attrs.value(QLatin1String("showOutlineSymbols")) != QLatin1String("0");
            showWhiteSpace = attrs.value(QLatin1String("showWhiteSpace")) != QLatin1String("0");
        }
    }
}

void WorksheetPrivate::loadXmlSheetFormatProps(QXmlStreamReader &reader)
{
    Q_ASSERT(reader.name() == QLatin1String("sheetFormatPr"));

    const QXmlStreamAttributes attributes = reader.attributes();
    XlsxSheetFormatProps formatProps;
    bool isSetWidth = false;

    // Retain default values
    for  (const QXmlStreamAttribute &attrib : attributes)
    {
        if(attrib.name() == QLatin1String("baseColWidth") )
        {
            formatProps.baseColWidth = attrib.value().toInt();
        }
        else if(attrib.name() == QLatin1String("customHeight"))
        {
            formatProps.customHeight = attrib.value() == QLatin1String("1");
        }
        else if(attrib.name() == QLatin1String("defaultColWidth"))
        {
            double dDefaultColWidth = attrib.value().toDouble();
            formatProps.defaultColWidth = dDefaultColWidth;
            isSetWidth = true;
        }
        else if(attrib.name() == QLatin1String("defaultRowHeight"))
        {
            formatProps.defaultRowHeight = attrib.value().toDouble();
        }
        else if(attrib.name() == QLatin1String("outlineLevelCol"))
        {
            formatProps.outlineLevelCol = attrib.value().toInt();
        }
        else if(attrib.name() == QLatin1String("outlineLevelRow"))
        {
            formatProps.outlineLevelRow = attrib.value().toInt();
        }
        else if(attrib.name() == QLatin1String("thickBottom"))
        {
            formatProps.thickBottom = attrib.value() == QLatin1String("1");
        }
        else if(attrib.name() == QLatin1String("thickTop"))
        {
            formatProps.thickTop  = attrib.value() == QLatin1String("1");
        }
        else if(attrib.name() == QLatin1String("zeroHeight"))
        {
            formatProps.zeroHeight = attrib.value() == QLatin1String("1");
        }
    }

    // if (formatProps.defaultColWidth == 0.0)
    if ( !isSetWidth )
    {
        //not set
        double dCalcWidth = WorksheetPrivate::calculateColWidth(formatProps.baseColWidth);
        formatProps.defaultColWidth = dCalcWidth;
    }

    // [dev54]
    // Where is code of setting 'formatProps'?
    this->sheetFormatProps = formatProps;

}
double WorksheetPrivate::calculateColWidth(int characters)
{
    //TODO
    //Take normal style' font maximum width and add padding and margin pixels
    // return characters + 0.5;
    return characters;
}

void WorksheetPrivate::loadXmlHyperlinks(QXmlStreamReader &reader)
{
    Q_ASSERT(reader.name() == QLatin1String("hyperlinks"));

    while (!reader.atEnd() && !(reader.name() == QLatin1String("hyperlinks")
            && reader.tokenType() == QXmlStreamReader::EndElement)) {
        reader.readNextStartElement();
        if (reader.tokenType() == QXmlStreamReader::StartElement && reader.name() == QLatin1String("hyperlink")) {
            QXmlStreamAttributes attrs = reader.attributes();
            CellReference pos(attrs.value(QLatin1String("ref")).toString());
            if (pos.isValid()) { //Valid
                QSharedPointer<XlsxHyperlinkData> link(new XlsxHyperlinkData);
                link->display = attrs.value(QLatin1String("display")).toString();
                link->tooltip = attrs.value(QLatin1String("tooltip")).toString();
                link->location = attrs.value(QLatin1String("location")).toString();

                if (attrs.hasAttribute(QLatin1String("r:id"))) {
                    link->linkType = XlsxHyperlinkData::External;
                    XlsxRelationship ship = relationships->getRelationshipById(attrs.value(QLatin1String("r:id")).toString());
                    link->target = ship.target;
                } else {
                    link->linkType = XlsxHyperlinkData::Internal;
                }

                urlTable[pos.row()][pos.column()] = link;
            }
        }
    }
}

QList <QSharedPointer<XlsxColumnInfo> > WorksheetPrivate::getColumnInfoList(int colFirst, int colLast)
{
    QList <QSharedPointer<XlsxColumnInfo> > columnsInfoList;
    if (isColumnRangeValid(colFirst,colLast))
    {
        QList<int> nodes = getColumnIndexes(colFirst, colLast);

        for (int idx = 0; idx < nodes.size(); ++idx)
        {
            int colStart = nodes[idx];
            auto it = colsInfo.constFind(colStart);
            if (it != colsInfo.constEnd())
            {
                columnsInfoList.append(*it);
            }
            else
            {
                int colEnd = (idx == nodes.size() - 1) ? colLast : nodes[idx+1] - 1;
                QSharedPointer<XlsxColumnInfo> info(new XlsxColumnInfo(colStart, colEnd, false));
                colsInfo.insert(colFirst, info);
                columnsInfoList.append(info);
                for (int c = colStart; c <= colEnd; ++c)
                {
                    colsInfoHelper[c] = info;
                }
            }
        }
    }

    return columnsInfoList;
}

QList <QSharedPointer<XlsxRowInfo> > WorksheetPrivate::getRowInfoList(int rowFirst, int rowLast)
{
    QList <QSharedPointer<XlsxRowInfo> > rowInfoList;

    int min_col = dimension.firstColumn() < 1 ? 1 : dimension.firstColumn();

    for(int row = rowFirst; row <= rowLast; ++row) {
        if (checkDimensions(row, min_col, false, true))
            continue;

        QSharedPointer<XlsxRowInfo> rowInfo;
        if ((rowsInfo[row]).isNull()){
            rowsInfo[row] = QSharedPointer<XlsxRowInfo>(new XlsxRowInfo());
        }
        rowInfoList.append(rowsInfo[row]);
    }

    return rowInfoList;
}

bool Worksheet::loadFromXmlFile(QIODevice *device)
{
    Q_D(Worksheet);

    QXmlStreamReader reader(device);
    while (!reader.atEnd()) {
        auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            const auto &a = reader.attributes();
            if (reader.name() == QLatin1String("dimension")) {
                QString range = a.value(QLatin1String("ref")).toString();
                d->dimension = CellRange(range);
            }
            else if (reader.name() == QLatin1String("sheetViews"))
                d->loadXmlSheetViews(reader);
            else if (reader.name() == QLatin1String("sheetFormatPr"))
                d->loadXmlSheetFormatProps(reader);
            else if (reader.name() == QLatin1String("cols"))
                d->loadXmlColumnsInfo(reader);
            else if (reader.name() == QLatin1String("sheetData"))
                d->loadXmlSheetData(reader);
            else if (reader.name() == QLatin1String("sheetProtection")) {
                SheetProtection s;
                s.read(reader);
                d->sheetProtection = s;
            }
            else if (reader.name() == QLatin1String("mergeCells"))
                d->loadXmlMergeCells(reader);
            else if (reader.name() == QLatin1String("dataValidations"))
                d->loadXmlDataValidations(reader);
            else if (reader.name() == QLatin1String("conditionalFormatting")) {
                ConditionalFormatting cf;
                cf.loadFromXml(reader, workbook()->styles());
                d->conditionalFormattingList.append(cf);
            }
            else if (reader.name() == QLatin1String("hyperlinks"))
                d->loadXmlHyperlinks(reader);
            else if(reader.name() == QLatin1String("pageSetup"))
                d->pageSetup.read(reader);
            else if(reader.name() == QLatin1String("pageMargins"))
                d->pageMargins.read(reader);
            else if(reader.name() == QLatin1String("headerFooter"))
                d->headerFooter.read(reader);
            else if (reader.name() == QLatin1String("drawing")) {
                QString rId = a.value(QStringLiteral("r:id")).toString();
                QString name = d->relationships->getRelationshipById(rId).target;

                const auto parts = splitPath(filePath());
                QString path = QDir::cleanPath(parts.first() + QLatin1String("/") + name);

                d->drawing = std::make_shared<Drawing>(this, F_LoadFromExists);
                d->drawing->setFilePath(path);
            }
            else if (reader.name() == QLatin1String("picture")) {
                QString rId = a.value(QLatin1String("r:id")).toString();
                QString name = d->relationships->getRelationshipById(rId).target;

                const auto parts = splitPath(filePath());
                QString path = QDir::cleanPath(parts.first() + QLatin1String("/") + name);

                bool exist = false;
                const auto mfs = d->workbook->mediaFiles();
                for (const auto &mf : mfs) {
                    if (mf->fileName() == path) {
                        //already exist
                        exist = true;
                        d->pictureFile = mf;
                        break;
                    }
                }
                if (!exist) {
                    d->pictureFile = QSharedPointer<MediaFile>(new MediaFile(path));
                    d->workbook->addMediaFile(d->pictureFile, true);
                }
            }
            else if (reader.name() == QLatin1String("extLst"))
                d->extLst.read(reader);
        }
    }

    d->validateDimension();
    return true;
}

/*
 *  Documents imported from Google Docs does not contain dimension data.
 */
void WorksheetPrivate::validateDimension()
{
    if (dimension.isValid() || cellTable.isEmpty())
        return;

    const auto firstRow = cellTable.constBegin().key();

    const auto lastRow = (--cellTable.constEnd()).key();

    int firstColumn = -1;
    int lastColumn = -1;

    for ( auto&& it = cellTable.constBegin()
            ; it != cellTable.constEnd()
            ; ++it )
    {
        Q_ASSERT(!it.value().isEmpty());

        if (firstColumn == -1 || it.value().constBegin().key() < firstColumn)
            firstColumn = it.value().constBegin().key();

        if (lastColumn == -1 || (--it.value().constEnd()).key() > lastColumn)
        {
            lastColumn = (--it.value().constEnd()).key();
        }
    }

    CellRange cr(firstRow, firstColumn, lastRow, lastColumn);

    if (cr.isValid())
        dimension = cr;
}

/*!
 * \internal
 *  Unit test can use this member to get sharedString object.
 */
SharedStrings *WorksheetPrivate::sharedStrings() const
{
    return workbook->sharedStrings();
}

QMap<CellReference, std::shared_ptr<Cell> > Worksheet::getFullCells(int* maxRow, int* maxCol)
{
    Q_D(const Worksheet);

    // return values
    (*maxRow) = -1;
    (*maxCol) = -1;
    QMap<CellReference, std::shared_ptr<Cell> > ret;

    // QString privateName = d->name; // name of sheet (not object type)
    // qDebug() << privateName ;

    if (d->type != AbstractSheet::Type::Worksheet) {
        qWarning("unsupported sheet type.");
        return ret;
    }

    for (auto _it = d->cellTable.constBegin(); _it != d->cellTable.constEnd(); ++_it)
    {
        int keyI = _it.key(); // key (cell row)
        const auto &valI = _it.value();

        for (auto _iit = valI.constBegin(); _iit != valI.constEnd(); ++_iit )
        {
            int keyII = _iit.key(); // key (cell column)
            std::shared_ptr<Cell> ptrCell = _iit.value(); // value

            CellReference cl;

            cl.setRow(keyI);
            if (keyI > (*maxRow) )  (*maxRow) = keyI;

            cl.setColumn(keyII);
            if ( keyII > (*maxCol) ) (*maxCol) = keyII;

            ret.insert(cl, ptrCell);
        }
    }

    return ret;
}

}
