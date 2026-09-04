#include "core/image/ImageMetadata.h"

#include <QtMath>

#include <array>
#include <cmath>

namespace purrview::core {

namespace {
QString decimalText(double value, int precision, const QLocale& locale) {
    QString text = locale.toString(value, 'f', precision);
    const QString decimalPoint = locale.decimalPoint();
    while (text.endsWith(QLatin1Char('0')) && text.contains(decimalPoint)) {
        text.chop(1);
    }
    if (text.endsWith(decimalPoint)) {
        text.chop(1);
    }
    return text;
}
} // namespace

bool CameraMetadata::isEmpty() const {
    return manufacturer.isEmpty() && model.isEmpty() && lens.isEmpty() && !aperture.has_value() &&
           !exposureTimeSeconds.has_value() && !iso.has_value() && !focalLength.has_value() &&
           !focalLength35mm.has_value() && exposureProgram.isEmpty() && meteringMode.isEmpty() &&
           flash.isEmpty() && whiteBalance.isEmpty();
}

bool GpsMetadata::isValid() const {
    return latitude.has_value() && longitude.has_value() && std::isfinite(*latitude) &&
           std::isfinite(*longitude) && *latitude >= -90.0 && *latitude <= 90.0 &&
           *longitude >= -180.0 && *longitude <= 180.0;
}

QString friendlyImageFormat(const QString& format) {
    const QString normalized = format.trimmed().toLower();
    if (normalized == QStringLiteral("jpg") || normalized == QStringLiteral("jpeg")) {
        return QStringLiteral("JPEG");
    }
    if (normalized == QStringLiteral("png")) {
        return QStringLiteral("PNG");
    }
    if (normalized == QStringLiteral("webp")) {
        return QStringLiteral("WebP");
    }
    if (normalized == QStringLiteral("bmp")) {
        return QStringLiteral("BMP");
    }
    if (normalized == QStringLiteral("gif")) {
        return QStringLiteral("GIF");
    }
    if (normalized == QStringLiteral("tif") || normalized == QStringLiteral("tiff")) {
        return QStringLiteral("TIFF");
    }
    if (normalized == QStringLiteral("heif")) {
        return QStringLiteral("HEIF");
    }
    if (normalized == QStringLiteral("heic")) {
        return QStringLiteral("HEIC");
    }
    if (normalized == QStringLiteral("avif")) {
        return QStringLiteral("AVIF");
    }
    if (normalized == QStringLiteral("icns")) {
        return QStringLiteral("ICNS");
    }
    return normalized.toUpper();
}

QString formatFileSize(qint64 bytes, const QLocale& locale) {
    if (bytes < 0) {
        return {};
    }
    constexpr std::array<const char*, 4> units = {"B", "KB", "MB", "GB"};
    double value = static_cast<double>(bytes);
    std::size_t unit = 0;
    while (value >= 1024.0 && unit + 1 < units.size()) {
        value /= 1024.0;
        ++unit;
    }
    const int precision = unit == 0 || value >= 10.0 ? 0 : 1;
    return QStringLiteral("%1 %2").arg(decimalText(value, precision, locale),
                                       QString::fromLatin1(units[unit]));
}

QString formatMegapixels(const QSize& size, const QLocale& locale) {
    if (!size.isValid()) {
        return {};
    }
    const double megapixels = static_cast<double>(size.width()) * size.height() / 1'000'000.0;
    const double rounded = std::round(megapixels);
    const int precision = std::abs(megapixels - rounded) < 0.05 ? 0 : 1;
    return QStringLiteral("%1 MP").arg(decimalText(megapixels, precision, locale));
}

QString formatAperture(double aperture, const QLocale& locale) {
    if (!std::isfinite(aperture) || aperture <= 0.0) {
        return {};
    }
    return QStringLiteral("f/%1").arg(decimalText(aperture, 1, locale));
}

QString formatExposureTime(double seconds, const QLocale& locale) {
    if (!std::isfinite(seconds) || seconds <= 0.0) {
        return {};
    }
    if (seconds < 1.0) {
        const int denominator = qMax(1, qRound(1.0 / seconds));
        return QStringLiteral("1/%1 s").arg(denominator);
    }
    const int precision = std::abs(seconds - std::round(seconds)) < 0.01 ? 0 : 1;
    return QStringLiteral("%1 s").arg(decimalText(seconds, precision, locale));
}

QString formatFocalLength(double millimeters, const QLocale& locale) {
    if (!std::isfinite(millimeters) || millimeters <= 0.0) {
        return {};
    }
    const int precision = std::abs(millimeters - std::round(millimeters)) < 0.05 ? 0 : 1;
    return QStringLiteral("%1 mm").arg(decimalText(millimeters, precision, locale));
}

QString formatMetadataDate(const QDateTime& dateTime, const QLocale& locale) {
    return dateTime.isValid() ? locale.toString(dateTime, QStringLiteral("d MMM yyyy · HH:mm"))
                              : QString();
}

QString orientationDescription(const ImageMetadata& metadata) {
    if (!metadata.pixelSize.isValid() ||
        metadata.pixelSize.width() == metadata.pixelSize.height()) {
        return {};
    }
    const bool swapsAxes = metadata.orientation >= 5 && metadata.orientation <= 8;
    const int width = swapsAxes ? metadata.pixelSize.height() : metadata.pixelSize.width();
    const int height = swapsAxes ? metadata.pixelSize.width() : metadata.pixelSize.height();
    return width > height ? QStringLiteral("Paisagem") : QStringLiteral("Retrato");
}

} // namespace purrview::core
