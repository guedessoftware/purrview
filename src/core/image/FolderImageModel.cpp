#include "core/image/FolderImageModel.h"

#include "core/image/ImageFormatSupport.h"

#include <QCollator>
#include <QDir>
#include <QFileInfo>
#include <QMetaObject>
#include <QSet>

#include <algorithm>

namespace impage::core {

namespace {
QString normalizedPath(const QString& path) {
    return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

QStringView nextToken(QStringView value, qsizetype& position, bool& numeric) {
    if (position >= value.size()) {
        numeric = false;
        return {};
    }
    numeric = value.at(position).isDigit();
    const qsizetype start = position;
    while (position < value.size() && value.at(position).isDigit() == numeric) {
        ++position;
    }
    return value.sliced(start, position - start);
}

int compareNumericTokens(QStringView first, QStringView second) {
    while (first.size() > 1 && first.front() == QLatin1Char('0')) {
        first = first.sliced(1);
    }
    while (second.size() > 1 && second.front() == QLatin1Char('0')) {
        second = second.sliced(1);
    }
    if (first.size() != second.size()) {
        return first.size() < second.size() ? -1 : 1;
    }
    const int lexical = first.compare(second, Qt::CaseSensitive);
    return lexical < 0 ? -1 : lexical > 0 ? 1 : 0;
}

int naturalCompare(const QString& first, const QString& second, const QCollator& collator) {
    const QStringView firstView(first);
    const QStringView secondView(second);
    qsizetype firstPosition = 0;
    qsizetype secondPosition = 0;
    while (firstPosition < firstView.size() && secondPosition < secondView.size()) {
        bool firstNumeric = false;
        bool secondNumeric = false;
        const QStringView firstToken = nextToken(firstView, firstPosition, firstNumeric);
        const QStringView secondToken = nextToken(secondView, secondPosition, secondNumeric);
        int comparison = 0;
        if (firstNumeric && secondNumeric) {
            comparison = compareNumericTokens(firstToken, secondToken);
        } else {
            comparison = collator.compare(firstToken, secondToken);
        }
        if (comparison != 0) {
            return comparison;
        }
    }
    if (firstPosition != firstView.size() || secondPosition != secondView.size()) {
        return firstPosition == firstView.size() ? -1 : 1;
    }
    return 0;
}
} // namespace

FolderImageModel::FolderImageModel(ImageSession& session, ThumbnailCache& thumbnailCache,
                                   QObject* parent)
    : QAbstractListModel(parent), session_(session), thumbnailCache_(thumbnailCache) {
    scanPool_.setMaxThreadCount(1);
    scanPool_.setExpiryTimeout(10'000);
    refreshTimer_.setSingleShot(true);
    refreshTimer_.setInterval(180);

    connect(&session_, &ImageSession::currentImageChanged, this,
            &FolderImageModel::updateSessionCurrentRole);
    connect(&session_, &ImageSession::selectionChanged, this, [this] {
        if (!items_.empty()) {
            emit dataChanged(index(0), index(count() - 1), {SelectedRole});
        }
        emit selectedCountChanged();
    });
    connect(&thumbnailCache_, &ThumbnailCache::thumbnailReady, this,
            [this](const QString& path, const QString& key, const QSize& sourceSize) {
                const int itemIndex = indexOfPath(path);
                if (itemIndex < 0 || thumbnailCache_.cacheKeyForFile(path) != key) {
                    return;
                }
                FolderItem& item = items_[static_cast<std::size_t>(itemIndex)];
                item.thumbnailKey = key;
                item.pixelSize = sourceSize;
                item.valid = true;
                emit dataChanged(index(itemIndex), index(itemIndex),
                                 {ThumbnailUrlRole, ValidRole, WidthRole, HeightRole});
            });
    connect(&thumbnailCache_, &ThumbnailCache::thumbnailFailed, this,
            [this](const QString& path, const QString&) {
                const int itemIndex = indexOfPath(path);
                if (itemIndex < 0) {
                    return;
                }
                items_[static_cast<std::size_t>(itemIndex)].valid = false;
                emit dataChanged(index(itemIndex), index(itemIndex), {ValidRole});
            });
    connect(&watcher_, &QFileSystemWatcher::directoryChanged, this,
            [this] { refreshTimer_.start(); });
    connect(&refreshTimer_, &QTimer::timeout, this, &FolderImageModel::refresh);
}

FolderImageModel::~FolderImageModel() {
    scanPool_.clear();
    scanPool_.waitForDone();
}

int FolderImageModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : count();
}

QVariant FolderImageModel::data(const QModelIndex& modelIndex, int role) const {
    if (!modelIndex.isValid() || modelIndex.row() < 0 || modelIndex.row() >= count()) {
        return {};
    }
    const FolderItem& item = items_[static_cast<std::size_t>(modelIndex.row())];
    switch (role) {
    case FileNameRole:
        return item.fileName;
    case FilePathRole:
        return item.filePath;
    case SourceUrlRole:
        return QUrl::fromLocalFile(item.filePath);
    case ThumbnailUrlRole:
        return item.thumbnailKey.isEmpty()
                   ? QUrl()
                   : QUrl(QStringLiteral("image://impage-thumbnail/%1").arg(item.thumbnailKey));
    case CurrentRole:
        return modelIndex.row() == currentIndex();
    case SelectedRole:
        return isPathSelected(item.filePath);
    case ValidRole:
        return item.valid;
    case WidthRole:
        return item.pixelSize.width();
    case HeightRole:
        return item.pixelSize.height();
    default:
        return {};
    }
}

QHash<int, QByteArray> FolderImageModel::roleNames() const {
    return {{FileNameRole, "fileName"}, {FilePathRole, "filePath"},
            {SourceUrlRole, "source"},  {ThumbnailUrlRole, "thumbnailSource"},
            {CurrentRole, "current"},   {SelectedRole, "selected"},
            {ValidRole, "valid"},       {WidthRole, "pixelWidth"},
            {HeightRole, "pixelHeight"}};
}

int FolderImageModel::count() const {
    return static_cast<int>(items_.size());
}

int FolderImageModel::currentIndex() const {
    const ImageEntry* current = session_.currentImage();
    return current == nullptr ? -1 : indexOfPath(current->sourcePath);
}

QUrl FolderImageModel::directoryUrl() const {
    return directoryPath_.isEmpty() ? QUrl() : QUrl::fromLocalFile(directoryPath_);
}

bool FolderImageModel::scanning() const {
    return scanning_;
}

int FolderImageModel::selectedCount() const {
    return static_cast<int>(selectedPaths().size());
}

QStringList FolderImageModel::selectedPaths() const {
    QSet<QString> selectedSessionPaths;
    for (const ImageEntry& image : session_.images()) {
        if (image.selected) {
            selectedSessionPaths.insert(image.sourcePath);
        }
    }
    QStringList selected;
    for (const FolderItem& item : items_) {
        if (selectedSessionPaths.contains(item.filePath)) {
            selected.push_back(item.filePath);
        }
    }
    return selected;
}

QString FolderImageModel::pathAt(int itemIndex) const {
    return itemIndex < 0 || itemIndex >= count()
               ? QString()
               : items_[static_cast<std::size_t>(itemIndex)].filePath;
}

QUrl FolderImageModel::sourceAt(int itemIndex) const {
    const QString path = pathAt(itemIndex);
    return path.isEmpty() ? QUrl() : QUrl::fromLocalFile(path);
}

int FolderImageModel::indexOfPath(const QString& path) const {
    if (path.isEmpty()) {
        return -1;
    }
    const QString target = normalizedPath(path);
    const auto found =
        std::find_if(items_.cbegin(), items_.cend(),
                     [&target](const FolderItem& item) { return item.filePath == target; });
    return found == items_.cend() ? -1 : static_cast<int>(std::distance(items_.cbegin(), found));
}

void FolderImageModel::openFromImage(const QString& imagePath) {
    const QFileInfo image(imagePath);
    if (imagePath.isEmpty() || image.absolutePath().isEmpty()) {
        beginScan({}, {});
        return;
    }
    beginScan(image.absolutePath(), normalizedPath(imagePath));
}

bool FolderImageModel::scanFromImageSynchronously(const QString& imagePath) {
    const QFileInfo image(imagePath);
    const QString directory = image.absolutePath();
    const QString requestedPath = normalizedPath(imagePath);
    const int previousCurrent = currentIndex();
    const quint64 generation = ++scanGeneration_;
    scanning_ = true;
    emit scanningChanged();
    applyScan(directory, requestedPath, previousCurrent, generation, enumerate(directory));
    return indexOfPath(requestedPath) >= 0;
}

void FolderImageModel::refresh() {
    if (!directoryPath_.isEmpty()) {
        const ImageEntry* current = session_.currentImage();
        beginScan(directoryPath_, current == nullptr ? QString() : current->sourcePath);
    }
}

bool FolderImageModel::refreshSynchronously() {
    if (directoryPath_.isEmpty()) {
        return false;
    }
    const ImageEntry* current = session_.currentImage();
    const QString requestedPath = current == nullptr ? QString() : current->sourcePath;
    const int previousCurrent = currentIndex();
    const quint64 generation = ++scanGeneration_;
    scanning_ = true;
    emit scanningChanged();
    applyScan(directoryPath_, requestedPath, previousCurrent, generation,
              enumerate(directoryPath_));
    return true;
}

void FolderImageModel::setWatchingEnabled(bool enabled) {
    if (watchingEnabled_ == enabled) {
        return;
    }
    watchingEnabled_ = enabled;
    updateWatcher();
}

void FolderImageModel::requestThumbnail(int itemIndex) {
    if (itemIndex < 0 || itemIndex >= count()) {
        return;
    }
    FolderItem& item = items_[static_cast<std::size_t>(itemIndex)];
    if (!item.thumbnailKey.isEmpty()) {
        if (thumbnailCache_.contains(item.thumbnailKey)) {
            return;
        }
        item.thumbnailKey.clear();
        item.thumbnailRequested = false;
        emit dataChanged(index(itemIndex), index(itemIndex), {ThumbnailUrlRole});
    }
    if (item.thumbnailRequested) {
        return;
    }

    const QString cacheKey = thumbnailCache_.cacheKeyForFile(item.filePath);
    if (thumbnailCache_.contains(cacheKey)) {
        item.thumbnailKey = cacheKey;
        emit dataChanged(index(itemIndex), index(itemIndex), {ThumbnailUrlRole});
        return;
    }

    item.thumbnailRequested = true;
    const QString requestedKey = thumbnailCache_.requestThumbnail(item.filePath);
    if (requestedKey.isEmpty()) {
        item.valid = false;
        emit dataChanged(index(itemIndex), index(itemIndex), {ValidRole});
    }
}

void FolderImageModel::requestAround(int centerIndex, int radius) {
    if (centerIndex < 0 || count() == 0) {
        return;
    }
    const int first = std::max(0, centerIndex - std::max(0, radius));
    const int last = std::min(count() - 1, centerIndex + std::max(0, radius));
    requestThumbnail(centerIndex);
    for (int offset = 1; centerIndex - offset >= first || centerIndex + offset <= last; ++offset) {
        requestThumbnail(centerIndex - offset);
        requestThumbnail(centerIndex + offset);
    }
}

std::vector<FolderImageModel::FolderItem>
FolderImageModel::enumerate(const QString& directoryPath) {
    std::vector<FolderItem> result;
    const QDir directory(directoryPath);
    if (!directory.exists() || !directory.isReadable()) {
        return result;
    }

    const QFileInfoList files =
        directory.entryInfoList(QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);
    result.reserve(static_cast<std::size_t>(files.size()));
    for (const QFileInfo& file : files) {
        if (isSupportedImageFile(file.absoluteFilePath())) {
            result.push_back({.fileName = file.fileName(),
                              .filePath = normalizedPath(file.absoluteFilePath()),
                              .thumbnailKey = {},
                              .pixelSize = {},
                              .thumbnailRequested = false,
                              .valid = true});
        }
    }

    QCollator collator;
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(result.begin(), result.end(),
              [&collator](const FolderItem& first, const FolderItem& second) {
                  const int comparison = naturalCompare(first.fileName, second.fileName, collator);
                  return comparison == 0 ? first.fileName < second.fileName : comparison < 0;
              });
    return result;
}

void FolderImageModel::beginScan(const QString& directoryPath,
                                 const QString& requestedCurrentPath) {
    const int previousCurrent = currentIndex();
    const quint64 generation = ++scanGeneration_;
    if (!scanning_) {
        scanning_ = true;
        emit scanningChanged();
    }

    scanPool_.start([this, directoryPath, requestedCurrentPath, previousCurrent, generation] {
        std::vector<FolderItem> items = enumerate(directoryPath);
        QMetaObject::invokeMethod(
            this,
            [this, directoryPath, requestedCurrentPath, previousCurrent, generation,
             items = std::move(items)]() mutable {
                applyScan(directoryPath, requestedCurrentPath, previousCurrent, generation,
                          std::move(items));
            },
            Qt::QueuedConnection);
    });
}

void FolderImageModel::applyScan(const QString& directoryPath, const QString& requestedCurrentPath,
                                 int previousCurrentIndex, quint64 generation,
                                 std::vector<FolderItem> items) {
    if (generation != scanGeneration_) {
        return;
    }

    const QString previousDirectory = directoryPath_;
    beginResetModel();
    directoryPath_ = directoryPath;
    items_ = std::move(items);
    endResetModel();
    emit countChanged();
    emit selectedCountChanged();
    if (previousDirectory != directoryPath_) {
        emit directoryChanged();
    }
    updateWatcher();

    scanning_ = false;
    emit scanningChanged();
    lastCurrentIndex_ = currentIndex();
    emit currentIndexChanged();
    requestAround(lastCurrentIndex_);
    emit scanCompleted(requestedCurrentPath, indexOfPath(requestedCurrentPath) >= 0,
                       previousCurrentIndex);
}

void FolderImageModel::updateSessionCurrentRole() {
    const int nextCurrentIndex = currentIndex();
    if (lastCurrentIndex_ >= 0 && lastCurrentIndex_ < count()) {
        emit dataChanged(index(lastCurrentIndex_), index(lastCurrentIndex_), {CurrentRole});
    }
    if (nextCurrentIndex >= 0 && nextCurrentIndex < count()) {
        emit dataChanged(index(nextCurrentIndex), index(nextCurrentIndex), {CurrentRole});
    }
    lastCurrentIndex_ = nextCurrentIndex;
    emit currentIndexChanged();
    requestAround(nextCurrentIndex);
}

void FolderImageModel::updateWatcher() {
    if (!watcher_.directories().isEmpty()) {
        watcher_.removePaths(watcher_.directories());
    }
    if (watchingEnabled_ && !directoryPath_.isEmpty() && QDir(directoryPath_).exists()) {
        watcher_.addPath(directoryPath_);
    }
}

bool FolderImageModel::isPathSelected(const QString& path) const {
    return std::any_of(
        session_.images().cbegin(), session_.images().cend(),
        [&path](const ImageEntry& image) { return image.selected && image.sourcePath == path; });
}

} // namespace impage::core
