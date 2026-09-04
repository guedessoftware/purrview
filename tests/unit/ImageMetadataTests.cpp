#include "core/image/ImageMetadataModel.h"
#include "core/image/ImageMetadataService.h"

#ifdef PURRVIEW_HAS_EXIV2
#include <exiv2/exiv2.hpp>
#endif

#include <QCoreApplication>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>
#include <QThread>

#include <cmath>
#include <iostream>

namespace {

int failures = 0;

void check(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

QString createImage(const QTemporaryDir& directory, const QString& name, const QSize& size,
                    const QColor& color) {
    const QString path = directory.filePath(name);
    QImage image(size, QImage::Format_RGB32);
    image.fill(color);
    check(image.save(path), "metadata image fixture can be saved");
    return path;
}

#ifdef PURRVIEW_HAS_EXIV2
void addExifFixture(const QString& path) {
    Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path.toStdString(), false);
    check(image != nullptr, "Exiv2 can open fixture for metadata writing");
    if (!image) {
        return;
    }
    image->readMetadata();
    Exiv2::ExifData& exif = image->exifData();
    exif["Exif.Image.Make"] = "Canon";
    exif["Exif.Image.Model"] = "EOS R6";
    exif["Exif.Photo.LensModel"] = "RF 24-70mm F2.8";
    exif["Exif.Photo.DateTimeOriginal"] = "2026:04:24 16:23:10";
    exif["Exif.Image.Orientation"] = uint16_t{1};
    exif["Exif.Photo.FNumber"] = Exiv2::URational{56, 10};
    exif["Exif.Photo.ExposureTime"] = Exiv2::URational{1, 320};
    exif["Exif.Photo.ISOSpeedRatings"] = uint16_t{100};
    exif["Exif.Photo.FocalLength"] = Exiv2::URational{35, 1};
    exif["Exif.Photo.FocalLengthIn35mmFilm"] = uint16_t{52};
    exif["Exif.Photo.ExposureProgram"] = uint16_t{3};
    exif["Exif.Photo.MeteringMode"] = uint16_t{5};
    exif["Exif.Photo.Flash"] = uint16_t{0};
    exif["Exif.Photo.WhiteBalance"] = uint16_t{0};
    exif["Exif.GPSInfo.GPSLatitudeRef"] = "S";
    exif["Exif.GPSInfo.GPSLatitude"] = "3/1 7/1 8481/1000";
    exif["Exif.GPSInfo.GPSLongitudeRef"] = "W";
    exif["Exif.GPSInfo.GPSLongitude"] = "60/1 1/1 1823/100";
    exif["Exif.GPSInfo.GPSAltitude"] = Exiv2::URational{92, 1};
    image->setExifData(exif);
    image->writeMetadata();
}
#endif

} // namespace

