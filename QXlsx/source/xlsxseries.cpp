#include "xlsxseries.h"

#include <optional>
#include "xlsxabstractsheet.h"

namespace QXlsx {

/// ErrorBarsPrivate

class ErrorBarsPrivate : public QSharedData
{
public:
    ErrorBarsPrivate();
    ErrorBarsPrivate(const ErrorBarsPrivate &other);
    ~ErrorBarsPrivate();

    std::optional<ErrorBars::Direction> direction;
    ErrorBars::Type barType = ErrorBars::Type::Both;
    ErrorBars::ValueType valueType = ErrorBars::ValueType::Fixed;
    std::optional<bool> hideEndCap;
    DataSource plusData;
    DataSource minusData;
    std::optional<double> value;
    ShapeFormat shape;

    bool operator==(const ErrorBarsPrivate &other) const;
};

ErrorBarsPrivate::ErrorBarsPrivate()
{

}
ErrorBarsPrivate::ErrorBarsPrivate(const ErrorBarsPrivate &other) :
    QSharedData(other),
    direction{other.direction},
    barType{other.barType},
    valueType{other.valueType},
    hideEndCap{other.hideEndCap},
    plusData{other.plusData},
    minusData{other.minusData},
    value{other.value},
    shape{other.shape}
{

}
ErrorBarsPrivate::~ErrorBarsPrivate()
{

}

bool ErrorBarsPrivate::operator==(const ErrorBarsPrivate &other) const
{
    if (direction != other.direction) return false;
    if (barType != other.barType) return false;
    if (valueType != other.valueType) return false;
    if (hideEndCap != other.hideEndCap) return false;
    if (plusData != other.plusData) return false;
    if (minusData != other.minusData) return false;
    if (value != other.value) return false;
    if (shape != other.shape) return false;
    return true;
}

/// ErrorBars

ErrorBars::ErrorBars()
{

}
ErrorBars::ErrorBars(Direction direction, Type barType, ValueType valueType)
{
    d = new ErrorBarsPrivate;
    d->direction = direction;
    d->barType = barType;
    d->valueType = valueType;
}
ErrorBars::ErrorBars(const ErrorBars &other) : d{other.d}
{

}
ErrorBars &ErrorBars::operator=(const ErrorBars &other)
{
    if (*this != other) d = other.d;
    return *this;
}
ErrorBars::~ErrorBars()
{

}

std::optional<ErrorBars::Direction> ErrorBars::direction() const
{
    if (d) return d->direction;
    return {};
}
void ErrorBars::setDirection(ErrorBars::Direction direction)
{
    if (!d) d = new ErrorBarsPrivate;
    d->direction = direction;
}
ErrorBars::Type ErrorBars::barType() const// default = Type::Both;
{
    if (d) return d->barType;
    return ErrorBars::Type::Both;
}
void ErrorBars::setBarType(Type barType)
{
    if (!d) d = new ErrorBarsPrivate;
    d->barType = barType;
}
ErrorBars::ValueType ErrorBars::valueType() const //default = ValueType::Fixed;
{
    if (d) return d->valueType;
    return ErrorBars::ValueType::Fixed;
}
void ErrorBars::setValueType(ErrorBars::ValueType valueType)
{
    if (!d) d = new ErrorBarsPrivate;
    d->valueType = valueType;
}
std::optional<bool> ErrorBars::hideEndCap() const
{
    if (d) return d->hideEndCap;
    return {};
}
void ErrorBars::setHideEndCap(bool hide)
{
    if (!d) d = new ErrorBarsPrivate;
    d->hideEndCap = hide;
}
DataSource ErrorBars::plusData() const
{
    if (d) return d->plusData;
    return {};
}
DataSource &ErrorBars::plusData()
{
    if (!d) d = new ErrorBarsPrivate;
    return d->plusData;
}
void ErrorBars::setPlusData(const DataSource &plusData)
{
    if (!d) d = new ErrorBarsPrivate;
    d->plusData = plusData;
    d->valueType = ValueType::Custom;
}
DataSource ErrorBars::minusData() const
{
    if (d) return d->minusData;
    return {};
}
DataSource &ErrorBars::minusData()
{
    if (!d) d = new ErrorBarsPrivate;
    return d->minusData;
}
void ErrorBars::setMinusData(const DataSource &minusData)
{
    if (!d) d = new ErrorBarsPrivate;
    d->minusData = minusData;
    d->valueType = ValueType::Custom;
}
std::optional<double> ErrorBars::value() const
{
    if (d) return d->value;
    return {};
}
void ErrorBars::setValue(double value)
{
    if (!d) d = new ErrorBarsPrivate;
    d->value = value;
}
ShapeFormat ErrorBars::shape() const
{
    if (d) return d->shape;
    return {};
}
ShapeFormat &ErrorBars::shape()
{
    if (!d) d = new ErrorBarsPrivate;
    return d->shape;
}
void ErrorBars::setShape(const ShapeFormat &shape)
{
    if (!d) d = new ErrorBarsPrivate;
    d->shape = shape;
}

ErrorBars::operator QVariant() const
{
    const auto& cref
#if QT_VERSION >= 0x060000 // Qt 6.0 or over
        = QMetaType::fromType<ErrorBars>();
#else
        = qMetaTypeId<ErrorBars>() ;
#endif
    return QVariant(cref, this);
}

bool ErrorBars::isValid() const
{
    if (d)
        return true;
    return false;
}

void ErrorBars::read(QXmlStreamReader &reader)
{
    if (!d) d = new ErrorBarsPrivate;
    const auto &name = reader.name();

    while (!reader.atEnd()) {
        auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            const auto &a = reader.attributes();
            if (name == QLatin1String("errDir")) {
                Direction dir; fromString(a.value(QLatin1String("val")).toString(), dir);
                d->direction = dir;
            }
            else if (name == QLatin1String("errBarType")) {
                Type t; fromString(a.value(QLatin1String("val")).toString(), t);
                d->barType = t;
            }
            else if (name == QLatin1String("errValType")) {
                ValueType t; fromString(a.value(QLatin1String("val")).toString(), t);
                d->valueType = t;
            }
            else if (name == QLatin1String("noEndCap")) {
                parseAttributeBool(a, QLatin1String("val"), d->hideEndCap);
            }
            else if (name == QLatin1String("plus")) {
                reader.readNextStartElement();
                d->plusData.read(reader);
            }
            else if (name == QLatin1String("minus")) {
                reader.readNextStartElement();
                d->minusData.read(reader);
            }
            else if (name == QLatin1String("val")) {
                d->value = a.value(QLatin1String("val")).toDouble();
            }
            else if (name == QLatin1String("spPr")) {
                d->shape.read(reader);
            }
            else reader.skipCurrentElement();
        }
        else if (token == QXmlStreamReader::EndElement && reader.name() == name)
            break;
    }
}

