#ifndef XLSXTEXT_H
#define XLSXTEXT_H

#include "xlsxglobal.h"
#include "xlsxformat.h"
#include <QVariant>
#include <QStringList>
#include <QSharedDataPointer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QTextBlock>

#include "xlsxmain.h"
#include "xlsxfont.h"
#include "xlsxshapeformat.h"


QT_BEGIN_NAMESPACE_XLSX

class Text;

/**
 * @brief The TextProperties class is used to set the text body layout
 */
class QXLSX_EXPORT TextProperties
{
    //TODO: convert to shared data
public:
    enum class OverflowType
    {
        Overflow,
        Ellipsis,
        Clip
    };
    enum class VerticalType
    {
        EastAsianVertical,
        Horizontal,
        MongolianVertical,
        Vertical,
        Vertical270,
        WordArtVertical,
        WordArtVerticalRtl,
    };
    enum class Anchor
    {
        Bottom,
        Center,
        Distributed,
        Justified,
        Top
    };
    enum class TextAutofit
    {
        NoAutofit,
        NormalAutofit,
        ShapeAutofit
    };

    //Specifies whether the before and after paragraph spacing defined by the user is to be respected
    std::optional<bool> spcFirstLastPara;
    std::optional<OverflowType> verticalOverflow;
    std::optional<OverflowType> horizontalOverflow;
    std::optional<VerticalType> verticalOrientation;
    std::optional<Angle> rotation;
    std::optional<bool> wrap;
    Coordinate leftInset;
    Coordinate rightInset;
    Coordinate topInset;
    Coordinate bottomInset;
    std::optional<int> columnCount;
    Coordinate columnSpace;
    std::optional<bool> columnsRtl;
    std::optional<bool> fromWordArt;
    std::optional<Anchor> anchor;
    std::optional<bool> anchorCentering;
    std::optional<bool> forceAntiAlias;
    std::optional<bool> upright;
    std::optional<bool> compatibleLineSpacing;
    std::optional<PresetTextShape> textShape;

    // textScale and lineSpaceReduction are only valid if textAutofit == NormalAutofit
    std::optional<TextAutofit> textAutofit;
    std::optional<double> fontScale; // in %
    std::optional<double> lineSpaceReduction; // in %

    std::optional<Scene3D> scene3D;
    // either text3D or z
    std::optional<Shape3D> text3D;
    Coordinate z;

    //QByteArray idKey() const;
    bool isValid() const;

    bool operator ==(const TextProperties &other) const;
    bool operator !=(const TextProperties &other) const;

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer) const;
};

/**
 * @brief The CharacterProperties class
 * is used to set the text character properties
 */
class QXLSX_EXPORT CharacterProperties
{
public:
    enum class UnderlineType
    {
        None,
        Words,
        Single,
        Double,
        Heavy,
        Dotted,
        DottedHeavy,
        Dash,
        DashHeavy,
        DashLong,
        DashLongHeavy,
        DotDash,
        DotDashHeavy,
        DotDotDash,
        DotDotDashHeavy,
        Wavy,
        WavyHeavy,
        WavyDouble,
    };
    enum class StrikeType
    {
        None,
        Single,
        Double
    };
    enum class CapitalizationType
    {
        None,
        AllCaps,
        SmallCaps
    };

    std::optional<bool> kumimoji;
    QString language; // f.e. "en-US"
    QString alternateLanguage; // f.e. "en-US"
    std::optional<double> fontSize; //in pt
    std::optional<bool> bold;
    std::optional<bool> italic;
    std::optional<UnderlineType> underline;
    std::optional<StrikeType> strike;
    std::optional<double> kerningFontSize; //in pt
    std::optional<CapitalizationType> capitalization;
    std::optional<TextPoint> spacing;
    std::optional<bool> normalizeHeights;
    std::optional<double> baseline;
    std::optional<bool> noProofing;
    std::optional<bool> proofingNeeded;
    std::optional<bool> checkForSmartTagsNeeded;
    std::optional<bool> spellingErrorFound;
    std::optional<int> smartTagId;
    QString bookmarkLinkTarget;
    LineFormat line;
    FillFormat fill;
    Effect effect;
    Color highlightColor;

    std::optional<bool> textUnderlineFollowsText;
    LineFormat textUnderline;

