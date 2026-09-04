#include "core/image/ImageSession.h"
#include "core/image/ImageFormatSupport.h"

#include <QFileInfo>
#include <QImageReader>
#include <QLoggingCategory>
#include <QSet>

#include <algorithm>

Q_LOGGING_CATEGORY(logSession, "purrview.session")

namespace purrview::core {

ImageSession::ImageSession(QObject* parent) : QAbstractListModel(parent) {
    qCDebug(logSession) << "Image session created";
}

int ImageSession::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(images_.size());
}

QVariant ImageSession::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= count()) {
        return {};
    }

    const ImageEntry& image = images_[static_cast<std::size_t>(index.row())];
    switch (role) {
    case IdRole:
        return image.id.toString(QUuid::WithoutBraces);
    case SourceRole:
        return QUrl::fromLocalFile(image.sourcePath);
    case FileNameRole:
        return image.fileName;
    case SelectedRole:
        return image.selected;
    case CurrentRole:
        return index.row() == currentIndex_;
    case WidthRole:
        return image.pixelSize.width();
    case HeightRole:
        return image.pixelSize.height();
    case RotationRole:
        return image.rotationDegrees;
    case ValidRole:
        return image.valid;
    default:
        return {};
    }
}

QHash<int, QByteArray> ImageSession::roleNames() const {
    return {{IdRole, "imageId"},         {SourceRole, "source"},     {FileNameRole, "fileName"},
            {SelectedRole, "selected"},  {CurrentRole, "current"},   {WidthRole, "pixelWidth"},
            {HeightRole, "pixelHeight"}, {RotationRole, "rotation"}, {ValidRole, "valid"}};
}

int ImageSession::count() const {
    return static_cast<int>(images_.size());
}

int ImageSession::currentIndex() const {
    return currentIndex_;
}

QString ImageSession::currentImageId() const {
    const ImageEntry* image = currentImage();
    return image == nullptr ? QString() : image->id.toString(QUuid::WithoutBraces);
}

QUrl ImageSession::currentImageSource() const {
    const ImageEntry* image = currentImage();
    return image == nullptr ? QUrl() : QUrl::fromLocalFile(image->sourcePath);
}

QUrl ImageSession::sourceFolder() const {
    return sourceFolderPath_.isEmpty() ? QUrl() : QUrl::fromLocalFile(sourceFolderPath_);
}

int ImageSession::selectedCount() const {
    return static_cast<int>(std::count_if(images_.cbegin(), images_.cend(),
                                          [](const ImageEntry& image) { return image.selected; }));
}

const std::vector<ImageEntry>& ImageSession::images() const {
    return images_;
}

std::vector<ImageEntry> ImageSession::selectedImages() const {
    std::vector<ImageEntry> selected;
    selected.reserve(static_cast<std::size_t>(selectedCount()));
    std::copy_if(images_.cbegin(), images_.cend(), std::back_inserter(selected),
                 [](const ImageEntry& image) { return image.selected; });
    return selected;
}

const ImageEntry* ImageSession::currentImage() const {
    if (currentIndex_ < 0 || currentIndex_ >= count()) {
        return nullptr;
    }
    return &images_[static_cast<std::size_t>(currentIndex_)];
}

std::optional<ImageId> ImageSession::addImage(const QString& path, QString* error) {
    QStringList errors;
    const QList<ImageId> ids = addImages({path}, &errors);
    if (error != nullptr) {
        *error = errors.isEmpty() ? QString() : errors.constLast();
    }
    return ids.isEmpty() ? std::nullopt : std::optional<ImageId>(ids.constFirst());
}

std::optional<ImageId> ImageSession::addImageReference(const QString& path, QString* error) {
    QStringList errors;
    const QList<ImageId> images = addImageReferences({path}, true, &errors);
    if (error != nullptr) {
        *error = errors.isEmpty() ? QString() : errors.constLast();
    }
    return images.isEmpty() ? std::nullopt : std::optional<ImageId>(images.constFirst());
}