void ErrorBars::write(QXmlStreamWriter &writer, const QString &name) const
{
    if (!d) return;
    writer.writeStartElement(name);
    if (d->direction.has_value())
        writeEmptyElement(writer, QLatin1String("c:errDir"), toString(d->direction.value()));
    writeEmptyElement(writer, QLatin1String("c:errBarType"), toString(d->barType));
    writeEmptyElement(writer, QLatin1String("c:errValType"), toString(d->valueType));
    writeEmptyElement(writer, QLatin1String("c:noEndCap"), d->hideEndCap);

    if (d->plusData.isValid()) d->plusData.write(writer, QLatin1String("c:plus"));
    if (d->minusData.isValid()) d->minusData.write(writer, QLatin1String("c:minus"));
    writeEmptyElement(writer, QLatin1String("c:value"), d->value);
    d->shape.write(writer, QLatin1String("c:spPr"));
    writer.writeEndElement();
}

bool ErrorBars::operator==(const ErrorBars &other) const
{
    if (d == other.d) return true;
    if (!d || !other.d) return false;
    return *this->d.constData() == *other.d.constData();
}
bool ErrorBars::operator!=(const ErrorBars &other) const
{
    return !operator==(other);
}

/// SeriesPrivate

class SeriesPrivate : public QSharedData
{
public:
    SeriesPrivate();
    SeriesPrivate(const SeriesPrivate &other);
    ~SeriesPrivate();
    bool operator==(const SeriesPrivate &other) const;

    //Properties common to all series types
    Series::Type type = Series::Type::None;
    int index;
    int order;
    Text name; // either ref or string
    ShapeFormat shape;
    QList<DataPoint> dataPoints; //data point formatting, except for SurfaceSer
    Labels labels; // except for SurfaceSer

    //Line, Radar, Bar, Area, Pie, Surface - cat
    //Scatter, Bubble - xVal;
    DataSource xVal;
    //Line, Radar, Bar, Area, Pie, Surface - val
    //Scatter, Bubble - yVal
    DataSource yVal;
    DataSource bubbleVal; //Bubble - bubbleSize


    //Line, Scatter, Bar, Area, Bubble
    QList<TrendLine> trendLines;

    //Line, Scatter, Bar, Area, Bubble
    ErrorBars errorBars;

    //Line, Scatter, Radar
    MarkerFormat marker;

    //Bar, Bubble
    std::optional<bool> invertIfNegative;

    //Bar, Area
    std::optional<PictureOptions> pictureOptions;

    //Line, Scatter
    std::optional<bool> smooth;

    //Properties for Bar series
    std::optional<Series::BarShape> barShape;

    //Properties for Pie series
    std::optional<int> pieExplosion;

    //Properties for Bubble series
    std::optional<bool> bubble3D;
};

SeriesPrivate::SeriesPrivate()
{

}

SeriesPrivate::SeriesPrivate(const SeriesPrivate &other) : QSharedData(other),
    type{other.type}, index{other.index}, order{other.order}, name{other.name},
    shape{other.shape}, dataPoints{other.dataPoints}, labels{other.labels},
    xVal{other.xVal}, yVal{other.yVal}, bubbleVal{other.bubbleVal},
    trendLines{other.trendLines}, errorBars{other.errorBars}, marker{other.marker},
    invertIfNegative{other.invertIfNegative}, pictureOptions{other.pictureOptions},
    smooth{other.smooth}, barShape{other.barShape}, pieExplosion{other.pieExplosion},
    bubble3D{other.bubble3D}
{

}

SeriesPrivate::~SeriesPrivate()
{

}

bool SeriesPrivate::operator==(const SeriesPrivate &other) const
{
    if (type != other.type) return false;
    if (index != other.index) return false;
    if (order != other.order) return false;
    if (name != other.name) return false;
    if (shape != other.shape) return false;
    if (dataPoints != other.dataPoints) return false;
    if (labels != other.labels) return false;
    if (xVal != other.xVal) return false;
    if (yVal != other.yVal) return false;
    if (bubbleVal != other.bubbleVal) return false;
    if (trendLines != other.trendLines) return false;
    if (errorBars != other.errorBars) return false;
    if (marker != other.marker) return false;
    if (invertIfNegative != other.invertIfNegative) return false;
    if (pictureOptions != other.pictureOptions) return false;
    if (smooth != other.smooth) return false;
    if (barShape != other.barShape) return false;
    if (pieExplosion != other.pieExplosion) return false;
    if (bubble3D != other.bubble3D) return false;
    return true;
}

/// End of SeriesPrivate

/// Series

Series::Series()
{

}

Series::Series(int index, int order)
{
    d = new SeriesPrivate;
    d->type = Type::None;
    d->index = index;
    d->order = order;
}

Series::Series(Series::Type type)
{
    d = new SeriesPrivate;
    d->type = type;
}

Series::Series(const Series &other): d(other.d)
{

}

Series::~Series()
{

}

Series &Series::operator=(const Series &other)
{
    if (*this != other)
        d = other.d;
    return *this;
}

bool Series::operator ==(const Series &other) const
{
    if (d == other.d) return true;
    if (!d || !other.d) return false;
    return (*this->d.constData() == *other.d.constData());
}

