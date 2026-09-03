#include "core/image/ImageMetadataService.h"

#include "core/image/Exiv2MetadataBackend.h"

#include <QColorSpace>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QMimeDatabase>
#include <QThread>

#include <algorithm>

Q_LOGGING_CATEGORY(logMetadata, "impage.metadata", QtInfoMsg)

namespace impage::core {

namespace {
QString colorSpaceName(const QColorSpace& colorSpace) {
    if (!colorSpace.isValid()) {
        return {};
    }
    if (!colorSpace.description().trimmed().isEmpty()) {
        return colorSpace.description().trimmed();
    }
    if (colorSpace == QColorSpace(QColorSpace::SRgb)) {
        return QStringLiteral("sRGB");
    }
    if (colorSpace == QColorSpace(QColorSpace::DisplayP3)) {
        return QStringLiteral("Display P3");
    }
    if (colorSpace == QColorSpace(QColorSpace::AdobeRgb)) {
        return QStringLiteral("Adobe RGB");
    }
    return QStringLiteral("Perfil ICC incorporado");
}
} // namespace

ImageMetadataService::ImageMetadataService(QObject* parent) : QObject(parent) {
    threadPool_.setMaxThreadCount(std::clamp(QThread::idealThreadCount(), 1, 2));
    threadPool_.setExpiryTimeout(15'000);
}

ImageMetadataService::~ImageMetadataService() {
    threadPool_.clear();
    threadPool_.waitForDone();
}

quint64 ImageMetadataService::requestMetadata(const QString& path) {
    const quint64 requestId = ++nextRequestId_;
    const QString absolutePath = QFileInfo(path).absoluteFilePath();
    const QString cacheKey = cacheKeyForFile(absolutePath);
    if (cacheKey.isEmpty()) {
        QMetaObject::invokeMethod(
            this,
            [this, requestId, absolutePath] {
                emit metadataFailed(requestId, absolutePath,
                                    QStringLiteral("Não foi possível acessar o arquivo."));
            },
            Qt::QueuedConnection);
        return requestId;
    }

    if (const auto cached = cache_.constFind(cacheKey); cached != cache_.cend()) {
        ++cacheHitCount_;
        qCDebug(logMetadata) << "Metadata cache hit for" << absolutePath;
        const ImageMetadata metadata = cached.value();
        QMetaObject::invokeMethod(
            this,
            [this, requestId, absolutePath, metadata] {
                emit metadataReady(requestId, absolutePath, metadata, true);
            },
            Qt::QueuedConnection);
        return requestId;
    }

    pending_[cacheKey].push_back({requestId, absolutePath});
    if (pending_[cacheKey].size() > 1) {
        return requestId;
    }

    qCDebug(logMetadata) << "Metadata cache miss; loading" << absolutePath;
    ++loadCount_;
    threadPool_.start([this, cacheKey, absolutePath] {
        ImageMetadata metadata = loadMetadata(absolutePath);
        QMetaObject::invokeMethod(
            this,
            [this, cacheKey, absolutePath, metadata = std::move(metadata)]() mutable {
                finishRequest(cacheKey, absolutePath, std::move(metadata));
            },
            Qt::QueuedConnection);
    });
    return requestId;
}

ImageMetadata ImageMetadataService::readMetadataNow(const QString& path, bool* cacheHit) {
    const QString absolutePath = QFileInfo(path).absoluteFilePath();
    const QString cacheKey = cacheKeyForFile(absolutePath);
    if (!cacheKey.isEmpty()) {
        if (const auto cached = cache_.constFind(cacheKey); cached != cache_.cend()) {
            ++cacheHitCount_;
            if (cacheHit != nullptr) {
                *cacheHit = true;
            }
            return cached.value();
        }
    }
    if (cacheHit != nullptr) {
        *cacheHit = false;
    }
    if (cacheKey.isEmpty()) {
        ImageMetadata missing = basicMetadata(absolutePath, {}, false);
        missing.warning = QStringLiteral("Não foi possível acessar o arquivo.");
        return missing;
    }

    ++loadCount_;
    ImageMetadata metadata = loadMetadata(absolutePath);
    if (cacheKeyForFile(absolutePath) == cacheKey) {
        insertCache(cacheKey, metadata);
    }
    return metadata;
}

ImageMetadata ImageMetadataService::basicMetadata(const QString& path, const QSize& knownSize,
                                                  bool inspectImage) {
    const QFileInfo file(path);
    ImageMetadata metadata;
    metadata.absolutePath = file.absoluteFilePath();
    metadata.fileName = file.fileName();
    metadata.pixelSize = knownSize;
    metadata.format = friendlyImageFormat(file.suffix());
    if (!file.exists() || !file.isFile() || !file.isReadable()) {
        return metadata;
    }

    metadata.fileSize = file.size();
    metadata.modifiedAt = file.lastModified();
    metadata.createdAt = file.birthTime();
    QMimeDatabase mimeDatabase;
    metadata.mimeType = mimeDatabase
                            .mimeTypeForFile(file, inspectImage ? QMimeDatabase::MatchContent
                                                                : QMimeDatabase::MatchExtension)
                            .name();

    if (!inspectImage) {
        metadata.readable = knownSize.isValid();
        return metadata;
    }

    QImageReader reader(file.absoluteFilePath());
    reader.setAutoTransform(true);
    metadata.readable = reader.canRead();
    if (!metadata.readable) {
        metadata.warning = QStringLiteral("Alguns metadados não puderam ser lidos.");
        return metadata;
    }
    const QSize readerSize = reader.size();
    if (readerSize.isValid()) {
        metadata.pixelSize = readerSize;
    }
    const QByteArray detectedFormat = reader.format();
    if (!detectedFormat.isEmpty()) {
        metadata.format = friendlyImageFormat(QString::fromLatin1(detectedFormat));
    }

    reader.setScaledSize(QSize(32, 32));
    const QImage colorProbe = reader.read();
    if (!colorProbe.isNull()) {
        metadata.colorSpace = colorSpaceName(colorProbe.colorSpace());
        metadata.colorProfile = metadata.colorSpace;
    }
    return metadata;
}

QString ImageMetadataService::cacheKeyForFile(const QString& path) const {
    const QFileInfo file(path);
    if (!file.exists() || !file.isFile() || !file.isReadable()) {
        return {};
    }
    const QByteArray identity = file.absoluteFilePath().toUtf8() + '\0' +
                                QByteArray::number(file.size()) + '\0' +
                                QByteArray::number(file.lastModified().toMSecsSinceEpoch());
    return QString::fromLatin1(
        QCryptographicHash::hash(identity, QCryptographicHash::Sha256).toHex());
}

int ImageMetadataService::cacheEntryCount() const {
    return cache_.size();
}

int ImageMetadataService::loadCount() const {
    return loadCount_;
}

int ImageMetadataService::cacheHitCount() const {
    return cacheHitCount_;
}

void ImageMetadataService::clear() {
    cache_.clear();
    cacheOrder_.clear();
}

void ImageMetadataService::invalidateFile(const QString& path) {
    const QString key = cacheKeyForFile(path);
    if (!key.isEmpty()) {
        cache_.remove(key);
        cacheOrder_.removeAll(key);
    }
}

ImageMetadata ImageMetadataService::loadMetadata(const QString& path) {
    qCDebug(logMetadata) << "Metadata load start for" << path;
    ImageMetadata metadata = basicMetadata(path);
    QString advancedWarning;
    const bool advancedMetadataRead =
        populateAdvancedMetadataWithExiv2(path, metadata, &advancedWarning);
    if (!advancedWarning.isEmpty()) {
        qCDebug(logMetadata) << "Metadata parser warning for" << path << advancedWarning;
        metadata.warning = advancedWarning;
    } else if (!advancedMetadataRead) {
        qCDebug(logMetadata) << "Advanced metadata unavailable for" << path;
    }
    return metadata;
}

void ImageMetadataService::finishRequest(const QString& cacheKey, const QString& path,
                                         ImageMetadata metadata) {
    const QList<PendingRequest> requests = pending_.take(cacheKey);
    if (requests.isEmpty()) {
        return;
    }
    if (cacheKeyForFile(path) != cacheKey) {
        for (const PendingRequest& request : requests) {
            emit metadataFailed(request.requestId, request.path,
                                QStringLiteral("O arquivo mudou durante a leitura."));
        }
        return;
    }

    insertCache(cacheKey, metadata);
    for (const PendingRequest& request : requests) {
        emit metadataReady(request.requestId, request.path, metadata, false);
    }
}

void ImageMetadataService::insertCache(const QString& cacheKey, ImageMetadata metadata) {
    cacheOrder_.removeAll(cacheKey);
    cache_.insert(cacheKey, std::move(metadata));
    cacheOrder_.push_back(cacheKey);
    while (cacheOrder_.size() > MaximumCacheEntries) {
        cache_.remove(cacheOrder_.takeFirst());
    }
}

} // namespace impage::core
