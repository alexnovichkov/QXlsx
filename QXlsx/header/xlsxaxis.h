#ifndef XLSXAXIS_H
#define XLSXAXIS_H

#include <QFont>
#include <QColor>
#include <QByteArray>
#include <QList>
#include <QVariant>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QSharedData>

#include "xlsxglobal.h"
#include "xlsxshapeformat.h"
#include "xlsxutility_p.h"
#include "xlsxtitle.h"
#include "xlsxtext.h"

QT_BEGIN_NAMESPACE_XLSX

class Worksheet;
class WorksheetPrivate;
class RichStringPrivate;
class SharedStrings;

/**
 * @brief The DisplayUnits struct specifies the units properties for the value axis
 */
struct QXLSX_EXPORT DisplayUnits
{
    enum class BuiltInUnit
    {
        Hundreds,
        Thousands,
        TenThousands,
        HundredThousands,
        Millions,
        TenMillions,
        HundredMillions,
        Billions,
        Trillions,
    };
    DisplayUnits();
    DisplayUnits(double customUnit);
    DisplayUnits(BuiltInUnit unit);

    std::optional<double> customUnit() const;
    void setCustomUnit(double customUnit);

    std::optional<BuiltInUnit> builtInUnit() const;
    void setBuiltInUnit(BuiltInUnit builtInUnit);

    Layout layout() const;
    Layout &layout();
    void setLayout(const Layout &layout);

    /**
     * @brief labelVisible returns the visibility of the display units label
     * @return
     */
    bool labelVisible() const;
    /**
     * @brief setLabelVisible shows/hides the display units label
     * @param visible
     */
    void setLabelVisible(bool visible);

    Text text() const;
    Text &text();
    void setText(const Text &text);

    Text textProperties() const;
    Text &textProperties();
    void setTextProperties(const Text &textProperties);

    ShapeFormat shape() const;
    ShapeFormat &shape();
    void setShape(const ShapeFormat &shape);

    bool isValid() const;

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer, const QString &name) const;

    bool operator ==(const DisplayUnits &other) const;
    bool operator !=(const DisplayUnits &other) const;
private:
    SERIALIZE_ENUM(BuiltInUnit,
    {
        {BuiltInUnit::Hundreds, "hundreds"},
        {BuiltInUnit::Thousands, "thousands"},
        {BuiltInUnit::TenThousands, "tenThousands"},
        {BuiltInUnit::HundredThousands, "hundredThousands"},
        {BuiltInUnit::Millions, "millions"},
        {BuiltInUnit::TenMillions, "tenMillions"},
        {BuiltInUnit::HundredMillions, "hundredMillions"},
        {BuiltInUnit::Billions, "billions"},
        {BuiltInUnit::Trillions, "trillions"},
    });

    std::optional<double> mCustomUnit;
    std::optional<BuiltInUnit> mBuiltInUnit;
    Layout mLayout;
    ShapeFormat mShape;
    Text mText;
    Text mTextProperties;
    bool mLabelVisible = true;
};

class AxisPrivate;

