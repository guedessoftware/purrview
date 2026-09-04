#include "core/image/ImageSession.h"

#include <QCoreApplication>
#include <QFile>
#include <QImage>
#include <QTemporaryDir>

#include <iostream>

namespace {

int failures = 0;

void check(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

QString createImage(const QTemporaryDir& directory, const QString& name, const QColor& color) {
    const QString path = directory.filePath(name);
    QImage image(80, 60, QImage::Format_ARGB32_Premultiplied);
    image.fill(color);
    check(image.save(path), "image fixture can be saved");
    return path;
}

void testEmptyAndAdding(const QStringList& paths) {
    purrview::core::ImageSession session;
    check(session.count() == 0, "empty session has zero images");
    check(session.currentIndex() == -1, "empty session has no current index");
    check(session.currentImage() == nullptr, "empty session has no current image");

    const auto firstId = session.addImage(paths.at(0));
    check(firstId.has_value(), "first valid image is accepted");
    check(session.count() == 1, "first image increments session count");
    check(session.currentIndex() == 0, "first image becomes current");
    check(session.currentImage() != nullptr && session.currentImage()->id == *firstId,
          "current image identifies first entry");

    const QList<purrview::core::ImageId> remainingIds = session.addImages({paths.at(1), paths.at(2)});
    check(remainingIds.size() == 2, "multiple valid images are accepted");
    check(session.count() == 3, "multiple images increment session count");
    check(session.images().at(0).sourcePath == paths.at(0), "first image order is preserved");
    check(session.images().at(1).sourcePath == paths.at(1), "second image order is preserved");
    check(session.images().at(2).sourcePath == paths.at(2), "third image order is preserved");
    check(session.data(session.index(1), purrview::core::ImageSession::SourceRole).toUrl() ==
              QUrl::fromLocalFile(paths.at(1)),
          "QML source role exposes a local file URL");
}

void testCurrentImageAndRemoval(const QStringList& paths) {
    purrview::core::ImageSession session;
    const QList<purrview::core::ImageId> ids = session.addImages(paths.mid(0, 3));
    int currentSignalCount = 0;
    QObject::connect(&session, &purrview::core::ImageSession::currentImageChanged,
                     [&currentSignalCount] { ++currentSignalCount; });

    session.setCurrentIndex(2);
    check(session.currentIndex() == 2, "current index can be changed");
    check(session.currentImage()->id == ids.at(2), "current image follows current index");
    check(currentSignalCount == 1, "current image emits one signal when changed");
    session.setCurrentIndex(2);
    check(currentSignalCount == 1, "setting the same index emits no redundant signal");

    const purrview::core::ImageId stableCurrentId = session.currentImage()->id;
    check(session.removeImage(ids.at(0)), "image before current can be removed");
    check(session.currentIndex() == 1, "removing before current corrects its index");
    check(session.currentImage()->id == stableCurrentId,
          "current image identity survives removal before it");

    check(session.removeImage(stableCurrentId), "current image can be removed");
    check(session.currentIndex() == 0, "adjacent image is selected after current removal");
    check(session.currentImage()->id == ids.at(1),
          "previous image is selected when removed current was last");

    check(session.removeImage(ids.at(1)), "last remaining image can be removed");
    check(session.count() == 0, "removing last image empties session");
    check(session.currentIndex() == -1, "empty session restores invalid current index");
    check(session.currentImage() == nullptr, "empty session restores null current image");
}

void testSelection(const QStringList& paths) {
    purrview::core::ImageSession session;
    const QList<purrview::core::ImageId> ids = session.addImages(paths.mid(0, 3));
    int selectionSignalCount = 0;
    QObject::connect(&session, &purrview::core::ImageSession::selectionChanged,
                     [&selectionSignalCount] { ++selectionSignalCount; });

    check(session.selectImage(ids.at(0)), "image can be selected");
    check(session.selectImage(ids.at(2)), "multiple images can be selected");
    check(session.selectedCount() == 2, "selected count tracks multiple selection");
    check(session.selectedImages().at(0).id == ids.at(0) &&
              session.selectedImages().at(1).id == ids.at(2),
          "selected images preserve session order");
    check(session.currentImage()->id == ids.at(0),
          "selection changes do not change the current image");

    check(session.toggleSelection(ids.at(0)), "selection can be toggled off");
    check(session.selectedCount() == 1, "toggle updates selected count");
    session.clearSelection();
    check(session.selectedCount() == 0, "selection can be cleared");
    session.selectAll();
    check(session.selectedCount() == session.count(), "all images can be selected");
    check(session.removeImage(ids.at(1)) && session.selectedCount() == 2,
          "removing a selected image removes it from the selection consistently");
    check(selectionSignalCount == 6, "selection emits one relevant signal per operation");
}

void testClearDuplicatesAndValidation(const QStringList& paths, const QTemporaryDir& directory) {
    purrview::core::ImageSession session;
    const auto first = session.addImage(paths.at(0));
    const auto duplicate = session.addImage(paths.at(0));
    check(first.has_value() && duplicate.has_value(), "duplicate paths are intentionally allowed");
    check(*first != *duplicate, "duplicate entries receive stable independent IDs");
    check(session.images().at(0).sourcePath == session.images().at(1).sourcePath,
          "duplicate entries retain the requested source path");

    QString error;
    check(!session.addImage(directory.filePath(QStringLiteral("missing.png")), &error).has_value(),
          "missing file is rejected");
    check(!error.isEmpty(), "rejected image returns a clear error");
    check(session.count() == 2, "invalid image does not mutate session");

    const QString corruptPath = directory.filePath(QStringLiteral("corrupt.png"));
    QFile corrupt(corruptPath);
    check(corrupt.open(QIODevice::WriteOnly), "corrupt image reference can be created");
    corrupt.write("not a PNG");
    corrupt.close();
    error = QStringLiteral("stale error");
    const auto corruptReference = session.addImageReference(corruptPath, &error);
    check(corruptReference.has_value() && !session.images().back().valid,
          "supported corrupt files can remain as invalid navigable references");
    check(error.isEmpty(), "successful image reference clears an earlier decode error");

    check(session.setRotation(*first, 90), "logical rotation accepts right angles");
    check(session.images().at(0).rotationDegrees == 90, "logical rotation is stored in session");
    check(!session.setRotation(*first, 45), "unsupported rotation angle is rejected");

    int clearedSignals = 0;
    QObject::connect(&session, &purrview::core::ImageSession::sessionCleared,
                     [&clearedSignals] { ++clearedSignals; });
    session.clear();
    check(session.count() == 0 && session.currentIndex() == -1,
          "clear restores an empty consistent session");
    check(clearedSignals == 1, "clear emits the session cleared signal once");
}

} // namespace

int main(int argc, char* argv[]) {
    QCoreApplication application(argc, argv);
    QTemporaryDir directory;
    check(directory.isValid(), "temporary fixture directory is available");

    const QStringList paths = {
        createImage(directory, QStringLiteral("one.png"), QColor(210, 30, 45)),
        createImage(directory, QStringLiteral("two.png"), QColor(30, 180, 80)),
        createImage(directory, QStringLiteral("three.png"), QColor(40, 90, 210))};

    testEmptyAndAdding(paths);
    testCurrentImageAndRemoval(paths);
    testSelection(paths);
    testClearDuplicatesAndValidation(paths, directory);

    return failures == 0 ? 0 : 1;
}
