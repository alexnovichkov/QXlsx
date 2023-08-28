/** \file xlsxseries.h
 *
 */

#ifndef XLSXSERIES_H
#define XLSXSERIES_H

#include <QtGlobal>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QSharedData>

#include "xlsxglobal.h"
#include "xlsxshapeformat.h"
#include "xlsxmarkerformat.h"
#include "xlsxutility_p.h"
#include "xlsxtitle.h"
#include "xlsxlabel.h"

namespace QXlsx {

class SeriesPrivate;

/**
 * @brief The PictureOptions class specifies the picture options to be used on the data point, series, wall, or floor.
 */
class QXLSX_EXPORT PictureOptions
{
public:
    /**
     * @brief The PictureFormat enum specifies the possible ways to place a picture on a data point, series, wall, or floor.
     */
    enum class PictureFormat
    {
        Stretch, /**<  the picture shall be anisotropic stretched to fill the data point, series, wall or floor. */
        Stack, /**< the picture shall be stacked. */
        StackScale /**<  the picture shall be stacked after being scaled so that
its height is one pictureStackUnit. Does not apply to walls or floor. */
    };
    /**
     * @brief  specifies whether the picture shall be applied to the front of the point or series.
     * If not set, the default value is true.
     */
    std::optional<bool> applyToFront;
    /**
     * @brief specifies whether the picture shall be applied to the sides of the point or series.
     * If not set, the default value is true.
     */
    std::optional<bool> applyToSides;
    /**
     * @brief  specifies whether the picture shall be applied to the end of the point or series.
     * If not set, the default value is true.
     */
    std::optional<bool> applyToEnd;
    /**
     * @brief specifies the stretching and stacking of the picture on the data point, series, wall, or floor.
     */
    std::optional<PictureFormat> format;
    /**
     * @brief  specifies the unit for each picture on the chart. This element
     * applies only if the picture format is StackScale.
     */
    std::optional<double> pictureStackUnit;

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer, const QString &name) const;
    bool operator==(const PictureOptions &other) const;
    bool operator!=(const PictureOptions &other) const;
};

class QXLSX_EXPORT DataPoint
{
public:
    int index;
    std::optional<bool> invertIfNegative;
    std::optional<bool> bubble3D;
    MarkerFormat marker;
    std::optional<int> explosion;
    ShapeFormat shape;
    std::optional<PictureOptions> pictureOptions;

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer) const;
    bool operator==(const DataPoint &other) const;
    bool operator!=(const DataPoint &other) const;
};



/**
 * @brief The DataSource class represents data source for the series
 */
class QXLSX_EXPORT DataSource
{
    //TODO: convert to QSharedData
public:
    /**
     * @brief The Type enum specifies the type of data source for the series:
     * Type::NumberReference and Type::StringReference allow to set data
     * reference (f.e. "Sheet1!$A$1:$A$10"),
     * Type::NumberLiterals and Type::StringLiterals allow to set number or
     * string data directly,
     * Type::MultiLevel is used to set multileveled data reference,
     * Type::None means invalid data source
     */
    enum class Type
    {
        Invalid, /**< invalid data */
        NumberReference, /**< reference to number data*/
        StringReference, /**< reference to string data */
        NumberLiterals, /**< number data specified as a vector of double */
        StringLiterals, /**< string data specified as a string list */
        MultiLevel, /**< multileveled (mixed) data */
    };
    /**
     * @brief creates invalid DataSource
     */
    DataSource();
    /**
     * @brief creates DataSource of specified type
     * @param type
     */
    explicit DataSource(Type type);
    /**
     * @brief creates DataSource of type NumberLiterals and sets numberData to it
     * @param numberData non-empty vector of data. If numberData is empty, DataSource is invalid.
     */
    explicit DataSource(const QVector<double> &numberData);
    /**
     * @brief creates DataSource of type StringLiterals and sets stringData to it
     * @param stringData non-empty list of data. If stringData is empty, DataSource is invalid.
     */
    explicit DataSource(const QStringList &stringData);
    /**
     * @brief creates DataSource of specified type and sets data reference
     * @param type either NumberReference, StringReference, or MultiLevel
     * @param reference string like "Sheet1!$A$1:$A$10"
     */
    explicit DataSource(Type type, const QString &reference);

    Type type;
    QVector<QString> stringData;
    QVector<double> numberData;
    QString reference;
    QString formatCode;

    bool isValid() const;

    bool operator==(const DataSource &other) const;
    bool operator!=(const DataSource &other) const;

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer, const QString &name) const;
};