int main(int argc, char* argv[]) {
    QCoreApplication application(argc, argv);
    QTemporaryDir directory;
    check(directory.isValid(), "metadata fixture directory is available");

    const QString jpeg =
        createImage(directory, QStringLiteral("plain.jpg"), QSize(1200, 800), Qt::red);
    const QString png =
        createImage(directory, QStringLiteral("screen.png"), QSize(640, 480), Qt::blue);
    const QString corruptPath = directory.filePath(QStringLiteral("corrupt.jpg"));
    QFile corrupt(corruptPath);
    check(corrupt.open(QIODevice::WriteOnly), "corrupt metadata fixture can be created");
    corrupt.write("not a JPEG");
    corrupt.close();

    purrview::core::ImageMetadataService service;
    bool cacheHit = true;
    const purrview::core::ImageMetadata jpegMetadata = service.readMetadataNow(jpeg, &cacheHit);
    check(!cacheHit && jpegMetadata.readable, "JPEG without EXIF has readable basic metadata");
    check(jpegMetadata.pixelSize == QSize(1200, 800), "JPEG dimensions are extracted");
    check(jpegMetadata.format == QStringLiteral("JPEG"), "actual JPEG format is friendly");
    check(jpegMetadata.fileSize > 0 && jpegMetadata.modifiedAt.isValid(),
          "file size and modification date are extracted");

    const purrview::core::ImageMetadata pngMetadata = service.readMetadataNow(png);
    check(pngMetadata.readable && pngMetadata.pixelSize == QSize(640, 480) &&
              pngMetadata.format == QStringLiteral("PNG"),
          "PNG basic metadata is supported");
    const purrview::core::ImageMetadata corruptMetadata = service.readMetadataNow(corruptPath);
    check(!corruptMetadata.readable && !corruptMetadata.warning.isEmpty(),
          "corrupt image degrades to file metadata with a small warning");
    const purrview::core::ImageMetadata missing =
        service.readMetadataNow(directory.filePath(QStringLiteral("missing.jpg")));
    check(!missing.readable && !missing.warning.isEmpty(),
          "missing image returns a non-fatal metadata error");

    check(purrview::core::formatFileSize(845, QLocale::c()) == QStringLiteral("845 B"),
          "byte size formatting is human readable");
    check(purrview::core::formatFileSize(4'700'000, QLocale::c()).endsWith(QStringLiteral("MB")),
          "megabyte size formatting uses a friendly unit");
    check(purrview::core::formatMegapixels(QSize(6000, 4000), QLocale::c()) ==
              QStringLiteral("24 MP"),
          "megapixels are calculated from dimensions");
    check(purrview::core::formatAperture(5.6, QLocale::c()) == QStringLiteral("f/5.6"),
          "aperture uses photographic formatting");
    const QLocale brazilianPortuguese(QLocale::Portuguese, QLocale::Brazil);
    check(purrview::core::formatAperture(5.6, brazilianPortuguese) == QStringLiteral("f/5,6"),
          "aperture respects the active locale decimal separator");
    check(purrview::core::formatExposureTime(1.0 / 320.0, QLocale::c()) == QStringLiteral("1/320 s"),
          "short exposure uses reciprocal notation");
    check(purrview::core::formatExposureTime(2.0, QLocale::c()) == QStringLiteral("2 s"),
          "long exposure uses seconds notation");

    const int loadsBeforeHit = service.loadCount();
    const purrview::core::ImageMetadata cached = service.readMetadataNow(jpeg, &cacheHit);
    check(cacheHit && cached.absolutePath == jpegMetadata.absolutePath &&
              service.loadCount() == loadsBeforeHit,
          "metadata cache hit avoids a second parse");

    service.invalidateFile(jpeg);
    const purrview::core::ImageMetadata explicitlyInvalidated =
        service.readMetadataNow(jpeg, &cacheHit);
    check(!cacheHit && explicitlyInvalidated.absolutePath == jpegMetadata.absolutePath,
          "metadata cache entry can be invalidated after a file operation");

    QThread::msleep(2);
    QImage changed(QSize(333, 222), QImage::Format_RGB32);
    changed.fill(Qt::green);
    check(changed.save(jpeg), "metadata source can change for invalidation test");
    const purrview::core::ImageMetadata invalidated = service.readMetadataNow(jpeg, &cacheHit);
    check(!cacheHit && invalidated.pixelSize == QSize(333, 222),
          "size or mtime change invalidates metadata cache");

#ifdef PURRVIEW_HAS_EXIV2
    const QString exifJpeg =
        createImage(directory, QStringLiteral("camera.jpg"), QSize(3000, 2000), Qt::yellow);
    addExifFixture(exifJpeg);
    const purrview::core::ImageMetadata exif = service.readMetadataNow(exifJpeg);
    check(exif.advancedMetadataAvailable && exif.camera.manufacturer == QStringLiteral("Canon") &&
              exif.camera.model == QStringLiteral("EOS R6"),
          "camera make and model are extracted through optional Exiv2 backend");
    check(exif.camera.aperture.has_value() && std::abs(*exif.camera.aperture - 5.6) < 0.01,
          "EXIF aperture is extracted");
    check(exif.camera.exposureTimeSeconds.has_value() &&
              std::abs(*exif.camera.exposureTimeSeconds - 1.0 / 320.0) < 0.0001,
          "EXIF exposure is extracted");
    check(exif.camera.iso == std::optional<int>(100) && exif.camera.focalLength.has_value() &&
              std::abs(*exif.camera.focalLength - 35.0) < 0.01,
          "EXIF ISO and focal length are extracted");
    check(exif.gps.isValid() && std::abs(*exif.gps.latitude + 3.119) < 0.001 &&
              std::abs(*exif.gps.longitude + 60.0217) < 0.001 &&
              exif.gps.altitude == std::optional<double>(92.0),
          "GPS coordinates and altitude are parsed without network access");
    check(exif.capturedAt.isValid(), "EXIF capture date has priority-ready representation");

    const QLocale originalLocale = QLocale();
    QLocale::setDefault(brazilianPortuguese);
    purrview::core::ImageMetadataModel presentation;
    presentation.setMetadata(exif);
    check(presentation.hasCameraSection() && presentation.hasGpsSection() &&
              presentation.cameraName() == QStringLiteral("Canon EOS R6") &&
              presentation.apertureText() == QStringLiteral("f/5,6"),
          "typed presentation model exposes formatted optional sections");
    QLocale::setDefault(originalLocale);
#endif

    return failures == 0 ? 0 : 1;
}