    std::optional<bool> textUnderlineFillFollowsText;
    FillFormat textUnderlineFill;
    Font latinFont;
    Font eastAsianFont;
    Font complexScriptFont;
    Font symbolFont;
    std::optional<bool> rightToLeft;

    //TODO: hyperlinks
//          <xsd:element name="hlinkClick" type="CT_Hyperlink" minOccurs="0" maxOccurs="1"/>
//          <xsd:element name="hlinkMouseOver" type="CT_Hyperlink" minOccurs="0" maxOccurs="1"/>

    bool operator ==(const CharacterProperties &other) const;
    bool operator !=(const CharacterProperties &other) const;

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer, const QString &name) const;
    static CharacterProperties from(const QTextCharFormat &format);
private:
    SERIALIZE_ENUM(UnderlineType, {
        {UnderlineType::None, "none"},
        {UnderlineType::Words, "words"},
        {UnderlineType::Single, "sng"},
        {UnderlineType::Double, "dbl"},
        {UnderlineType::Heavy, "heavy"},
        {UnderlineType::Dotted, "dotted"},
        {UnderlineType::DottedHeavy, "dottedHeavy"},
        {UnderlineType::Dash, "dash"},
        {UnderlineType::DashHeavy, "dashHeavy"},
        {UnderlineType::DashLong, "dashLong"},
        {UnderlineType::DashLongHeavy, "dashLongHeavy"},
        {UnderlineType::DotDash, "dotDash"},
        {UnderlineType::DotDashHeavy, "dotDashHeavy"},
        {UnderlineType::DotDotDash, "dotDotDash"},
        {UnderlineType::DotDotDashHeavy, "dotDotDashHeavy"},
        {UnderlineType::Wavy, "wavy"},
        {UnderlineType::WavyHeavy, "wavyHeavy"},
        {UnderlineType::WavyDouble, "wavyDbl"},
    });
    SERIALIZE_ENUM(StrikeType, {
        {StrikeType::None, "noStrike"},
        {StrikeType::Single, "sngStrike"},
        {StrikeType::Double, "dblStrike"},
    });
    SERIALIZE_ENUM(CapitalizationType, {
        {CapitalizationType::None,"none"},
        {CapitalizationType::AllCaps, "all"},
        {CapitalizationType::SmallCaps, "small"},
    });
};

/**
 * @brief The Size class
 * The class is used to set a size either in points or in percents.
 * Size can either be set as a whole positive number (int) or as a percentage (double).
 */
class QXLSX_EXPORT Size
{
public:
    Size();
    explicit Size(uint points); // [0..158400]
    explicit Size(double percents); // 34.0 = 34.0%

    int toPoints() const;
    double toPercents() const;
    QString toString() const;

    void setPoints(int points);
    void setPercents(double percents);

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer, const QString &name) const;

    bool isValid() const;
    bool isPoints() const;
    bool isPercents() const;

    bool operator==(const Size &other) const {return val == other.val;}
    bool operator!=(const Size &other) const {return val != other.val;}
private:
    QVariant val;
};

/**
 * @brief The ParagraphProperties class
 * The class is used with the Paragraph object to set its properties: spacing, indent, alignment etc.
 */
