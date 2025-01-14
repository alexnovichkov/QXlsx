#ifndef XLSXEFFECT_H
#define XLSXEFFECT_H

#include "xlsxglobal.h"

#include <QSharedData>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

#include "xlsxutility_p.h"
#include "xlsxmain.h"
#include "xlsxfillformat.h"

namespace QXlsx {

class EffectPrivate;


//TODO: completely rewrite Effect to simplify use.
//TODO: add methods to add effects directly to shapes.

/**
 * @brief The Effect class represents effects for shapes.
 */
class QXLSX_EXPORT Effect
{
public:
    enum class Type
    {
        List,
        DAG
    };
    enum class FillBlendMode
    {
        Overlay,
        Multiply,
        Screen,
        Darken,
        Lighten
    };

    Effect();
    explicit Effect(Type type);
    Effect(const Effect &other);
    ~Effect();
    Effect &operator=(const Effect &other);

    bool operator == (const Effect &other) const;
    bool operator != (const Effect &other) const;

    bool isValid() const;

    /**
     * @brief type returns the type of the effect list
     * @return List if it is a simple effect list with fixed order of applying,
     * DAG if it is a directed acyclic graph of effects (not implemented)
     */
    Type type() const;
    /**
     * @brief setType sets the type of the effect list
     * @param type List if it is a simple effect list with fixed order of applying,
     * DAG if it is a directed acyclic graph of effects (not implemented)
     */
    void setType(Type type);

    /**
     * @brief blurRadius returns the radius of blur effect applied to the entire shape
     * @return
     */
    Coordinate blurRadius() const;
    /**
     * @brief setBlurRadius sets the radius of blur effect applied to the entire shape
     * @param newBlurRadius
     */
    void setBlurRadius(const Coordinate &newBlurRadius);
    /**
     * @brief blurGrow returns whether the bounds of the object should be grown as a result of the blurring
     * @return
     */
    std::optional<bool> blurGrow() const;
    /**
     * @brief setBlurGrow specifies whether the bounds of the object should be grown as a result of the blurring
     * @param newBlurGrow
     */
    void setBlurGrow(bool newBlurGrow);
    /**
     * @brief fillOverlay returns the additional fill to the object
     * @return
     */
    FillFormat &fillOverlay();
    FillFormat fillOverlay() const;
    /**
     * @brief setFillOverlay specifies an additional fill to the object
     * @param newFillOverlay
     */
    void setFillOverlay(const FillFormat &newFillOverlay);
    /**
     * @brief fillBlendMode returns how to blend the additional fill with the base effect
     * @return
     */
    Effect::FillBlendMode fillBlendMode() const;
    /**
     * @brief setFillBlendMode specifies how to blend the additional fill with the base effect
     * @param newFillBlendMode
     */
    void setFillBlendMode(FillBlendMode newFillBlendMode);
    /**
     * @brief glowColor returns the color for the glow effect
     * @return
     */
    Color glowColor() const;
    /**
     * @brief setGlowColor specifies a color for the glow effect
     * @param newGlowColor
     */
    void setGlowColor(const Color &newGlowColor);
    /**
     * @brief glowRadius returns the radius for the glow effect
     * @return
     */
    Coordinate glowRadius() const;
    /**
     * @brief setGlowRadius specifies the radius for the glow effect
     * @param newGlowRadius
     */
    void setGlowRadius(const Coordinate &newGlowRadius);

    Color innerShadowColor() const;
    void setInnerShadowColor(const Color &newInnerShadowColor);
    Coordinate innerShadowBlurRadius() const;
    void setInnerShadowBlurRadius(const Coordinate &newInnerShadowBlurRadius);
    Coordinate innerShadowOffset() const;
    void setInnerShadowOffset(const Coordinate &newInnerShadowOffset);
    Angle innerShadowDirection() const;
    void setInnerShadowDirection(Angle newInnerShadowDirection);

