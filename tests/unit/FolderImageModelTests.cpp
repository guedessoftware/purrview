#include "core/image/FolderImageModel.h"
#include "core/image/ImageFormatSupport.h"

#include <QCoreApplication>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
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
    QImage image(80, 50, QImage::Format_ARGB32_Premultiplied);
    image.fill(color);
    check(image.save(path), "folder image fixture can be saved");
    return path;
}

int indexForName(const impage::core::FolderImageModel& model, const QString& name) {
    for (int row = 0; row < model.count(); ++row) {
        if (model.data(model.index(row), impage::core::FolderImageModel::FileNameRole).toString() ==
            name) {
            return row;
        }
    }
    return -1;
}

} // namespace

int main(int argc, char* argv[]) {
    QCoreApplication application(argc, argv);
    impage::core::ImageSession session;
    impage::core::ThumbnailCache cache;
    impage::core::FolderImageModel model(session, cache);
    model.setWatchingEnabled(false);

    QTemporaryDir emptyDirectory;
    check(emptyDirectory.isValid(), "empty folder fixture is available");
    check(!model.scanFromImageSynchronously(emptyDirectory.filePath(QStringLiteral("none.png"))),
          "missing initial image is not found in an empty folder");
    check(model.count() == 0 && model.currentIndex() == -1,
          "empty folder produces an empty catalog");

    QTemporaryDir directory;
    check(directory.isValid(), "folder fixture is available");
    const QString first = createImage(directory, QStringLiteral("foto1.png"), Qt::red);
    const QString second = createImage(directory, QStringLiteral("foto2.png"), Qt::green);
    const QString tenth = createImage(directory, QStringLiteral("foto10.png"), Qt::blue);
    const QString unicode =
        createImage(directory, QStringLiteral("férias3.png"), QColor(120, 60, 180));
    QFile textFile(directory.filePath(QStringLiteral("not-an-image.txt")));
    check(textFile.open(QIODevice::WriteOnly), "non-image fixture can be created");
    textFile.write("plain text");
    textFile.close();

    const auto initialId = session.addImage(second);
    check(initialId.has_value(), "initial session image is valid");
    check(model.scanFromImageSynchronously(second), "initial image is found during folder scan");
    check(model.count() == 4, "folder model filters non-image files");
    const int firstIndex = indexForName(model, QStringLiteral("foto1.png"));
    const int secondIndex = indexForName(model, QStringLiteral("foto2.png"));
    const int tenthIndex = indexForName(model, QStringLiteral("foto10.png"));
    check(firstIndex >= 0 && firstIndex < secondIndex && secondIndex < tenthIndex,
          "numeric file names use natural ordering");
    check(indexForName(model, QStringLiteral("férias3.png")) >= 0,
          "Unicode file names are preserved");
    check(model.currentIndex() == secondIndex,
          "folder current index is derived from the shared session");
    check(
        model.data(model.index(secondIndex), impage::core::FolderImageModel::CurrentRole).toBool(),
        "current role marks the session image");
    check(
        !model.data(model.index(firstIndex), impage::core::FolderImageModel::CurrentRole).toBool(),
        "non-current item is not marked current");

    const auto tenthId = session.addImage(tenth);
    check(tenthId.has_value() && session.setCurrentImage(*tenthId),
          "session can navigate to another catalog item");
    check(model.currentIndex() == tenthIndex, "external current change updates folder index");
    check(session.selectImage(*tenthId), "catalog session item can be selected");
    check(
        model.data(model.index(tenthIndex), impage::core::FolderImageModel::SelectedRole).toBool(),
        "selected role mirrors ImageSession selection");

    check(impage::core::isSupportedImageFile(first), "runtime-supported PNG is recognized");
    check(!impage::core::isSupportedImageFile(textFile.fileName()),
          "unsupported extension is rejected centrally");
    QSet<QString> runtimeFormats;
    for (const QByteArray& format : QImageReader::supportedImageFormats()) {
        runtimeFormats.insert(QString::fromLatin1(format).toLower());
    }
    for (const QString& format : {QStringLiteral("avif"), QStringLiteral("heif"),
                                  QStringLiteral("heic"), QStringLiteral("icns")}) {
        if (runtimeFormats.contains(format)) {
            check(impage::core::supportedImageSuffixes().contains(format),
                  "new runtime image codec is enabled by the central format policy");
        }
    }

    QSet<QByteArray> writableFormats;
    for (const QByteArray& format : QImageWriter::supportedImageFormats()) {
        writableFormats.insert(format.toLower());
    }
    QTemporaryDir codecDirectory;
    check(codecDirectory.isValid(), "runtime codec fixture directory is available");
    QImage codecFixture(QSize(64, 64), QImage::Format_ARGB32);
    codecFixture.fill(QColor(35, 110, 210));
    for (const QByteArray& format : {QByteArrayLiteral("avif"), QByteArrayLiteral("heif"),
                                     QByteArrayLiteral("heic"), QByteArrayLiteral("icns")}) {
        if (!runtimeFormats.contains(QString::fromLatin1(format)) ||
            !writableFormats.contains(format)) {
            continue;
        }
        const QString codecPath =
            codecDirectory.filePath(QStringLiteral("codec.%1").arg(QString::fromLatin1(format)));
        QImageWriter writer(codecPath, format);
        check(writer.write(codecFixture), "new image codec can encode a test fixture");
        QImageReader reader(codecPath);
        check(reader.canRead() && !reader.read().isNull(),
              "new image codec can decode a real test fixture");
        check(impage::core::isSupportedImageFile(codecPath),
              "real codec fixture is accepted by the application policy");
    }
    check(model.sourceAt(firstIndex).isLocalFile(), "catalog exposes local source URLs");
    check(model.pathAt(-1).isEmpty() && model.pathAt(model.count()).isEmpty(),
          "catalog path access observes boundaries");

    check(QFile::remove(second), "folder image can be removed for refresh test");
    check(model.refreshSynchronously(), "folder can refresh synchronously");
    check(model.count() == 3 && model.indexOfPath(second) == -1,
          "removed file disappears from the catalog");
    check(model.selectedCount() == 1 && model.selectedPaths() == QStringList{tenth},
          "selection remains synchronized after the active folder changes");

    check(!model.scanFromImageSynchronously(QStringLiteral("/path/that/does/not/exist/photo.jpg")),
          "invalid folder path cannot locate an image");
    check(model.count() == 0, "invalid folder path clears the catalog safely");

    Q_UNUSED(unicode)
    return failures == 0 ? 0 : 1;
}