bool Series::operator !=(const Series &other) const
{
    return !operator==(other);
}

int Series::index() const
{
    if (d) return d->index;
    return -1;
}

void Series::setIndex(int index)
{
    if (!d) d = new SeriesPrivate;
    d->index = index;
}

Series::Type Series::type() const
{
    if (d) return d->type;
    return Type::None;
}

void Series::setType(Series::Type type)
{
    if (!d) d = new SeriesPrivate;
    d->type = type;
}

int Series::order() const
{
    if (d) return d->order;
    return -1;
}

void Series::setOrder(int order)
{
    if (!d) d = new SeriesPrivate;
    d->order = order;
}

DataSource Series::categorySource() const
{
    if (d) return d->xVal;
    return {};
}

DataSource &Series::categorySource()
{
    if (!d) d = new SeriesPrivate;
    return d->xVal;
}

DataSource Series::valueSource() const
{
    if (d) return d->yVal;
    return {};
}

DataSource &Series::valueSource()
{
    if (!d) d = new SeriesPrivate;
    return d->yVal;
}

DataSource Series::bubbleSizeSource() const
{
    if (d) return d->bubbleVal;
    return {};
}

DataSource &Series::bubbleSizeSource()
{
    if (!d) d = new SeriesPrivate;
    return d->bubbleVal;
}
void Series::setCategoryData(const DataSource &data)
{
    if (!d) d = new SeriesPrivate;
    d->xVal = data;
}
void Series::setCategoryData(const QString &reference, DataSource::Type type)
{
    if (!d) d = new SeriesPrivate;
    d->xVal = DataSource(type, reference);
}
void Series::setValueData(const DataSource &data)
{
    if (!d) d = new SeriesPrivate;
    d->yVal = data;
}
void Series::setValueData(const QString &reference)
{
    if (!d) d = new SeriesPrivate;
    d->yVal = DataSource(DataSource::Type::NumberReference, reference);
}
void Series::setBubbleSizeData(const DataSource &data)
{
    if (!d) d = new SeriesPrivate;
    d->bubbleVal = data;
}
void Series::setBubbleSizeData(const QString &reference)
{
    if (!d) d = new SeriesPrivate;
    d->bubbleVal = DataSource(DataSource::Type::NumberReference, reference);
}

void Series::setCategoryData(const QVector<double> &data)
{
    if (!d) d = new SeriesPrivate;
    d->xVal = DataSource(data);
}

void Series::setValueData(const QVector<double> &data)
{
    if (!d) d = new SeriesPrivate;
    d->yVal = DataSource(data);
}

void Series::setCategoryData(const QStringList &data)
{
    if (!d) d = new SeriesPrivate;
    d->xVal = DataSource(data);
}

void Series::setData(const QVector<double> &category, const QVector<double> &value)
{
    setCategoryData(category);
    setValueData(value);
}

void Series::setData(const QStringList &category, const QVector<double> &value)
{
    setCategoryData(category);
    setValueData(value);
}

void Series::setDataSource(const QString &categoryReference, const QString &valueReference)
{
    setCategoryData(categoryReference);
    setValueData(valueReference);
}

void Series::setBubbleSizeData(const QVector<double> &data)
{
    if (!d) d = new SeriesPrivate;
    d->bubbleVal = DataSource(data);
}

void Series::setNameReference(const QString &reference)
{
    if (!d) d = new SeriesPrivate;
    d->name.setStringReference(reference);
}

void Series::setNameReference(const CellRange &ref, AbstractSheet *sheet)
{
    QString name = ref.toString();
    if (sheet) name.prepend(sheet->name()+"!");
}

void Series::setName(const QString &name)
{
    if (!d) d = new SeriesPrivate;
    d->name.setPlainString(name);
}

QString Series::name() const
{
    if (d) return d->name.toPlainString();
    return {};
}

bool Series::nameIsReference() const
{
    if (d) return d->name.isStringReference();
    return false;
}

void Series::setLine(const LineFormat &line)
{
    if (!d) d = new SeriesPrivate;
    d->shape.setLine(line);
}

LineFormat Series::line() const
{
    if (d && d->shape.isValid()) return d->shape.line();
    return {};
}

LineFormat &Series::line()
{
    if (!d) d = new SeriesPrivate;
    return d->shape.line();
}

void Series::setShape(const ShapeFormat &shape)
{
    if (!d) d = new SeriesPrivate;
    d->shape = shape;
}

ShapeFormat Series::shape() const
{
    if (!d) return d->shape;
    return {};
}

ShapeFormat &Series::shape()
{
    if (!d) d = new SeriesPrivate;
    return d->shape;
}

void Series::setMarker(const MarkerFormat &marker)
{
    if (!d) d = new SeriesPrivate;
    d->marker = marker;
}

MarkerFormat Series::marker() const
{
    if (d) return d->marker;
    return {};
}

MarkerFormat &Series::marker()
{
    if (!d) d = new SeriesPrivate;
    return d->marker;
}

void Series::setLabels(QVector<int> labels, Label::ShowParameters showFlags, Label::Position pos)
{
    if (!d) d = new SeriesPrivate;
    for (int label: qAsConst(labels))
        d->labels.addLabel(label, showFlags, pos);
    d->labels.setVisible(false);
}

void Series::setDefaultLabels(Label::ShowParameters showFlags, Label::Position pos)
{
    if (!d) d = new SeriesPrivate;
    Labels l(showFlags, pos);
    l.setVisible(true);
    d->labels = l;
}

Labels Series::defaultLabels() const
{
    if (d) return d->labels;
    return {};
}

Labels &Series::defaultLabels()
{
    if (!d) d = new SeriesPrivate;
    return d->labels;
}

std::optional<std::reference_wrapper<Label> > Series::label(int index)
{
    if (!d) d = new SeriesPrivate;
    return d->labels.labelForPoint(index);
}

Label Series::label(int index) const
{
    if (d) return d->labels.labelForPoint(index);
    return {};
}