class QXLSX_EXPORT ParagraphProperties
{
//    <xsd:complexType name="CT_TextParagraphProperties">
//      <xsd:sequence>
//        <xsd:element name="lnSpc" type="CT_TextSpacing" minOccurs="0" maxOccurs="1"/>
//        <xsd:element name="spcBef" type="CT_TextSpacing" minOccurs="0" maxOccurs="1"/>
//        <xsd:element name="spcAft" type="CT_TextSpacing" minOccurs="0" maxOccurs="1"/>
//        <xsd:group ref="EG_TextBulletColor" minOccurs="0" maxOccurs="1"/>
//        <xsd:group ref="EG_TextBulletSize" minOccurs="0" maxOccurs="1"/>
//        <xsd:group ref="EG_TextBulletTypeface" minOccurs="0" maxOccurs="1"/>
//        <xsd:group ref="EG_TextBullet" minOccurs="0" maxOccurs="1"/>
//        <xsd:element name="tabLst" type="CT_TextTabStopList" minOccurs="0" maxOccurs="1"/>
//        <xsd:element name="defRPr" type="CT_TextCharacterProperties" minOccurs="0" maxOccurs="1"/>
//        <xsd:element name="extLst" type="CT_OfficeArtExtensionList" minOccurs="0" maxOccurs="1"/>
//      </xsd:sequence>
//      <xsd:attribute name="marL" type="ST_TextMargin" use="optional"/>
//      <xsd:attribute name="marR" type="ST_TextMargin" use="optional"/>
//      <xsd:attribute name="lvl" type="ST_TextIndentLevelType" use="optional"/>
//      <xsd:attribute name="indent" type="ST_TextIndent" use="optional"/>
//      <xsd:attribute name="algn" type="ST_TextAlignType" use="optional"/>
//      <xsd:attribute name="defTabSz" type="ST_Coordinate32" use="optional"/>
//      <xsd:attribute name="rtl" type="xsd:boolean" use="optional"/>
//      <xsd:attribute name="eaLnBrk" type="xsd:boolean" use="optional"/>
//      <xsd:attribute name="fontAlgn" type="ST_TextFontAlignType" use="optional"/>
//      <xsd:attribute name="latinLnBrk" type="xsd:boolean" use="optional"/>
//      <xsd:attribute name="hangingPunct" type="xsd:boolean" use="optional"/>
//    </xsd:complexType>
public:
    enum class TextAlign
    {
        Left,
        Center,
        Right,
        Justify,
        JustifyLow,
        Distrubute,
        DistrubuteThai,
    };

    enum class FontAlign
    {
        Auto,
        Top,
        Center,
        Base,
        Bottom
    };

    enum class BulletType
    {
        None,
        AutoNumber,
        Char,
        Blip
    };

    enum class BulletAutonumberType
    {
        AlphaLowcaseParentheses, // (a), (b), (c)
        AlphaUppercaseParentheses, // (A), (B), (C)
        AlphaLowcaseRightParentheses, // a), b), c)
        AlphaUppercaseRightParentheses, // A), B), C)
        AlphaLowcasePeriod, // a., b., c.
        AlphaUppercasePeriod, // A., B., C.
        ArabicParentheses, // (1), (2), (3)
        ArabicRightParentheses, // 1), 2), 3)
        ArabicPeriod, // 1., 2., 3.
        ArabicPlain, // 1, 2, 3
        RomanLowcaseParentheses, // (i), (ii), (iii)
        RomanUppercaseParentheses, // (I), (II), (III)
        RomanLowcaseRightParentheses, // i), ii), iii)
        RomanUppercaseRightParentheses, // I), II), III)
        RomanLowcasePeriod, // i., ii., iii.
        RomanUppercasePeriod, // I., II, III.
        Circle,
        CircleWindingsBlack,
        CircleWindingsWhite,
        ArabicDoubleBytePeriod,
        ArabicDoubleByte,
        SimplifiedChinesePeriod,
        SimplifiedChinese,
        TraditionalChinesePeriod,
        TraditionalChinese,
        JapanesePeriod,
        JapaneseKorean,
        JapaneseKoreanPeriod,
        BidiArabic1Minus,
        BidiArabic2Minus,
        BidiHebrewMinus,
        ThaiAlphaPeriod,
        ThaiAlphaRightParentheses,
        ThaiAlphaParentheses,
        ThaiNumberPeriod,
        ThaiNumberRightParentheses,
        ThaiNumberParentheses,
        HindiAlphaPeriod,
        HindiNumberPeriod,
        HindiNumberParentheses,
        HindiAlpha1Period,
    };

    enum class TabAlign
    {
        Left,
        Center,
        Right,
        Decimal
    };

    Coordinate leftMargin;
    Coordinate rightMargin;
    Coordinate indent;
    std::optional<int> indentLevel; //[0..8]
    std::optional<TextAlign> align;
    Coordinate defaultTabSize;
    std::optional<bool> rtl;
    std::optional<bool> eastAsianLineBreak;
    std::optional<bool> latinLineBreak;
    std::optional<bool> hangingPunctuation;
    std::optional<FontAlign> fontAlign;

    std::optional<Size> lineSpacing;
    std::optional<Size> spacingBefore;
    std::optional<Size> spacingAfter;

