#include "DataTypes.h"

#include <cmath>
#include <algorithm>


void RgbToHsv(const RgbColor& color, float& outH, float& outS, float& outV) {
    float minComponent = std::min(color.R, std::min(color.G, color.B));
    float maxComponent = std::max(color.R, std::max(color.G, color.B));
    float delta = maxComponent - minComponent;

    if (delta < 0.00001f) {
        // All RGB components are roughly equal, so treat this as gray.
        outH = NAN;
        outS = 0.f;
        outV = maxComponent;
        return;
    }

    if (maxComponent == 0) {
        // Color is black.
        outH = NAN;
        outS = 0.f;
        outV = 0.f;
        return;
    }

    // Determine hue by mapping the RGB values onto the color wheel. Note that in this case the wheel has a circumfrence
    //   of 6.f, since each pair of subtractions can return a value between -1.f and 1.f.
    if (maxComponent == color.R) {
        // Red: between magenta and yellow.
        outH = (color.G - color.B) / delta;
    }
    else if (maxComponent == color.G) {
        // Green: between yellow and cyan.
        outH = 2.f + (color.B - color.R) / delta;
    }
    else {
        // Blue: between cyan and magenta.
        outH = 4.f + (color.R - color.G) / delta;
    }

    // Reduce the hue to the range [0,1], wrapping negative values.
    outH /= 6.f;
    if (outH < 0.f) outH += 1.f;

    // Saturation reflects how gray the color is and is thus based on how close the RGB components are.
    outS = delta / maxComponent;

    // Value is the intensity of the color.
    outV = maxComponent;
}

RgbColor HsvToRgb(float H, float S, float V) {

    if (isnan(H)) {
        // Monochromatic color.
        return RgbColor(V, V, V);
    }

    // Convert hue into RGB. Note that we need to multiply the hue by 6 here so that the resulting values are between
    //   0 and 1.
    RgbColor outRgb;
    outRgb.R = std::clamp(std::abs(H * 6 - 3) - 1, 0.f, 1.f);
    outRgb.G = std::clamp(2 - std::abs(H * 6 - 2), 0.f, 1.f);
    outRgb.B = std::clamp(2 - std::abs(H * 6 - 4), 0.f, 1.f);

    // Saturate.
    outRgb.R = (outRgb.R - 1) * S + 1;
    outRgb.G = (outRgb.G - 1) * S + 1;
    outRgb.B = (outRgb.B - 1) * S + 1;

    // Apply value.
    outRgb.R *= V;
    outRgb.G *= V;
    outRgb.B *= V;

    // Clamp.
    outRgb.R = std::clamp(outRgb.R, 0.f, 1.f);
    outRgb.G = std::clamp(outRgb.G, 0.f, 1.f);
    outRgb.B = std::clamp(outRgb.B, 0.f, 1.f);
    return outRgb;
}

RgbColor RgbColor::lerpHSV(RgbColor a, RgbColor b, float t) {

    // Convert to HSV.
    float aH, aS, aV, bH, bS, bV;
    RgbToHsv(a, aH, aS, aV);
    RgbToHsv(b, bH, bS, bV);

    float outH;
    if (isnan(aH)) {
        outH = bH; // NOTE: bH might be NAN but that's okay; we're just lerping between shades of gray.
    }
    else if (isnan(bH)) {
        outH = aH;
    }
    else {
        // Determine what direction we're interpolating the hue in.
        float delta = bH - aH;
        if (std::abs(delta) > 0.5f) {
            // We always want to move the minimum distance around the color wheel, so if the angle is greater than 180 degrees, wrap it
            //   around so that it's less than 180 degrees but in the opposite direction.
            delta -= delta > 0 ? 1.f : -1.f;
        }

        // Interpolate the hue, then wrap it to the range [0,360].
        outH = aH + delta * t;
        if (outH < 0) {
            outH += 1.f;
        }
        else if (outH > 1.f) {
            outH -= 1.f;
        }
    }

    // Interpolate saturation and value, which are straightforward.
    float outS = aS + (bS - aS) * t;
    float outV = aV + (bV - aV) * t;

    return HsvToRgb(outH, outS, outV);
}

float Vector2::length() const {
	return sqrt(X*X + Y*Y);
}

Vector2 Vector2::normalized() const {
	return *this / length();
}

float Vector3::length() const {
	return sqrt(X * X + Y * Y + Z * Z);
}

Vector3 Vector3::normalized() const {
	return *this / length();
}