QList<DataPoint> Series::dataPoints() const
{
    if (d) return d->dataPoints;
    return {};
}

DataPoint Series::dataPoint(int index) const
{
    if (!d) return {};

    auto it = std::find_if(d->dataPoints.begin(), d->dataPoints.end(), [index](const DataPoint &dp)
                           {return dp.index == index;});
    if (it != d->dataPoints.end()) return *it;
    return {};
}

std::optional<std::reference_wrapper<DataPoint> > Series::dataPoint(int index)
{
    if (index < 0) return std::nullopt;
    if (!d) d = new SeriesPrivate;

    auto it = std::find_if(d->dataPoints.begin(), d->dataPoints.end(), [index](const DataPoint &dp)
                           {return dp.index == index;});
    if (it != d->dataPoints.end()) return *it;
    DataPoint dp; dp.index = index;
    d->dataPoints.append(dp);
    return d->dataPoints.last();
}

bool Series::hasDataPoint(int index) const
{
    if (d)
        return (std::find_if(d->dataPoints.cbegin(), d->dataPoints.cend(), [index](const DataPoint &dp)
                         {return dp.index == index;}) != d->dataPoints.cend());
    return false;
}

bool Series::removeDataPoint(int index)
{
    if (!d) return false;
    for (int i=0; i<d->dataPoints.size(); ++i) {
        if (d->dataPoints[i].index == index) {
            d->dataPoints.removeAt(i);
            return true;
        }
    }
    return false;
}

void Series::removeDataPoints()
{
    if (d) d->dataPoints.clear();
}

int Series::dataPointsCount() const
{
    if (d) return d->dataPoints.size();
    return 0;
}




std::optional<bool> Series::invertColorsIfNegative() const
{
    if (d) return d->invertIfNegative;
    return {};
}

void Series::setInvertColorsIfNegative(bool invert)
{
    if (!d) d = new SeriesPrivate;
    d->invertIfNegative = invert;
}

std::optional<Series::BarShape> Series::barShape() const
{
    if (d) return d->barShape;
    return {};
}

void Series::setBarShape(Series::BarShape barShape)
{
    if (!d) d = new SeriesPrivate;
    d->barShape = barShape;
}

std::optional<bool> Series::bubble3D() const
{
    if (d) return d->bubble3D;
    return {};
}

void Series::setBubble3D(bool val)
{
    if (!d) d = new SeriesPrivate;
    d->bubble3D = val;
}

std::optional<PictureOptions> Series::pictureOptions() const
{
    if (d) return d->pictureOptions;
    return {};
}

void Series::setPictureOptions(const PictureOptions &pictureOptions)
{
    if (!d) d = new SeriesPrivate;
    d->pictureOptions = pictureOptions;
}

ErrorBars &Series::errorBars()
{
    if (!d) d = new SeriesPrivate;
    return d->errorBars;
}

ErrorBars Series::errorBars() const
{
    if (d) return d->errorBars;
    return {};
}

void Series::setErrorBars(const ErrorBars &errorBars)
{
    if (!d) d = new SeriesPrivate;
    d->errorBars = errorBars;
}

void Series::read(QXmlStreamReader &reader)
{
    if (!d) d = new SeriesPrivate;

    const auto &name = reader.name();
    while (!reader.atEnd()) {
        auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            const auto &a = reader.attributes();
            if (reader.name() == QLatin1String("idx")) {
//                parseAttributeInt(a, QLatin1String(""), );
                parseAttributeInt(a, QLatin1String("val"), d->index);
            }
            else if (reader.name() == QLatin1String("order")) {
                parseAttributeInt(a, QLatin1String("val"), d->order);
            }
            else if (reader.name() == QLatin1String("tx")) {
                d->name.read(reader);
            }
            else if (reader.name() == QLatin1String("spPr")) {
                d->shape.read(reader);
            }
            else if (reader.name() == QLatin1String("marker")) {
                d->marker.read(reader);
            }
            else if (reader.name() == QLatin1String("dPt")) {
                DataPoint p;
                p.read(reader);
                d->dataPoints.append(p);
            }
            else if (reader.name() == QLatin1String("dLbls")) {
                d->labels.read(reader);
            }
            else if (reader.name() == QLatin1String("cat")) {
                reader.readNextStartElement();
                d->xVal.read(reader);
            }
            else if (reader.name() == QLatin1String("xVal")) {
                reader.readNextStartElement();
                d->xVal.read(reader);
            }
            else if (reader.name() == QLatin1String("val")) {
                reader.readNextStartElement();
                d->yVal.read(reader);
            }
            else if (reader.name() == QLatin1String("yVal")) {
                reader.readNextStartElement();
                d->yVal.read(reader);
            }
            else if (reader.name() == QLatin1String("invertIfNegative")) {
                parseAttributeBool(a, QLatin1String("val"), d->invertIfNegative);
            }
            else if (reader.name() == QLatin1String("shape")) {
                BarShape b;
                fromString(a.value(QLatin1String("val")).toString(), b);
                d->barShape = b;
            }
            else if (reader.name() == QLatin1String("explosion")) {
                parseAttributeInt(a, QLatin1String("val"), d->pieExplosion);
            }
            else if (reader.name() == QLatin1String("bubble3D")) {
                parseAttributeBool(a, QLatin1String("val"), d->bubble3D);
            }
            else if (reader.name() == QLatin1String("pictureOptions")) {
                PictureOptions p; p.read(reader);
                d->pictureOptions = p;
            }
            else reader.skipCurrentElement();
//            else if (reader.name() == QLatin1String("")) {}
//            else if (reader.name() == QLatin1String("")) {}
//            else if (reader.name() == QLatin1String("")) {}
//            else if (reader.name() == QLatin1String("")) {}
//            else if (reader.name() == QLatin1String("")) {}
        }
        else if (token == QXmlStreamReader::EndElement && reader.name() == name)
            break;
    }
}

