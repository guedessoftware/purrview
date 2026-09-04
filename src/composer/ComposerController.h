#pragma once

#include "composer/ComposerSessionAdapter.h"
#include "core/document/DocumentModel.h"
#include "core/image/ImageSession.h"
#include "core/layout/LayoutEngine.h"
#include "core/navigation/ComposerActivationContext.h"
#include "core/render/PageRenderer.h"
#include "platform/printing/PrintService.h"

#include <QObject>
#include <QPainter>
#include <QRectF>
#include <QSet>
#include <QTemporaryDir>
#include <QVariantList>

namespace purrview::composer {

class ComposerController : public QObject {
    Q_OBJECT
    Q_PROPERTY(int rows READ rows WRITE setRows NOTIFY documentChanged)
    Q_PROPERTY(int columns READ columns WRITE setColumns NOTIFY documentChanged)
    Q_PROPERTY(double marginTop READ marginTop WRITE setMarginTop NOTIFY documentChanged)
    Q_PROPERTY(double marginRight READ marginRight WRITE setMarginRight NOTIFY documentChanged)
    Q_PROPERTY(double marginBottom READ marginBottom WRITE setMarginBottom NOTIFY documentChanged)
    Q_PROPERTY(double marginLeft READ marginLeft WRITE setMarginLeft NOTIFY documentChanged)
    Q_PROPERTY(double horizontalSpacing READ horizontalSpacing WRITE setHorizontalSpacing NOTIFY
                   documentChanged)
    Q_PROPERTY(
        double verticalSpacing READ verticalSpacing WRITE setVerticalSpacing NOTIFY documentChanged)
    Q_PROPERTY(bool landscape READ landscape WRITE setLandscape NOTIFY documentChanged)
    Q_PROPERTY(int paperSize READ paperSize WRITE setPaperSize NOTIFY documentChanged)
    Q_PROPERTY(QString paperName READ paperName NOTIFY documentChanged)
    Q_PROPERTY(int placementMode READ placementMode WRITE setPlacementMode NOTIFY documentChanged)
    Q_PROPERTY(int imageCount READ imageCount NOTIFY documentChanged)
    Q_PROPERTY(QVariantList imageThumbnails READ imageThumbnails NOTIFY thumbnailsChanged)
    Q_PROPERTY(int selectedImageCount READ selectedImageCount NOTIFY thumbnailsChanged)
    Q_PROPERTY(int pageCount READ pageCount NOTIFY documentChanged)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(QString layoutError READ layoutError NOTIFY documentChanged)
    Q_PROPERTY(bool printDialogLoading READ printDialogLoading NOTIFY printDialogLoadingChanged)
    Q_PROPERTY(bool canPasteImages READ canPasteImages NOTIFY clipboardChanged)
    Q_PROPERTY(purrview::core::ImageSession* imageSession READ imageSession CONSTANT)
    Q_PROPERTY(double pageWidthMm READ pageWidthMm NOTIFY documentChanged)
    Q_PROPERTY(double pageHeightMm READ pageHeightMm NOTIFY documentChanged)

  public:
    explicit ComposerController(core::ImageSession& imageSession, QObject* parent = nullptr);

    [[nodiscard]] int rows() const;
    [[nodiscard]] int columns() const;
    [[nodiscard]] double marginTop() const;
    [[nodiscard]] double marginRight() const;
    [[nodiscard]] double marginBottom() const;
    [[nodiscard]] double marginLeft() const;
    [[nodiscard]] double horizontalSpacing() const;
    [[nodiscard]] double verticalSpacing() const;
    [[nodiscard]] bool landscape() const;
    [[nodiscard]] int paperSize() const;
    [[nodiscard]] QString paperName() const;
    [[nodiscard]] int placementMode() const;
    [[nodiscard]] int imageCount() const;
    [[nodiscard]] QVariantList imageThumbnails() const;
    [[nodiscard]] int selectedImageCount() const;
    [[nodiscard]] int pageCount() const;
    [[nodiscard]] int currentPage() const;
    [[nodiscard]] QString layoutError() const;
    [[nodiscard]] bool printDialogLoading() const;
    [[nodiscard]] bool canPasteImages() const;
    [[nodiscard]] core::ImageSession* imageSession();
    [[nodiscard]] double pageWidthMm() const;
    [[nodiscard]] double pageHeightMm() const;

    void setRows(int value);
    void setColumns(int value);
    void setMarginTop(double value);
    void setMarginRight(double value);
    void setMarginBottom(double value);
    void setMarginLeft(double value);
    void setHorizontalSpacing(double value);
    void setVerticalSpacing(double value);
    void setLandscape(bool value);
    void setPaperSize(int value);
    void setPlacementMode(int value);
    void setCurrentPage(int value);

    Q_INVOKABLE void addImages(const QVariantList& urls);
    void preserveDocumentImages();
    Q_INVOKABLE void pasteImages();
    Q_INVOKABLE void clearImages();
    Q_INVOKABLE void selectImage(int index, bool toggle = false, bool range = false);
    Q_INVOKABLE void selectAllImages();
    Q_INVOKABLE void clearImageSelection();
    Q_INVOKABLE void duplicateSelectedImages();
    Q_INVOKABLE void removeSelectedImages();
    Q_INVOKABLE void moveImages(int sourceIndex, int targetIndex);
    Q_INVOKABLE void moveImagesToPosition(int sourceIndex, int positionIndex);
    Q_INVOKABLE int imageIndexAtPagePosition(double xRatio, double yRatio,
                                             bool includeEmptyCell = false) const;
    Q_INVOKABLE void setGridPreset(int rows, int columns);
    Q_INVOKABLE void printDocument();

    void activate(const core::ComposerActivationContext& context);

    void paintPreview(QPainter& painter, const QRectF& targetRect);

  signals:
    void documentChanged();
    void errorOccurred(const QString& message);
    void printFinished();
    void printDialogLoadingChanged();
    void clipboardChanged();
    void currentPageChanged();
    void thumbnailsChanged();

  private:
    void updateMargins(double top, double right, double bottom, double left);
    void notifyDocumentChanged();
    void appendImportedImages(const QList<core::ImageId>& importedIds,
                              const QList<core::ImageId>& previousExplicitIds,
                              bool wasUsingExplicitImages);
    [[nodiscard]] QList<int> sortedSelectedIndexes() const;
    void setSelectedIndexes(const QSet<int>& indexes, int anchor);
    void normalizeImageSelection();

    core::DocumentModel document_;
    core::ImageSession& imageSession_;
    ComposerSessionAdapter sessionAdapter_;
    core::LayoutEngine layoutEngine_;
    core::PageRenderer renderer_;
    platform::PrintService printService_;
    bool printDialogLoading_ = false;
    int currentPage_ = 0;
    QSet<int> selectedImageIndexes_;
    int selectionAnchor_ = -1;
    QTemporaryDir clipboardDirectory_;
    quint64 clipboardImageCounter_ = 0;
};

} // namespace purrview::composer
