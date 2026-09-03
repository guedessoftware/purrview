#include "composer/ComposerController.h"

#include <QClipboard>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QMimeData>
#include <QPixmap>
#include <QSet>
#include <QTimer>
#include <QUrl>

#include <algorithm>
#include <cmath>
#include <utility>

Q_LOGGING_CATEGORY(logApp, "impage.app")

namespace impage::composer {

namespace {
bool fuzzyEqual(double first, double second) {
    return std::abs(first - second) < 0.0001;
}
} // namespace

ComposerController::ComposerController(core::ImageSession& imageSession, QObject* parent)
    : QObject(parent), imageSession_(imageSession), sessionAdapter_(imageSession_, document_),
      printService_(renderer_) {
    connect(QGuiApplication::clipboard(), &QClipboard::dataChanged, this,
            &ComposerController::clipboardChanged);
    connect(&printService_, &platform::PrintService::dialogOpened, this, [this] {
        printDialogLoading_ = false;
        emit printDialogLoadingChanged();
    });
    connect(&printService_, &platform::PrintService::printFinished, this,
            &ComposerController::printFinished);
    connect(&printService_, &platform::PrintService::printFailed, this,
            [this](const QString& message) {
                printDialogLoading_ = false;
                emit printDialogLoadingChanged();
                qCWarning(logApp) << "Print operation failed";
                emit errorOccurred(message);
            });
    connect(&sessionAdapter_, &ComposerSessionAdapter::documentImagesChanged, this, [this] {
        renderer_.clearCache();
        normalizeImageSelection();
        emit thumbnailsChanged();
        notifyDocumentChanged();
    });
}

int ComposerController::rows() const {
    return document_.grid().rows();
}
int ComposerController::columns() const {
    return document_.grid().columns();
}
double ComposerController::marginTop() const {
    return document_.page().marginTopMm();
}
double ComposerController::marginRight() const {
    return document_.page().marginRightMm();
}
double ComposerController::marginBottom() const {
    return document_.page().marginBottomMm();
}
double ComposerController::marginLeft() const {
    return document_.page().marginLeftMm();
}
double ComposerController::horizontalSpacing() const {
    return document_.grid().horizontalSpacingMm();
}
double ComposerController::verticalSpacing() const {
    return document_.grid().verticalSpacingMm();
}
bool ComposerController::landscape() const {
    return document_.page().orientation() == core::PageModel::Orientation::Landscape;
}
int ComposerController::paperSize() const {
    return static_cast<int>(document_.page().paperSize());
}
QString ComposerController::paperName() const {
    switch (document_.page().paperSize()) {
    case core::PageModel::PaperSize::A4:
        return QStringLiteral("A4");
    case core::PageModel::PaperSize::A3:
        return QStringLiteral("A3");
    case core::PageModel::PaperSize::A5:
        return QStringLiteral("A5");
    case core::PageModel::PaperSize::Letter:
        return QStringLiteral("Carta");
    case core::PageModel::PaperSize::Legal:
        return QStringLiteral("Ofício / Legal");
    case core::PageModel::PaperSize::Photo10x15:
        return QStringLiteral("Foto 10 × 15 cm");
    }
    return QStringLiteral("A4");
}
int ComposerController::placementMode() const {
    return static_cast<int>(document_.placementMode());
}
int ComposerController::imageCount() const {
    return static_cast<int>(document_.images().size());
}
QVariantList ComposerController::imageThumbnails() const {
    QVariantList thumbnails;
    thumbnails.reserve(static_cast<qsizetype>(document_.images().size()));
    int index = 0;
    for (const core::ImageItem& image : document_.images()) {
        QVariantMap thumbnail;
        thumbnail.insert(QStringLiteral("source"), QUrl::fromLocalFile(image.source));
        thumbnail.insert(QStringLiteral("name"), QFileInfo(image.source).fileName());
        thumbnail.insert(QStringLiteral("selected"), selectedImageIndexes_.contains(index));
        thumbnails.push_back(thumbnail);
        ++index;
    }
    return thumbnails;
}
int ComposerController::selectedImageCount() const {
    return selectedImageIndexes_.size();
}
int ComposerController::pageCount() const {
    return document_.pageCount();
}
int ComposerController::currentPage() const {
    return currentPage_;
}
QString ComposerController::layoutError() const {
    return layoutEngine_.calculate(document_).error;
}
bool ComposerController::printDialogLoading() const {
    return printDialogLoading_;
}
bool ComposerController::canPasteImages() const {
    const QMimeData* mimeData = QGuiApplication::clipboard()->mimeData();
    return mimeData != nullptr && (mimeData->hasImage() || mimeData->hasUrls());
}
core::ImageSession* ComposerController::imageSession() {
    return &imageSession_;
}
double ComposerController::pageWidthMm() const {
    return document_.page().widthMm();
}
double ComposerController::pageHeightMm() const {
    return document_.page().heightMm();
}

void ComposerController::setRows(int value) {
    value = std::clamp(value, 1, 20);
    if (value == rows()) {
        return;
    }
    document_.grid().setRows(value);
    notifyDocumentChanged();
}

void ComposerController::setColumns(int value) {
    value = std::clamp(value, 1, 20);
    if (value == columns()) {
        return;
    }
    document_.grid().setColumns(value);
    notifyDocumentChanged();
}

void ComposerController::setMarginTop(double value) {
    if (!fuzzyEqual(value, marginTop())) {
        updateMargins(value, marginRight(), marginBottom(), marginLeft());
    }
}

void ComposerController::setMarginRight(double value) {
    if (!fuzzyEqual(value, marginRight())) {
        updateMargins(marginTop(), value, marginBottom(), marginLeft());
    }
}

void ComposerController::setMarginBottom(double value) {
    if (!fuzzyEqual(value, marginBottom())) {
        updateMargins(marginTop(), marginRight(), value, marginLeft());
    }
}

void ComposerController::setMarginLeft(double value) {
    if (!fuzzyEqual(value, marginLeft())) {
        updateMargins(marginTop(), marginRight(), marginBottom(), value);
    }
}

void ComposerController::setHorizontalSpacing(double value) {
    if (fuzzyEqual(value, horizontalSpacing())) {
        return;
    }
    document_.grid().setHorizontalSpacingMm(value);
    notifyDocumentChanged();
}

void ComposerController::setVerticalSpacing(double value) {
    if (fuzzyEqual(value, verticalSpacing())) {
        return;
    }
    document_.grid().setVerticalSpacingMm(value);
    notifyDocumentChanged();
}

void ComposerController::setLandscape(bool value) {
    if (value == landscape()) {
        return;
    }
    document_.page().setOrientation(value ? core::PageModel::Orientation::Landscape
                                          : core::PageModel::Orientation::Portrait);
    notifyDocumentChanged();
}

void ComposerController::setPaperSize(int value) {
    value = std::clamp(value, static_cast<int>(core::PageModel::PaperSize::A4),
                       static_cast<int>(core::PageModel::PaperSize::Photo10x15));
    const auto paperSize = static_cast<core::PageModel::PaperSize>(value);
    if (paperSize == document_.page().paperSize()) {
        return;
    }
    document_.page().setPaperSize(paperSize);
    notifyDocumentChanged();
}

void ComposerController::setPlacementMode(int value) {
    value = std::clamp(value, static_cast<int>(core::PlacementMode::Fit),
                       static_cast<int>(core::PlacementMode::Stretch));
    const auto mode = static_cast<core::PlacementMode>(value);
    if (mode == document_.placementMode()) {
        return;
    }
    document_.setPlacementMode(mode);
    notifyDocumentChanged();
}

void ComposerController::setCurrentPage(int value) {
    const int clamped = std::clamp(value, 0, pageCount() - 1);
    if (clamped == currentPage_) {
        return;
    }
    currentPage_ = clamped;
    emit currentPageChanged();
}

void ComposerController::addImages(const QVariantList& urls) {
    const bool wasUsingExplicitImages = sessionAdapter_.usesExplicitImages();
    const QList<core::ImageId> previousExplicitIds = sessionAdapter_.explicitImageIds();
    QStringList paths;
    QStringList errors;
    for (const QVariant& value : urls) {
        const QUrl url = value.toUrl();
        if (url.isLocalFile()) {
            paths.push_back(url.toLocalFile());
        } else {
            errors.push_back(
                QStringLiteral("Apenas imagens armazenadas neste computador são aceitas."));
        }
    }

    const QList<core::ImageId> importedIds = imageSession_.addImages(paths, &errors);
    if (!importedIds.isEmpty()) {
        appendImportedImages(importedIds, previousExplicitIds, wasUsingExplicitImages);
    }
    if (!errors.isEmpty()) {
        emit errorOccurred(errors.constLast());
    }
}

void ComposerController::preserveDocumentImages() {
    sessionAdapter_.freezeCurrentImages();
}

void ComposerController::pasteImages() {
    const QMimeData* mimeData = QGuiApplication::clipboard()->mimeData();
    if (mimeData == nullptr) {
        emit errorOccurred(QStringLiteral("A área de transferência está vazia."));
        return;
    }

    if (mimeData->hasUrls()) {
        QVariantList urls;
        for (const QUrl& url : mimeData->urls()) {
            if (url.isLocalFile()) {
                urls.push_back(url);
            }
        }
        if (!urls.isEmpty()) {
            addImages(urls);
            return;
        }
    }

    if (!mimeData->hasImage()) {
        emit errorOccurred(
            QStringLiteral("A área de transferência não contém uma imagem ou arquivos de imagem."));
        return;
    }

    const QVariant imageData = mimeData->imageData();
    QImage image;
    if (imageData.canConvert<QImage>()) {
        image = imageData.value<QImage>();
    }
    if (image.isNull() && imageData.canConvert<QPixmap>()) {
        image = imageData.value<QPixmap>().toImage();
    }
    if (image.isNull()) {
        emit errorOccurred(QStringLiteral("Não foi possível ler a imagem copiada."));
        return;
    }
    if (!clipboardDirectory_.isValid()) {
        emit errorOccurred(QStringLiteral("Não foi possível preparar a imagem copiada."));
        return;
    }

    const QString path = clipboardDirectory_.filePath(
        QStringLiteral("clipboard-%1.png").arg(++clipboardImageCounter_));
    if (!image.save(path, "PNG")) {
        emit errorOccurred(QStringLiteral("Não foi possível armazenar a imagem copiada."));
        return;
    }

    const bool wasUsingExplicitImages = sessionAdapter_.usesExplicitImages();
    const QList<core::ImageId> previousExplicitIds = sessionAdapter_.explicitImageIds();
    QString error;
    const std::optional<core::ImageId> importedId = imageSession_.addImage(path, &error);
    if (importedId.has_value()) {
        appendImportedImages({*importedId}, previousExplicitIds, wasUsingExplicitImages);
    } else {
        emit errorOccurred(error);
    }
}

void ComposerController::clearImages() {
    imageSession_.clear();
}

void ComposerController::selectImage(int index, bool toggle, bool range) {
    if (index < 0 || index >= imageCount()) {
        return;
    }

    QSet<int> nextSelection = selectedImageIndexes_;
    int nextAnchor = selectionAnchor_;
    if (range && selectionAnchor_ >= 0 && selectionAnchor_ < imageCount()) {
        nextSelection.clear();
        const int first = std::min(selectionAnchor_, index);
        const int last = std::max(selectionAnchor_, index);
        for (int candidate = first; candidate <= last; ++candidate) {
            nextSelection.insert(candidate);
        }
    } else if (toggle) {
        if (nextSelection.contains(index)) {
            nextSelection.remove(index);
        } else {
            nextSelection.insert(index);
        }
        nextAnchor = index;
    } else {
        nextSelection = {index};
        nextAnchor = index;
    }
    setSelectedIndexes(nextSelection, nextAnchor);
    setCurrentPage(index / std::max(1, document_.imagesPerPage()));
}

void ComposerController::selectAllImages() {
    QSet<int> indexes;
    for (int index = 0; index < imageCount(); ++index) {
        indexes.insert(index);
    }
    setSelectedIndexes(indexes, indexes.isEmpty() ? -1 : 0);
}

void ComposerController::clearImageSelection() {
    setSelectedIndexes({}, -1);
}

void ComposerController::duplicateSelectedImages() {
    const QList<int> selectedIndexes = sortedSelectedIndexes();
    if (selectedIndexes.isEmpty()) {
        return;
    }

    sessionAdapter_.freezeCurrentImages();
    const QList<core::ImageId> imageIds = sessionAdapter_.explicitImageIds();
    const QSet<int> selectedSet(selectedIndexes.cbegin(), selectedIndexes.cend());
    QList<core::ImageId> duplicatedIds;
    QSet<int> duplicateIndexes;
    duplicatedIds.reserve(imageIds.size() + selectedIndexes.size());
    for (int index = 0; index < imageIds.size(); ++index) {
        duplicatedIds.push_back(imageIds.at(index));
        if (selectedSet.contains(index)) {
            duplicateIndexes.insert(duplicatedIds.size());
            duplicatedIds.push_back(imageIds.at(index));
        }
    }

    const int firstDuplicate =
        *std::min_element(duplicateIndexes.cbegin(), duplicateIndexes.cend());
    selectedImageIndexes_ = duplicateIndexes;
    selectionAnchor_ = firstDuplicate;
    currentPage_ = firstDuplicate / std::max(1, document_.imagesPerPage());
    sessionAdapter_.useExplicitImages(duplicatedIds);
    emit currentPageChanged();
}

void ComposerController::removeSelectedImages() {
    const QList<int> selectedIndexes = sortedSelectedIndexes();
    if (selectedIndexes.isEmpty()) {
        return;
    }

    sessionAdapter_.freezeCurrentImages();
    const QList<core::ImageId> imageIds = sessionAdapter_.explicitImageIds();
    const QSet<int> selectedSet(selectedIndexes.cbegin(), selectedIndexes.cend());
    QList<core::ImageId> remainingIds;
    remainingIds.reserve(imageIds.size() - selectedIndexes.size());
    for (int index = 0; index < imageIds.size(); ++index) {
        if (!selectedSet.contains(index)) {
            remainingIds.push_back(imageIds.at(index));
        }
    }

    selectedImageIndexes_.clear();
    selectionAnchor_ = -1;
    if (!remainingIds.isEmpty()) {
        const int nearestIndex =
            std::min(selectedIndexes.constFirst(), static_cast<int>(remainingIds.size()) - 1);
        selectedImageIndexes_.insert(nearestIndex);
        selectionAnchor_ = nearestIndex;
        currentPage_ = nearestIndex / std::max(1, document_.imagesPerPage());
    }
    sessionAdapter_.useExplicitImages(remainingIds);
    emit currentPageChanged();
}

void ComposerController::moveImages(int sourceIndex, int targetIndex) {
    if (sourceIndex < 0 || sourceIndex >= imageCount()) {
        return;
    }
    const bool movingSelection = selectedImageIndexes_.contains(sourceIndex);
    const QList<int> selectedIndexes =
        movingSelection ? sortedSelectedIndexes() : QList<int>{sourceIndex};

    sessionAdapter_.freezeCurrentImages();
    const QList<core::ImageId> imageIds = sessionAdapter_.explicitImageIds();
    targetIndex = std::clamp(targetIndex, 0, static_cast<int>(imageIds.size()));
    const QSet<int> selectedSet(selectedIndexes.cbegin(), selectedIndexes.cend());
    QList<core::ImageId> movedIds;
    QList<core::ImageId> remainingIds;
    for (int index = 0; index < imageIds.size(); ++index) {
        if (selectedSet.contains(index)) {
            movedIds.push_back(imageIds.at(index));
        } else {
            remainingIds.push_back(imageIds.at(index));
        }
    }

    const int selectedBeforeTarget = static_cast<int>(
        std::count_if(selectedIndexes.cbegin(), selectedIndexes.cend(),
                      [targetIndex](int selectedIndex) { return selectedIndex < targetIndex; }));
    const int insertionIndex =
        std::clamp(targetIndex - selectedBeforeTarget, 0, static_cast<int>(remainingIds.size()));
    QList<core::ImageId> reorderedIds = remainingIds;
    for (int offset = 0; offset < movedIds.size(); ++offset) {
        reorderedIds.insert(insertionIndex + offset, movedIds.at(offset));
    }
    if (reorderedIds == imageIds) {
        emit thumbnailsChanged();
        return;
    }

    selectedImageIndexes_.clear();
    if (movingSelection) {
        for (int offset = 0; offset < movedIds.size(); ++offset) {
            selectedImageIndexes_.insert(insertionIndex + offset);
        }
        selectionAnchor_ = insertionIndex;
    } else {
        selectionAnchor_ = -1;
    }
    currentPage_ = insertionIndex / std::max(1, document_.imagesPerPage());
    sessionAdapter_.useExplicitImages(reorderedIds);
    emit currentPageChanged();
}

void ComposerController::moveImagesToPosition(int sourceIndex, int positionIndex) {
    if (sourceIndex < 0 || sourceIndex >= imageCount() || positionIndex < 0) {
        emit thumbnailsChanged();
        return;
    }
    positionIndex = std::clamp(positionIndex, 0, imageCount() - 1);
    const int insertionBoundary = positionIndex + (sourceIndex < positionIndex ? 1 : 0);
    moveImages(sourceIndex, insertionBoundary);
}

int ComposerController::imageIndexAtPagePosition(double xRatio, double yRatio,
                                                 bool includeEmptyCell) const {
    if (xRatio < 0.0 || xRatio > 1.0 || yRatio < 0.0 || yRatio > 1.0) {
        return -1;
    }
    const core::PageLayout layout = layoutEngine_.calculate(document_);
    if (!layout.isValid()) {
        return -1;
    }

    const QPointF pagePoint(xRatio * layout.pageWidthMm, yRatio * layout.pageHeightMm);
    for (int cellIndex = 0; cellIndex < static_cast<int>(layout.cellsMm.size()); ++cellIndex) {
        if (!layout.cellsMm.at(static_cast<std::size_t>(cellIndex)).contains(pagePoint)) {
            continue;
        }
        const int imageIndex = currentPage_ * document_.imagesPerPage() + cellIndex;
        if (imageIndex < imageCount()) {
            return imageIndex;
        }
        return includeEmptyCell ? imageCount() : -1;
    }
    return -1;
}

void ComposerController::setGridPreset(int rows, int columns) {
    rows = std::clamp(rows, 1, 20);
    columns = std::clamp(columns, 1, 20);
    if (rows == document_.grid().rows() && columns == document_.grid().columns()) {
        return;
    }
    document_.grid().setRows(rows);
    document_.grid().setColumns(columns);
    notifyDocumentChanged();
}

void ComposerController::printDocument() {
    if (printDialogLoading_) {
        return;
    }
    if (document_.images().empty()) {
        emit errorOccurred(QStringLiteral("Adicione pelo menos uma imagem antes de imprimir."));
        return;
    }
    if (!layoutError().isEmpty()) {
        emit errorOccurred(layoutError());
        return;
    }
    printDialogLoading_ = true;
    emit printDialogLoadingChanged();
    QTimer::singleShot(50, this, [this] { printService_.openPrintDialog(document_); });
}

void ComposerController::activate(const core::ComposerActivationContext& context) {
    selectedImageIndexes_.clear();
    selectionAnchor_ = -1;
    if (context.source == core::ActivationSource::Viewer) {
        // Viewer entry replaces only the document image list. Page/grid/placement settings remain
        // intact, so returning to an adjusted composition never silently loses layout work.
        sessionAdapter_.useExplicitImages(context.imageIds);
    } else {
        sessionAdapter_.useCompleteSession();
    }
    currentPage_ = 0;
    emit currentPageChanged();
}

void ComposerController::paintPreview(QPainter& painter, const QRectF& targetRect) {
    const bool rendered = renderer_.render(painter, document_, targetRect,
                                           core::PageRenderer::Purpose::Preview, currentPage_);
    if (!rendered) {
        qCWarning(logApp) << "Preview layout could not be rendered";
    }
}

void ComposerController::updateMargins(double top, double right, double bottom, double left) {
    document_.page().setMarginsMm(top, right, bottom, left);
    notifyDocumentChanged();
}

void ComposerController::appendImportedImages(const QList<core::ImageId>& importedIds,
                                              const QList<core::ImageId>& previousExplicitIds,
                                              bool wasUsingExplicitImages) {
    if (!wasUsingExplicitImages) {
        sessionAdapter_.useCompleteSession();
        return;
    }
    QList<core::ImageId> nextIds = previousExplicitIds;
    nextIds.append(importedIds);
    sessionAdapter_.useExplicitImages(nextIds);
}

QList<int> ComposerController::sortedSelectedIndexes() const {
    QList<int> indexes(selectedImageIndexes_.cbegin(), selectedImageIndexes_.cend());
    std::sort(indexes.begin(), indexes.end());
    return indexes;
}

void ComposerController::setSelectedIndexes(const QSet<int>& indexes, int anchor) {
    if (selectedImageIndexes_ == indexes && selectionAnchor_ == anchor) {
        return;
    }
    selectedImageIndexes_ = indexes;
    selectionAnchor_ = anchor;
    emit thumbnailsChanged();
}

void ComposerController::normalizeImageSelection() {
    QSet<int> validIndexes;
    for (int index : std::as_const(selectedImageIndexes_)) {
        if (index >= 0 && index < imageCount()) {
            validIndexes.insert(index);
        }
    }
    selectedImageIndexes_ = std::move(validIndexes);
    if (selectionAnchor_ < 0 || selectionAnchor_ >= imageCount()) {
        selectionAnchor_ = -1;
    }
}

void ComposerController::notifyDocumentChanged() {
    const int clampedPage = std::clamp(currentPage_, 0, pageCount() - 1);
    if (clampedPage != currentPage_) {
        currentPage_ = clampedPage;
        emit currentPageChanged();
    }
    emit documentChanged();
}

} // namespace impage::composer
