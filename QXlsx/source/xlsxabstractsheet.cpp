// xlsxabstractsheet.cpp

#include <QtGlobal>
#include <QBuffer>
#include <QFileInfo>
#include <QDir>

#include "xlsxabstractsheet.h"
#include "xlsxabstractsheet_p.h"
#include "xlsxworkbook.h"
#include "xlsxutility_p.h"
#include <QDebug>

namespace QXlsx {

AbstractSheetPrivate::AbstractSheetPrivate(AbstractSheet *p, AbstractSheet::CreateFlag flag)
    : AbstractOOXmlFilePrivate(p, flag)
{
    type = AbstractSheet::Type::Worksheet;
    sheetState = AbstractSheet::Visibility::Visible;
}

AbstractSheetPrivate::~AbstractSheetPrivate()
{
}

void AbstractSheetPrivate::loadXmlSheetViews(QXmlStreamReader &reader)
{
    const auto &name = reader.name();

    while (!reader.atEnd()) {
        const auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("sheetView")) {
                SheetView view;
                view.read(reader);
                sheetViews << view;
            }
        }
        else if (token == QXmlStreamReader::EndElement && reader.name() == name)
            break;
    }
}

void AbstractSheetPrivate::loadXmlPicture(QXmlStreamReader &reader)
{
    const auto &a = reader.attributes();
    QString rId = a.value(QLatin1String("r:id")).toString();
    QString name = relationships->getRelationshipById(rId).target;

    const auto parts = splitPath(filePathInPackage);
    QString path = QDir::cleanPath(parts.first() + QLatin1String("/") + name);

    bool exist = false;
    const auto mfs = workbook->mediaFiles();
    for (const auto &mf : mfs) {
        if (auto media = mf.lock(); media->fileName() == path) {
            //already exist
            exist = true;
            pictureFile = media;
            break;
        }
    }
    if (!exist) {
        pictureFile = QSharedPointer<MediaFile>(new MediaFile(path));
        workbook->addMediaFile(pictureFile, true);
    }
}

void AbstractSheetPrivate::loadXmlDrawing(QXmlStreamReader &reader)
{
    Q_Q(AbstractSheet);
    const auto &a = reader.attributes();
    QString rId = a.value(QStringLiteral("r:id")).toString();
    QString name = relationships->getRelationshipById(rId).target;

    const auto parts = splitPath(filePathInPackage);
    QString path = QDir::cleanPath(parts.first() + QLatin1String("/") + name);

    drawing = std::make_shared<Drawing>(q, AbstractOOXmlFile::F_LoadFromExists);
    drawing->setFilePath(path);
}

void AbstractSheetPrivate::saveXmlSheetViews(QXmlStreamWriter &writer, bool saveWorksheet) const
{
    auto views = sheetViews;
    if (sheetViews.isEmpty()) views << SheetView();
    writer.writeStartElement(QLatin1String("sheetViews"));
    for (auto &view: views)
        if (view.isValid()) view.write(writer, QLatin1String("sheetView"), saveWorksheet);
    writer.writeEndElement(); //sheetViews
}

void AbstractSheetPrivate::saveXmlDrawings(QXmlStreamWriter &writer) const
{
    if (!drawing)
        return;

    int idx = workbook->drawings().indexOf(drawing.get());
    relationships->addWorksheetRelationship(QLatin1String("/drawing"), QString("../drawings/drawing%1.xml").arg(idx+1));

    writer.writeEmptyElement(QLatin1String("drawing"));
    writer.writeAttribute(QLatin1String("r:id"), QString("rId%1").arg(relationships->count()));
}

void AbstractSheetPrivate::saveXmlPicture(QXmlStreamWriter &writer) const
{
    if (pictureFile) {
        relationships->addDocumentRelationship(QStringLiteral("/image"),
                                               QStringLiteral("../media/image%1.%2")
                                                   .arg(pictureFile->index()+1)
                                                   .arg(pictureFile->suffix()));
        writer.writeEmptyElement(QStringLiteral("picture"));
        writer.writeAttribute(QStringLiteral("r:id"),
                              QStringLiteral("rId%1").arg(relationships->count()));
    }
}

AbstractSheet::AbstractSheet(const QString &name, int id, Workbook *workbook, AbstractSheetPrivate *d) :
    AbstractOOXmlFile(d)
{
    d_func()->name = name;
    d_func()->id = id;
    d_func()->workbook = workbook;
}

