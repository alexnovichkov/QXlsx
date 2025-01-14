#include "xlsxfont.h"
#include <QSharedData>

namespace QXlsx {

class FontPrivate: public QSharedData
{
public:
    FontPrivate() : QSharedData() {}
    FontPrivate(const FontPrivate &other)
        : QSharedData(other)
    {}
    ~FontPrivate() {}

    bool operator ==(const FontPrivate &other) const;

    QFont font;

    //CT_TextFont:
    //typeface string required - use QFont
    //panose string optional
    std::optional<QString> panose;
    std::optional<Font::PitchFamily> pitchFamily;
    std::optional<Font::Charset> charset;
};

Font::Font()
{
    // d is nullptr
}

Font::Font(QFont font) : d{new FontPrivate}
{
    d->font = font;
}

Font::Font(const Font &other) : d(other.d)
{

}

Font &Font::operator =(const Font &other)
{
    if (*this != other)
        d = other.d;
    return *this;
}

Font::~Font()
{

}

bool Font::isValid() const
{
    if (d) return true;
    return false;
}

bool Font::operator ==(const Font &other) const
{
    if (d == other.d) return true;
    if (!d || !other.d) return false;
    return (*d.constData() == *other.d.constData());
}

bool Font::operator !=(const Font &other) const
{
    return !operator==(other);
}

QFont Font::font() const
{
    if (d) return d->font;
    return QFont();
}

QString Font::typeface() const
{
    if (d) return d->font.family();
    return {};
}

std::optional<Font::Charset> Font::charset() const
{
    if (d) return d->charset.value_or(Charset::Default);
    return {};
}

std::optional<Font::PitchFamily> Font::pitchAndFamilySubstitute() const
{
    if (d) return d->pitchFamily;
    return {};
}

void Font::setFont(const QFont &font)
{
    if (!d) d = new FontPrivate;
    d->font = font;
}

void Font::setTypeface(const QString &typeface)
{
    if (!d) d = new FontPrivate;
    d->font.setFamily(typeface);
}

void Font::setCharset(Font::Charset charset)
{
    if (!d) d = new FontPrivate;
    d->charset = charset;
}

void Font::setPitchAndFamilySubstitute(Font::PitchFamily val)
{
    if (!d) d = new FontPrivate;
    d->pitchFamily = val;
}

void Font::write(QXmlStreamWriter &writer, const QString &name) const
{
    writer.writeEmptyElement(name);
    writer.writeAttribute(QLatin1String("typeface"), d->font.family());
    if (d->panose.has_value()) writer.writeAttribute(QLatin1String("panose"), d->panose.value());
    if (d->pitchFamily.has_value())
        writer.writeAttribute(QLatin1String("pitchFamily"),
                              QString::number(static_cast<int>(d->pitchFamily.value())));
    if (d->charset.has_value())
        writer.writeAttribute(QLatin1String("charset"),
                              QString::number(static_cast<int>(d->charset.value())));
}

void Font::read(QXmlStreamReader &reader)
{
    if (!d) d = new FontPrivate;

    const auto &a = reader.attributes();
    if (a.hasAttribute(QLatin1String("typeface"))) d->font.setFamily(a.value(QLatin1String("typeface")).toString());
    if (a.hasAttribute(QLatin1String("panose"))) d->panose = a.value(QLatin1String("typeface")).toString();
    if (a.hasAttribute(QLatin1String("pitchFamily")))
        d->pitchFamily = static_cast<PitchFamily>(a.value(QLatin1String("pitchFamily")).toInt());
    if (a.hasAttribute(QLatin1String("charset")))
        d->charset = static_cast<Charset>(a.value(QLatin1String("charset")).toInt());
}

QXlsx::Font::operator QVariant() const
{
    const auto& ref
#if QT_VERSION >= 0x060000 // Qt 6.0 or over
        = QMetaType::fromType<Font>();
#else
        = qMetaTypeId<Font>() ;
#endif
    return QVariant(ref, this);
}

QDebug operator<<(QDebug dbg, const Font &c)
{
    QDebugStateSaver state(dbg);
    dbg.nospace() << "QXlsx::Font(" << c.font() << ")";
    return dbg;
}

bool FontPrivate::operator ==(const FontPrivate &other) const
{
    return (font == other.font && panose == other.panose
            && pitchFamily == other.pitchFamily && charset == other.charset);
}

}