class QXLSX_EXPORT ErrorBars
{
public:
    enum class Direction
    {
        Horizontal, //x
        Vertical //y
    };
    enum class Type
    {
        Plus,
        Minus,
        Both
    };
    enum class ValueType
    {
        Custom,
        Fixed,
        Percentage,
        StdDeviation,
        StdError
    };

    std::optional<Direction> direction;
    Type barType = Type::Both;
    ValueType valueType = ValueType::Fixed;
    std::optional<bool> noEndCap;
    DataSource plus;
    DataSource minus;
    std::optional<double> value;
    ShapeFormat shape;

    bool isValid() const;

    bool operator==(const ErrorBars &other) const;
    bool operator!=(const ErrorBars &other) const;

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer, const QString &name) const;
private:
    SERIALIZE_ENUM(Direction,
    {
        {Direction::Vertical, "y"},
        {Direction::Horizontal, "x"}
    });
    SERIALIZE_ENUM(Type,
    {
        {Type::Both, "both"},
        {Type::Plus, "plus"},
        {Type::Minus, "minus"}
    });
    SERIALIZE_ENUM(ValueType,
    {
        {ValueType::Custom, "cust"},
        {ValueType::Fixed, "fixedVal"},
        {ValueType::Percentage, "percentage"},
        {ValueType::StdDeviation, "stdDev"},
        {ValueType::StdError, "stdErr"},
    });
};

/**
 * @brief Represents a trend line for Line, Scatter, Bar, Area, Bubble series
 */
class QXLSX_EXPORT TrendLine
{
    //TODO: convert to shared data
public:
    /**
     * @brief The LineType enum specifies the trend line type
     */
    enum class Type
    {
        Invalid, /**< invalid type */
        Exponential, /**< Exponential trend line*/
        Linear, /**< Linear trend line (default)*/
        Logarithmic, /**< Logarithmic trend line*/
        MovingAverage, /**< MovingAverage trend line*/
        Polynomial, /**< Polynomial trend line*/
        Power, /**< Power trend line*/
    };

    TrendLine();
    TrendLine(Type type);
    TrendLine(const TrendLine &other);
    TrendLine &operator=(const TrendLine &other);
    ~TrendLine();

    bool operator==(const TrendLine &other) const;
    bool operator!=(const TrendLine &other) const;

    /**
    * @brief trend line type. See TrendLine::Type enum.
    */
    Type type = Type::Invalid; //required

    /**
     * @brief trend line name to be shown in the chart legend.
     *
     * Can be empty, it means the default-constructed trend line name will be shown in the legend.
     * Can be overwritten in the label.
     *
     */
    QString name; //optional
    /**
     * @brief line and fill properties for the trend line.
     * If not valid, the trend line is solid black
     */
    ShapeFormat shape; //optional

    /**
     * @brief order of the polynomial trend line.
     *
     * If not set, the default value is 2.
     */
    std::optional<int> order; //2..6
    /**
     * @brief period of the moving average trend line.
     *
     * If not set, the default value is 2.
     */
    std::optional<int> period; //2..
    /**
     * @brief measure of the trend line forward projection
     */
    std::optional<double> forward;
    /**
     * @brief measure of the trend line backward projection
     */
    std::optional<double> backward;
    /**
     * @brief specifies the Y-coordinate of intersection of the trend line with the Y axis
     */
    std::optional<double> intercept;
    /**
     * @brief specifies whether to display the approximation reliability value on the trend line label.
     */
    std::optional<bool> dispRSqr;
    /**
     * @brief specifies whether to display the trend line formula on its label.
     */
    std::optional<bool> dispEq;
    /**
     * @brief specifies the number format of the trend line's label.
     */
    QString numberFormat;
    /**
     * @brief specifies the trend line's label parameters (rich text, layout, line and fill etc.)
     */
    Title label;

    bool isValid() const;

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer, const QString &name) const;
private:
    friend class QXlsx::Title;
    void readTrendLineLabel(QXmlStreamReader &reader);
    ExtensionList extLst;
    SERIALIZE_ENUM(Type,
    {
        {Type::Exponential, "exp"},
        {Type::Linear, "linear"},
        {Type::Logarithmic, "log"},
        {Type::MovingAverage, "movingAvg"},
        {Type::Polynomial, "poly"},
        {Type::Power, "power"},
    });
};

/**
 * @brief manages chart series
 */
class QXLSX_EXPORT Series
{
public:
    enum class Type {
        Line,
        Scatter,
        Radar,
        Bar,
        Area,
        Pie,
        Bubble,
        Surface,
        None
    };
    enum class BarShape
    {
        Cone,
        ConeToMax,
        Box,
        Cylinder,
        Pyramid,
        PyramidToMax,
    };

