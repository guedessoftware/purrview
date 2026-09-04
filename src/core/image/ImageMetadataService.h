#pragma once

#include "core/image/ImageMetadata.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QStringList>
#include <QThreadPool>

namespace purrview::core {

class ImageMetadataService : public QObject {
    Q_OBJECT

  public:
    explicit ImageMetadataService(QObject* parent = nullptr);
    ~ImageMetadataService() override;

    [[nodiscard]] quint64 requestMetadata(const QString& path);
    [[nodiscard]] ImageMetadata readMetadataNow(const QString& path, bool* cacheHit = nullptr);
    [[nodiscard]] static ImageMetadata
    basicMetadata(const QString& path, const QSize& knownSize = {}, bool inspectImage = true);
    [[nodiscard]] QString cacheKeyForFile(const QString& path) const;
    [[nodiscard]] int cacheEntryCount() const;
    [[nodiscard]] int loadCount() const;
    [[nodiscard]] int cacheHitCount() const;
    void clear();
    void invalidateFile(const QString& path);

  signals:
    void metadataReady(quint64 requestId, const QString& path,
                       const purrview::core::ImageMetadata& metadata, bool cacheHit);
    void metadataFailed(quint64 requestId, const QString& path, const QString& error);

  private:
    struct PendingRequest {
        quint64 requestId = 0;
        QString path;
    };

    [[nodiscard]] static ImageMetadata loadMetadata(const QString& path);
    void finishRequest(const QString& cacheKey, const QString& path, ImageMetadata metadata);
    void insertCache(const QString& cacheKey, ImageMetadata metadata);

    static constexpr int MaximumCacheEntries = 256;

    QHash<QString, ImageMetadata> cache_;
    QStringList cacheOrder_;
    QHash<QString, QList<PendingRequest>> pending_;
    QThreadPool threadPool_;
    quint64 nextRequestId_ = 0;
    int loadCount_ = 0;
    int cacheHitCount_ = 0;
};

} // namespace purrview::core
