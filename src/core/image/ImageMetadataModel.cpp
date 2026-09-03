#include "core/image/ImageMetadataModel.h"

#include <QLocale>

#include <cmath>

namespace impage::core {

ImageMetadataModel::ImageMetadataModel(QObject* parent) : QObject(parent) {}

bool ImageMetadataModel::loading() const {
    return loading_;
}

QString ImageMetadataModel::fileName() const {
    return metadata_.fileName;
}

QString ImageMetadataModel::absolutePath() const {
    return metadata_.absolutePath;
}

QString ImageMetadataModel::format() const {
    return metadata_.format;
}

QString ImageMetadataModel::fileSizeText() const {
    return formatFileSize(metadata_.fileSize);
}

QString ImageMetadataModel::dimensionsText() const {
    return metadata_.pixelSize.isValid()
               ? QStringLiteral("%1 × %2 px")
                     .arg(QLocale().toString(metadata_.pixelSize.width()),
                          QLocale().toString(metadata_.pixelSize.height()))
               : QString();
}

QString ImageMetadataModel::megapixelsText() const {
    return formatMegapixels(metadata_.pixelSize);
}

QString ImageMetadataModel::dateLabel() const {
    if (metadata_.capturedAt.isValid()) {
        return QStringLiteral("Capturada em");
    }
    if (metadata_.modifiedAt.isValid()) {
        return QStringLiteral("Modificada em");
    }
    if (metadata_.createdAt.isValid()) {
        return QStringLiteral("Criada em");
    }
    return {};
}

QString ImageMetadataModel::dateText() const {
    if (metadata_.capturedAt.isValid()) {
        return formatMetadataDate(metadata_.capturedAt);
    }
    if (metadata_.modifiedAt.isValid()) {
        return formatMetadataDate(metadata_.modifiedAt);
    }
    return formatMetadataDate(metadata_.createdAt);
}

QString ImageMetadataModel::orientationText() const {
    return orientationDescription(metadata_);
}

QString ImageMetadataModel::colorProfileText() const {
    return metadata_.colorProfile.isEmpty() ? metadata_.colorSpace : metadata_.colorProfile;
}

bool ImageMetadataModel::hasImageSection() const {
    return !dimensionsText().isEmpty() || !orientationText().isEmpty() ||
           !colorProfileText().isEmpty();
}

bool ImageMetadataModel::hasCameraSection() const {
    return !metadata_.camera.isEmpty();
}

QString ImageMetadataModel::cameraName() const {
    const QString make = metadata_.camera.manufacturer.trimmed();
    const QString model = metadata_.camera.model.trimmed();
    if (make.isEmpty()) {
        return model;
    }
    if (model.isEmpty() || model.startsWith(make, Qt::CaseInsensitive)) {
        return model.isEmpty() ? make : model;
    }
    return QStringLiteral("%1 %2").arg(make, model);
}

QString ImageMetadataModel::lens() const {
    return metadata_.camera.lens;
}

QString ImageMetadataModel::apertureText() const {
    return metadata_.camera.aperture.has_value() ? formatAperture(*metadata_.camera.aperture)
                                                  : QString();
}

QString ImageMetadataModel::exposureText() const {
    return metadata_.camera.exposureTimeSeconds.has_value()
               ? formatExposureTime(*metadata_.camera.exposureTimeSeconds)
               : QString();
}

QString ImageMetadataModel::isoText() const {
    return metadata_.camera.iso.has_value() && *metadata_.camera.iso > 0
               ? QLocale().toString(*metadata_.camera.iso)
               : QString();
}

QString ImageMetadataModel::focalLengthText() const {
    return metadata_.camera.focalLength.has_value()
               ? formatFocalLength(*metadata_.camera.focalLength)
               : QString();
}

QString ImageMetadataModel::focalLength35mmText() const {
    return metadata_.camera.focalLength35mm.has_value()
               ? QStringLiteral("%1 equivalente")
                     .arg(formatFocalLength(*metadata_.camera.focalLength35mm))
               : QString();
}

QString ImageMetadataModel::exposureProgram() const {
    return metadata_.camera.exposureProgram;
}

QString ImageMetadataModel::meteringMode() const {
    return metadata_.camera.meteringMode;
}

QString ImageMetadataModel::flash() const {
    return metadata_.camera.flash;
}

QString ImageMetadataModel::whiteBalance() const {
    return metadata_.camera.whiteBalance;
}

bool ImageMetadataModel::hasGpsSection() const {
    return metadata_.gps.isValid();
}

QString ImageMetadataModel::latitudeText() const {
    return metadata_.gps.latitude.has_value()
               ? QLocale::c().toString(*metadata_.gps.latitude, 'f', 6)
               : QString();
}

QString ImageMetadataModel::longitudeText() const {
    return metadata_.gps.longitude.has_value()
               ? QLocale::c().toString(*metadata_.gps.longitude, 'f', 6)
               : QString();
}

QString ImageMetadataModel::altitudeText() const {
    if (!metadata_.gps.altitude.has_value() || !std::isfinite(*metadata_.gps.altitude)) {
        return {};
    }
    return QStringLiteral("%1 m").arg(QLocale().toString(*metadata_.gps.altitude, 'f', 0));
}

QString ImageMetadataModel::warning() const {
    return metadata_.warning;
}

const ImageMetadata& ImageMetadataModel::value() const {
    return metadata_;
}

void ImageMetadataModel::setBasicMetadata(ImageMetadata metadata) {
    metadata_ = std::move(metadata);
    emit metadataChanged();
}

void ImageMetadataModel::setMetadata(ImageMetadata metadata) {
    metadata_ = std::move(metadata);
    emit metadataChanged();
    setLoading(false);
}

void ImageMetadataModel::setLoading(bool loading) {
    if (loading_ == loading) {
        return;
    }
    loading_ = loading;
    emit loadingChanged();
}

void ImageMetadataModel::clear() {
    metadata_ = {};
    emit metadataChanged();
    setLoading(false);
}

} // namespace impage::core