QString AbstractSheet::name() const
{
    Q_D(const AbstractSheet);
    return d->name;
}

bool AbstractSheet::setName(const QString &newName)
{
    return rename(newName);
}

bool AbstractSheet::rename(const QString &sheetName)
{
    Q_D(AbstractSheet);

    QString name = createSafeSheetName(sheetName);
    for (int i = 0; i < d->workbook->sheetsCount(); ++i) {
        if (d->workbook->sheet(i)->name() == name)
            return false;
    }

    d->name = name;
    return true;
}

AbstractSheet::Type AbstractSheet::type() const
{
    Q_D(const AbstractSheet);
    return d->type;
}

void AbstractSheet::setType(Type type)
{
    Q_D(AbstractSheet);
    d->type = type;
}

AbstractSheet::Visibility AbstractSheet::visibility() const
{
    Q_D(const AbstractSheet);
    return d->sheetState;
}

void AbstractSheet::setVisibility(Visibility visibility)
{
    Q_D(AbstractSheet);
    d->sheetState = visibility;
}

bool AbstractSheet::isHidden() const
{
    Q_D(const AbstractSheet);
    return d->sheetState != Visibility::Visible;
}

bool AbstractSheet::isVisible() const
{
    return !isHidden();
}

void AbstractSheet::setHidden(bool hidden)
{
    Q_D(AbstractSheet);
    if (!hidden) d->sheetState = Visibility::Visible;
    else if (d->sheetState == Visibility::Visible) d->sheetState = Visibility::Hidden;
}

void AbstractSheet::setVisible(bool visible)
{
    Q_D(AbstractSheet);
    if (visible) d->sheetState = Visibility::Visible;
    else if (d->sheetState == Visibility::Visible) d->sheetState = Visibility::Hidden;
}

HeaderFooter AbstractSheet::headerFooter() const
{
    Q_D(const AbstractSheet);
    return d->headerFooter;
}

HeaderFooter &AbstractSheet::headerFooter()
{
    Q_D(AbstractSheet);
    return d->headerFooter;
}

std::optional<bool> AbstractSheet::differentOddEvenPage() const
{
    Q_D(const AbstractSheet);
    return d->headerFooter.differentOddEven;
}

void AbstractSheet::setDifferentOddEvenPage(bool different)
{
    Q_D(AbstractSheet);
    d->headerFooter.differentOddEven = different;
}

std::optional<bool> AbstractSheet::differentFirstPage() const
{
    Q_D(const AbstractSheet);
    return d->headerFooter.differentFirst;
}

void AbstractSheet::setDifferentFirstPage(bool different)
{
    Q_D(AbstractSheet);
    d->headerFooter.differentFirst = different;
}

QString AbstractSheet::header() const
{
    Q_D(const AbstractSheet);
    if (d->headerFooter.differentFirst.value_or(false) || d->headerFooter.differentOddEven.value_or(false))
        return {};
    return d->headerFooter.oddHeader;
}

void AbstractSheet::setHeader(const QString &header)
{
    Q_D(AbstractSheet);
    d->headerFooter.differentFirst = false;
    d->headerFooter.differentOddEven = false;
    d->headerFooter.oddHeader = header;
}

QString AbstractSheet::oddHeader() const
{
    Q_D(const AbstractSheet);
    if (d->headerFooter.differentOddEven.value_or(false)) return d->headerFooter.oddHeader;
    return {};
}

void AbstractSheet::setOddHeader(const QString &header)
{
    Q_D(AbstractSheet);
    d->headerFooter.differentOddEven = true;
    d->headerFooter.oddHeader = header;
}

QString AbstractSheet::evenHeader() const
{
    Q_D(const AbstractSheet);
    if (d->headerFooter.differentOddEven.value_or(false)) return d->headerFooter.evenHeader;
    return {};
}

void AbstractSheet::setEvenHeader(const QString &header)
{
    Q_D(AbstractSheet);
    d->headerFooter.differentOddEven = true;
    d->headerFooter.evenHeader = header;
}

QString AbstractSheet::firstHeader() const
{
    Q_D(const AbstractSheet);
    if (d->headerFooter.differentFirst.value_or(false)) return d->headerFooter.firstHeader;
    return {};
}