    Series();
    Series(int index, int order);
    explicit Series(Type type);
    Series(const Series &other);
    ~Series();
    Series &operator=(const Series &other);

    Type type() const;
    void setType(Type type);

    int index() const;
    void setIndex(int index);

    int order() const;
    void setOrder(int order);

    DataSource categoryDataSource() const;
    DataSource &categoryDataSource();
    DataSource valueDataSource() const;
    DataSource &valueDataSource();
    DataSource bubbleSizeDataSource() const;
    DataSource &bubbleSizeDataSource();
    /**
     * @brief setCategorySource sets data source for the category or the X axis.
     *
     * @param reference data range in string form (f.e. "Sheet1!$A$1:$A$10")
     */
    void setCategorySource(const QString &reference, DataSource::Type type = DataSource::Type::NumberReference);
    /**
     * @brief setValueSource sets data source for the value, or Y axis
     * @param reference data range in string form (f.e. "Sheet1!$B$1:$B$10")
     */
    void setValueSource(const QString &reference, DataSource::Type type = DataSource::Type::NumberReference);
    /**
     * @brief setBubbleSizeSource sets data source for the bubble sizes in bubble charts
     * @param reference data range in string form (f.e. "Sheet1!$B$1:$B$10")
     */
    void setBubbleSizeSource(const QString &reference, DataSource::Type type = DataSource::Type::NumberReference);
    //TODO: setters that use CellRange
    /**
     * @brief setCategoryData sets data for the category axis or the X axis
     * @param data vector of the same length as value data.
     */
    void setCategoryData(const QVector<double> &data);
    /**
     * @brief setValueData sets data for the value, or Y axis
     * @param data vector of the same length as category data.
     */
    void setValueData(const QVector<double> &data);
    /**
     * @brief setCategoryData sets data for the category axis or the X axis
     * @param data vector of the same length as value data.
     */
    void setCategoryData(const QStringList &data);
    /**
     * @brief setData sets numerical data for both category and value axes.
     * @param category
     * @param value
     */
    void setData(const QVector<double> &category, const QVector<double> &value);
    /**
     * @brief setData sets string data for the category axis and numerical data
     * for the value axis.
     * @param category
     * @param value
     */
    void setData(const QStringList &category, const QVector<double> &value);
    /**
     * @brief setDataSource sets numerical data references for both category and value axes.
     * @param categoryReference reference string (Sheet1!$A$1:$A$10) for numerical data
     * @param valueReference reference string (Sheet1!$B$1:$B$10) for numerical data
     */
    void setDataSource(const QString &categoryReference, const QString &valueReference);

    void setBubbleSizeData(const QVector<double> &data);

    /**
     * @brief setNameReference sets reference for the series name
     * @param reference string like "Sheet1!$A$1:$A$2"
     */
    void setNameReference(const QString &reference);
    /**
     * @brief setName sets a plain-string name as a series name
     * @param name
     */
    void setName(const QString &name);
    /**
     * @brief returns the series name
     * @return
     */
    QString name() const;
    /**
     * @brief returns whether the series name is a reference.
     * @return true if the series name is a string like "Sheet1!$A$1:$A$2".
     */
    bool nameIsReference() const;

    void setCategoryData(const DataSource &data);
    void setValueData(const DataSource &data);
    void setBubbleSizeData(const DataSource &data);

    DataSource categoryData() const;
    DataSource valueData() const;
    DataSource bubbleSizeData() const;

    DataSource &categoryData();
    DataSource &valueData();
    DataSource &bubbleSizeData();

    /**
     * @brief setLine sets line format for the series shape. If the series shape
     * is invalid, creates a new shape with no fill etc. and sets it as the series shape.
     * @param line the series' shape line.
     */
    void setLine(const LineFormat &line);
    /**
     * @brief line returns the series line or invalid line if series is invalid.
     * @return
     */
    LineFormat line() const;
    /**
     * @brief line returns reference to the series line. Both series and
     * series shape must be valid
     * @return reference to line.
     */
    LineFormat &line();

    void setShape(const ShapeFormat &shape);
    ShapeFormat shape() const;
    ShapeFormat &shape();

    /**
     * @brief setMarker sets marker format for line, scatter or radar series
     * @param marker
     */
    void setMarker(const MarkerFormat &marker);
    MarkerFormat marker() const;
    MarkerFormat &marker();


