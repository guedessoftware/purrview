#include "core/image/ThumbnailCache.h"

#include "core/image/ImageFormatSupport.h"

#include <QCryptographicHash>
#include <QFileInfo>
#include <QImageReader>
#include <QMetaObject>
#include <QMutexLocker>
#include <QThread>

#include <algorithm>

namespace purrview::core {

ThumbnailCache::ThumbnailCache(QObject* parent) : QObject(parent) {
    cache_.setMaxCost(DefaultMaximumCostBytes);
    threadPool_.setMaxThreadCount(std::clamp(QThread::idealThreadCount(), 1, 4));
    threadPool_.setExpiryTimeout(15'000);
}

ThumbnailCache::~ThumbnailCache() {
    threadPool_.clear();
    threadPool_.waitForDone();
}

QString ThumbnailCache::cacheKeyForFile(const QString& path) const {
    const QFileInfo file(path);
    if (!file.exists() || !file.isFile()) {
        return {};
    }
    const QByteArray identity = file.absoluteFilePath().toUtf8() + '\0' +
                                QByteArray::number(file.size()) + '\0' +
                                QByteArray::number(file.lastModified().toMSecsSinceEpoch());
    return QString::fromLatin1(
        QCryptographicHash::hash(identity, QCryptographicHash::Sha256).toHex());
}

bool ThumbnailCache::contains(const QString& cacheKey) const {
    QMutexLocker lock(&cacheMutex_);
    return !cacheKey.isEmpty() && cache_.contains(cacheKey);
}

QImage ThumbnailCache::imageForKey(const QString& cacheKey) const {
    QMutexLocker lock(&cacheMutex_);
    const QImage* image = cache_.object(cacheKey);
    return image == nullptr ? QImage() : *image;
}

qsizetype ThumbnailCache::maximumCostBytes() const {
    QMutexLocker lock(&cacheMutex_);
    return cache_.maxCost();
}

qsizetype ThumbnailCache::currentCostBytes() const {
    QMutexLocker lock(&cacheMutex_);
    return cache_.totalCost();
}

int ThumbnailCache::generationCount() const {
    return generationCount_.load();
}

void ThumbnailCache::setMaximumCostBytes(qsizetype bytes) {
    QMutexLocker lock(&cacheMutex_);
    cache_.setMaxCost(std::max<qsizetype>(1, bytes));
}

void ThumbnailCache::clear() {
    QMutexLocker lock(&cacheMutex_);
    cache_.clear();
}

QString ThumbnailCache::requestThumbnail(const QString& path, int maximumSide) {
    const QString cacheKey = cacheKeyForFile(path);
    if (cacheKey.isEmpty() || contains(cacheKey) || pendingKeys_.contains(cacheKey)) {
        return cacheKey;
    }

    pendingKeys_.insert(cacheKey);
    maximumSide = std::max(1, maximumSide);
    threadPool_.start([this, path, cacheKey, maximumSide] {
        GeneratedThumbnail thumbnail = generateThumbnail(path, maximumSide);
        QMetaObject::invokeMethod(
            this,
            [this, path, cacheKey, thumbnail = std::move(thumbnail)]() mutable {
                finishRequest(path, cacheKey, thumbnail);
            },
            Qt::QueuedConnection);
    });
    return cacheKey;
}

QImage ThumbnailCache::thumbnailNow(const QString& path, QString* cacheKey, bool* cacheHit,
                                    int maximumSide) {
    const QString key = cacheKeyForFile(path);
    if (cacheKey != nullptr) {
        *cacheKey = key;
    }
    QImage cached = imageForKey(key);
    if (!cached.isNull()) {
        if (cacheHit != nullptr) {
            *cacheHit = true;
        }
        return cached;
    }
    if (cacheHit != nullptr) {
        *cacheHit = false;
    }
    if (key.isEmpty()) {
        return {};
    }

    GeneratedThumbnail thumbnail = generateThumbnail(path, std::max(1, maximumSide));
    if (!thumbnail.image.isNull() && cacheKeyForFile(path) == key) {
        insert(key, thumbnail.image);
        ++generationCount_;
    }
    return thumbnail.image;
}

ThumbnailCache::GeneratedThumbnail ThumbnailCache::generateThumbnail(const QString& path,
                                                                     int maximumSide) {
    QImageReader reader(path);
    reader.setAutoTransform(true);
    const QSize sourceSize = reader.size();
    if (!reader.canRead() || !isImageSizeWithinLimits(sourceSize)) {
        return {};
    }

    QSize decodeSize = sourceSize;
    if (decodeSize.width() > maximumSide || decodeSize.height() > maximumSide) {
        decodeSize.scale(maximumSide, maximumSide, Qt::KeepAspectRatio);
        reader.setScaledSize(decodeSize);
    }

    QImage image = reader.read();
    if (!image.isNull() && (image.width() > maximumSide || image.height() > maximumSide)) {
        image =
            image.scaled(maximumSide, maximumSide, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return {.image = std::move(image), .sourceSize = sourceSize};
}

void ThumbnailCache::finishRequest(const QString& path, const QString& cacheKey,
                                   const GeneratedThumbnail& thumbnail) {
    pendingKeys_.remove(cacheKey);
    if (cacheKeyForFile(path) != cacheKey || thumbnail.image.isNull()) {
        emit thumbnailFailed(path, cacheKey);
        return;
    }

    insert(cacheKey, thumbnail.image);
    ++generationCount_;
    emit thumbnailReady(path, cacheKey, thumbnail.sourceSize);
}

void ThumbnailCache::insert(const QString& cacheKey, const QImage& image) {
    const qsizetype cost = std::max<qsizetype>(1, image.sizeInBytes());
    QMutexLocker lock(&cacheMutex_);
    cache_.insert(cacheKey, new QImage(image), cost);
}

} // namespace purrview::core