void AbstractSheet::setFirstHeader(const QString &header)
{
    Q_D(AbstractSheet);
    d->headerFooter.differentFirst = true;
    d->headerFooter.firstHeader = header;
}

void AbstractSheet::setHeaders(const QString &oddHeader, const QString &evenHeader, const QString &firstHeader)
{
    Q_D(AbstractSheet);
    d->headerFooter.oddHeader = oddHeader;
    if (!evenHeader.isEmpty()) {
        d->headerFooter.differentOddEven = true;
        d->headerFooter.evenHeader = evenHeader;
    }
    if (!firstHeader.isEmpty()) {
        d->headerFooter.differentFirst = true;
        d->headerFooter.firstHeader = firstHeader;
    }
}

/// Footers
///
///

QString AbstractSheet::footer() const
{
    Q_D(const AbstractSheet);
    if (d->headerFooter.differentFirst.value_or(false) || d->headerFooter.differentOddEven.value_or(false))
        return {};
    return d->headerFooter.oddFooter;
}

void AbstractSheet::setFooter(const QString &footer)
{
    Q_D(AbstractSheet);
    d->headerFooter.differentFirst = false;
    d->headerFooter.differentOddEven = false;
    d->headerFooter.oddFooter = footer;
}

QString AbstractSheet::oddFooter() const
{
    Q_D(const AbstractSheet);
    if (d->headerFooter.differentOddEven.value_or(false)) return d->headerFooter.oddFooter;
    return {};
}

void AbstractSheet::setOddFooter(const QString &footer)
{
    Q_D(AbstractSheet);
    d->headerFooter.differentOddEven = true;
    d->headerFooter.oddFooter = footer;
}

QString AbstractSheet::evenFooter() const
{
    Q_D(const AbstractSheet);
    if (d->headerFooter.differentOddEven.value_or(false)) return d->headerFooter.evenFooter;
    return {};
}

void AbstractSheet::setEvenFooter(const QString &footer)
{
    Q_D(AbstractSheet);
    d->headerFooter.differentOddEven = true;
    d->headerFooter.evenFooter = footer;
}

QString AbstractSheet::firstFooter() const
{
    Q_D(const AbstractSheet);
    if (d->headerFooter.differentFirst.value_or(false)) return d->headerFooter.firstFooter;
    return {};
}

void AbstractSheet::setFirstFooter(const QString &footer)
{
    Q_D(AbstractSheet);
    d->headerFooter.differentFirst = true;
    d->headerFooter.firstFooter = footer;
}

void AbstractSheet::setFooters(const QString &oddFooter, const QString &evenFooter, const QString &firstFooter)
{
    Q_D(AbstractSheet);
    d->headerFooter.oddFooter = oddFooter;
    if (!evenFooter.isEmpty()) {
        d->headerFooter.differentOddEven = true;
        d->headerFooter.evenFooter = evenFooter;
    }
    if (!firstFooter.isEmpty()) {
        d->headerFooter.differentFirst = true;
        d->headerFooter.firstFooter = firstFooter;
    }
}

std::optional<bool> AbstractSheet::isPublished() const
{
    Q_D(const AbstractSheet);
    return d->sheetProperties.published;
}

void AbstractSheet::setPublished(bool published)
{
    Q_D(AbstractSheet);
    d->sheetProperties.published = published;
}

QString AbstractSheet::codeName() const
{
    Q_D(const AbstractSheet);
    return d->sheetProperties.codeName;
}

bool AbstractSheet::setCodeName(const QString &codeName)
{
    Q_D(AbstractSheet);
    const auto sheets = d->workbook->sheets();
    if (std::find_if(sheets.cbegin(), sheets.cend(),
                     [codeName, this](auto sheet){
                         return sheet && sheet != this && sheet->codeName() == codeName;
                     }) != sheets.cbegin()) return false;
    d->sheetProperties.codeName = codeName;
    return true;
}

Color AbstractSheet::tabColor() const
{
    Q_D(const AbstractSheet);
    return d->sheetProperties.tabColor;
}

void AbstractSheet::setTabColor(const Color &color)
{
    Q_D(AbstractSheet);
    d->sheetProperties.tabColor = color;
}

void AbstractSheet::setTabColor(const QColor &color)
{
    Q_D(AbstractSheet);
    d->sheetProperties.tabColor = Color(color);
}