void Series::write(QXmlStreamWriter &writer) const
{
    Q_ASSERT(d->type != Type::None);

    writer.writeStartElement(QLatin1String("c:ser"));
    writeEmptyElement(writer, QLatin1String("c:idx"), d->index);
    writeEmptyElement(writer, QLatin1String("c:order"), d->order);
    if (d->name.isValid()) d->name.write(writer, QLatin1String("c:tx"));
    if (d->shape.isValid()) d->shape.write(writer, QLatin1String("c:spPr"));
    if (d->type == Type::Bubble || d->type == Type::Bar)
        writeEmptyElement(writer, QLatin1String("c:invertIfNegative"), d->invertIfNegative);
    if (!d->dataPoints.isEmpty() && d->type != Type::Surface) {
        for (auto &dPt: qAsConst(d->dataPoints)) dPt.write(writer);
    }
    if (d->labels.isValid() && d->type != Type::Surface) d->labels.write(writer);
    QString xVal = (d->type == Type::Scatter || d->type == Type::Bubble) ? QLatin1String("c:xVal") : QLatin1String("c:cat");
    d->xVal.write(writer, xVal);
    QString yVal = (d->type == Type::Scatter || d->type == Type::Bubble) ? QLatin1String("c:yVal") : QLatin1String("c:val");
    d->yVal.write(writer, yVal);
    if (d->type == Type::Bubble) d->bubbleVal.write(writer, QLatin1String("c:bubbleSize"));

    if (d->type == Type::Line || d->type == Type::Scatter || d->type == Type::Radar)
        if (d->marker.isValid()) d->marker.write(writer);
    if (d->type == Type::Line || d->type == Type::Scatter)
        writeEmptyElement(writer, QLatin1String("c:smooth"), d->smooth);
    if (d->barShape.has_value() && d->type == Type::Bar) {
        writer.writeEmptyElement(QLatin1String("c:shape"));
        writer.writeAttribute(QLatin1String("val"), toString(d->barShape.value()));
    }
    if (d->type == Type::Pie)
        writeEmptyElement(writer, QLatin1String("c:explosion"), d->pieExplosion);
    if (d->type == Type::Bubble)
        writeEmptyElement(writer, QLatin1String("c:bubble3D"), d->bubble3D);
    if (d->type == Type::Area || d->type == Type::Bar)
        if (d->pictureOptions.has_value())
            d->pictureOptions->write(writer, QLatin1String("c:pictureOptions"));
    if (d->type == Type::Line || d->type == Type::Scatter || d->type == Type::Bar ||
        d->type == Type::Area || d->type == Type::Bubble)
        d->errorBars.write(writer, QLatin1String("c:errBars"));

    writer.writeEndElement();
}

/// End of Series

/// PictureOptions

void PictureOptions::read(QXmlStreamReader &reader)
{
    const auto &name = reader.name();
    while (!reader.atEnd()) {
        auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            const auto &a = reader.attributes();
            if (reader.name() == QLatin1String("applyToFront"))
                parseAttributeBool(a, QLatin1String("val"), applyToFront);
            else if (reader.name() == QLatin1String("applyToSides"))
                parseAttributeBool(a, QLatin1String("val"), applyToSides);
            else if (reader.name() == QLatin1String("applyToEnd"))
                parseAttributeBool(a, QLatin1String("val"), applyToEnd);
            else if (reader.name() == QLatin1String("pictureFormat")) {
                auto s = a.value(QLatin1String("val"));
                if (s == QLatin1String("stretch")) format = PictureFormat::Stretch;
                if (s == QLatin1String("stack")) format = PictureFormat::Stack;
                if (s == QLatin1String("stackScale")) format = PictureFormat::StackScale;
            }
            else if (reader.name() == QLatin1String("pictureStackUnit"))
                parseAttributeDouble(a, QLatin1String("val"), pictureStackUnit);
            else reader.skipCurrentElement();
        }
        else if (token == QXmlStreamReader::EndElement && reader.name() == name)
            break;
    }
}

void PictureOptions::write(QXmlStreamWriter &writer, const QString &name) const
{
    writer.writeStartElement(name);
    if (applyToFront.has_value()) {
        writer.writeEmptyElement(QLatin1String("c:applyToFront"));
        writer.writeAttribute(QLatin1String("val"), toST_Boolean(applyToFront.value()));
    }
    if (applyToSides.has_value()) {
        writer.writeEmptyElement(QLatin1String("c:applyToSides"));
        writer.writeAttribute(QLatin1String("val"), toST_Boolean(applyToSides.value()));
    }
    if (applyToEnd.has_value()) {
        writer.writeEmptyElement(QLatin1String("c:applyToEnd"));
        writer.writeAttribute(QLatin1String("val"), toST_Boolean(applyToEnd.value()));
    }
    if (format.has_value()) {
        writer.writeEmptyElement(QLatin1String("c:pictureFormat"));
        switch (format.value()) {
        case PictureFormat::Stretch: writer.writeAttribute(QLatin1String("val"), QLatin1String("stretch")); break;
        case PictureFormat::Stack: writer.writeAttribute(QLatin1String("val"), QLatin1String("stack")); break;
        case PictureFormat::StackScale: writer.writeAttribute(QLatin1String("val"), QLatin1String("stackScale")); break;
        }
    }
    if (pictureStackUnit.has_value()) {
        writer.writeEmptyElement(QLatin1String("c:pictureStackUnit"));
        writer.writeAttribute(QLatin1String("val"), QString::number(pictureStackUnit.value()));
    }
    writer.writeEndElement();
}

bool PictureOptions::operator==(const PictureOptions &other) const
{
    if (applyToFront != other.applyToFront) return false;
    if (applyToSides != other.applyToSides) return false;
    if (applyToEnd != other.applyToEnd) return false;
    if (format != other.format) return false;
    if (pictureStackUnit != other.pictureStackUnit) return false;
    return true;
}

bool PictureOptions::operator!=(const PictureOptions &other) const
{
    return !operator==(other);
}

/// End of PictureOptions

/// DataPoint