class QXLSX_EXPORT Axis
{
public:
    /**
     * @brief The Scaling class is used to set the axis range and scaling parameters
     */
    struct Scaling {
        std::optional<double> logBase;
        std::optional<bool> reversed;
        std::optional<double> min;
        std::optional<double> max;
        /**
         * @brief isValid returns true if some properties of axis scaling were set, false otherwise.
         * @return
         */
        bool isValid() const;
        void write(QXmlStreamWriter &writer) const;
        void read(QXmlStreamReader &reader);
        bool operator ==(const Scaling &other) const;
        bool operator !=(const Scaling &other) const;
    };
    /**
     * @brief The Type enum specifies the axis type
     */
    enum class Type { None = (-1), Category, Value, Date, Series };
    /**
     * @brief The Position enum specifies the axis position
     */
    enum class Position { None = (-1), Left, Right, Top, Bottom };
    /**
     * @brief The CrossesType enum specifies the possible crossing points for an axis.
     */
    enum class CrossesType {
        Minimum, /**< Axis crosses at the minimum value of the chart. */
        Maximum, /**< Axis crosses at the maximum value of the chart. */
        Position, /**< Axis crosses at the value specified with setCrossesAt() */
        AutoZero /**< The category axis crosses at the zero point of the value axis (if possible),
                      or the minimum value (if the minimum is greater than zero) or the maximum
                      (if the maximum is less than zero) */
    };
    /**
     * @brief The CrossesBetweenType enum  specifies the possible crossing states of an axis.
     */
    enum class CrossesBetweenType
    {
        Between, /**< the value axis shall cross the category axis between data markers. */
        MidCat /**< the value axis shall cross the category axis at the midpoint of a category. */
    };
    /**
     * @brief The TickMark enum specifies the possible positions for tick marks.
     */
    enum class TickMark
    {
        None, /**< there shall be no tick marks.*/
        In, /**< the tick marks shall be inside the plot area. */
        Out, /**<  the tick marks shall be outside the plot area. */
        Cross /**< the tick marks shall cross the axis. */
    };
    /**
     * @brief The LabelAlignment enum specifies the possible ways to align the tick labels
     */
    enum class LabelAlignment
    {
        Center, /**< the text shall be centered */
        Left, /**< the text shall be left justified */
        Right /**< the text shall be right justified */
    };
    /**
     * @brief The TimeUnit enum  specifies a unit of time to use on a date axis.
     */
    enum class TimeUnit
    {
        Days, /**< the chart data shall be shown in days. */
        Months, /**< the chart data shall be shown in months. */
        Years /**<  the chart data shall be shown in years. */
    };

    /**
     * @brief creates an invalid axis. To make it valid set type and position and add this axis on the chart
     * using Chart::addAxis(const Axis &axis) method.
     */
    Axis();
    /**
     * @brief creates an axis with specified type and position. To add this axis on the chart
     * use Chart::addAxis(const Axis &axis) method.
     * @param type
     * @param position
     */
    Axis(Type type, Position position);
    /**
     * @brief creates a copy of the other axis. The copy will have the same id as the other axis, so use
     * Chart::addAxis(const Axis &axis) method to add the copy on the chart and to set a new unique id.
     * @param other
     */
    Axis(const Axis &other);
    ~Axis();

    bool isValid() const;

    Type type() const;
    void setType(Type type);
    Position position() const;
    void setPosition(Position position);

    std::optional<bool> visible() const;
    void setVisible(bool visible);

    int id() const;
    void setId(int id);

    int crossAxis() const;
    void setCrossAxis(Axis *axis);
    void setCrossAxis(int axisId);

    std::optional<double> crossesAt() const;
    void setCrossesAt(double val);
    std::optional<CrossesType> crossesType() const;
    void setCrossesType(CrossesType val);

    /**
     * @brief setMajorGridLines sets the axis major grid lines as a ShapeFormat
     * @param val
     */
    void setMajorGridLines(const ShapeFormat &val);
    ShapeFormat &majorGridLines();
    ShapeFormat majorGridLines() const;

    void setMinorGridLines(const ShapeFormat &val);
    ShapeFormat &minorGridLines();
    ShapeFormat minorGridLines() const;

    /**
     * @brief setMajorGridLines sets the axis major grid lines properties
     * @param color
     * @param width
     * @param strokeType
     */
    void setMajorGridLines(const QColor &color, double width, LineFormat::StrokeType strokeType);
    /**
     * @brief setMajorGridLines turns on/off the default major grid lines
     * @param on
     */
    void setMajorGridLines(bool on);
    void setMinorGridLines(const QColor &color, double width, LineFormat::StrokeType strokeType);
    void setMinorGridLines(bool on);

    void setMajorTickMark(TickMark tickMark);
    void setMinorTickMark(TickMark tickMark);
    std::optional<Axis::TickMark> majorTickMark() const;
    std::optional<Axis::TickMark> minorTickMark() const;

    QString titleAsString() const;
    Title title() const;
    Title &title();
    void setTitle(const QString &title);
    void setTitle(const Title &title);

    Text &textProperties();
    Text textProperties() const;
    void setTextProperties(const Text &textProperties);

    QString numberFormat() const;
    void setNumberFormat(const QString &formatCode);

    Scaling &scaling();
    Scaling scaling() const;
    void setScaling(Scaling scaling);

