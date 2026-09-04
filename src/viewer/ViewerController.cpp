#include "viewer/ViewerController.h"

#include "core/image/ImageFormatSupport.h"

#include <QAbstractItemModel>
#include <QClipboard>
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QHash>
#include <QImageReader>
#include <QLoggingCategory>
#include <QSet>

#include <algorithm>

Q_LOGGING_CATEGORY(logViewerStartup, "purrview.viewer.startup")

namespace purrview::viewer {

ViewerController::ViewerController(core::ImageSession& imageSession,
                                   core::ThumbnailCache& thumbnailCache,
                                   core::ImageMetadataService& metadataService, QObject* parent)
    : QObject(parent), imageSession_(imageSession), metadataService_(metadataService),
      folderModel_(imageSession, thumbnailCache) {
    startupTimer_.start();
    trashFunction_ = [](const QString& path, QString* pathInTrash) {
        return QFile::moveToTrash(path, pathInTrash);
    };
    connect(&imageSession_, &core::ImageSession::currentImageChanged, this,
            &ViewerController::handleCurrentImageChanged);
    connect(&imageSession_, &core::ImageSession::countChanged, this, [this] {
        emit imageCountChanged();
        emit navigationChanged();
    });
    connect(&imageSession_, &core::ImageSession::selectionChanged, this,
            &ViewerController::selectionChanged);
    connect(&imageSession_, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex& first, const QModelIndex& last, const QList<int>& roles) {
                const int sessionCurrentIndex = imageSession_.currentIndex();
                if (sessionCurrentIndex >= first.row() && sessionCurrentIndex <= last.row() &&
                    (roles.isEmpty() || roles.contains(core::ImageSession::RotationRole))) {
                    emit rotationChanged();
                }
            });
    connect(&folderModel_, &core::FolderImageModel::countChanged, this, [this] {
        emit imageCountChanged();
        emit navigationChanged();
    });
    connect(&folderModel_, &core::FolderImageModel::currentIndexChanged, this, [this] {
        emit currentImageChanged();
        emit navigationChanged();
    });
    connect(&folderModel_, &core::FolderImageModel::scanCompleted, this,
            &ViewerController::handleFolderScanCompleted);
    connect(
        &metadataService_, &core::ImageMetadataService::metadataReady, this,
        [this](quint64 requestId, const QString& path, const core::ImageMetadata& metadata, bool) {
            const core::ImageEntry* current = imageSession_.currentImage();
            if (requestId != activeMetadataRequestId_ || path != activeMetadataPath_ ||
                current == nullptr || current->sourcePath != path) {
                return;
            }
            metadataModel_.setMetadata(metadata);
        });
    connect(&metadataService_, &core::ImageMetadataService::metadataFailed, this,
            [this](quint64 requestId, const QString& path, const QString& error) {
                const core::ImageEntry* current = imageSession_.currentImage();
                if (requestId != activeMetadataRequestId_ || path != activeMetadataPath_ ||
                    current == nullptr || current->sourcePath != path) {
                    return;
                }
                core::ImageMetadata metadata = metadataModel_.value();
                metadata.warning = error;
                metadataModel_.setMetadata(std::move(metadata));
            });
    ensureCatalogForCurrentImage();
    loadCurrentMetadata();
}

core::ImageSession* ViewerController::imageSession() {
    return &imageSession_;
}

ViewerState* ViewerController::state() {
    return &state_;
}

core::FolderImageModel* ViewerController::folderModel() {
    return &folderModel_;
}

core::ImageMetadataModel* ViewerController::metadata() {
    return &metadataModel_;
}

QUrl ViewerController::currentImageUrl() const {
    return imageSession_.currentImageSource();
}

QString ViewerController::currentFileName() const {
    const core::ImageEntry* image = imageSession_.currentImage();
    return image == nullptr ? QString() : image->fileName;
}

QString ViewerController::currentFilePath() const {
    const core::ImageEntry* image = imageSession_.currentImage();
    return image == nullptr ? QString() : image->sourcePath;
}

bool ViewerController::currentImageSelected() const {
    const core::ImageEntry* image = imageSession_.currentImage();
    return image != nullptr && image->selected;
}

