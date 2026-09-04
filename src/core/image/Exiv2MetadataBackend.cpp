#include "core/image/Exiv2MetadataBackend.h"

#ifdef PURRVIEW_HAS_EXIV2
#include <exiv2/exiv2.hpp>

#include <array>
#include <cmath>
#include <exception>
#include <optional>
#include <string_view>
#endif

namespace purrview::core {

#ifdef PURRVIEW_HAS_EXIV2
namespace {
using ExifIterator = Exiv2::ExifData::const_iterator;

ExifIterator findExif(const Exiv2::ExifData& data, std::string_view key) {
    return data.findKey(Exiv2::ExifKey(std::string(key)));
}

QString stringValue(const Exiv2::ExifData& data, std::string_view key) {
    const auto value = findExif(data, key);
    return value == data.end() ? QString()
                               : QString::fromUtf8(value->toString()).trimmed();
}

std::optional<double> floatingValue(const Exiv2::ExifData& data, std::string_view key) {
    const auto value = findExif(data, key);
    if (value == data.end() || value->count() == 0) {
        return std::nullopt;
    }
    const double result = value->toFloat();
    return std::isfinite(result) ? std::optional<double>(result) : std::nullopt;
}

std::optional<int> integerValue(const Exiv2::ExifData& data, std::string_view key) {
    const auto value = findExif(data, key);
    if (value == data.end() || value->count() == 0) {
        return std::nullopt;
    }
    return static_cast<int>(value->toInt64());
}

QDateTime exifDate(const Exiv2::ExifData& data) {
    constexpr std::array<std::string_view, 3> keys = {"Exif.Photo.DateTimeOriginal",
                                                      "Exif.Photo.DateTimeDigitized",
                                                      "Exif.Image.DateTime"};
    for (const std::string_view key : keys) {
        const QString value = stringValue(data, key);
        if (value.isEmpty()) {
            continue;
        }
        QDateTime parsed = QDateTime::fromString(value, QStringLiteral("yyyy:MM:dd HH:mm:ss"));
        if (!parsed.isValid()) {
            parsed = QDateTime::fromString(value, Qt::ISODate);
        }
        if (parsed.isValid()) {
            return parsed;
        }
    }
    return {};
}

QString exposureProgramName(int value) {
    switch (value) {
    case 1:
        return QStringLiteral("Manual");
    case 2:
        return QStringLiteral("Programa normal");
    case 3:
        return QStringLiteral("Prioridade de abertura");
    case 4:
        return QStringLiteral("Prioridade de obturador");
    case 5:
        return QStringLiteral("Programa criativo");
    case 6:
        return QStringLiteral("Programa de ação");
    case 7:
        return QStringLiteral("Retrato");
    case 8:
        return QStringLiteral("Paisagem");
    default:
        return {};
    }
}

QString meteringModeName(int value) {
    switch (value) {
    case 1:
        return QStringLiteral("Média");
    case 2:
        return QStringLiteral("Média ponderada ao centro");
    case 3:
        return QStringLiteral("Pontual");
    case 4:
        return QStringLiteral("Multi-ponto");
    case 5:
        return QStringLiteral("Padrão");
    case 6:
        return QStringLiteral("Parcial");
    default:
        return {};
    }
}

QString flashDescription(int value) {
    QString description = (value & 0x1) != 0 ? QStringLiteral("Disparou")
                                              : QStringLiteral("Não disparou");
    if ((value & 0x18) == 0x18) {
        description += QStringLiteral(" · automático");
    }
    if ((value & 0x40) != 0) {
        description += QStringLiteral(" · redução de olhos vermelhos");
    }
    return description;
}

QString whiteBalanceDescription(const Exiv2::ExifData& data) {
    if (const auto whiteBalance = integerValue(data, "Exif.Photo.WhiteBalance");
        whiteBalance.has_value()) {
        if (*whiteBalance == 0) {
            return QStringLiteral("Automático");
        }
        if (*whiteBalance == 1) {
            return QStringLiteral("Manual");
        }
    }
    const auto lightSource = integerValue(data, "Exif.Photo.LightSource");
    if (!lightSource.has_value()) {
        return {};
    }
    switch (*lightSource) {
    case 1:
        return QStringLiteral("Luz do dia");
    case 3:
        return QStringLiteral("Tungstênio");
    case 9:
        return QStringLiteral("Céu claro");
    case 10:
        return QStringLiteral("Nublado");
    case 11:
        return QStringLiteral("Sombra");
    case 12:
    case 13:
    case 14:
    case 15:
        return QStringLiteral("Fluorescente");
    case 17:
        return QStringLiteral("Iluminante padrão A");
    case 18:
        return QStringLiteral("Iluminante padrão B");
    case 19:
        return QStringLiteral("Iluminante padrão C");
    default:
        return {};
    }
}

std::optional<double> gpsCoordinate(const Exiv2::ExifData& data, std::string_view coordinateKey,
                                    std::string_view referenceKey) {
    const auto coordinate = findExif(data, coordinateKey);
    if (coordinate == data.end() || coordinate->count() < 3) {
        return std::nullopt;
    }
    double decimal = coordinate->toFloat(0) + coordinate->toFloat(1) / 60.0 +
                     coordinate->toFloat(2) / 3600.0;
    const QString reference = stringValue(data, referenceKey).toUpper();
    if (reference.startsWith(QLatin1Char('S')) || reference.startsWith(QLatin1Char('W'))) {
        decimal = -decimal;
    }
    return std::isfinite(decimal) ? std::optional<double>(decimal) : std::nullopt;
}
} // namespace
#endif

bool populateAdvancedMetadataWithExiv2(const QString& path, ImageMetadata& metadata,
                                       QString* warning) {
#ifdef PURRVIEW_HAS_EXIV2
    try {
        // useCurl=false guarantees that metadata parsing never performs a network request.
        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path.toStdString(), false);
        if (!image) {
            if (warning != nullptr) {
                *warning = QStringLiteral("O arquivo não foi reconhecido pelo leitor de EXIF.");
            }
            return false;
        }
        image->readMetadata();
        const Exiv2::ExifData& exif = image->exifData();
        metadata.advancedMetadataAvailable = !exif.empty() || !image->xmpData().empty() ||
                                             !image->iptcData().empty() ||
                                             image->iccProfileDefined();
        if (exif.empty()) {
            return metadata.advancedMetadataAvailable;
        }

        metadata.capturedAt = exifDate(exif);
        if (const auto orientation = integerValue(exif, "Exif.Image.Orientation");
            orientation.has_value()) {
            metadata.orientation = *orientation;
        }
        metadata.camera.manufacturer = stringValue(exif, "Exif.Image.Make");
        metadata.camera.model = stringValue(exif, "Exif.Image.Model");
        metadata.camera.lens = stringValue(exif, "Exif.Photo.LensModel");
        if (metadata.camera.lens.isEmpty()) {
            metadata.camera.lens = stringValue(exif, "Exif.Photo.LensSpecification");
        }
        metadata.camera.aperture = floatingValue(exif, "Exif.Photo.FNumber");
        metadata.camera.exposureTimeSeconds = floatingValue(exif, "Exif.Photo.ExposureTime");
        metadata.camera.iso = integerValue(exif, "Exif.Photo.ISOSpeedRatings");
        metadata.camera.focalLength = floatingValue(exif, "Exif.Photo.FocalLength");
        if (const auto focal35 = integerValue(exif, "Exif.Photo.FocalLengthIn35mmFilm");
            focal35.has_value() && *focal35 > 0) {
            metadata.camera.focalLength35mm = static_cast<double>(*focal35);
        }
        if (const auto program = integerValue(exif, "Exif.Photo.ExposureProgram");
            program.has_value()) {
            metadata.camera.exposureProgram = exposureProgramName(*program);
        }
        if (const auto metering = integerValue(exif, "Exif.Photo.MeteringMode");
            metering.has_value()) {
            metadata.camera.meteringMode = meteringModeName(*metering);
        }
        if (const auto flash = integerValue(exif, "Exif.Photo.Flash"); flash.has_value()) {
            metadata.camera.flash = flashDescription(*flash);
        }
        metadata.camera.whiteBalance = whiteBalanceDescription(exif);

        metadata.gps.latitude =
            gpsCoordinate(exif, "Exif.GPSInfo.GPSLatitude", "Exif.GPSInfo.GPSLatitudeRef");
        metadata.gps.longitude =
            gpsCoordinate(exif, "Exif.GPSInfo.GPSLongitude", "Exif.GPSInfo.GPSLongitudeRef");
        metadata.gps.altitude = floatingValue(exif, "Exif.GPSInfo.GPSAltitude");
        if (metadata.gps.altitude.has_value()) {
            if (const auto altitudeReference = integerValue(exif, "Exif.GPSInfo.GPSAltitudeRef");
                altitudeReference.has_value() && *altitudeReference == 1) {
                metadata.gps.altitude = -*metadata.gps.altitude;
            }
        }

        if (metadata.colorSpace.isEmpty()) {
            if (const auto colorSpace = integerValue(exif, "Exif.Photo.ColorSpace");
                colorSpace.has_value() && *colorSpace == 1) {
                metadata.colorSpace = QStringLiteral("sRGB");
            }
        }
        return metadata.advancedMetadataAvailable;
    } catch (const Exiv2::Error& error) {
        if (warning != nullptr) {
            *warning = QStringLiteral("Alguns metadados não puderam ser lidos: %1")
                           .arg(QString::fromUtf8(error.what()));
        }
        return false;
    } catch (const std::exception& error) {
        if (warning != nullptr) {
            *warning = QStringLiteral("Alguns metadados não puderam ser lidos: %1")
                           .arg(QString::fromUtf8(error.what()));
        }
        return false;
    }
#else
    Q_UNUSED(path)
    Q_UNUSED(metadata)
    Q_UNUSED(warning)
    return false;
#endif
}

} // namespace purrview::core
