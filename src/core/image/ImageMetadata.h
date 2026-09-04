#pragma once

#include <QDateTime>
#include <QLocale>
#include <QMetaType>
#include <QSize>
#include <QString>

#include <optional>

namespace purrview::core {

struct CameraMetadata {
    QString manufacturer;
    QString model;
    QString lens;
    std::optional<double> aperture;
    std::optional<double> exposureTimeSeconds;
    std::optional<int> iso;
    std::optional<double> focalLength;
    std::optional<double> focalLength35mm;
    QString exposureProgram;
    QString meteringMode;
    QString flash;
    QString whiteBalance;

    [[nodiscard]] bool isEmpty() const;
};

struct GpsMetadata {
    std::optional<double> latitude;
    std::optional<double> longitude;
    std::optional<double> altitude;

    [[nodiscard]] bool isValid() const;
};

struct ImageMetadata {
    QString fileName;
    QString absolutePath;
    qint64 fileSize = -1;
    QSize pixelSize;
    QString format;
    QString mimeType;
    QDateTime modifiedAt;
    QDateTime createdAt;
    QDateTime capturedAt;
    int orientation = 0;
    QString colorSpace;
    QString colorProfile;
    CameraMetadata camera;
    GpsMetadata gps;
    bool readable = false;
    bool advancedMetadataAvailable = false;
    QString warning;
};

[[nodiscard]] QString friendlyImageFormat(const QString& format);
[[nodiscard]] QString formatFileSize(qint64 bytes, const QLocale& locale = QLocale());
[[nodiscard]] QString formatMegapixels(const QSize& size, const QLocale& locale = QLocale());
[[nodiscard]] QString formatAperture(double aperture, const QLocale& locale = QLocale());
[[nodiscard]] QString formatExposureTime(double seconds, const QLocale& locale = QLocale());
[[nodiscard]] QString formatFocalLength(double millimeters,
                                        const QLocale& locale = QLocale());
[[nodiscard]] QString formatMetadataDate(const QDateTime& dateTime,
                                         const QLocale& locale = QLocale());
[[nodiscard]] QString orientationDescription(const ImageMetadata& metadata);

} // namespace purrview::core

Q_DECLARE_METATYPE(purrview::core::CameraMetadata)
Q_DECLARE_METATYPE(purrview::core::GpsMetadata)
Q_DECLARE_METATYPE(purrview::core::ImageMetadata)