void DataPoint::read(QXmlStreamReader &reader)
{
    const auto &name = reader.name();
    while (!reader.atEnd()) {
        auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            const auto &a = reader.attributes();
            if (reader.name() == QLatin1String("idx"))
                //parseAttributeInt(a, QLatin1String(""), );
                parseAttributeInt(a, QLatin1String("val"), index);
            else if (reader.name() == QLatin1String("invertIfNegative"))
                parseAttributeBool(a, QLatin1String("val"), invertIfNegative);
            else if (reader.name() == QLatin1String("marker"))
                marker.read(reader);
            else if (reader.name() == QLatin1String("explosion"))
                parseAttributeInt(a, QLatin1String("val"), explosion);
            else if (reader.name() == QLatin1String("bubble3D"))
                parseAttributeBool(a, QLatin1String("val"), bubble3D);
            else if (reader.name() == QLatin1String("spPr"))
                shape.read(reader);
            else if (reader.name() == QLatin1String("pictureOptions")) {
                PictureOptions p;
                p.read(reader);
                pictureOptions = p;
            }
            else reader.skipCurrentElement();
        }
        else if (token == QXmlStreamReader::EndElement && reader.name() == name)
            break;
    }
}

void DataPoint::write(QXmlStreamWriter &writer) const
{
    writer.writeStartElement(QLatin1String("c:dPt"));
    writer.writeEmptyElement(QLatin1String("c:idx"));
    writer.writeAttribute(QLatin1String("val"), QString::number(index));
    writeEmptyElement(writer, QLatin1String("c:invertIfNegative"), invertIfNegative);
    writeEmptyElement(writer, QLatin1String("c:bubble3D"), bubble3D);
    if (marker.isValid()) marker.write(writer);
    writeEmptyElement(writer, QLatin1String("c:explosion"), explosion);
    if (shape.isValid()) shape.write(writer, QLatin1String("c:spPr"));
    if (pictureOptions.has_value()) pictureOptions->write(writer, QLatin1String("c:pictureOptions"));

    writer.writeEndElement();
}

bool DataPoint::operator==(const DataPoint &other) const
{
    if (index != other.index) return false;
    if (invertIfNegative != other.invertIfNegative) return false;
    if (bubble3D != other.bubble3D) return false;
    if (marker != other.marker) return false;
    if (explosion != other.explosion) return false;
    if (shape != other.shape) return false;
    if (pictureOptions != other.pictureOptions) return false;

    return true;
}

bool DataPoint::operator!=(const DataPoint &other) const
{
    return !operator==(other);
}

/// End of DataPoint

/// DataSource

DataSource::DataSource() : type{Type::Invalid}
{

}

DataSource::DataSource(DataSource::Type type) : type{type}
{

}

DataSource::DataSource(const QVector<double> &numberData)
    : type{Type::NumberLiterals}, numberData{numberData}
{
    if (numberData.isEmpty()) type = Type::Invalid;
}

DataSource::DataSource(const QStringList &stringData)
    : type{Type::StringLiterals}, stringData{stringData.toVector()}
{
    if (stringData.isEmpty()) type = Type::Invalid;
}

DataSource::DataSource(DataSource::Type type, const QString &reference)
    : type{type}, reference{reference}
{
    // type checking
    if (!reference.isEmpty() && !(type == Type::NumberReference || type == Type::StringReference
                                 || type == Type::MultiLevel))
        type = Type::Invalid;
}

bool DataSource::isValid() const
{
    if (type == Type::Invalid) return false;
    if ((type == Type::NumberReference || type == Type::StringReference
        || type == Type::MultiLevel ) && reference.isEmpty()) return false;
    if (type == Type::NumberLiterals && numberData.isEmpty()) return false;
    if (type == Type::StringLiterals && stringData.isEmpty()) return false;
    return true;
}

bool DataSource::operator==(const DataSource &other) const
{
    if (type != other.type) return false;
    if (reference != other.reference) return false;
    if (numberData != other.numberData) return false;
    if (stringData != other.stringData) return false;
    if (formatCode != other.formatCode) return false;
    if (formatCodes != other.formatCodes) return false;
    if (multiLevelStringData != other.multiLevelStringData) return false;
    return true;
}

bool DataSource::operator!=(const DataSource &other) const
{
    return !operator==(other);
}

void DataSource::readNumData(QXmlStreamReader &reader)
{
    const auto &name = reader.name();
    while (!reader.atEnd()) {
        auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("formatCode"))
                formatCode = reader.readElementText();
            else if (reader.name() == QLatin1String("ptCount")) {
                numberData.resize(reader.attributes().value(QLatin1String("val")).toInt());
            }
            else if (reader.name() == QLatin1String("pt")) {
                int idx = reader.attributes().value(QLatin1String("idx")).toInt();
                QString format = reader.attributes().value(QLatin1String("formatCode")).toString();
                if (!format.isEmpty()) {
                    formatCodes[idx] = format;
                }
                numberData[idx] = reader.readElementText().toDouble();
            }
        }
        else if (token == QXmlStreamReader::EndElement && reader.name() == name)
            break;
    }
}

void DataSource::readStrData(QXmlStreamReader &reader)
{
    const auto &name = reader.name();
    while (!reader.atEnd()) {
        auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("ptCount")) {
                stringData.resize(reader.attributes().value(QLatin1String("val")).toInt());
            }
            else if (reader.name() == QLatin1String("pt")) {
                int idx = reader.attributes().value(QLatin1String("idx")).toInt();
                stringData[idx] = reader.readElementText();
            }
        }
        else if (token == QXmlStreamReader::EndElement && reader.name() == name)
            break;
    }
}

void DataSource::readMultiLvlStrData(QXmlStreamReader &reader)
{
    const auto &name = reader.name();
    while (!reader.atEnd()) {
        auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("lvl")) {
                multiLevelStringData.append(QVector<QString>{});
            }
            else if (reader.name() == QLatin1String("v")) {
                multiLevelStringData.last().append(reader.readElementText());
            }
        }
        else if (token == QXmlStreamReader::EndElement && reader.name() == name)
            break;
    }
}