    Color outerShadowColor() const;
    void setOuterShadowColor(const Color &newOuterShadowColor);
    Coordinate outerShadowBlurRadius() const;
    void setOuterShadowBlurRadius(const Coordinate &newOuterShadowBlurRadius);
    Coordinate outerShadowOffset() const;
    void setOuterShadowOffset(const Coordinate &newOuterShadowOffset);
    Angle outerShadowDirection() const;
    void setOuterShadowDirection(Angle newOuterShadowDirection);
    std::optional<double> outerShadowHorizontalScalingFactor() const;
    void setOuterShadowHorizontalScalingFactor(double newOuterShadowHorizontalScalingFactor);
    std::optional<double> outerShadowVerticalScalingFactor() const;
    void setOuterShadowVerticalScalingFactor(double newOuterShadowVerticalScalingFactor);
    Angle outerShadowHorizontalSkewFactor() const;
    void setOuterShadowHorizontalSkewFactor(Angle newOuterShadowHorizontalSkewFactor);
    Angle outerShadowVerticalSkewFactor() const;
    void setOuterShadowVerticalSkewFactor(Angle newOuterShadowVerticalSkewFactor);
    std::optional<bool> outerShadowRotateWithShape() const;
    void setOuterShadowRotateWithShape(bool newOuterShadowRotateWithShape);
    std::optional<FillFormat::Alignment> outerShadowAlignment() const;
    void setOuterShadowAlignment(FillFormat::Alignment newOuterShadowAlignment);
    Color presetShadowColor() const;
    void setPresetShadowColor(const Color &newPresetShadowColor);
    Coordinate presetShadowOffset() const;
    void setPresetShadowOffset(const Coordinate &newPresetShadowOffset);
    Angle presetShadowDirection() const;
    void setPresetShadowDirection(Angle newPresetShadowDirection);
    /**
     * @brief presetShadow returns the preset shadow type from the range [1..20]
     * @return
     */
    std::optional<int> presetShadow() const;
    /**
     * @brief setPresetShadow specifies the preset shadow type
     * @param newPresetShadow an int from the range [1..20]
     */
    void setPresetShadow(int newPresetShadow);
    Coordinate reflectionBlurRadius() const;
    void setReflectionBlurRadius(const Coordinate &newReflectionBlurRadius);
    std::optional<double> reflectionStartOpacity() const;
    void setReflectionStartOpacity(double newReflectionStartOpacity);
    std::optional<double> reflectionStartPosition() const;
    void setReflectionStartPosition(double newReflectionStartPosition);
    std::optional<double> reflectionEndOpacity() const;
    void setReflectionEndOpacity(double newReflectionEndOpacity);
    std::optional<double> reflectionEndPosition() const;
    void setReflectionEndPosition(double newReflectionEndPosition);
    Coordinate reflectionShadowOffset() const;
    void setReflectionShadowOffset(const Coordinate &newReflectionShadowOffset);
    Angle reflectionGradientDirection() const;
    void setReflectionGradientDirection(Angle newReflectionGradientDirection);
    Angle reflectionOffsetDirection() const;
    void setReflectionOffsetDirection(Angle newReflectionOffsetDirection);
    std::optional<double> reflectionHorizontalScalingFactor() const;
    void setReflectionHorizontalScalingFactor(double newReflectionHorizontalScalingFactor);
    std::optional<double> reflectionVerticalScalingFactor() const;
    void setReflectionVerticalScalingFactor(double newReflectionVerticalScalingFactor);
    Angle reflectionHorizontalSkewFactor() const;
    void setReflectionHorizontalSkewFactor(Angle newReflectionHorizontalSkewFactor);
    Angle reflectionVerticalSkewFactor() const;
    void setReflectionVerticalSkewFactor(Angle newReflectionVerticalSkewFactor);
    std::optional<FillFormat::Alignment> reflectionShadowAlignment() const;
    void setReflectionShadowAlignment(FillFormat::Alignment newReflectionShadowAlignment);
    std::optional<bool> reflectionRotateWithShape() const;
    void setReflectionRotateWithShape(bool newReflectionRotateWithShape);
    Coordinate softEdgesBlurRadius() const;
    void setSoftEdgesBlurRadius(const Coordinate &newSoftEdgesBlurRadius);

    void read(QXmlStreamReader &reader);
    void write(QXmlStreamWriter &writer) const;
private:
    SERIALIZE_ENUM(FillBlendMode, {
        {FillBlendMode::Overlay, "over"},
        {FillBlendMode::Multiply, "mult"},
        {FillBlendMode::Screen, "screen"},
        {FillBlendMode::Darken, "darken"},
        {FillBlendMode::Lighten, "lighten"}
    });
    void readEffectList(QXmlStreamReader &reader);
    void writeEffectList(QXmlStreamWriter &writer) const;
    QSharedDataPointer<EffectPrivate> d;
    friend QDebug operator<<(QDebug dbg, const Effect &e);
};

QDebug operator<<(QDebug dbg, const Effect &e);

} //namespace QXlsx

//template<>
//constexpr std::string_view enum_name<QXlsx::Effect::FillBlendMode>(QXlsx::Effect::FillBlendMode value) noexcept
//{
//    switch (value) {
//        case QXlsx::Effect::FillBlendMode::Overlay: return "over";
//        case QXlsx::Effect::FillBlendMode::Multiply: return "mult";
//        case QXlsx::Effect::FillBlendMode::Screen: return "screen";
//        case QXlsx::Effect::FillBlendMode::Darken: return "darken";
//        case QXlsx::Effect::FillBlendMode::Lighten: return "lighten";
//    }
//    return "";
//}

//template<>
//constexpr QXlsx::Effect::FillBlendMode enum_cast<QXlsx::Effect::FillBlendMode>(std::string value) noexcept
//{
//    if ()
//    switch (value) {
//        case QXlsx::Effect::FillBlendMode::Overlay: return "over";
//        case QXlsx::Effect::FillBlendMode::Multiply: return "mult";
//        case QXlsx::Effect::FillBlendMode::Screen: return "screen";
//        case QXlsx::Effect::FillBlendMode::Darken: return "darken";
//        case QXlsx::Effect::FillBlendMode::Lighten: return "lighten";
//    }
//    return "";
//}

//#if MAGIC_ENUM_SUPPORTED
//template <>
//constexpr magic_enum::customize::customize_t magic_enum::customize::enum_name<QXlsx::Effect::FillBlendMode>(QXlsx::Effect::FillBlendMode value) noexcept
//{
//    return QXlsx::enum_name(value);
//}
//#endif

Q_DECLARE_TYPEINFO(QXlsx::Effect, Q_MOVABLE_TYPE);

#endif // XLSXEFFECT_H