QMap<PageMargins::Position, double> AbstractSheet::pageMarginsInches() const
{
    Q_D(const AbstractSheet);
    return d->pageMargins.marginsInches();
}

QMap<PageMargins::Position, double> AbstractSheet::pageMarginsMm() const
{
    Q_D(const AbstractSheet);
    return d->pageMargins.marginsMm();
}

void AbstractSheet::setPageMarginsInches(double left, double top, double right, double bottom, double header, double footer)
{
    Q_D(AbstractSheet);
    d->pageMargins.setMarginsInches(left, top, right, bottom, header, footer);
}

void AbstractSheet::setPageMarginsMm(double left, double top, double right, double bottom, double header, double footer)
{
    Q_D(AbstractSheet);
    d->pageMargins.setMarginsMm(left, top, right, bottom, header, footer);
}

PageMargins AbstractSheet::pageMargins() const
{
    Q_D(const AbstractSheet);
    return d->pageMargins;
}

PageMargins &AbstractSheet::pageMargins()
{
    Q_D(AbstractSheet);
    return d->pageMargins;
}

void AbstractSheet::setPageMargins(const PageMargins &margins)
{
    Q_D(AbstractSheet);
    d->pageMargins = margins;
}

void AbstractSheet::setDefaultPageMargins()
{
    Q_D(AbstractSheet);
    d->pageMargins = PageMargins();
}

PageSetup AbstractSheet::pageSetup() const
{
    Q_D(const AbstractSheet);
    return d->pageSetup;
}

PageSetup &AbstractSheet::pageSetup()
{
    Q_D(AbstractSheet);
    return d->pageSetup;
}

void AbstractSheet::setPageSetup(const PageSetup &pageSetup)
{
    Q_D(AbstractSheet);
    d->pageSetup = pageSetup;
}

void AbstractSheet::setPaperSize(PageSetup::PaperSize paperSize)
{
    Q_D(AbstractSheet);
    d->pageSetup.setPaperSize(paperSize);
}

std::optional<PageSetup::PaperSize> AbstractSheet::paperSize() const
{
    Q_D(const AbstractSheet);
    return d->pageSetup.paperSize();
}

void AbstractSheet::setPaperSizeMM(double width, double height)
{
    Q_D(AbstractSheet);
    d->pageSetup.setPaperWidth(QString::number(width,'f')+"mm");
    d->pageSetup.setPaperHeight(QString::number(height,'f')+"mm");
}

void AbstractSheet::setPaperSizeInches(double width, double height)
{
    Q_D(AbstractSheet);
    d->pageSetup.setPaperWidth(QString::number(width,'f')+"in");
    d->pageSetup.setPaperHeight(QString::number(height,'f')+"in");
}

QString AbstractSheet::paperWidth() const
{
    Q_D(const AbstractSheet);
    return d->pageSetup.paperWidth();
}

void AbstractSheet::setPaperWidth(const QString &width)
{
    Q_D(AbstractSheet);
    d->pageSetup.setPaperWidth(width);
}

QString AbstractSheet::paperHeight() const
{
    Q_D(const AbstractSheet);
    return d->pageSetup.paperHeight();
}

void AbstractSheet::setPaperHeight(const QString &height)
{
    Q_D(AbstractSheet);
    d->pageSetup.setPaperHeight(height);
}

std::optional<PageSetup::Orientation> AbstractSheet::pageOrientation() const
{
    Q_D(const AbstractSheet);
    return d->pageSetup.orientation();
}

void AbstractSheet::setPageOrientation(PageSetup::Orientation orientation)
{
    Q_D(AbstractSheet);
    d->pageSetup.setOrientation(orientation);
}

std::optional<int> AbstractSheet::firstPageNumber() const
{
    Q_D(const AbstractSheet);
    return d->pageSetup.firstPageNumber();
}

void AbstractSheet::setFirstPageNumber(int number)
{
    Q_D(AbstractSheet);
    d->pageSetup.setFirstPageNumber(number);
    d->pageSetup.setUseFirstPageNumber(true);
}

std::optional<bool> AbstractSheet::useFirstPageNumber() const
{
    Q_D(const AbstractSheet);
    return d->pageSetup.useFirstPageNumber();
}

void AbstractSheet::setUseFirstPageNumber(bool use)
{
    Q_D(AbstractSheet);
    d->pageSetup.setUseFirstPageNumber(use);
}

