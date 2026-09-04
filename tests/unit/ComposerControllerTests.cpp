#include "composer/ComposerController.h"

#include <QApplication>
#include <QClipboard>
#include <QImage>
#include <QMimeData>
#include <QPixmap>
#include <QTemporaryDir>
#include <QUrl>

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

} // namespace

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    purrview::core::ImageSession imageSession;
    purrview::composer::ComposerController controller(imageSession);

    check(controller.rows() == 1 && controller.columns() == 1,
          "a new composition defaults to a 1x1 grid");
    int presetUpdates = 0;
    QObject::connect(&controller, &purrview::composer::ComposerController::documentChanged,
                     [&presetUpdates] { ++presetUpdates; });
    controller.setGridPreset(2, 3);
    check(controller.rows() == 2 && controller.columns() == 3,
          "grid preset updates rows and columns together");
    check(presetUpdates == 1, "grid preset emits one coherent document update");
    controller.setGridPreset(2, 3);
    check(presetUpdates == 1, "reapplying the active grid preset is a no-op");
    controller.setGridPreset(1, 1);

    controller.setPaperSize(1);
    check(controller.paperName() == QStringLiteral("A3") &&
              std::abs(controller.pageWidthMm() - 297.0) < 0.0001 &&
              std::abs(controller.pageHeightMm() - 420.0) < 0.0001,
          "paper selection updates the A3 preview geometry");
    controller.setLandscape(true);
    check(std::abs(controller.pageWidthMm() - 420.0) < 0.0001 &&
              std::abs(controller.pageHeightMm() - 297.0) < 0.0001,
          "paper geometry follows the selected orientation");
    controller.setLandscape(false);
    controller.setPaperSize(5);
    check(controller.paperName() == QStringLiteral("Foto 10 × 15 cm") &&
              std::abs(controller.pageWidthMm() - 100.0) < 0.0001 &&
              std::abs(controller.pageHeightMm() - 150.0) < 0.0001,
          "photo paper preset exposes the expected geometry");
    controller.setPaperSize(0);

    QImage copiedImage(80, 60, QImage::Format_ARGB32_Premultiplied);
    copiedImage.fill(QColor(40, 120, 220));
    QApplication::clipboard()->setImage(copiedImage);

    check(controller.canPasteImages(), "pixel image is recognized in the clipboard");
    controller.pasteImages();
    check(controller.imageCount() == 1, "pixel image is imported from the clipboard");
    check(controller.imageThumbnails().size() == 1, "imported image is exposed as a thumbnail");

    QApplication::clipboard()->setPixmap(QPixmap::fromImage(copiedImage));
    controller.pasteImages();
    check(controller.imageCount() == 2, "copied pixmap is imported from the clipboard");

    QTemporaryDir directory;
    check(directory.isValid(), "temporary source directory is available");
    const QString sourcePath = directory.filePath(QStringLiteral("copied-file.png"));
    check(copiedImage.save(sourcePath), "clipboard file fixture can be saved");

    auto* mimeData = new QMimeData;
    mimeData->setUrls({QUrl::fromLocalFile(sourcePath)});
    QApplication::clipboard()->setMimeData(mimeData);

    check(controller.canPasteImages(), "copied image file is recognized in the clipboard");
    controller.pasteImages();
    check(controller.imageCount() == 3, "copied image file is imported from the clipboard");
    check(controller.imageThumbnails().at(2).toMap().value(QStringLiteral("source")).toUrl() ==
              QUrl::fromLocalFile(sourcePath),
          "thumbnail source identifies the imported file");
    check(imageSession.count() == controller.imageCount(),
          "composer imports are stored in the shared session");

    int rotationUpdates = 0;
    QObject::connect(&controller, &purrview::composer::ComposerController::documentChanged,
                     [&rotationUpdates] { ++rotationUpdates; });
    check(imageSession.setRotation(imageSession.images().front().id, 90),
          "session image can be rotated by the viewer");
    check(rotationUpdates > 0, "composer refreshes when shared non-destructive rotation changes");

    check(imageSession.selectImage(imageSession.images().at(1).id),
          "session image can be selected for composition");
    check(controller.imageCount() == 3,
          "selection alone does not silently replace an active composer document");

    purrview::core::ComposerActivationContext viewerActivation{
        .imageIds = {imageSession.images().at(1).id},
        .source = purrview::core::ActivationSource::Viewer};
    controller.activate(viewerActivation);
    check(controller.imageCount() == 1,
          "explicit Viewer activation replaces the document image set");
    imageSession.clearSelection();
    check(controller.imageCount() == 1,
          "Viewer composition remains stable when selection later changes");

    viewerActivation.imageIds = {imageSession.images().at(2).id, imageSession.images().at(0).id};
    controller.activate(viewerActivation);
    check(controller.imageCount() == 2 &&
              controller.imageThumbnails().at(0).toMap().value(QStringLiteral("source")).toUrl() ==
                  QUrl::fromLocalFile(sourcePath) &&
              controller.imageThumbnails().at(1).toMap().value(QStringLiteral("source")).toUrl() !=
                  QUrl::fromLocalFile(sourcePath),
          "explicit activation order is preserved independently of session order");

    controller.addImages({QUrl::fromLocalFile(sourcePath)});
    check(controller.imageCount() == 3 && imageSession.count() == 4,
          "importing inside an explicit composition appends without restoring hidden entries");

    controller.clearImages();
    check(imageSession.count() == 0 && controller.imageCount() == 0,
          "clearing the composer clears the shared session consistently");

    const QString firstPath = directory.filePath(QStringLiteral("first.png"));
    const QString secondPath = directory.filePath(QStringLiteral("second.png"));
    const QString thirdPath = directory.filePath(QStringLiteral("third.png"));
    const QString fourthPath = directory.filePath(QStringLiteral("fourth.png"));
    check(copiedImage.save(firstPath) && copiedImage.save(secondPath) &&
              copiedImage.save(thirdPath) && copiedImage.save(fourthPath),
          "composition editing fixtures can be saved");
    controller.addImages({QUrl::fromLocalFile(firstPath), QUrl::fromLocalFile(secondPath),
                          QUrl::fromLocalFile(thirdPath)});
    check(controller.imageCount() == 3, "composition editing starts with three images");

    controller.setGridPreset(2, 2);
    check(controller.imageIndexAtPagePosition(0.25, 0.25, false) == 0 &&
              controller.imageIndexAtPagePosition(0.75, 0.25, false) == 1 &&
              controller.imageIndexAtPagePosition(0.25, 0.75, false) == 2,
          "preview hit testing maps occupied grid cells to composition images");
    check(controller.imageIndexAtPagePosition(0.75, 0.75, false) == -1 &&
              controller.imageIndexAtPagePosition(0.75, 0.75, true) == 3,
          "preview hit testing distinguishes an empty destination cell");
    controller.moveImagesToPosition(0, 2);
    check(controller.selectedImageCount() == 0 &&
              controller.imageThumbnails().at(2).toMap().value(QStringLiteral("name")) ==
                  QStringLiteral("first.png"),
          "direct preview-style drag reorders an image without selecting it");
    controller.moveImagesToPosition(2, 0);

    controller.selectImage(0, false, false);
    controller.selectImage(2, true, false);
    check(
        controller.selectedImageCount() == 2 &&
            controller.imageThumbnails().at(0).toMap().value(QStringLiteral("selected")).toBool() &&
            controller.imageThumbnails().at(2).toMap().value(QStringLiteral("selected")).toBool(),
        "individual thumbnails can form a multi-selection");

    controller.moveImages(0, 3);
    check(controller.imageThumbnails().at(0).toMap().value(QStringLiteral("name")).toString() ==
                  QStringLiteral("second.png") &&
              controller.imageThumbnails().at(1).toMap().value(QStringLiteral("name")).toString() ==
                  QStringLiteral("first.png") &&
              controller.imageThumbnails().at(2).toMap().value(QStringLiteral("name")).toString() ==
                  QStringLiteral("third.png"),
          "drag-style move preserves the relative order of selected images");

    controller.duplicateSelectedImages();
    check(controller.imageCount() == 5 && controller.selectedImageCount() == 2 &&
              controller.imageThumbnails().at(1).toMap().value(QStringLiteral("source")) ==
                  controller.imageThumbnails().at(2).toMap().value(QStringLiteral("source")) &&
              controller.imageThumbnails().at(3).toMap().value(QStringLiteral("source")) ==
                  controller.imageThumbnails().at(4).toMap().value(QStringLiteral("source")),
          "duplicating selected images creates adjacent printable occurrences");
    check(imageSession.count() == 3,
          "duplicating composition entries does not duplicate the shared source session");

    controller.removeSelectedImages();
    check(controller.imageCount() == 3 && imageSession.count() == 3,
          "removing duplicated occurrences affects only the composition");
    controller.selectImage(0, false, false);
    controller.selectImage(2, true, false);
    controller.removeSelectedImages();
    check(controller.imageCount() == 1 && imageSession.count() == 3 &&
              controller.imageThumbnails().constFirst().toMap().value(QStringLiteral("name")) ==
                  QStringLiteral("first.png"),
          "multiple selected photos can be removed without deleting their session entries");

    controller.addImages({QUrl::fromLocalFile(fourthPath)});
    check(controller.imageCount() == 2 &&
              controller.imageThumbnails().at(0).toMap().value(QStringLiteral("name")).toString() ==
                  QStringLiteral("first.png") &&
              controller.imageThumbnails().at(1).toMap().value(QStringLiteral("name")).toString() ==
                  QStringLiteral("fourth.png"),
          "new imports append without restoring photos removed from an edited composition");

    controller.clearImages();
    check(controller.imageCount() == 0 && controller.selectedImageCount() == 0,
          "clearing an edited composition also clears its thumbnail selection");

    return failures == 0 ? 0 : 1;
}