    QPair<double, double> range() const;
    void setRange(double min, double max);

    /**
     * @brief autoAxis specifies that this axis is a date or text axis based on
     * the data that is used for the axis labels, not a specific choice.
     *
     * Applicable to: Category axis, Date axis.
     *
     * @return valid bool value if the property is set, nullopt otherwise.
     */
    std::optional<bool> autoAxis() const;
    /**
     * @brief setAutoAxis sets that this axis is a date or text axis based on
     * the data that is used for the axis labels, not a specific choice.
     *
     * Applicable to: Category axis, Date axis.
     *
     * @param autoAxis
     */
    void setAutoAxis(bool autoAxis);

    /**
     * @brief labelAlignment returns labels alignment for the category axis.
     * @return
     */
    std::optional<Axis::LabelAlignment> labelAlignment() const;
    /**
     * @brief setLabelAlignment sets labels alignment for the category axis.
     * @param labelAlignment
     */
    void setLabelAlignment(Axis::LabelAlignment labelAlignment);

    /**
     * @brief labelOffset returns the distance of labels, in percents, from the axis.
     *
     * Applicable to: Category axis, Date axis.
     *
     * @return
     */
    std::optional<int> labelOffset() const;
    /**
     * @brief setLabelOffset sets the distance of labels, in percents, from the axis.
     *
     * Applicable to: Category axis, Date axis.
     *
     * @param labelOffset value in percents in the range [0..1000].
     */
    void setLabelOffset(int labelOffset);

    /**
     * @brief majorTickDistance returns the distance between major axis ticks.
     *
     * Applicable to: Date axis, Value axis.
     *
     * @return positive value if the property is set, nullopt otherwise.
     */
    std::optional<double> majorTickDistance() const;
    void setMajorTickDistance(double distance);

    std::optional<double> minorTickDistance() const;
    void setMinorTickDistance(double distance);

    /**
     * @brief baseTimeUnit returns the smallest time unit that is represented on the date axis.
     * @return
     */
    std::optional<Axis::TimeUnit> baseTimeUnit() const;
    /**
     * @brief setBaseTimeUnit sets the smallest time unit that is represented on the date axis.
     * @param baseTimeUnit
     */
    void setBaseTimeUnit(TimeUnit baseTimeUnit);
    /**
     * @brief majorTimeUnit returns time unit for major tick marks of a date axis.
     * @return
     */
    std::optional<Axis::TimeUnit> majorTimeUnit() const;
    void setMajorTimeUnit(TimeUnit majorTimeUnit);
    /**
     * @brief minorTimeUnit returns time unit for minor tick marks of a date axis.
     * @return
     */
    std::optional<Axis::TimeUnit> minorTimeUnit() const;
    void setMinorTimeUnit(TimeUnit minorTimeUnit);

    /**
     * @brief tickLabelSkip returns how many tick labels is skipped between label that is drawn.
     *
     * Applicable to: Category axis, Series axis.
     *
     * @return
     */
    std::optional<int> tickLabelSkip();
    /**
     * @brief setTickLabelSkip sets how many tick labels shall be skipped between label that is drawn.
     *
     * Applicable to: Category axis, Series axis.
     *
     * @param skip
     */
    void setTickLabelSkip(int skip);
    /**
     * @brief tickMarkSkip returns how many tick marks shall be skipped before
     * the next one shall be drawn.
     *
     * Applicable to: Category axis, Series axis.
     *
     * @return
     */
    std::optional<int> tickMarkSkip();
    /**
     * @brief setTickMarkSkip sets  how many tick marks shall be skipped before
     * the next one shall be drawn.
     *
     * Applicable to: Category axis, Series axis.
     *
     * @param skip
     */
    void setTickMarkSkip(int skip);

    /**
     * @brief noMultiLevelLabels specifies the labels on the category axis shall
     * be shown as flat text.
     * If this element is not included or is set to false, then the labels shall
     * be drawn as a hierarchy.
     * @return
     */
    std::optional<bool> noMultiLevelLabels() const;
    /**
     * @brief setNoMultiLevelLabels specifies the labels on the category axis shall
     * be shown as flat text.
     *
     * @param val true means that the labels shall be drawn as a flat text.
     * False means that the labels shall be drawn as a hierarchy.
     */
    void setNoMultiLevelLabels(bool val);