int ViewerController::currentPixelWidth() const {
    const core::ImageEntry* image = imageSession_.currentImage();
    return image == nullptr ? 0 : image->pixelSize.width();
}

int ViewerController::currentPixelHeight() const {
    const core::ImageEntry* image = imageSession_.currentImage();
    return image == nullptr ? 0 : image->pixelSize.height();
}

int ViewerController::currentIndex() const {
    const int folderIndex = folderModel_.currentIndex();
    return folderIndex >= 0 ? folderIndex : imageSession_.currentIndex();
}

int ViewerController::imageCount() const {
    return folderModel_.currentIndex() >= 0 ? folderModel_.count() : imageSession_.count();
}

int ViewerController::rotation() const {
    const core::ImageEntry* image = imageSession_.currentImage();
    return image == nullptr ? 0 : image->rotationDegrees;
}

bool ViewerController::canGoPrevious() const {
    return currentIndex() > 0;
}

bool ViewerController::canGoNext() const {
    return currentIndex() >= 0 && currentIndex() + 1 < imageCount();
}

QUrl ViewerController::previousImageUrl() const {
    const int folderIndex = folderModel_.currentIndex();
    return folderIndex > 0 ? folderModel_.sourceAt(folderIndex - 1) : QUrl();
}

QUrl ViewerController::nextImageUrl() const {
    const int folderIndex = folderModel_.currentIndex();
    return folderIndex >= 0 && folderIndex + 1 < folderModel_.count()
               ? folderModel_.sourceAt(folderIndex + 1)
               : QUrl();
}

int ViewerController::selectedImageCount() const {
    return imageSession_.selectedCount();
}

QString ViewerController::printActionText() const {
    return QStringLiteral("Imprimir");
}

QString ViewerController::printAccessibleName() const {
    return selectedImageCount() > 1
               ? QStringLiteral("Imprimir %1 imagens com PurrView").arg(selectedImageCount())
               : QStringLiteral("Imprimir imagem atual com PurrView");
}

double ViewerController::savedPanX() const {
    return navigationState_.panOffset.x();
}

double ViewerController::savedPanY() const {
    return navigationState_.panOffset.y();
}

double ViewerController::savedFilmstripContentX() const {
    return navigationState_.filmstripContentX;
}

QList<core::ImageId> ViewerController::printCandidateImages() const {
    if (imageSession_.selectedCount() == 0) {
        const core::ImageEntry* current = imageSession_.currentImage();
        return current == nullptr ? QList<core::ImageId>{} : QList<core::ImageId>{current->id};
    }

    QList<core::ImageId> ordered;
    QSet<core::ImageId> seen;
    QHash<QString, core::ImageId> selectedByPath;
    selectedByPath.reserve(imageSession_.selectedCount());
    for (const core::ImageEntry& image : imageSession_.images()) {
        if (image.selected && !selectedByPath.contains(image.sourcePath)) {
            selectedByPath.insert(image.sourcePath, image.id);
        }
    }
    for (const QString& path : folderModel_.selectedPaths()) {
        const auto selected = selectedByPath.constFind(path);
        if (selected != selectedByPath.cend() && !seen.contains(selected.value())) {
            ordered.push_back(selected.value());
            seen.insert(selected.value());
        }
    }
    for (const core::ImageEntry& image : imageSession_.images()) {
        if (image.selected && !seen.contains(image.id)) {
            ordered.push_back(image.id);
            seen.insert(image.id);
        }
    }
    return ordered;
}

const ViewerNavigationState& ViewerController::navigationState() const {
    return navigationState_;
}

void ViewerController::openImages(const QVariantList& urls) {
    QStringList errors;
    std::optional<core::ImageId> firstImage;
    for (const QVariant& value : urls) {
        const QUrl url = value.toUrl();
        if (url.isLocalFile()) {
            QString error;
            std::optional<core::ImageId> id = imageSession_.addImage(url.toLocalFile(), &error);
            if (!id.has_value() && core::isSupportedImageFile(url.toLocalFile())) {
                id = imageSession_.addImageReference(url.toLocalFile(), &error);
            }
            if (id.has_value() && !firstImage.has_value()) {
                firstImage = id;
            }
            if (!error.isEmpty()) {
                errors.push_back(error);
            }
        } else {
            errors.push_back(
                QStringLiteral("Apenas imagens armazenadas neste computador são aceitas."));
        }
    }

    if (firstImage.has_value()) {
        if (imageSession_.setCurrentImage(*firstImage)) {
            state_.clearError();
            ensureCatalogForCurrentImage();
        }
    } else if (!errors.isEmpty()) {
        state_.setError(errors.constLast());
    }
}