QList<ImageId> ImageSession::addImageReferences(const QStringList& paths, bool inspectImages,
                                                QStringList* errors) {
    std::vector<ImageEntry> references;
    references.reserve(static_cast<std::size_t>(paths.size()));
    for (const QString& path : paths) {
        const QFileInfo file(path);
        if (!file.exists() || !file.isFile() || !file.isReadable()) {
            if (errors != nullptr) {
                errors->push_back(
                    QStringLiteral("Não foi possível acessar a imagem: %1").arg(path));
            }
            continue;
        }
        if (!isSupportedImageFile(path)) {
            if (errors != nullptr) {
                errors->push_back(
                    QStringLiteral("Formato de imagem não suportado: %1").arg(file.fileName()));
            }
            continue;
        }

        QSize pixelSize;
        bool valid = true;
        if (inspectImages) {
            QImageReader reader(file.absoluteFilePath());
            reader.setAutoTransform(true);
            valid = reader.canRead() && isImageSizeWithinLimits(reader.size());
            if (valid) {
                pixelSize = reader.size();
            }
        }
        references.push_back({.id = QUuid::createUuid(),
                              .sourcePath = file.absoluteFilePath(),
                              .fileName = file.fileName(),
                              .pixelSize = pixelSize,
                              .valid = valid});
    }

    if (references.empty()) {
        return {};
    }
    const int firstRow = count();
    const int lastRow = firstRow + static_cast<int>(references.size()) - 1;
    beginInsertRows({}, firstRow, lastRow);
    QList<ImageId> ids;
    ids.reserve(static_cast<qsizetype>(references.size()));
    for (ImageEntry& image : references) {
        ids.push_back(image.id);
        images_.push_back(std::move(image));
    }
    endInsertRows();

    emit countChanged();
    recomputeSourceFolder();
    if (currentIndex_ == -1) {
        currentIndex_ = 0;
        emit dataChanged(index(0), index(0), {CurrentRole});
        emit currentImageChanged();
    }
    return ids;
}

QList<ImageId> ImageSession::addImages(const QStringList& paths, QStringList* errors) {
    std::vector<ImageEntry> validImages;
    validImages.reserve(static_cast<std::size_t>(paths.size()));

    for (const QString& path : paths) {
        QString error;
        std::optional<ImageEntry> image = validateImage(path, &error);
        if (image.has_value()) {
            validImages.push_back(std::move(*image));
        } else {
            qCWarning(logSession) << "Image rejected:" << error;
            if (errors != nullptr) {
                errors->push_back(error);
            }
        }
    }

    if (validImages.empty()) {
        return {};
    }

    const int firstRow = count();
    const int lastRow = firstRow + static_cast<int>(validImages.size()) - 1;
    beginInsertRows({}, firstRow, lastRow);
    QList<ImageId> ids;
    ids.reserve(static_cast<qsizetype>(validImages.size()));
    for (ImageEntry& image : validImages) {
        ids.push_back(image.id);
        images_.push_back(std::move(image));
    }
    endInsertRows();

    emit countChanged();
    recomputeSourceFolder();
    if (currentIndex_ == -1) {
        currentIndex_ = 0;
        emit dataChanged(index(0), index(0), {CurrentRole});
        emit currentImageChanged();
    }
    qCDebug(logSession) << validImages.size() << "image(s) added to session";
    return ids;
}