    /**
     * @brief setLabels adds individual labels to points
     * @param labels individual labels indexes starting from 0
     * @param showFlags
     * @param pos labels position
     */
    void setLabels(QVector<int> labels, QXlsx::Label::ShowParameters showFlags, QXlsx::Label::Position pos);
    /**
     * @brief setDefaultLabels turns on default (all) labels and sets parameters to showFlags and pos
     * @param showFlags
     * @param pos labels position
     */
    void setDefaultLabels(Label::ShowParameters showFlags, QXlsx::Label::Position pos);

    /**
     * @brief defaultLabels returns default labels properties
     * @return
     */
    Labels defaultLabels() const;
    Labels &defaultLabels();
    /**
     * @brief label returns reference to a label with index \index
     * @param index index of a label (not the index of a series dataPoint!)
     * @return reference to a label, fails if series is not valid
     */
    std::optional<std::reference_wrapper<Label> > label(int index);
    /**
     * @brief label returns label with index \index
     * @param index index of a label (not the index of a series dataPoint!)
     * @return copy of a label or invalid label if series is not valid or there is no such label
     */
    Label label(int index) const;

    /**
     * @brief label returns reference to a label for a specified data point
     * @param index index of a series data point
     * @return reference to a label, fails if series is not valid
     */
    std::optional<std::reference_wrapper<Label> > labelForPoint(int index);
    /**
     * @brief label returns label for a specified data point
     * @param index index of a series dataPoint
     * @return copy of a label or invalid label if series is not valid or there is no such label
     */
    Label labelForPoint(int index) const;

    //TODO: getters and setters for data points

    /**
     * @brief smooth returns series line smoothing
     * @return valid optional value if smoothing property is set, invalid otherwise
     */
    std::optional<bool> smooth() const;
    /**
     * @brief setSmooth sets series line smoothing
     * @param smooth
     */
    void setSmooth(bool smooth);

    /**
     * @brief invertColorsIfNegative specifies the parent element shall invert
     * its colors if the value is negative.
     *
     * Applicable to: bar series, bubble series
     *
     * @return valid optional value if invertIfNegative property is set, invalid otherwise
     */
    std::optional<bool> invertColorsIfNegative() const;
    /**
     * @brief setInvertColorsIfNegative sets the parent element shall invert
     * its colors if the value is negative.
     * @param invert
     */
    void setInvertColorsIfNegative(bool invert);

    /**
     * @brief barShape returns the bar shape for the bar series
     * @return  valid optional value if barShape property is set, invalid otherwise
     */
    std::optional<BarShape> barShape() const;
    /**
     * @brief setBarShape sets the bar shape for the bar series
     * @param barShape
     */
    void setBarShape(BarShape barShape);

    /**
     * @brief pieExplosion returns the amount the data point shall be moved
     * from the center of the pie.
     * @return valid optional value if pieExplosion property is set, invalid otherwise
     */
    std::optional<int> pieExplosion() const;
    /**
     * @brief setPieExplosion sets the amount the data point shall be moved
     * from the center of the pie.
     * @param val
     */
    void setPieExplosion(int val);

    /**
     * @brief bubble3D returns that the bubbles have a 3-D effect applied to them.
     * @return valid optional value if bubble3D property is set, invalid otherwise
     */
    std::optional<bool> bubble3D() const;
    /**
     * @brief setBubble3D sets that the bubbles have a 3-D effect applied to them.
     * @param val
     */
    void setBubble3D(bool val);

    /**
     * @brief pictureOptions returns the picture used on the bar and area series
     * @return valid optional value if pictureOptions property is set, invalid otherwise
     */
    std::optional<PictureOptions> pictureOptions() const;
    /**
     * @brief setPictureOptions sets the picture to be used on the bar and area series
     * @param pictureOptions
     */
    void setPictureOptions(const PictureOptions &pictureOptions);

    /**
     * @brief errorBars returns error bars properties for Line, Scatter, Bar,
     * Area, Bubble series
     * @return valid optional value if errorBars property is set, invalid otherwise
     */
    std::optional<ErrorBars> errorBars() const;
    /**
     * @brief setErrorBars sets error bars properties for Line, Scatter, Bar,
     * Area, Bubble series
     * @param errorBars
     */
    void setErrorBars(const ErrorBars &errorBars);

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer) const;

    bool operator ==(const Series &other) const;
    bool operator !=(const Series &other) const;
    SERIALIZE_ENUM(BarShape,
    {
        {BarShape::Cone, "cone"},
        {BarShape::ConeToMax, "coneToMax"},
        {BarShape::Box, "box"},
        {BarShape::Cylinder, "cylinder"},
        {BarShape::Pyramid, "pyramid"},
        {BarShape::PyramidToMax, "pyramidToMax"},
    });
private:
    QSharedDataPointer<SeriesPrivate> d;
};

}

#endif // XLSXSERIES_H