void ViewerController::previousImage() {
    if (folderModel_.currentIndex() >= 0) {
        activateFolderIndex(folderModel_.currentIndex() - 1);
    } else if (canGoPrevious()) {
        imageSession_.setCurrentIndex(currentIndex() - 1);
    }
}

void ViewerController::nextImage() {
    if (folderModel_.currentIndex() >= 0) {
        activateFolderIndex(folderModel_.currentIndex() + 1);
    } else if (canGoNext()) {
        imageSession_.setCurrentIndex(currentIndex() + 1);
    }
}

void ViewerController::activateFolderIndex(int index) {
    const QString path = folderModel_.pathAt(index);
    if (path.isEmpty()) {
        return;
    }
    QString error;
    const std::optional<core::ImageId> imageId = ensureSessionImage(path, &error);
    if (!imageId.has_value()) {
        state_.setError(error);
        return;
    }
    selectionAnchor_ = index;
    if (imageSession_.setCurrentImage(*imageId)) {
        state_.clearError();
    }
}

void ViewerController::toggleFolderSelection(int index) {
    const QString path = folderModel_.pathAt(index);
    const std::optional<core::ImageId> imageId = ensureSessionImage(path);
    if (imageId.has_value()) {
        const bool selectionChanged = imageSession_.toggleSelection(*imageId);
        Q_UNUSED(selectionChanged)
        selectionAnchor_ = index;
    }
}

void ViewerController::selectFolderRange(int index) {
    if (index < 0 || index >= folderModel_.count()) {
        return;
    }
    const int anchor =
        selectionAnchor_ >= 0 ? selectionAnchor_ : std::max(0, folderModel_.currentIndex());
    const int first = std::min(anchor, index);
    const int last = std::max(anchor, index);
    QHash<QString, core::ImageId> idsByPath;
    idsByPath.reserve(imageSession_.count() + last - first + 1);
    for (const core::ImageEntry& image : imageSession_.images()) {
        if (!idsByPath.contains(image.sourcePath)) {
            idsByPath.insert(image.sourcePath, image.id);
        }
    }
    QStringList missingPaths;
    for (int itemIndex = first; itemIndex <= last; ++itemIndex) {
        const QString path = folderModel_.pathAt(itemIndex);
        if (!idsByPath.contains(path)) {
            missingPaths.push_back(path);
        }
    }
    const QList<core::ImageId> addedReferences =
        imageSession_.addImageReferences(missingPaths, false);
    for (qsizetype addedIndex = 0; addedIndex < addedReferences.size(); ++addedIndex) {
        const QString& path = missingPaths.at(addedIndex);
        if (!idsByPath.contains(path)) {
            idsByPath.insert(path, addedReferences.at(addedIndex));
        }
    }

    QList<core::ImageId> rangeIds;
    rangeIds.reserve(last - first + 1);
    for (int itemIndex = first; itemIndex <= last; ++itemIndex) {
        const auto imageId = idsByPath.constFind(folderModel_.pathAt(itemIndex));
        if (imageId != idsByPath.cend()) {
            rangeIds.push_back(imageId.value());
        }
    }
    const bool rangeChanged = imageSession_.selectImages(rangeIds);
    Q_UNUSED(rangeChanged)
    selectionAnchor_ = anchor;
}