bool ImageSession::removeImage(const ImageId& id) {
    const int removedIndex = indexOf(id);
    if (removedIndex < 0) {
        return false;
    }

    const int previousCurrentIndex = currentIndex_;
    const ImageId previousCurrentId = currentImage() == nullptr ? ImageId() : currentImage()->id;
    const bool selectionChangedByRemoval = images_[static_cast<std::size_t>(removedIndex)].selected;

    beginRemoveRows({}, removedIndex, removedIndex);
    images_.erase(images_.begin() + removedIndex);
    if (images_.empty()) {
        currentIndex_ = -1;
    } else if (removedIndex < previousCurrentIndex) {
        currentIndex_ = previousCurrentIndex - 1;
    } else if (removedIndex == previousCurrentIndex) {
        currentIndex_ = std::min(removedIndex, count() - 1);
    }
    endRemoveRows();

    emit countChanged();
    recomputeSourceFolder();
    if (selectionChangedByRemoval) {
        emit selectionChanged();
    }

    const ImageId nextCurrentId = currentImage() == nullptr ? ImageId() : currentImage()->id;
    if (previousCurrentIndex != currentIndex_ || previousCurrentId != nextCurrentId) {
        if (currentIndex_ >= 0) {
            emit dataChanged(index(currentIndex_), index(currentIndex_), {CurrentRole});
        }
        emit currentImageChanged();
    }
    qCDebug(logSession) << "Image removed from session";
    return true;
}

bool ImageSession::removeImageById(const QString& id) {
    return removeImage(QUuid(id));
}

void ImageSession::clear() {
    if (images_.empty()) {
        return;
    }

    const bool hadSelection = selectedCount() > 0;
    beginResetModel();
    images_.clear();
    currentIndex_ = -1;
    sourceFolderPath_.clear();
    endResetModel();

    emit countChanged();
    emit currentImageChanged();
    emit sourceFolderChanged();
    if (hadSelection) {
        emit selectionChanged();
    }
    emit sessionCleared();
}

void ImageSession::setCurrentIndex(int nextIndex) {
    if (nextIndex < 0 || nextIndex >= count() || nextIndex == currentIndex_) {
        return;
    }
    const int previousIndex = currentIndex_;
    currentIndex_ = nextIndex;
    emitCurrentRowsChanged(previousIndex, currentIndex_);
    emit currentImageChanged();
}

bool ImageSession::setCurrentImage(const ImageId& id) {
    const int imageIndex = indexOf(id);
    if (imageIndex < 0) {
        return false;
    }
    setCurrentIndex(imageIndex);
    return true;
}

bool ImageSession::setCurrentImageById(const QString& id) {
    return setCurrentImage(QUuid(id));
}

bool ImageSession::selectImage(const ImageId& id) {
    return setSelected(id, true);
}

bool ImageSession::deselectImage(const ImageId& id) {
    return setSelected(id, false);
}

bool ImageSession::toggleSelection(const ImageId& id) {
    const int imageIndex = indexOf(id);
    if (imageIndex < 0) {
        return false;
    }
    return setSelected(id, !images_[static_cast<std::size_t>(imageIndex)].selected);
}

bool ImageSession::selectImages(const QList<ImageId>& ids) {
    if (ids.isEmpty()) {
        return false;
    }
    const QSet<ImageId> requested(ids.cbegin(), ids.cend());
    bool changed = false;
    for (ImageEntry& image : images_) {
        if (requested.contains(image.id) && !image.selected) {
            image.selected = true;
            changed = true;
        }
    }
    if (changed) {
        emit dataChanged(index(0), index(count() - 1), {SelectedRole});
        emit selectionChanged();
    }
    return changed;
}

bool ImageSession::replaceSelection(const QList<ImageId>& ids) {
    const QSet<ImageId> requested(ids.cbegin(), ids.cend());
    bool changed = false;
    for (ImageEntry& image : images_) {
        const bool selected = requested.contains(image.id);
        if (image.selected != selected) {
            image.selected = selected;
            changed = true;
        }
    }
    if (changed && !images_.empty()) {
        emit dataChanged(index(0), index(count() - 1), {SelectedRole});
        emit selectionChanged();
    }
    return changed;
}

bool ImageSession::toggleSelectionById(const QString& id) {
    return toggleSelection(QUuid(id));
}

void ImageSession::clearSelection() {
    const bool changed = replaceSelection({});
    Q_UNUSED(changed)
}

