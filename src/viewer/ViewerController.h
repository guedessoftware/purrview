#pragma once

#include "core/image/FolderImageModel.h"
#include "core/image/ImageMetadataModel.h"
#include "core/image/ImageMetadataService.h"
#include "core/image/ImageSession.h"
#include "core/image/ThumbnailCache.h"
#include "core/navigation/ComposerActivationContext.h"
#include "viewer/ViewerNavigationState.h"
#include "viewer/ViewerState.h"

#include <QElapsedTimer>
#include <QObject>
#include <QUrl>
#include <QVariantList>

#include <functional>

namespace purrview::viewer {

class ViewerController : public QObject {
    Q_OBJECT
    Q_PROPERTY(purrview::core::ImageSession* imageSession READ imageSession CONSTANT)
    Q_PROPERTY(purrview::viewer::ViewerState* state READ state CONSTANT)
    Q_PROPERTY(purrview::core::FolderImageModel* folderModel READ folderModel CONSTANT)
    Q_PROPERTY(purrview::core::ImageMetadataModel* metadata READ metadata CONSTANT)
    Q_PROPERTY(QUrl currentImageUrl READ currentImageUrl NOTIFY currentImageChanged)
    Q_PROPERTY(QString currentFileName READ currentFileName NOTIFY currentImageChanged)
    Q_PROPERTY(QString currentFilePath READ currentFilePath NOTIFY currentImageChanged)
    Q_PROPERTY(bool currentImageSelected READ currentImageSelected NOTIFY selectionChanged)
    Q_PROPERTY(int currentPixelWidth READ currentPixelWidth NOTIFY currentImageChanged)
    Q_PROPERTY(int currentPixelHeight READ currentPixelHeight NOTIFY currentImageChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentImageChanged)
    Q_PROPERTY(int imageCount READ imageCount NOTIFY imageCountChanged)
    Q_PROPERTY(int rotation READ rotation NOTIFY rotationChanged)
    Q_PROPERTY(bool canGoPrevious READ canGoPrevious NOTIFY navigationChanged)
    Q_PROPERTY(bool canGoNext READ canGoNext NOTIFY navigationChanged)
    Q_PROPERTY(QUrl previousImageUrl READ previousImageUrl NOTIFY navigationChanged)
    Q_PROPERTY(QUrl nextImageUrl READ nextImageUrl NOTIFY navigationChanged)
    Q_PROPERTY(int selectedImageCount READ selectedImageCount NOTIFY selectionChanged)
    Q_PROPERTY(QString printActionText READ printActionText NOTIFY selectionChanged)
    Q_PROPERTY(QString printAccessibleName READ printAccessibleName NOTIFY selectionChanged)
    Q_PROPERTY(double savedPanX READ savedPanX NOTIFY navigationSnapshotChanged)
    Q_PROPERTY(double savedPanY READ savedPanY NOTIFY navigationSnapshotChanged)
    Q_PROPERTY(
        double savedFilmstripContentX READ savedFilmstripContentX NOTIFY navigationSnapshotChanged)

  public:
    explicit ViewerController(core::ImageSession& imageSession,
                              core::ThumbnailCache& thumbnailCache,
                              core::ImageMetadataService& metadataService,
                              QObject* parent = nullptr);

    [[nodiscard]] core::ImageSession* imageSession();
    [[nodiscard]] ViewerState* state();
    [[nodiscard]] core::FolderImageModel* folderModel();
    [[nodiscard]] core::ImageMetadataModel* metadata();
    [[nodiscard]] QUrl currentImageUrl() const;
    [[nodiscard]] QString currentFileName() const;
    [[nodiscard]] QString currentFilePath() const;
    [[nodiscard]] bool currentImageSelected() const;
    [[nodiscard]] int currentPixelWidth() const;
    [[nodiscard]] int currentPixelHeight() const;
    [[nodiscard]] int currentIndex() const;
    [[nodiscard]] int imageCount() const;
    [[nodiscard]] int rotation() const;
    [[nodiscard]] bool canGoPrevious() const;
    [[nodiscard]] bool canGoNext() const;
    [[nodiscard]] QUrl previousImageUrl() const;
    [[nodiscard]] QUrl nextImageUrl() const;
    [[nodiscard]] int selectedImageCount() const;
    [[nodiscard]] QString printActionText() const;
    [[nodiscard]] QString printAccessibleName() const;
    [[nodiscard]] double savedPanX() const;
    [[nodiscard]] double savedPanY() const;
    [[nodiscard]] double savedFilmstripContentX() const;
    [[nodiscard]] QList<core::ImageId> printCandidateImages() const;
    [[nodiscard]] const ViewerNavigationState& navigationState() const;

    Q_INVOKABLE void openImages(const QVariantList& urls);
    Q_INVOKABLE void previousImage();
    Q_INVOKABLE void nextImage();
    Q_INVOKABLE void activateFolderIndex(int index);
    Q_INVOKABLE void toggleFolderSelection(int index);
    Q_INVOKABLE void selectFolderRange(int index);
    Q_INVOKABLE void selectAllFolderImages();
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void clearError();
    Q_INVOKABLE void dismissTransientState();
    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();
    Q_INVOKABLE void fitToWindow();
    Q_INVOKABLE void actualSize();
    Q_INVOKABLE void setCustomZoom(double scale);
    Q_INVOKABLE void updateFitScale(double scale);
    Q_INVOKABLE void rotateLeft();
    Q_INVOKABLE void rotateRight();
    Q_INVOKABLE void openComposer();
    Q_INVOKABLE void toggleFilmstrip();
    Q_INVOKABLE void toggleInfoPanel();
    Q_INVOKABLE void openCurrentFolder();
    Q_INVOKABLE void copyCurrentPath();
    Q_INVOKABLE void trashCurrentImage();
    Q_INVOKABLE void reportCurrentImageVisible();
    Q_INVOKABLE void captureViewState(double panX, double panY, double filmstripContentX);

    void captureNavigationState();
    void restoreNavigationState();

    using TrashFunction = std::function<bool(const QString&, QString*)>;
    void setTrashFunctionForTesting(TrashFunction function);

  signals:
    void currentImageChanged();
    void imageCountChanged();
    void rotationChanged();
    void navigationChanged();
    void selectionChanged();
    void navigationSnapshotChanged();
    void captureVisualStateRequested();
    void composerActivationRequested(const purrview::core::ComposerActivationContext& context);
    void noticeRequested(const QString& message);

  private:
    void handleCurrentImageChanged();
    void ensureCatalogForCurrentImage();
    void loadCurrentMetadata();
    [[nodiscard]] std::optional<core::ImageId> sessionImageForPath(const QString& path) const;
    [[nodiscard]] std::optional<core::ImageId> ensureSessionImage(const QString& path,
                                                                  QString* error = nullptr);
    [[nodiscard]] const core::ImageEntry* imageForId(const core::ImageId& id) const;
    void handleFolderScanCompleted(const QString& requestedPath, bool currentPresent,
                                   int previousIndex);

    core::ImageSession& imageSession_;
    core::ImageMetadataService& metadataService_;
    ViewerState state_;
    core::FolderImageModel folderModel_;
    core::ImageMetadataModel metadataModel_;
    quint64 activeMetadataRequestId_ = 0;
    QString activeMetadataPath_;
    int selectionAnchor_ = -1;
    ViewerNavigationState navigationState_;
    TrashFunction trashFunction_;
    bool firstImageVisibleReported_ = false;
    QElapsedTimer startupTimer_;
};

} // namespace purrview::viewer
