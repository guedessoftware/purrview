#include "core/image/ImageFormatSupport.h"

#include <QFileInfo>
#include <QImageReader>

namespace purrview::core {

const QSet<QString>& supportedImageSuffixes() {
    static const QSet<QString> suffixes = [] {
        static const QSet<QString> allowed = {
            QStringLiteral("png"),   QStringLiteral("jpg"),  QStringLiteral("jpeg"),
            QStringLiteral("webp"),  QStringLiteral("bmp"),  QStringLiteral("gif"),
            QStringLiteral("tif"),   QStringLiteral("tiff"), QStringLiteral("avif"),
            QStringLiteral("avifs"), QStringLiteral("heif"), QStringLiteral("heic"),
            QStringLiteral("hif"),   QStringLiteral("icns")};
        QSet<QString> result;
        for (const QByteArray& format : QImageReader::supportedImageFormats()) {
            const QString suffix = QString::fromLatin1(format).toLower();
            if (allowed.contains(suffix)) {
                result.insert(suffix);
            }
        }

        const auto addAlias = [&result](const QString& first, const QString& second) {
            if (result.contains(first) || result.contains(second)) {
                result.insert(first);
                result.insert(second);
            }
        };
        addAlias(QStringLiteral("jpg"), QStringLiteral("jpeg"));
        addAlias(QStringLiteral("tif"), QStringLiteral("tiff"));
        return result;
    }();
    return suffixes;
}

bool isSupportedImageFile(const QString& path) {
    const QString suffix = QFileInfo(path).suffix().toLower();
    return !suffix.isEmpty() && supportedImageSuffixes().contains(suffix);
}

bool isImageSizeWithinLimits(const QSize& size, qint64 maximumPixels) {
    if (!size.isValid() || size.width() <= 0 || size.height() <= 0 || maximumPixels <= 0) {
        return false;
    }
    return static_cast<qint64>(size.width()) <= maximumPixels / static_cast<qint64>(size.height());
}

} // namespace purrview::core