void ImageSession::selectAll() {
    if (images_.empty() || selectedCount() == count()) {
        return;
    }
    for (ImageEntry& image : images_) {
        image.selected = true;
    }
    emit dataChanged(index(0), index(count() - 1), {SelectedRole});
    emit selectionChanged();
}

bool ImageSession::setRotation(const ImageId& id, int degrees) {
    if (degrees % 90 != 0) {
        return false;
    }
    const int imageIndex = indexOf(id);
    if (imageIndex < 0) {
        return false;
    }
    const int normalized = ((degrees % 360) + 360) % 360;
    ImageEntry& image = images_[static_cast<std::size_t>(imageIndex)];
    if (image.rotationDegrees == normalized) {
        return true;
    }
    image.rotationDegrees = normalized;
    emit dataChanged(index(imageIndex), index(imageIndex), {RotationRole});
    return true;
}

std::optional<ImageEntry> ImageSession::validateImage(const QString& path, QString* error) {
    const QFileInfo file(path);
    if (!file.exists() || !file.isFile() || !file.isReadable()) {
        if (error != nullptr) {
            *error = QStringLiteral("Não foi possível acessar a imagem: %1").arg(path);
        }
        return std::nullopt;
    }
    if (!isSupportedImageFile(path)) {
        if (error != nullptr) {
            *error = QStringLiteral("Formato de imagem não suportado: %1").arg(file.fileName());
        }
        return std::nullopt;
    }

    QImageReader reader(file.absoluteFilePath());
    reader.setAutoTransform(true);
    if (!reader.canRead() || !reader.size().isValid()) {
        if (error != nullptr) {
            *error = QStringLiteral("Não foi possível abrir a imagem: %1").arg(file.fileName());
        }
        return std::nullopt;
    }
    if (!isImageSizeWithinLimits(reader.size())) {
        if (error != nullptr) {
            *error = QStringLiteral("A imagem é grande demais para ser aberta com segurança: %1")
                         .arg(file.fileName());
        }
        return std::nullopt;
    }

    return ImageEntry{.id = QUuid::createUuid(),
                      .sourcePath = file.absoluteFilePath(),
                      .fileName = file.fileName(),
                      .pixelSize = reader.size()};
}

int ImageSession::indexOf(const ImageId& id) const {
    const auto iterator = std::find_if(images_.cbegin(), images_.cend(),
                                       [&id](const ImageEntry& image) { return image.id == id; });
    return iterator == images_.cend() ? -1
                                      : static_cast<int>(std::distance(images_.cbegin(), iterator));
}

bool ImageSession::setSelected(const ImageId& id, bool selected) {
    const int imageIndex = indexOf(id);
    if (imageIndex < 0) {
        return false;
    }
    ImageEntry& image = images_[static_cast<std::size_t>(imageIndex)];
    if (image.selected == selected) {
        return true;
    }
    image.selected = selected;
    emit dataChanged(index(imageIndex), index(imageIndex), {SelectedRole});
    emit selectionChanged();
    return true;
}

void ImageSession::emitCurrentRowsChanged(int previousIndex, int nextIndex) {
    if (previousIndex >= 0 && previousIndex < count()) {
        emit dataChanged(index(previousIndex), index(previousIndex), {CurrentRole});
    }
    if (nextIndex >= 0 && nextIndex < count()) {
        emit dataChanged(index(nextIndex), index(nextIndex), {CurrentRole});
    }
}

void ImageSession::recomputeSourceFolder() {
    QString nextFolder;
    if (!images_.empty()) {
        nextFolder = QFileInfo(images_.front().sourcePath).absolutePath();
        const bool sameFolder =
            std::all_of(images_.cbegin(), images_.cend(), [&nextFolder](const ImageEntry& image) {
                return QFileInfo(image.sourcePath).absolutePath() == nextFolder;
            });
        if (!sameFolder) {
            nextFolder.clear();
        }
    }
    if (nextFolder != sourceFolderPath_) {
        sourceFolderPath_ = nextFolder;
        emit sourceFolderChanged();
    }
}

} // namespace purrview::core