std::optional<bool> AbstractSheet::printerDefaultsUsed() const
{
    Q_D(const AbstractSheet);
    return d->pageSetup.usePrinterDefaults();
}

void AbstractSheet::setPrinterDefaultsUsed(bool value)
{
    Q_D(AbstractSheet);
    d->pageSetup.setUsePrinterDefaults(value);
}

std::optional<bool> AbstractSheet::printBlackAndWhite() const
{
    Q_D(const AbstractSheet);
    return d->pageSetup.printBlackAndWhite();
}

void AbstractSheet::setPrintBlackAndWhite(bool value)
{
    Q_D(AbstractSheet);
    d->pageSetup.setPrintBlackAndWhite(value);
}

std::optional<bool> AbstractSheet::printDraft() const
{
    Q_D(const AbstractSheet);
    return d->pageSetup.printDraft();
}

void AbstractSheet::setPrintDraft(bool draft)
{
    Q_D(AbstractSheet);
    d->pageSetup.setPrintDraft(draft);
}

std::optional<int> AbstractSheet::horizontalDpi() const
{
    Q_D(const AbstractSheet);
    return d->pageSetup.horizontalDpi();
}

void AbstractSheet::setHorizontalDpi(int dpi)
{
    Q_D(AbstractSheet);
    if (dpi > 0) d->pageSetup.setHorizontalDpi(dpi);
}

std::optional<int> AbstractSheet::verticalDpi() const
{
    Q_D(const AbstractSheet);
    return d->pageSetup.verticalDpi();
}

void AbstractSheet::setVerticalDpi(int dpi)
{
    Q_D(AbstractSheet);
    if (dpi > 0) d->pageSetup.setVerticalDpi(dpi);
}

std::optional<int> AbstractSheet::copies() const
{
    Q_D(const AbstractSheet);
    return d->pageSetup.copies();
}

void AbstractSheet::setCopies(int count)
{
    Q_D(AbstractSheet);
    if (count > 0) d->pageSetup.setCopies(count);
}

void AbstractSheet::setBackgroundImage(const QImage &image)
{
    Q_D(AbstractSheet);

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");

    d->pictureFile = QSharedPointer<MediaFile>(new MediaFile(ba, QStringLiteral("png"), QStringLiteral("image/png")));
    d->workbook->addMediaFile(d->pictureFile);
}

void AbstractSheet::setBackgroundImage(const QString &fileName)
{
    Q_D(AbstractSheet);
    QByteArray ba;
    QString suffix = QFileInfo(fileName).suffix().toLower();
    if (suffix.toLower() == QStringLiteral("png") ||
        suffix.toLower() == QStringLiteral("jpg") ||
        suffix.toLower() == QStringLiteral("jpeg")) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly))
            ba = file.readAll();
    }
    else {
        QImage image(fileName);
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        suffix = QStringLiteral("png");
    }
    if (!ba.isEmpty()) {
        d->pictureFile = QSharedPointer<MediaFile>(new MediaFile(ba, suffix, QStringLiteral("image/")+suffix));
        d->workbook->addMediaFile(d->pictureFile);
    }
}

QImage AbstractSheet::backgroundImage() const
{
    Q_D(const AbstractSheet);
    QImage pic;
    pic.loadFromData(d->pictureFile->contents());
    return pic;
}

bool AbstractSheet::removeBackgroundImage()
{
    Q_D(AbstractSheet);
    if (d->pictureFile) {
        d->workbook->removeMediaFile(d->pictureFile);
        d->pictureFile.clear();
        return true;
    }
    return false;
}

std::optional<bool> AbstractSheet::isSelected(int viewIndex) const
{
    Q_D(const AbstractSheet);
    if (viewIndex >= 0 && viewIndex < d->sheetViews.size())
        return d->sheetViews.at(viewIndex).tabSelected;
    return {};
}

void AbstractSheet::setSelected(bool selected, int viewIndex)
{
    Q_D(AbstractSheet);

    if (viewIndex == 0 && d->sheetViews.isEmpty())
        d->sheetViews << SheetView();
    if (viewIndex >= 0 && viewIndex < d->sheetViews.size())
        d->sheetViews[viewIndex].tabSelected = selected;
}

std::optional<int> AbstractSheet::viewZoomScale(int viewIndex) const
{
    Q_D(const AbstractSheet);
    if (viewIndex >= 0 && viewIndex < d->sheetViews.size())
        return d->sheetViews.at(viewIndex).zoomScale;
    return {};
}