void ViewerController::selectAllFolderImages() {
    QList<core::ImageId> imageIds;
    if (folderModel_.count() > 0) {
        QHash<QString, core::ImageId> idsByPath;
        idsByPath.reserve(imageSession_.count() + folderModel_.count());
        for (const core::ImageEntry& image : imageSession_.images()) {
            if (!idsByPath.contains(image.sourcePath)) {
                idsByPath.insert(image.sourcePath, image.id);
            }
        }
        QStringList missingPaths;
        for (int index = 0; index < folderModel_.count(); ++index) {
            const QString path = folderModel_.pathAt(index);
            if (!idsByPath.contains(path)) {
                missingPaths.push_back(path);
            }
        }
        const QList<core::ImageId> addedReferences =
            imageSession_.addImageReferences(missingPaths, false);
        for (qsizetype addedIndex = 0; addedIndex < addedReferences.size(); ++addedIndex) {
            const QString& path = missingPaths.at(addedIndex);
            if (!idsByPath.contains(path)) {
                idsByPath.insert(path, addedReferences.at(addedIndex));
            }
        }
        imageIds.reserve(folderModel_.count());
        for (int index = 0; index < folderModel_.count(); ++index) {
            const auto imageId = idsByPath.constFind(folderModel_.pathAt(index));
            if (imageId != idsByPath.cend()) {
                imageIds.push_back(imageId.value());
            }
        }
        selectionAnchor_ = std::max(0, folderModel_.currentIndex());
    } else {
        imageIds.reserve(imageSession_.count());
        for (const core::ImageEntry& image : imageSession_.images()) {
            imageIds.push_back(image.id);
        }
    }
    const bool selectionChanged = imageSession_.replaceSelection(imageIds);
    Q_UNUSED(selectionChanged)
}

void ViewerController::clearSelection() {
    imageSession_.clearSelection();
}

void ViewerController::clearError() {
    state_.clearError();
}

void ViewerController::dismissTransientState() {
    if (state_.infoPanelVisible()) {
        state_.setInfoPanelVisible(false);
    } else if (selectedImageCount() > 0) {
        clearSelection();
    }
}

void ViewerController::zoomIn() {
    state_.zoomIn();
}

void ViewerController::zoomOut() {
    state_.zoomOut();
}

void ViewerController::fitToWindow() {
    state_.fitToWindow();
}

void ViewerController::actualSize() {
    state_.actualSize();
}

void ViewerController::setCustomZoom(double scale) {
    state_.setCustomZoom(scale);
}

void ViewerController::updateFitScale(double scale) {
    state_.updateFitScale(scale);
}

void ViewerController::rotateLeft() {
    const core::ImageEntry* image = imageSession_.currentImage();
    if (image != nullptr && imageSession_.setRotation(image->id, rotation() - 90)) {
        state_.requestPanReset();
    }
}

void ViewerController::rotateRight() {
    const core::ImageEntry* image = imageSession_.currentImage();
    if (image != nullptr && imageSession_.setRotation(image->id, rotation() + 90)) {
        state_.requestPanReset();
    }
}

void ViewerController::openComposer() {
    emit captureVisualStateRequested();
    captureNavigationState();

    QList<core::ImageId> validCandidates;
    int ignoredCandidates = 0;
    for (const core::ImageId& imageId : printCandidateImages()) {
        const core::ImageEntry* image = imageForId(imageId);
        if (image == nullptr || !image->valid) {
            ++ignoredCandidates;
            continue;
        }
        const QFileInfo file(image->sourcePath);
        QImageReader reader(file.absoluteFilePath());
        if (!file.exists() || !file.isReadable() ||
            !core::isSupportedImageFile(image->sourcePath) || !reader.canRead()) {
            ++ignoredCandidates;
            continue;
        }
        validCandidates.push_back(imageId);
    }

    if (validCandidates.isEmpty()) {
        state_.setError(QStringLiteral("Nenhuma imagem válida está disponível para impressão."));
        return;
    }
    if (ignoredCandidates > 0) {
        state_.setError(ignoredCandidates == 1
                            ? QStringLiteral("Uma imagem indisponível será ignorada.")
                            : QStringLiteral("%1 imagens indisponíveis serão ignoradas.")
                                  .arg(ignoredCandidates));
    } else {
        state_.clearError();
    }
    emit composerActivationRequested(
        {.imageIds = validCandidates, .source = core::ActivationSource::Viewer});
}

void ViewerController::toggleFilmstrip() {
    state_.toggleFilmstrip();
}