void DataSource::read(QXmlStreamReader &reader)
{
    const auto &name = reader.name();
    if (type == Type::Invalid) {
        if (name == QLatin1String("numRef")) type = Type::NumberReference;
        if (name == QLatin1String("numLit")) type = Type::NumberLiterals;
        if (name == QLatin1String("strRef")) type = Type::StringReference;
        if (name == QLatin1String("strLit")) type = Type::StringLiterals;
        if (name == QLatin1String("multiLvlStrRef")) type = Type::MultiLevel;
    }

    while (!reader.atEnd()) {
        auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (name == QLatin1String("numRef")) {
                if (reader.name() == QLatin1String("f")) reference = reader.readElementText();
                else if (reader.name() == QLatin1String("numCashe")) {
                    readNumData(reader);
                }
                else if (reader.name() == QLatin1String("extLst"))
                    extLst.read(reader);
            }
            else if (name == QLatin1String("numLit")) {
                readNumData(reader);
            }
            else if (name == QLatin1String("strRef")) {
                if (reader.name() == QLatin1String("f")) reference = reader.readElementText();
                else if (reader.name() == QLatin1String("strCashe")) {
                    readStrData(reader);
                }
            }
            else if (name == QLatin1String("strLit")) {
                readStrData(reader);
            }
            else if (name == QLatin1String("multiLvlStrRef")) {
                if (reader.name() == QLatin1String("f"))
                    reference = reader.readElementText();
                else if (reader.name() == QLatin1String("multiLvlStrCache")) {
                    readMultiLvlStrData(reader);
                }
            }
            else reader.skipCurrentElement();
        }
        else if (token == QXmlStreamReader::EndElement && reader.name() == name)
            break;
    }
}

void DataSource::writeNumData(QXmlStreamWriter &writer, const QString &name) const
{
    writer.writeStartElement(name);
    if (!formatCode.isEmpty()) writer.writeTextElement(QLatin1String("c:formatCode"), formatCode);
    writeEmptyElement(writer, QLatin1String("ptCount"), int(numberData.size()));
    for (int idx = 0; idx < numberData.size(); ++idx) {
        writer.writeStartElement(QLatin1String("c:pt"));
        writer.writeAttribute(QLatin1String("idx"), QString::number(idx));
        writeAttribute(writer, QLatin1String("formatCode"), formatCodes.value(idx));
        writer.writeTextElement(QLatin1String("c:v"), QString::number(numberData[idx]));
        writer.writeEndElement();
    }
    writer.writeEndElement();
}

void DataSource::writeStrData(QXmlStreamWriter &writer, const QString &name, const QVector<QString> &data) const
{
    writer.writeStartElement(name);
    writeEmptyElement(writer, QLatin1String("ptCount"), int(data.size()));
    for (int idx = 0; idx < data.size(); ++idx) {
        writer.writeStartElement(QLatin1String("c:pt"));
        writer.writeAttribute(QLatin1String("idx"), QString::number(idx));
        writer.writeTextElement(QLatin1String("c:v"), data[idx]);
        writer.writeEndElement();
    }
    writer.writeEndElement();
}

void DataSource::write(QXmlStreamWriter &writer, const QString &name) const
{
    if (type == Type::Invalid) {
        return;
    }

    writer.writeStartElement(name);
    if (type == Type::NumberReference && !reference.isEmpty()) {
        //numRef
        writer.writeStartElement(QLatin1String("c:numRef"));
        writer.writeTextElement(QLatin1String("c:f"), reference);
        if (!numberData.isEmpty())
            writeNumData(writer, QLatin1String("c:numCashe"));
        writer.writeEndElement();
    }
    if (type == Type::NumberLiterals && !numberData.isEmpty()) {
        //numLit
        writeNumData(writer, QLatin1String("c:numLit"));
    }
    if (type == Type::StringReference && !reference.isEmpty()) {
        //strRef
        writer.writeStartElement(QLatin1String("c:strRef"));
        writer.writeTextElement(QLatin1String("c:f"), reference);
        if (!stringData.isEmpty())
            writeStrData(writer, QLatin1String("strCashe"), stringData);
        writer.writeEndElement();
    }
    if (type == Type::StringLiterals && !stringData.isEmpty()) {
        //strLit
        writeStrData(writer, QLatin1String("strLit"), stringData);
    }
    if (type == Type::MultiLevel && !reference.isEmpty()) {
        writer.writeStartElement(QLatin1String("c:multiLvlStrRef"));
        writer.writeTextElement(QLatin1String("c:f"), reference);
        if (!multiLevelStringData.isEmpty()) {
            writer.writeStartElement(QLatin1String("c:multiLvlStrCashe"));
            writeEmptyElement(writer, QLatin1String("ptCount"), int(multiLevelStringData.size()));
            for (const auto &lvl: qAsConst(multiLevelStringData)) {
                writer.writeStartElement(QLatin1String("c:lvl"));
                for (int idx = 0; idx < lvl.size(); ++idx) {
                    writer.writeStartElement(QLatin1String("c:pt"));
                    writeAttribute(writer, QLatin1String("idx"), idx);
                    writer.writeTextElement(QLatin1String("c:v"), lvl[idx]);
                    writer.writeEndElement(); //c:pt
                }
                writer.writeEndElement(); //c:lvl
            }
            writer.writeEndElement();
        }

        writer.writeEndElement();
    }
    extLst.write(writer, QLatin1String("c:extLst"));

    writer.writeEndElement();
}

/// End of DataSource

/// TrendLine

TrendLine::TrendLine() : type{TrendLine::Type::Invalid}
{

}
TrendLine::TrendLine(Type type) : type{type}
{

}
TrendLine::TrendLine(const TrendLine &other) :
    type{other.type}, name{other.name}, shape{other.shape}, order{other.order},
    period{other.period}, forward{other.forward}, backward{other.backward},
    intercept{other.intercept}, dispRSqr{other.dispRSqr}, dispEq{other.dispEq},
    numberFormat{other.numberFormat}, label{other.label}
{

}