    std::optional<Color> bulletColor;
    std::optional<Size> bulletSize;
    std::optional<Font> bulletFont;
    std::optional<BulletType> bulletType;
    BulletAutonumberType bulletAutonumberType; //makes sense only if bulletType == Autonumber
    std::optional<int> bulletAutonumberStart;
    QString bulletChar; //makes sense only if bulletType == Char

    std::optional<CharacterProperties> defaultTextCharacterProperties;

    QList<QPair<Coordinate, TabAlign>> tabStops;

    bool isValid() const;

    bool operator==(const ParagraphProperties &other) const;
    bool operator!=(const ParagraphProperties &other) const;

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer, const QString &name) const;

    static ParagraphProperties from(const QTextBlock &block);
private:
    SERIALIZE_ENUM(
        BulletAutonumberType,
        {
            {BulletAutonumberType::AlphaLowcaseParentheses, "alphaLcParenBoth"},   // (a), (b), (c)
            {BulletAutonumberType::AlphaUppercaseParentheses, "alphaUcParenBoth"}, // (A), (B), (C)
            {BulletAutonumberType::AlphaLowcaseRightParentheses, "alphaLcParenR"}, // a), b), c)
            {BulletAutonumberType::AlphaUppercaseRightParentheses, "alphaUcParenR"}, // A), B), C)
            {BulletAutonumberType::AlphaLowcasePeriod, "alphaLcPeriod"},             // a., b., c.
            {BulletAutonumberType::AlphaUppercasePeriod, "alphaUcPeriod"},           // A., B., C.
            {BulletAutonumberType::ArabicParentheses, "arabicParenBoth"},        // (1), (2), (3)
            {BulletAutonumberType::ArabicRightParentheses, "arabicParenR"},      // 1), 2), 3)
            {BulletAutonumberType::ArabicPeriod, "arabicPeriod"},                // 1., 2., 3.
            {BulletAutonumberType::ArabicPlain, "arabicPlain"},                  // 1, 2, 3
            {BulletAutonumberType::RomanLowcaseParentheses, "romanLcParenBoth"}, // (i), (ii), (iii)
            {BulletAutonumberType::RomanUppercaseParentheses,
             "romanUcParenBoth"}, // (I), (II), (III)
            {BulletAutonumberType::RomanLowcaseRightParentheses, "romanLcParenR"}, // i), ii), iii)
            {BulletAutonumberType::RomanUppercaseRightParentheses, "romanUcParenR"}, // I), II), III)
            {BulletAutonumberType::RomanLowcasePeriod, "romanLcPeriod"},   // i., ii., iii.
            {BulletAutonumberType::RomanUppercasePeriod, "romanUcPeriod"}, //I., II, III.
            {BulletAutonumberType::Circle, "circleNumDbPlain"},
            {BulletAutonumberType::CircleWindingsBlack, "circleNumWdBlackPlain"},
            {BulletAutonumberType::CircleWindingsWhite, "circleNumWdWhitePlain"},
            {BulletAutonumberType::ArabicDoubleBytePeriod, "arabicDbPeriod"},
            {BulletAutonumberType::ArabicDoubleByte, "arabicDbPlain"},
            {BulletAutonumberType::SimplifiedChinesePeriod, "ea1ChsPeriod"},
            {BulletAutonumberType::SimplifiedChinese, "ea1ChsPlain"},
            {BulletAutonumberType::TraditionalChinesePeriod, "ea1ChtPeriod"},
            {BulletAutonumberType::TraditionalChinese, "ea1ChtPlain"},
            {BulletAutonumberType::JapanesePeriod, "ea1JpnChsDbPeriod"},
            {BulletAutonumberType::JapaneseKorean, "ea1JpnKorPlain"},
            {BulletAutonumberType::JapaneseKoreanPeriod, "ea1JpnKorPeriod"},
            {BulletAutonumberType::BidiArabic1Minus, "arabic1Minus"},
            {BulletAutonumberType::BidiArabic2Minus, "arabic2Minus"},
            {BulletAutonumberType::BidiHebrewMinus, "hebrew2Minus"},
            {BulletAutonumberType::ThaiAlphaPeriod, "thaiAlphaPeriod"},
            {BulletAutonumberType::ThaiAlphaRightParentheses, "thaiAlphaParenR"},
            {BulletAutonumberType::ThaiAlphaParentheses, "thaiAlphaParenBoth"},
            {BulletAutonumberType::ThaiNumberPeriod, "thaiNumPeriod"},
            {BulletAutonumberType::ThaiNumberRightParentheses, "thaiNumParenR"},
            {BulletAutonumberType::ThaiNumberParentheses, "thaiNumParenBoth"},
            {BulletAutonumberType::HindiAlphaPeriod, "hindiAlphaPeriod"},
            {BulletAutonumberType::HindiNumberPeriod, "hindiNumPeriod"},
            {BulletAutonumberType::HindiNumberParentheses, "hindiNumParenR"},
            {BulletAutonumberType::HindiAlpha1Period, "hindiAlpha1Period"},
        });
    SERIALIZE_ENUM(TabAlign,
                   {
                       {TabAlign::Left, "l"},
                       {TabAlign::Right, "r"},
                       {TabAlign::Center, "ctr"},
                       {TabAlign::Decimal, "dec"},
                   });
    SERIALIZE_ENUM(TextAlign, {
        {TextAlign::Left, "l"},
        {TextAlign::Right, "r"},
        {TextAlign::Center, "ctr"},
        {TextAlign::Justify, "just"},
        {TextAlign::JustifyLow, "justLow"},
        {TextAlign::Distrubute, "dist"},
        {TextAlign::DistrubuteThai, "thaiDist"},
    });
    SERIALIZE_ENUM(FontAlign, {
                      {FontAlign::Top, "t"},
                      {FontAlign::Bottom, "b"},
                      {FontAlign::Auto, "auto"},
                      {FontAlign::Center, "ctr"},
                      {FontAlign::Base, "base"},
                   });
    void readTabStops(QXmlStreamReader &reader);
    void writeTabStops(QXmlStreamWriter &writer, const QString &name) const;
};