    /**
     * @brief crossesBetween specifies whether the value axis crosses the category
     * axis between categories.
     * @return
     */
    std::optional<CrossesBetweenType> crossesBetween() const;
    /**
     * @brief setCrossesBetween sets where the value axis crosses the category
     * axis.
     * @param crossesBetween Between means the value axis shall cross the category axis
     * between categories. MidCat means the value axis shall cross the category
     * axis on categories.
     */
    void setCrossesBetween(CrossesBetweenType crossesBetween);

    /**
     * @brief displayUnits  returns the properties of the display units for the value axis
     * @return
     */
    DisplayUnits displayUnits() const;
    DisplayUnits &displayUnits();
    void setDisplayUnits(const DisplayUnits &displayUnits);

    void write(QXmlStreamWriter &writer) const;
    void read(QXmlStreamReader &reader);

    bool operator == (const Axis &other) const;
    bool operator != (const Axis &other) const;

private:
    SERIALIZE_ENUM(Type, {
        {Type::None, "none"},
        {Type::Category, "cat"},
        {Type::Value, "val"},
        {Type::Date, "date"},
        {Type::Series, "ser"},
    });

    SERIALIZE_ENUM(Position, {
        {Position::None, "none"},
        {Position::Top, "t"},
        {Position::Bottom, "b"},
        {Position::Left, "l"},
        {Position::Right, "r"}
    });
    SERIALIZE_ENUM(TickMark, {
        {TickMark::None, "none"},
        {TickMark::In, "in"},
        {TickMark::Out, "out"},
        {TickMark::Cross, "cross"},
    });
    SERIALIZE_ENUM(LabelAlignment, {
        {LabelAlignment::Left, "l"},
        {LabelAlignment::Center, "ctr"},
        {LabelAlignment::Right, "r"},
    });
    SERIALIZE_ENUM(TimeUnit, {
        {TimeUnit::Days, "days"},
        {TimeUnit::Months, "months"},
        {TimeUnit::Years, "years"},
    });
    SERIALIZE_ENUM(CrossesBetweenType, {
        {CrossesBetweenType::MidCat, "midCat"},
        {CrossesBetweenType::Between, "between"},
    });

    friend QDebug operator<<(QDebug, const Axis &axis);

    QSharedDataPointer<AxisPrivate> d;
};

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const Axis &axis);
#endif

class AxisPrivate : public QSharedData
{
public:
    int id = -1;
    Axis::Scaling scaling;
    std::optional<bool> visible;
    Axis::Position position = Axis::Position::None;
    Axis::Type type = Axis::Type::None;

    int crossAxis = -1;
    std::optional<Axis::CrossesType> crossesType;
    std::optional<double> crossesPosition;

    bool majorGridlinesOn = false;
    bool minorGridlinesOn = false;
    ShapeFormat majorGridlines;
    ShapeFormat minorGridlines;

    Title title;
    Text textProperties;
    NumberFormat numberFormat;

    std::optional<Axis::TickMark> majorTickMark;
    std::optional<Axis::TickMark> minorTickMark;

    //Cat, Date
    std::optional<bool> axAuto;
    std::optional<int> labelOffset;

    //Cat
    std::optional<Axis::LabelAlignment> labelAlignment;
    std::optional<bool> noMultiLevelLabels;

    //Date, Val
    std::optional<double> majorUnit;
    std::optional<double> minorUnit;

    //Date
    std::optional<Axis::TimeUnit> baseTimeUnit;
    std::optional<Axis::TimeUnit> majorTimeUnit;
    std::optional<Axis::TimeUnit> minorTimeUnit;

    //Cat, Ser
    std::optional<int> tickLabelSkip;
    std::optional<int> tickMarkSkip;

    //Val
    std::optional<Axis::CrossesBetweenType> crossesBetween;
    DisplayUnits displayUnits;

    AxisPrivate();
    AxisPrivate(const AxisPrivate &other);
    ~AxisPrivate();

    bool operator == (const AxisPrivate &other) const;
};

QT_END_NAMESPACE_XLSX



#endif // XLSXAXIS_H