void ViewerController::toggleInfoPanel() {
    state_.toggleInfoPanel();
}

void ViewerController::openCurrentFolder() {
    const core::ImageEntry* current = imageSession_.currentImage();
    if (current == nullptr) {
        return;
    }
    if (!QDesktopServices::openUrl(
            QUrl::fromLocalFile(QFileInfo(current->sourcePath).absolutePath()))) {
        state_.setError(QStringLiteral("Não foi possível abrir a localização da imagem."));
    }
}

void ViewerController::copyCurrentPath() {
    const QString path = currentFilePath();
    if (path.isEmpty()) {
        return;
    }
    QGuiApplication::clipboard()->setText(path, QClipboard::Clipboard);
    emit noticeRequested(QStringLiteral("Caminho copiado."));
}

void ViewerController::trashCurrentImage() {
    const core::ImageEntry* current = imageSession_.currentImage();
    if (current == nullptr) {
        return;
    }

    const QString sourcePath = current->sourcePath;
    const QString fileName = current->fileName;
    const int folderIndex = folderModel_.currentIndex();
    QString replacementPath;
    if (folderModel_.count() > 1 && folderIndex >= 0) {
        const int replacementIndex =
            folderIndex + 1 < folderModel_.count() ? folderIndex + 1 : folderIndex - 1;
        replacementPath = folderModel_.pathAt(replacementIndex);
    }

    QString pathInTrash;
    metadataService_.invalidateFile(sourcePath);
    if (!trashFunction_ || !trashFunction_(sourcePath, &pathInTrash)) {
        state_.setError(
            QStringLiteral("Não foi possível enviar “%1” para a lixeira.").arg(fileName));
        return;
    }

    QList<core::ImageId> removedIds;
    for (const core::ImageEntry& image : imageSession_.images()) {
        if (image.sourcePath == sourcePath) {
            removedIds.push_back(image.id);
        }
    }
    if (!replacementPath.isEmpty()) {
        if (const auto replacement = ensureSessionImage(replacementPath); replacement.has_value()) {
            (void)imageSession_.setCurrentImage(*replacement);
        }
    }
    for (const core::ImageId& id : removedIds) {
        (void)imageSession_.removeImage(id);
    }
    folderModel_.refresh();
    state_.clearError();
    emit noticeRequested(QStringLiteral("“%1” foi enviada para a lixeira.").arg(fileName));
}

void ViewerController::reportCurrentImageVisible() {
    if (firstImageVisibleReported_) {
        return;
    }
    firstImageVisibleReported_ = true;
    qCInfo(logViewerStartup) << "First image visible after" << startupTimer_.elapsed()
                             << "ms from Viewer construction";
}

void ViewerController::setTrashFunctionForTesting(TrashFunction function) {
    trashFunction_ = std::move(function);
}

void ViewerController::captureViewState(double panX, double panY, double filmstripContentX) {
    navigationState_.panOffset = QPointF(panX, panY);
    navigationState_.filmstripContentX = std::max(0.0, filmstripContentX);
    emit navigationSnapshotChanged();
}

void ViewerController::captureNavigationState() {
    const core::ImageEntry* current = imageSession_.currentImage();
    navigationState_.currentImageId = current == nullptr ? core::ImageId{} : current->id;
    navigationState_.selectedImageIds.clear();
    for (const core::ImageEntry& image : imageSession_.images()) {
        if (image.selected) {
            navigationState_.selectedImageIds.push_back(image.id);
        }
    }
    navigationState_.zoomFactor = state_.zoomFactor();
    navigationState_.zoomMode = static_cast<int>(state_.zoomMode());
    navigationState_.rotationDegrees = rotation();
    navigationState_.infoPanelVisible = state_.infoPanelVisible();
    navigationState_.filmstripVisible = state_.filmstripVisible();
    navigationState_.filmstripCurrentIndex = folderModel_.currentIndex();
    emit navigationSnapshotChanged();
}