TrendLine &TrendLine::operator=(const TrendLine &other)
{
    if (*this != other) {
        type = other.type;
        name = other.name;
        shape = other.shape;
        order = other.order;
        period = other.period;
        forward = other.forward;
        backward = other.backward;
        intercept = other.intercept;
        dispRSqr = other.dispRSqr;
        dispEq = other.dispEq;
        numberFormat = other.numberFormat;
        label = other.label;
    }
    return *this;
}

TrendLine::~TrendLine()
{

}

bool TrendLine::operator==(const TrendLine &other) const
{
    if (type != other.type) return false;
    if (name != other.name) return false;
    if (shape != other.shape) return false;
    if (order != other.order) return false;
    if (period != other.period) return false;
    if (forward != other.forward) return false;
    if (backward != other.backward) return false;
    if (intercept != other.intercept) return false;
    if (dispRSqr != other.dispRSqr) return false;
    if (dispEq != other.dispEq) return false;
    if (numberFormat != other.numberFormat) return false;
    if (label != other.label) return false;
    return true;
}

bool TrendLine::operator!=(const TrendLine &other) const
{
    return !operator==(other);
}

bool TrendLine::isValid() const
{
    if (type == Type::Invalid) return false;
    if (!name.isEmpty()) return true;
    if (shape.isValid()) return true;
    if (order.has_value()) return true;
    if (period.has_value()) return true;
    if (forward.has_value()) return true;
    if (backward.has_value()) return true;
    if (intercept.has_value()) return true;
    if (dispRSqr.has_value()) return true;
    if (dispEq.has_value()) return true;
    if (!numberFormat.isEmpty()) return true;
    if (label.isValid()) return true;
    return false;
}

void TrendLine::read(QXmlStreamReader &reader)
{
    const auto &name = reader.name();
    while (!reader.atEnd()) {
        auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            const auto &a = reader.attributes();
            if (reader.name() == QLatin1String("name"))
                this->name = reader.readElementText();
            else if (reader.name() == QLatin1String("spPr"))
                shape.read(reader);
            else if (reader.name() == QLatin1String("trendlineType")) {
                Type t; fromString(a.value(QLatin1String("val")).toString(), t);
                type = t;
            }
            else if (reader.name() == QLatin1String("order"))
                parseAttributeInt(a, QLatin1String("val"), order);
            else if (reader.name() == QLatin1String("period"))
                parseAttributeInt(a, QLatin1String("val"), period);
            else if (reader.name() == QLatin1String("forward"))
                parseAttributeDouble(a, QLatin1String("val"), forward);
            else if (reader.name() == QLatin1String("backward"))
                parseAttributeDouble(a, QLatin1String("val"), backward);
            else if (reader.name() == QLatin1String("intercept"))
                parseAttributeDouble(a, QLatin1String("val"), intercept);
            else if (reader.name() == QLatin1String("dispRSqr"))
                parseAttributeBool(a, QLatin1String("val"), dispRSqr);
            else if (reader.name() == QLatin1String("dispEq"))
                parseAttributeBool(a, QLatin1String("val"), dispEq);
            else if (reader.name() == QLatin1String("trendlineLbl")) {
                readTrendLineLabel(reader);
            }
            else if (reader.name() == QLatin1String("extLst"))
                extLst.read(reader);
            else reader.skipCurrentElement();
        }
        else if (token == QXmlStreamReader::EndElement && reader.name() == name)
            break;
    }
}

void TrendLine::readTrendLineLabel(QXmlStreamReader &reader)
{
    const auto &name = reader.name();
    label.read(reader);
    while (!reader.atEnd()) {
        auto token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            const auto &a = reader.attributes();
            if (reader.name() == QLatin1String("layout"))
                label.layout().read(reader);
            else if (reader.name() == QLatin1String("tx"))
                label.text().read(reader);
            else if (reader.name() == QLatin1String("numFmt"))
                numberFormat = a.value(QLatin1String("val")).toString();
            else if (reader.name() == QLatin1String("spPr"))
                label.shape().read(reader);
            else if (reader.name() == QLatin1String("txPr")) {
                label.textFormat().read(reader);
            }
            else reader.skipCurrentElement();
        }
        else if (token == QXmlStreamReader::EndElement && reader.name() == name)
            break;
    }
}

void TrendLine::write(QXmlStreamWriter &writer, const QString &name) const
{
    if (!isValid()) return;

    writer.writeStartElement(name);

    if (!this->name.isEmpty()) writer.writeTextElement(QLatin1String("c:name"), this->name);
    shape.write(writer, QLatin1String("c:spPr"));
    writeEmptyElement(writer, QLatin1String("c:trendlineType"), toString(type));
    switch (type) {
        case Type::MovingAverage:
            writeEmptyElement(writer, QLatin1String("c:period"), period); break;
        case Type::Polynomial:
            writeEmptyElement(writer, QLatin1String("c:order"), order); break;
        default: break;
    }
    writeEmptyElement(writer, QLatin1String("c:forward"), forward);
    writeEmptyElement(writer, QLatin1String("c:backward"), backward);
    writeEmptyElement(writer, QLatin1String("c:intercept"), intercept);
    writeEmptyElement(writer, QLatin1String("c:dispRSqr"), dispRSqr);
    writeEmptyElement(writer, QLatin1String("c:dispEq"), dispEq);
    if (label.isValid() || !numberFormat.isEmpty()) {
        writer.writeStartElement(QLatin1String("c:trendlineLbl"));
        label.layout().write(writer, "c:layout");
        label.text().write(writer, "c:tx");
        writeEmptyElement(writer, QLatin1String("c:numFmt"), numberFormat);
        label.shape().write(writer, "c:spPr");

        if (label.textFormat().isValid() && label.text().type() == Text::Type::StringRef)
            label.textFormat().write(writer, QLatin1String("c:txPr"));
        writer.writeEndElement();
    }


    writer.writeEndElement();
}

/// End of TrendLine

}




