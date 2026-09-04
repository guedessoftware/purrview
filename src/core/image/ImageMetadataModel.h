#pragma once

#include "core/image/ImageMetadata.h"

#include <QObject>

namespace purrview::core {

class ImageMetadataModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString fileName READ fileName NOTIFY metadataChanged)
    Q_PROPERTY(QString absolutePath READ absolutePath NOTIFY metadataChanged)
    Q_PROPERTY(QString format READ format NOTIFY metadataChanged)
    Q_PROPERTY(QString fileSizeText READ fileSizeText NOTIFY metadataChanged)
    Q_PROPERTY(QString dimensionsText READ dimensionsText NOTIFY metadataChanged)
    Q_PROPERTY(QString megapixelsText READ megapixelsText NOTIFY metadataChanged)
    Q_PROPERTY(QString dateLabel READ dateLabel NOTIFY metadataChanged)
    Q_PROPERTY(QString dateText READ dateText NOTIFY metadataChanged)
    Q_PROPERTY(QString orientationText READ orientationText NOTIFY metadataChanged)
    Q_PROPERTY(QString colorProfileText READ colorProfileText NOTIFY metadataChanged)
    Q_PROPERTY(bool hasImageSection READ hasImageSection NOTIFY metadataChanged)
    Q_PROPERTY(bool hasCameraSection READ hasCameraSection NOTIFY metadataChanged)
    Q_PROPERTY(QString cameraName READ cameraName NOTIFY metadataChanged)
    Q_PROPERTY(QString lens READ lens NOTIFY metadataChanged)
    Q_PROPERTY(QString apertureText READ apertureText NOTIFY metadataChanged)
    Q_PROPERTY(QString exposureText READ exposureText NOTIFY metadataChanged)
    Q_PROPERTY(QString isoText READ isoText NOTIFY metadataChanged)
    Q_PROPERTY(QString focalLengthText READ focalLengthText NOTIFY metadataChanged)
    Q_PROPERTY(QString focalLength35mmText READ focalLength35mmText NOTIFY metadataChanged)
    Q_PROPERTY(QString exposureProgram READ exposureProgram NOTIFY metadataChanged)
    Q_PROPERTY(QString meteringMode READ meteringMode NOTIFY metadataChanged)
    Q_PROPERTY(QString flash READ flash NOTIFY metadataChanged)
    Q_PROPERTY(QString whiteBalance READ whiteBalance NOTIFY metadataChanged)
    Q_PROPERTY(bool hasGpsSection READ hasGpsSection NOTIFY metadataChanged)
    Q_PROPERTY(QString latitudeText READ latitudeText NOTIFY metadataChanged)
    Q_PROPERTY(QString longitudeText READ longitudeText NOTIFY metadataChanged)
    Q_PROPERTY(QString altitudeText READ altitudeText NOTIFY metadataChanged)
    Q_PROPERTY(QString warning READ warning NOTIFY metadataChanged)

  public:
    explicit ImageMetadataModel(QObject* parent = nullptr);

    [[nodiscard]] bool loading() const;
    [[nodiscard]] QString fileName() const;
    [[nodiscard]] QString absolutePath() const;
    [[nodiscard]] QString format() const;
    [[nodiscard]] QString fileSizeText() const;
    [[nodiscard]] QString dimensionsText() const;
    [[nodiscard]] QString megapixelsText() const;
    [[nodiscard]] QString dateLabel() const;
    [[nodiscard]] QString dateText() const;
    [[nodiscard]] QString orientationText() const;
    [[nodiscard]] QString colorProfileText() const;
    [[nodiscard]] bool hasImageSection() const;
    [[nodiscard]] bool hasCameraSection() const;
    [[nodiscard]] QString cameraName() const;
    [[nodiscard]] QString lens() const;
    [[nodiscard]] QString apertureText() const;
    [[nodiscard]] QString exposureText() const;
    [[nodiscard]] QString isoText() const;
    [[nodiscard]] QString focalLengthText() const;
    [[nodiscard]] QString focalLength35mmText() const;
    [[nodiscard]] QString exposureProgram() const;
    [[nodiscard]] QString meteringMode() const;
    [[nodiscard]] QString flash() const;
    [[nodiscard]] QString whiteBalance() const;
    [[nodiscard]] bool hasGpsSection() const;
    [[nodiscard]] QString latitudeText() const;
    [[nodiscard]] QString longitudeText() const;
    [[nodiscard]] QString altitudeText() const;
    [[nodiscard]] QString warning() const;
    [[nodiscard]] const ImageMetadata& value() const;

    void setBasicMetadata(ImageMetadata metadata);
    void setMetadata(ImageMetadata metadata);
    void setLoading(bool loading);
    void clear();

  signals:
    void metadataChanged();
    void loadingChanged();

  private:
    ImageMetadata metadata_;
    bool loading_ = false;
};

} // namespace purrview::core
