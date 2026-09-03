#include "viewer/ViewerController.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>
#include <QThread>

#include <cmath>
#include <iostream>

namespace {

int failures = 0;

void check(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

QString createImage(const QTemporaryDir& directory, const QString& name, const QSize& size,
                    const QColor& color) {
    const QString path = directory.filePath(name);
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(color);
    check(image.save(path), "viewer image fixture can be saved");
    return path;
}

QString pathForId(const impage::core::ImageSession& session, const impage::core::ImageId& imageId) {
    const auto image = std::find_if(
        session.images().cbegin(), session.images().cend(),
        [&imageId](const impage::core::ImageEntry& candidate) { return candidate.id == imageId; });
    return image == session.images().cend() ? QString() : image->sourcePath;
}

} // namespace

int main(int argc, char* argv[]) {
    QCoreApplication application(argc, argv);
    impage::core::ImageSession session;
    impage::core::ThumbnailCache thumbnailCache;
    impage::core::ImageMetadataService metadataService;
    impage::viewer::ViewerController controller(session, thumbnailCache, metadataService);
    controller.folderModel()->setWatchingEnabled(false);

    check(controller.imageCount() == 0, "viewer starts with an empty session");
    check(controller.currentIndex() == -1, "empty viewer has no current index");
    check(controller.currentImageUrl().isEmpty(), "empty viewer has no current URL");
    check(!controller.canGoPrevious() && !controller.canGoNext(),
          "empty viewer disables navigation");

    check(!controller.state()->toolbarPinned(), "floating toolbar starts in adaptive opacity mode");
    controller.state()->toggleToolbarPinned();
    check(controller.state()->toolbarPinned(), "floating toolbar can be pinned at active opacity");
    controller.state()->setToolbarPinned(false);

    controller.state()->setInactivityIntervalForTesting(15);
    controller.state()->setFullScreen(true);
    check(controller.state()->fullScreen() && controller.state()->controlsVisible() &&
              controller.state()->cursorVisible(),
          "fullscreen starts with controls and cursor visible");
    QElapsedTimer inactivityDeadline;
    inactivityDeadline.start();
    while (controller.state()->controlsVisible() && inactivityDeadline.elapsed() < 200) {
        application.processEvents(QEventLoop::AllEvents, 10);
        QThread::msleep(2);
    }
    check(!controller.state()->controlsVisible() && !controller.state()->cursorVisible(),
          "fullscreen inactivity hides controls and cursor from one central timer");
    controller.state()->notifyActivity();
    check(controller.state()->controlsVisible() && controller.state()->cursorVisible(),
          "new activity immediately restores controls and cursor");
    controller.state()->setInteractionBlocked(true);
    QThread::msleep(25);
    application.processEvents();
    check(controller.state()->controlsVisible(),
          "active interaction prevents fullscreen auto-hide");
    controller.state()->setInteractionBlocked(false);
    controller.state()->setFullScreen(false);

    QTemporaryDir directory;
    check(directory.isValid(), "temporary viewer fixture directory is available");
    const QString firstPath =
        createImage(directory, QStringLiteral("landscape.jpg"), QSize(160, 90), Qt::red);
    const QString secondPath =
        createImage(directory, QStringLiteral("portrait.png"), QSize(90, 160), Qt::green);
    const QList<impage::core::ImageId> ids = session.addImages({firstPath, secondPath});
    check(controller.folderModel()->scanFromImageSynchronously(firstPath),
          "viewer discovers the current image folder");

    check(controller.imageCount() == 2, "viewer observes session image count");
    check(controller.currentIndex() == 0, "viewer derives current index from session");
    check(controller.currentImageUrl() == QUrl::fromLocalFile(firstPath),
          "viewer exposes the current session image");
    check(controller.currentPixelWidth() == 160 && controller.currentPixelHeight() == 90,
          "viewer exposes source pixel dimensions for deterministic fit");
    check(!controller.canGoPrevious() && controller.canGoNext(),
          "first image exposes correct navigation limits");

    controller.nextImage();
    check(session.currentIndex() == 1 && controller.currentIndex() == 1,
          "next image updates the shared session");
    check(controller.currentImageUrl() == QUrl::fromLocalFile(secondPath),
          "next image becomes visible");
    check(controller.canGoPrevious() && !controller.canGoNext(),
          "last image exposes correct navigation limits");
    controller.nextImage();
    check(controller.currentIndex() == 1, "next image does not wrap at the end");
    controller.previousImage();
    check(controller.currentIndex() == 0, "previous image navigates backward");
    controller.previousImage();
    check(controller.currentIndex() == 0, "previous image does not wrap at the start");

    controller.rotateRight();
    check(controller.rotation() == 90, "rotate right advances by 90 degrees");
    controller.rotateRight();
    controller.rotateRight();
    controller.rotateRight();
    check(controller.rotation() == 0, "right rotation normalizes after 360 degrees");
    controller.rotateLeft();
    check(controller.rotation() == 270, "rotate left normalizes negative rotation");

    check(controller.state()->zoomMode() == impage::viewer::ViewerState::ZoomMode::Fit,
          "viewer opens in fit mode");
    check(controller.state()->fitMode(), "viewer exposes fit mode directly to QML");
    controller.updateFitScale(0.4);
    check(std::abs(controller.state()->zoomFactor() - 0.4) < 0.0001,
          "fit scale is tracked by viewer state");
    controller.zoomIn();
    check(controller.state()->zoomMode() == impage::viewer::ViewerState::ZoomMode::Custom,
          "zoom command changes mode to custom");
    check(std::abs(controller.state()->zoomFactor() - 0.5) < 0.0001,
          "zoom command uses a progressive multiplier");
    controller.actualSize();
    check(controller.state()->zoomMode() == impage::viewer::ViewerState::ZoomMode::ActualSize &&
              std::abs(controller.state()->zoomFactor() - 1.0) < 0.0001,
          "actual size sets logical 100 percent zoom");
    controller.fitToWindow();
    check(controller.state()->zoomMode() == impage::viewer::ViewerState::ZoomMode::Fit,
          "fit command restores fit mode");

    controller.setCustomZoom(100.0);
    check(std::abs(controller.state()->zoomFactor() - 8.0) < 0.0001,
          "custom zoom is clamped to 800 percent");
    controller.setCustomZoom(0.001);
    check(std::abs(controller.state()->zoomFactor() - 0.05) < 0.0001,
          "custom zoom is clamped to 5 percent");

    check(session.setCurrentImage(ids.at(1)), "session current image can change externally");
    check(controller.currentIndex() == 1 &&
              controller.state()->zoomMode() == impage::viewer::ViewerState::ZoomMode::Fit,
          "viewer reacts to external current image changes and resets to fit");

    int composerRequests = 0;
    impage::core::ComposerActivationContext activationContext;
    QObject::connect(&controller, &impage::viewer::ViewerController::composerActivationRequested,
                     [&composerRequests, &activationContext](const auto& context) {
                         ++composerRequests;
                         activationContext = context;
                     });
    controller.openComposer();
    check(composerRequests == 1 && activationContext.imageIds == QList{ids.at(1)} &&
              activationContext.source == impage::core::ActivationSource::Viewer,
          "viewer requests composer navigation with an explicit current-image contract");

    QTemporaryDir catalogDirectory;
    check(catalogDirectory.isValid(), "catalog integration fixture directory is available");
    const QString photo1 =
        createImage(catalogDirectory, QStringLiteral("photo1.png"), QSize(120, 80), Qt::red);
    const QString photo2 =
        createImage(catalogDirectory, QStringLiteral("photo2.png"), QSize(120, 80), Qt::green);
    const QString photo10 =
        createImage(catalogDirectory, QStringLiteral("photo10.png"), QSize(120, 80), Qt::blue);
    const QString corruptPath = catalogDirectory.filePath(QStringLiteral("corrupt.png"));
    QFile corrupt(corruptPath);
    check(corrupt.open(QIODevice::WriteOnly), "corrupt catalog fixture can be created");
    corrupt.write("not an image");
    corrupt.close();

    impage::core::ImageSession catalogSession;
    const auto initialCatalogImage = catalogSession.addImage(photo2);
    check(initialCatalogImage.has_value(), "catalog session starts from one explicit image");
    impage::core::ThumbnailCache catalogCache;
    impage::core::ImageMetadataService catalogMetadataService;
    impage::viewer::ViewerController catalogController(catalogSession, catalogCache,
                                                       catalogMetadataService);
    catalogController.folderModel()->setWatchingEnabled(false);
    check(catalogController.folderModel()->scanFromImageSynchronously(photo2),
          "controller catalog discovers the current image folder");

    const int corruptIndex = catalogController.folderModel()->indexOfPath(corruptPath);
    const int photo1Index = catalogController.folderModel()->indexOfPath(photo1);
    const int photo2Index = catalogController.folderModel()->indexOfPath(photo2);
    const int photo10Index = catalogController.folderModel()->indexOfPath(photo10);
    check(catalogController.imageCount() == 4 && catalogSession.count() == 1,
          "folder catalog stays separate from the active image session");
    check(corruptIndex == 0 && photo1Index < photo2Index && photo2Index < photo10Index,
          "controller consumes the folder model natural order");
    check(catalogController.currentIndex() == photo2Index,
          "initial folder position follows the shared session current image");
    check(catalogController.previousImageUrl() == QUrl::fromLocalFile(photo1) &&
              catalogController.nextImageUrl() == QUrl::fromLocalFile(photo10),
          "viewer exposes only immediate neighbors for preloading");

    catalogController.nextImage();
    check(catalogSession.currentImage()->sourcePath == photo10 && catalogSession.count() == 2,
          "folder navigation lazily adds only the visited image to the session");
    catalogController.nextImage();
    check(catalogSession.currentImage()->sourcePath == photo10,
          "folder navigation does not wrap after the last image");
    catalogController.previousImage();
    check(catalogSession.currentImage()->sourcePath == photo2,
          "folder navigation moves backward in catalog order");
    catalogController.activateFolderIndex(photo1Index);
    check(catalogSession.currentImage()->sourcePath == photo1 && catalogSession.count() == 3,
          "thumbnail activation updates the session current image without importing the folder");
    int catalogRotationSignals = 0;
    QObject::connect(&catalogController, &impage::viewer::ViewerController::rotationChanged,
                     [&catalogRotationSignals] { ++catalogRotationSignals; });
    catalogController.rotateRight();
    check(catalogController.rotation() == 90 && catalogRotationSignals == 1,
          "rotation notifies QML when folder and session indexes differ");

    catalogController.toggleFolderSelection(photo10Index);
    check(catalogSession.selectedCount() == 1,
          "control-style selection toggles independently from the current image");
    catalogController.selectFolderRange(photo1Index);
    check(catalogSession.selectedCount() == 3,
          "shift-style range selection follows the catalog order and anchor");
    const QList<impage::core::ImageId> orderedCandidates = catalogController.printCandidateImages();
    check(orderedCandidates.size() == 3 &&
              pathForId(catalogSession, orderedCandidates.at(0)) == photo1 &&
              pathForId(catalogSession, orderedCandidates.at(1)) == photo2 &&
              pathForId(catalogSession, orderedCandidates.at(2)) == photo10,
          "print candidates preserve visual folder order rather than click order");
    check(catalogController.currentFileName() == QFileInfo(photo1).fileName(),
          "range selection remains independent from the current image");

    catalogController.clearSelection();
    check(catalogController.printCandidateImages().size() == 1 &&
              pathForId(catalogSession, catalogController.printCandidateImages().constFirst()) ==
                  photo1,
          "without selection the current image is the sole print candidate");
    catalogController.activateFolderIndex(corruptIndex);
    check(catalogSession.currentImage()->sourcePath == corruptPath &&
              !catalogSession.currentImage()->valid && catalogController.canGoNext(),
          "corrupt catalog item remains navigable as an invalid session reference");
    catalogController.nextImage();
    check(catalogSession.currentImage()->sourcePath == photo1,
          "navigation can continue after a corrupt image");

    catalogController.selectAllFolderImages();
    check(catalogController.selectedImageCount() == 4 &&
              catalogController.printActionText() == QStringLiteral("Imprimir") &&
              catalogController.printAccessibleName().contains(QStringLiteral("4 imagens")),
          "print action stays concise while its accessible name describes the selection");
    catalogController.clearSelection();

    catalogController.activateFolderIndex(photo2Index);
    check(QFile::remove(photo2), "current catalog image can be removed");
    check(catalogController.folderModel()->refreshSynchronously(),
          "controller catalog can refresh after current removal");
    check(catalogSession.currentImage()->sourcePath == photo10 &&
              catalogController.folderModel()->indexOfPath(photo2) == -1,
          "removed current image falls forward to the item at its previous position");

    check(catalogController.state()->filmstripVisible(), "filmstrip is visible by default");
    catalogController.toggleFilmstrip();
    check(!catalogController.state()->filmstripVisible(),
          "toolbar command toggles filmstrip visibility in viewer state");

    check(!catalogController.state()->infoPanelVisible(), "information panel starts closed");
    catalogController.toggleInfoPanel();
    check(catalogController.state()->infoPanelVisible(),
          "information panel visibility is controlled by persistent viewer state");

    QString lastRapidPath;
    for (int imageNumber = 0; imageNumber < 50; ++imageNumber) {
        lastRapidPath = createImage(
            catalogDirectory,
            QStringLiteral("rapid-%1.png").arg(imageNumber, 2, 10, QLatin1Char('0')),
            QSize(64 + imageNumber, 40 + imageNumber), QColor::fromHsv(imageNumber * 7, 180, 220));
    }
    check(catalogController.folderModel()->refreshSynchronously(),
          "rapid navigation fixtures are discovered");
    for (int imageNumber = 0; imageNumber < 50; ++imageNumber) {
        const QString path = catalogDirectory.filePath(
            QStringLiteral("rapid-%1.png").arg(imageNumber, 2, 10, QLatin1Char('0')));
        catalogController.activateFolderIndex(catalogController.folderModel()->indexOfPath(path));
    }
    QElapsedTimer metadataDeadline;
    metadataDeadline.start();
    while (catalogController.metadata()->loading() && metadataDeadline.elapsed() < 5'000) {
        application.processEvents(QEventLoop::AllEvents, 20);
        QThread::msleep(2);
    }
    for (int iteration = 0; iteration < 20; ++iteration) {
        application.processEvents(QEventLoop::AllEvents, 10);
        QThread::msleep(1);
    }
    check(!catalogController.metadata()->loading() &&
              catalogController.metadata()->absolutePath() == lastRapidPath,
          "late metadata results never overwrite the current image after 50 rapid changes");

    catalogController.activateFolderIndex(catalogController.folderModel()->indexOfPath(photo1));
    catalogController.toggleFolderSelection(catalogController.folderModel()->indexOfPath(photo10));
    catalogController.setCustomZoom(1.75);
    catalogController.rotateRight();
    catalogController.captureViewState(23.0, -11.0, 145.0);
    catalogController.captureNavigationState();
    const int savedRotation = catalogController.rotation();

    catalogController.activateFolderIndex(
        catalogController.folderModel()->indexOfPath(lastRapidPath));
    catalogController.clearSelection();
    catalogController.fitToWindow();
    catalogController.toggleInfoPanel();
    catalogController.toggleFilmstrip();
    catalogController.restoreNavigationState();
    check(catalogController.currentImageUrl() == QUrl::fromLocalFile(photo1) &&
              catalogController.selectedImageCount() == 1 &&
              std::abs(catalogController.state()->zoomFactor() - 1.75) < 0.0001 &&
              catalogController.rotation() == savedRotation &&
              catalogController.state()->infoPanelVisible() &&
              !catalogController.state()->filmstripVisible() &&
              catalogController.savedPanX() == 23.0 && catalogController.savedPanY() == -11.0 &&
              catalogController.savedFilmstripContentX() == 145.0,
          "ViewerNavigationState restores current, selection, zoom, rotation and visual state");

    catalogController.toggleFolderSelection(catalogController.folderModel()->indexOfPath(photo1));
    check(QFile::remove(photo10), "a selected print candidate can disappear before activation");
    impage::core::ComposerActivationContext filteredActivation;
    int filteredActivationCount = 0;
    QObject::connect(&catalogController,
                     &impage::viewer::ViewerController::composerActivationRequested,
                     [&filteredActivation, &filteredActivationCount](const auto& context) {
                         filteredActivation = context;
                         ++filteredActivationCount;
                     });
    catalogController.openComposer();
    check(filteredActivationCount == 1 && filteredActivation.imageIds.size() == 1 &&
              pathForId(catalogSession, filteredActivation.imageIds.constFirst()) == photo1 &&
              !catalogController.state()->errorString().isEmpty(),
          "missing candidates are ignored while valid selected images still activate Composer");

    QTemporaryDir largeDirectory;
    check(largeDirectory.isValid(), "large lightweight selection fixture is available");
    QString largeFirstPath;
    constexpr int largeCollectionSize = 300;
    for (int imageNumber = 0; imageNumber < largeCollectionSize; ++imageNumber) {
        const QString path = createImage(
            largeDirectory,
            QStringLiteral("collection-%1.png").arg(imageNumber, 3, 10, QLatin1Char('0')),
            QSize(24, 16), QColor::fromHsv(imageNumber * 3, 160, 210));
        if (imageNumber == 0) {
            largeFirstPath = path;
        }
    }
    impage::core::ImageSession largeSession;
    const auto largeInitialImage = largeSession.addImage(largeFirstPath);
    check(largeInitialImage.has_value(), "large collection starts from one decoded image");
    impage::core::ThumbnailCache largeCache;
    impage::core::ImageMetadataService largeMetadataService;
    impage::viewer::ViewerController largeController(largeSession, largeCache,
                                                     largeMetadataService);
    largeController.folderModel()->setWatchingEnabled(false);
    check(largeController.folderModel()->scanFromImageSynchronously(largeFirstPath),
          "large active folder is discovered");
    largeController.selectAllFolderImages();
    const int lightweightReferences = static_cast<int>(std::count_if(
        largeSession.images().cbegin(), largeSession.images().cend(),
        [](const impage::core::ImageEntry& image) { return !image.pixelSize.isValid(); }));
    check(largeSession.count() == largeCollectionSize &&
              largeController.selectedImageCount() == largeCollectionSize &&
              lightweightReferences == largeCollectionSize - 1,
          "selecting 300 images stores IDs/references without decoding full originals");
    for (int navigationStep = 0; navigationStep < largeCollectionSize * 2; ++navigationStep) {
        largeController.nextImage();
    }
    check(largeController.currentIndex() == largeCollectionSize - 1,
          "rapid navigation across a large catalog remains stable at its boundary");

    QTemporaryDir trashDirectory;
    check(trashDirectory.isValid(), "trash behavior fixture directory is available");
    const QString trashFirst =
        createImage(trashDirectory, QStringLiteral("trash-first.png"), QSize(32, 24), Qt::red);
    const QString trashSecond =
        createImage(trashDirectory, QStringLiteral("trash-second.png"), QSize(32, 24), Qt::blue);
    impage::core::ImageSession trashSession;
    check(trashSession.addImage(trashFirst).has_value(), "trash fixture starts in session");
    impage::core::ThumbnailCache trashCache;
    impage::core::ImageMetadataService trashMetadata;
    impage::viewer::ViewerController trashController(trashSession, trashCache, trashMetadata);
    trashController.folderModel()->setWatchingEnabled(false);
    check(trashController.folderModel()->scanFromImageSynchronously(trashFirst),
          "trash fixture folder is available");
    trashController.setTrashFunctionForTesting([](const QString& path, QString* destination) {
        const QString moved = path + QStringLiteral(".trashed");
        if (destination != nullptr) {
            *destination = moved;
        }
        return QFile::rename(path, moved);
    });
    QString trashNotice;
    QObject::connect(&trashController, &impage::viewer::ViewerController::noticeRequested,
                     [&trashNotice](const QString& message) { trashNotice = message; });
    trashController.trashCurrentImage();
    check(!QFileInfo::exists(trashFirst) && trashSession.count() == 1 &&
              trashSession.currentImage()->sourcePath == trashSecond && !trashNotice.isEmpty(),
          "safe trash action advances to the next image and removes stale session references");

    return failures == 0 ? 0 : 1;
}