class QXLSX_EXPORT ListStyleProperties
{
public:
    QVector<ParagraphProperties> vals;
    bool isEmpty() const;
    ParagraphProperties defaultParagraphProperties() const;
    ParagraphProperties &defaultParagraphProperties();
    void setDefaultParagraphProperties(const ParagraphProperties &properties);
    ParagraphProperties value(int level) const;
    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer, const QString &name) const;

    bool operator==(const ListStyleProperties &other) const;
    bool operator!=(const ListStyleProperties &other) const;
};

class TextRun
{
public:
    enum class Type
    {
        None,
        Regular,
        LineBreak,
        TextField
    };
    TextRun();
    TextRun(const TextRun &other);
    TextRun(Type type, const QString &text, const CharacterProperties &properties);
    TextRun(Type type);

    bool operator ==(const TextRun &other) const;
    bool operator !=(const TextRun &other) const;

    Type type = Type::None;
    QString text;
    std::optional<CharacterProperties> characterProperties;

    //text field properties
    std::optional<ParagraphProperties> paragraphProperties;
    QString guid; //required
    QString fieldType; //optional

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer) const;
};

/**
 * @brief The Paragraph class
 * The class is used to represent a paragraph in the text.
 * Each paragraph has properties (\see ParagraphProperties class), and zero to
 * infinity text runs (\see TextRun), i.e. blocks of texts with the specific formatting.
 */
class Paragraph
{
public:
    //TODO: convert to shared data
    std::optional<ParagraphProperties> paragraphProperties;
    QList<TextRun> textRuns;
    std::optional<CharacterProperties> endParagraphDefaultCharacterProperties;

    bool isValid() const;

    bool operator ==(const Paragraph &other) const;
    bool operator !=(const Paragraph &other) const;

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer, const QString &name) const;
};

class TextPrivate;

class QXLSX_EXPORT Text
{
    //<xsd:complexType name="CT_Tx">
    //  <xsd:sequence>
    //    <xsd:choice minOccurs="1" maxOccurs="1">
    //      <xsd:element name="strRef" type="CT_StrRef" minOccurs="1" maxOccurs="1"/>
    //      <xsd:element name="rich" type="a:CT_TextBody" minOccurs="1" maxOccurs="1"/>
    //    </xsd:choice>
    //  </xsd:sequence>
    //</xsd:complexType>
public:
    enum class Type {
        None,
        StringRef,
        RichText,
        PlainText
    };

