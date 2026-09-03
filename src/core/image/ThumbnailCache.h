#pragma once

#include <QCache>
#include <QImage>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <QSize>
#include <QString>
#include <QThreadPool>

#include <atomic>

namespace impage::core {

class ThumbnailCache : public QObject {
    Q_OBJECT

  public:
    static constexpr int DefaultMaximumCostBytes = 128 * 1024 * 1024;
    static constexpr int DefaultMaximumSide = 256;

    explicit ThumbnailCache(QObject* parent = nullptr);
    ~ThumbnailCache() override;

    [[nodiscard]] QString cacheKeyForFile(const QString& path) const;
    [[nodiscard]] bool contains(const QString& cacheKey) const;
    [[nodiscard]] QImage imageForKey(const QString& cacheKey) const;
    [[nodiscard]] int maximumCostBytes() const;
    [[nodiscard]] int currentCostBytes() const;
    [[nodiscard]] int generationCount() const;

    void setMaximumCostBytes(int bytes);
    void clear();
    [[nodiscard]] QString requestThumbnail(const QString& path,
                                           int maximumSide = DefaultMaximumSide);
    [[nodiscard]] QImage thumbnailNow(const QString& path, QString* cacheKey = nullptr,
                                      bool* cacheHit = nullptr,
                                      int maximumSide = DefaultMaximumSide);

  signals:
    void thumbnailReady(const QString& path, const QString& cacheKey, const QSize& sourceSize);
    void thumbnailFailed(const QString& path, const QString& cacheKey);

  private:
    struct GeneratedThumbnail {
        QImage image;
        QSize sourceSize;
    };

    [[nodiscard]] static GeneratedThumbnail generateThumbnail(const QString& path, int maximumSide);
    void finishRequest(const QString& path, const QString& cacheKey, GeneratedThumbnail thumbnail);
    void insert(const QString& cacheKey, const QImage& image);

    mutable QMutex cacheMutex_;
    QCache<QString, QImage> cache_;
    QSet<QString> pendingKeys_;
    QThreadPool threadPool_;
    std::atomic_int generationCount_ = 0;
};

} // namespace impage::core
