#include "core/image/ThumbnailCache.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>
#include <QTimer>

#include <iostream>

namespace {

int failures = 0;

void check(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

QString saveImage(const QTemporaryDir& directory, const QString& name, const QSize& size,
                  const QColor& color) {
    const QString path = directory.filePath(name);
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(color);
    check(image.save(path), "thumbnail fixture can be saved");
    return path;
}

} // namespace

int main(int argc, char* argv[]) {
    QCoreApplication application(argc, argv);
    QTemporaryDir directory;
    check(directory.isValid(), "thumbnail fixture directory is available");
    const QString source =
        saveImage(directory, QStringLiteral("large.png"), QSize(1000, 600), Qt::red);

    purrview::core::ThumbnailCache cache;
    QString firstKey;
    bool cacheHit = true;
    const QImage first = cache.thumbnailNow(source, &firstKey, &cacheHit);
    check(!first.isNull() && first.width() <= 256 && first.height() <= 256,
          "thumbnail is generated with a bounded size");
    check(!cacheHit && cache.generationCount() == 1,
          "first thumbnail request is a cache miss and generation");
    check(cache.contains(firstKey) && !cache.imageForKey(firstKey).isNull(),
          "generated thumbnail is retained in memory");

    QString secondKey;
    const QImage second = cache.thumbnailNow(source, &secondKey, &cacheHit);
    check(cacheHit && secondKey == firstKey && second.cacheKey() == first.cacheKey(),
          "second request is served from the same cache entry");
    check(cache.generationCount() == 1, "cache hit avoids regeneration");
    check(cache.imageForKey(QStringLiteral("unknown-key")).isNull(), "unknown key is a cache miss");

    const QString missing = directory.filePath(QStringLiteral("missing.png"));
    check(cache.thumbnailNow(missing).isNull(), "missing file cannot generate a thumbnail");
    const QString corruptPath = directory.filePath(QStringLiteral("corrupt.png"));
    QFile corrupt(corruptPath);
    check(corrupt.open(QIODevice::WriteOnly), "corrupt thumbnail fixture can be created");
    corrupt.write("not a PNG");
    corrupt.close();
    check(cache.thumbnailNow(corruptPath).isNull(), "corrupt image produces no thumbnail");

    QImage changed(620, 620, QImage::Format_ARGB32_Premultiplied);
    changed.fill(Qt::green);
    check(changed.save(source), "source can be modified for invalidation test");
    QString changedKey;
    const QImage changedThumbnail = cache.thumbnailNow(source, &changedKey, &cacheHit);
    check(!changedThumbnail.isNull(), "modified source still generates a thumbnail");
    check(!cacheHit && changedKey != firstKey,
          "file size or mtime change invalidates the previous cache key");

    cache.setMaximumCostBytes(280'000);
    const QString secondSource =
        saveImage(directory, QStringLiteral("second.png"), QSize(900, 900), Qt::blue);
    const QImage boundedThumbnail = cache.thumbnailNow(secondSource);
    check(!boundedThumbnail.isNull(), "second cache entry can be generated");
    check(cache.currentCostBytes() <= cache.maximumCostBytes(),
          "memory cache stays within its configured byte limit");

    const QString asyncSource =
        saveImage(directory, QStringLiteral("async.png"), QSize(1200, 800), Qt::yellow);
    bool asyncReady = false;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(&cache, &purrview::core::ThumbnailCache::thumbnailReady, &loop,
                     [&](const QString& path, const QString&, const QSize&) {
                         if (path == asyncSource) {
                             asyncReady = true;
                             loop.quit();
                         }
                     });
    const QString asyncKey = cache.requestThumbnail(asyncSource);
    timeout.start(5'000);
    loop.exec();
    check(asyncReady && cache.contains(asyncKey),
          "asynchronous generation returns through the main thread and populates cache");

    cache.clear();
    check(cache.currentCostBytes() == 0, "cache can release all thumbnail memory");
    return failures == 0 ? 0 : 1;
}