void ViewerController::restoreNavigationState() {
    const bool selectionChanged = imageSession_.replaceSelection(navigationState_.selectedImageIds);
    Q_UNUSED(selectionChanged)
    if (!navigationState_.currentImageId.isNull()) {
        const bool currentRestored = imageSession_.setCurrentImage(navigationState_.currentImageId);
        const bool rotationRestored = imageSession_.setRotation(navigationState_.currentImageId,
                                                                navigationState_.rotationDegrees);
        Q_UNUSED(currentRestored)
        Q_UNUSED(rotationRestored)
    }
    state_.setFilmstripVisible(navigationState_.filmstripVisible);
    state_.setInfoPanelVisible(navigationState_.infoPanelVisible);
    state_.restoreZoom(static_cast<ViewerState::ZoomMode>(navigationState_.zoomMode),
                       navigationState_.zoomFactor);
    emit navigationSnapshotChanged();
}

void ViewerController::handleCurrentImageChanged() {
    state_.resetForImage();
    ensureCatalogForCurrentImage();
    loadCurrentMetadata();
    emit currentImageChanged();
    emit rotationChanged();
    emit navigationChanged();
}

void ViewerController::loadCurrentMetadata() {
    const core::ImageEntry* current = imageSession_.currentImage();
    if (current == nullptr) {
        activeMetadataPath_.clear();
        ++activeMetadataRequestId_;
        metadataModel_.clear();
        return;
    }

    activeMetadataPath_ = QFileInfo(current->sourcePath).absoluteFilePath();
    metadataModel_.setBasicMetadata(
        core::ImageMetadataService::basicMetadata(activeMetadataPath_, current->pixelSize, false));
    metadataModel_.setLoading(true);
    activeMetadataRequestId_ = metadataService_.requestMetadata(activeMetadataPath_);
}

void ViewerController::ensureCatalogForCurrentImage() {
    const core::ImageEntry* current = imageSession_.currentImage();
    if (current == nullptr) {
        return;
    }
    const QString currentDirectory = QFileInfo(current->sourcePath).absolutePath();
    if (folderModel_.directoryUrl().toLocalFile() != currentDirectory) {
        folderModel_.openFromImage(current->sourcePath);
    }
}

std::optional<core::ImageId> ViewerController::sessionImageForPath(const QString& path) const {
    const QString absolutePath = QFileInfo(path).absoluteFilePath();
    const auto found = std::find_if(imageSession_.images().cbegin(), imageSession_.images().cend(),
                                    [&absolutePath](const core::ImageEntry& image) {
                                        return image.sourcePath == absolutePath;
                                    });
    return found == imageSession_.images().cend() ? std::nullopt
                                                  : std::optional<core::ImageId>(found->id);
}

std::optional<core::ImageId> ViewerController::ensureSessionImage(const QString& path,
                                                                  QString* error) {
    if (path.isEmpty()) {
        return std::nullopt;
    }
    const std::optional<core::ImageId> existing = sessionImageForPath(path);
    if (existing.has_value()) {
        return existing;
    }
    std::optional<core::ImageId> added = imageSession_.addImage(path, error);
    if (!added.has_value() && core::isSupportedImageFile(path)) {
        added = imageSession_.addImageReference(path, error);
    }
    return added;
}

const core::ImageEntry* ViewerController::imageForId(const core::ImageId& id) const {
    const auto image =
        std::find_if(imageSession_.images().cbegin(), imageSession_.images().cend(),
                     [&id](const core::ImageEntry& candidate) { return candidate.id == id; });
    return image == imageSession_.images().cend() ? nullptr : &*image;
}

void ViewerController::handleFolderScanCompleted(const QString& requestedPath, bool currentPresent,
                                                 int previousIndex) {
    if (currentPresent) {
        emit imageCountChanged();
        emit navigationChanged();
        return;
    }

    const core::ImageEntry* current = imageSession_.currentImage();
    if (current == nullptr || current->sourcePath != QFileInfo(requestedPath).absoluteFilePath()) {
        return;
    }
    const core::ImageId removedId = current->id;
    if (folderModel_.count() > 0) {
        activateFolderIndex(std::clamp(previousIndex, 0, folderModel_.count() - 1));
    }
    const bool removed = imageSession_.removeImage(removedId);
    Q_UNUSED(removed)
}

} // namespace purrview::viewer