void AbstractSheet::setViewZoomScale(int scale, int viewIndex)
{
    Q_D(AbstractSheet);
    if (viewIndex == 0 && d->sheetViews.isEmpty())
        d->sheetViews << SheetView();
    if (viewIndex >= 0 && viewIndex < d->sheetViews.size())
        d->sheetViews[viewIndex].zoomScale = scale;
}

int AbstractSheet::workbookViewId(int viewIndex) const
{
    Q_D(const AbstractSheet);
    if (viewIndex >= 0 && viewIndex < d->sheetViews.size())
        return d->sheetViews.at(viewIndex).workbookViewId;
    return -1;
}

void AbstractSheet::setWorkbookViewId(int id, int viewIndex)
{
    Q_D(AbstractSheet);
    if (viewIndex == 0 && d->sheetViews.isEmpty())
        d->sheetViews << SheetView();
    if (viewIndex >= 0 && viewIndex < d->sheetViews.size())
        d->sheetViews[viewIndex].workbookViewId = id;
}

SheetView AbstractSheet::view(int index) const
{
    Q_D(const AbstractSheet);
    return d->sheetViews.value(index);
}

SheetView &AbstractSheet::view(int index)
{
    Q_D(AbstractSheet);
    return d->sheetViews[index];
}

SheetView AbstractSheet::lastView() const
{
    Q_D(const AbstractSheet);
    return d->sheetViews.last();
}

SheetView &AbstractSheet::lastView()
{
    Q_D(AbstractSheet);
    if(d->sheetViews.isEmpty()) d->sheetViews << SheetView();
    return d->sheetViews.last();
}

int AbstractSheet::viewsCount() const
{
    Q_D(const AbstractSheet);
    return d->sheetViews.size();
}

int AbstractSheet::addView()
{
    Q_D(AbstractSheet);
    d->sheetViews << SheetView();
    return d->sheetViews.size()-1;
}

bool AbstractSheet::removeView(int index)
{
    Q_D(AbstractSheet);
    if (index < 0 || index >= d->sheetViews.size()) return false;
    d->sheetViews.removeAt(index);
    return true;
}

SheetProtection AbstractSheet::sheetProtection() const
{
    Q_D(const AbstractSheet);
    return d->sheetProtection;
}

SheetProtection &AbstractSheet::sheetProtection()
{
    Q_D(AbstractSheet);
    return d->sheetProtection;
}

void AbstractSheet::setSheetProtection(const SheetProtection &sheetProtection)
{
    Q_D(AbstractSheet);
    d->sheetProtection = sheetProtection;
    d->sheetProtection.setProtectSheet(true);
}

bool AbstractSheet::isSheetProtected() const
{
    Q_D(const AbstractSheet);
    return d->sheetProtection.isValid();
}

bool AbstractSheet::isPasswordProtectionSet() const
{
    Q_D(const AbstractSheet);
    if (d->sheetProtection.isValid()) {
        return !d->sheetProtection.protection().algorithmName.isEmpty() and
               !d->sheetProtection.protection().hashValue.isEmpty();
    }
    return false;
}

bool AbstractSheet::setPasswordProtection(const QString &password,
                                          const QString &algorithm,
                                          const QString &salt,
                                          int spinCount)
{
    if (password.isEmpty() || PasswordProtection::algorithmForName(algorithm) == -1)
        return false;

    Q_D(AbstractSheet);

    d->sheetProtection.protection() = PasswordProtection(password, algorithm, salt, spinCount);
    d->sheetProtection.setProtectSheet(true);
    return true;
}

void AbstractSheet::setDefaultSheetProtection()
{
    Q_D(AbstractSheet);
    d->sheetProtection = SheetProtection();
    d->sheetProtection.setProtectSheet(true);
}

void AbstractSheet::removeSheetProtection()
{
    Q_D(AbstractSheet);
    d->sheetProtection = SheetProtection();
}

int AbstractSheet::id() const
{
    Q_D(const AbstractSheet);
    return d->id;
}

Drawing *AbstractSheet::drawing() const
{
    Q_D(const AbstractSheet);
    return d->drawing.get();
}

Workbook *AbstractSheet::workbook() const
{
    Q_D(const AbstractSheet);
    return d->workbook;
}

}