    Text(); // null text of type None
    explicit Text(const QString& text); // text of type PlainText with no formatting
    explicit Text(Type type); //null text of specific type

    Text(const Text &other);
    ~Text();

    void setType(Type type);
    Type type() const;

    void setPlainString(const QString &s);
    QString toPlainString() const;

    void setHtml(const QString &text, bool detectHtml = true);
    QString toHtml() const;

    void setStringReference(const QString &ref);
    void setStringCashe(const QStringList &cashe);
    QString stringReference() const;
    QStringList stringCashe() const;

    bool isRichString() const; //only checks type, ignores actual formatting
    bool isPlainString() const;
    bool isStringReference() const;
//    bool isNull() const;
//    bool isEmpty() const;
    bool isValid() const;

    /**
     * @brief textProperties text body properties
     * @return
     */
    TextProperties textProperties() const;
    TextProperties &textProperties();
    void setTextProperties(const TextProperties &textProperties);

    ParagraphProperties defaultParagraphProperties() const;
    ParagraphProperties &defaultParagraphProperties();
    void setDefaultParagraphProperties(const ParagraphProperties &defaultParagraphProperties);

    CharacterProperties defaultCharacterProperties() const;
    CharacterProperties &defaultCharacterProperties();
    void setDefaultCharacterProperties(const CharacterProperties &defaultCharacterProperties);

    /**
     * @brief addFragment adds new text run to the last paragraph
     * @param text plain text
     * @param format text characters format
     */
    void addFragment(const QString &text, const CharacterProperties &format);

    /**
     * @brief fragmentText returns text of the fragment with index \index from the
     * last paragraph.
     * @param index index of the text fragment (a.k.a. text run)
     * @return
     */
    QString fragmentText(int index) const;
    /**
     * @brief fragmentFormat format of the fragment with index \index from the
     * last paragraph.
     * @param index index of the text fragment (a.k.a. text run)
     * @return
     */
    CharacterProperties fragmentFormat(int index) const;

    void read(QXmlStreamReader &reader, bool diveInto = true);
    void write(QXmlStreamWriter &writer, const QString &name, bool diveInto = true) const;

    operator QVariant() const;

    Text &operator=(const Text &other);

    bool operator ==(const Text &other) const;
    bool operator !=(const Text &other) const;
private:
    void readStringReference(QXmlStreamReader &reader);
    void readRichString(QXmlStreamReader &reader);
    void readPlainString(QXmlStreamReader &reader);
    void readParagraph(QXmlStreamReader &reader);

    void writeStringReference(QXmlStreamWriter &writer, const QString &name) const;
    void writeRichString(QXmlStreamWriter &writer, const QString &name) const;
    void writePlainString(QXmlStreamWriter &writer, const QString &name) const;
    void writeParagraphs(QXmlStreamWriter &writer) const;

    //TODO: all comparison operators
//    friend bool operator==(const Text &rs1, const Text &rs2);
//    friend bool operator!=(const Text &rs1, const Text &rs2);
//    friend bool operator<(const Text &rs1, const Text &rs2);
//    friend bool operator==(const Text &rs1, const QString &rs2);
//    friend bool operator==(const QString &rs1, const Text &rs2);
//    friend bool operator!=(const Text &rs1, const QString &rs2);
//    friend bool operator!=(const QString &rs1, const Text &rs2);
    friend QDebug operator<<(QDebug dbg, const Text &rs);

    QSharedDataPointer<TextPrivate> d;
};

QDebug operator<<(QDebug dbg, const Text &t);
QDebug operator<<(QDebug dbg, const TextProperties &t);
QDebug operator<<(QDebug dbg, const ListStyleProperties &t);
QDebug operator<<(QDebug dbg, const Paragraph &t);
QDebug operator<<(QDebug dbg, const ParagraphProperties &t);
QDebug operator<<(QDebug dbg, const CharacterProperties &t);
QDebug operator<<(QDebug dbg, const TextRun &t);
QDebug operator<<(QDebug dbg, const ParagraphProperties::TabAlign &t);

QT_END_NAMESPACE_XLSX

Q_DECLARE_METATYPE(QXlsx::Text)

#endif // XLSXTEXT_H
