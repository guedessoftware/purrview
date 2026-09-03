#include "core/document/DocumentModel.h"
#include "core/layout/ImagePlacementEngine.h"
#include "core/layout/LayoutEngine.h"
#include "core/layout/Units.h"
#include "core/render/PageRenderer.h"

#include <QCoreApplication>
#include <QImage>
#include <QPainter>
#include <QTemporaryDir>

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

void checkNear(double actual, double expected, const char* description) {
    check(std::abs(actual - expected) < 0.0001, description);
}

void testLayoutOneByOne() {
    impage::core::DocumentModel document;
    document.grid().setRows(1);
    document.grid().setColumns(1);
    document.page().setMarginsMm(10.0, 10.0, 10.0, 10.0);

    const impage::core::PageLayout layout = impage::core::LayoutEngine().calculate(document);
    check(layout.isValid(), "1x1 layout is valid");
    check(layout.cellsMm.size() == 1, "1x1 layout contains one cell");
    checkNear(layout.cellsMm[0].x(), 10.0, "1x1 cell starts at left margin");
    checkNear(layout.cellsMm[0].width(), 190.0, "1x1 cell has expected width");
    checkNear(layout.cellsMm[0].height(), 277.0, "1x1 cell has expected height");
}

void testLayoutTwoByTwo() {
    impage::core::DocumentModel document;
    document.grid().setRows(2);
    document.grid().setColumns(2);
    document.grid().setHorizontalSpacingMm(4.0);
    document.grid().setVerticalSpacingMm(6.0);

    const impage::core::PageLayout layout = impage::core::LayoutEngine().calculate(document);
    check(layout.cellsMm.size() == 4, "2x2 layout contains four cells");
    checkNear(layout.cellsMm[0].width(), 93.0, "2x2 cell width includes spacing");
    checkNear(layout.cellsMm[0].height(), 135.5, "2x2 cell height includes spacing");
    checkNear(layout.cellsMm[3].x(), 107.0, "last cell x is correct");
    checkNear(layout.cellsMm[3].y(), 151.5, "last cell y is correct");
}

void testLandscapeLayout() {
    impage::core::DocumentModel document;
    document.page().setOrientation(impage::core::PageModel::Orientation::Landscape);
    document.grid().setRows(3);
    document.grid().setColumns(4);

    const impage::core::PageLayout layout = impage::core::LayoutEngine().calculate(document);
    checkNear(layout.pageWidthMm, 297.0, "landscape page width is A4 long edge");
    checkNear(layout.pageHeightMm, 210.0, "landscape page height is A4 short edge");
    check(layout.cellsMm.size() == 12, "3x4 layout contains twelve cells");
}

void testPaperSizes() {
    impage::core::PageModel page;
    check(page.paperSize() == impage::core::PageModel::PaperSize::A4,
          "A4 is the default paper size");
    checkNear(page.widthMm(), 210.0, "default A4 portrait width");
    checkNear(page.heightMm(), 297.0, "default A4 portrait height");

    page.setPaperSize(impage::core::PageModel::PaperSize::A3);
    checkNear(page.widthMm(), 297.0, "A3 portrait width");
    checkNear(page.heightMm(), 420.0, "A3 portrait height");
    page.setOrientation(impage::core::PageModel::Orientation::Landscape);
    checkNear(page.widthMm(), 420.0, "A3 landscape width");
    checkNear(page.heightMm(), 297.0, "A3 landscape height");

    page.setOrientation(impage::core::PageModel::Orientation::Portrait);
    page.setPaperSize(impage::core::PageModel::PaperSize::A5);
    checkNear(page.widthMm(), 148.0, "A5 portrait width");
    checkNear(page.heightMm(), 210.0, "A5 portrait height");
    page.setPaperSize(impage::core::PageModel::PaperSize::Letter);
    checkNear(page.widthMm(), 215.9, "Letter portrait width");
    checkNear(page.heightMm(), 279.4, "Letter portrait height");
    page.setPaperSize(impage::core::PageModel::PaperSize::Legal);
    checkNear(page.widthMm(), 215.9, "Legal portrait width");
    checkNear(page.heightMm(), 355.6, "Legal portrait height");
    page.setPaperSize(impage::core::PageModel::PaperSize::Photo10x15);
    checkNear(page.widthMm(), 100.0, "10x15 photo portrait width");
    checkNear(page.heightMm(), 150.0, "10x15 photo portrait height");
}

void testInvalidLayout() {
    impage::core::DocumentModel document;
    document.page().setMarginsMm(120.0, 120.0, 120.0, 120.0);
    const impage::core::PageLayout layout = impage::core::LayoutEngine().calculate(document);
    check(!layout.isValid(), "oversized margins produce invalid layout");
    check(layout.cellsMm.empty(), "invalid layout has no cells");

    document.page().setMarginsMm(10.0, 10.0, 10.0, 10.0);
    document.grid().setRows(0);
    check(!impage::core::LayoutEngine().calculate(document).isValid(),
          "zero rows produce invalid layout");
}

void testDocumentPagination() {
    impage::core::DocumentModel document;
    document.grid().setRows(2);
    document.grid().setColumns(2);
    check(document.imagesPerPage() == 4, "2x2 grid has capacity for four images per page");
    check(document.pageCount() == 1, "empty document still exposes one preview page");

    for (int index = 0; index < 8; ++index) {
        document.addImage({.source = QString::number(index), .pixelSize = QSize(10, 10)});
    }
    check(document.pageCount() == 2, "eight images in a 2x2 grid create two pages");

    document.grid().setColumns(3);
    check(document.pageCount() == 2, "pagination responds to grid capacity changes");
}

void testImagePlacement() {
    const impage::core::ImagePlacementEngine engine;
    const QRectF squareCell(0.0, 0.0, 100.0, 100.0);

    const auto fit =
        engine.calculate(QSizeF(200.0, 100.0), squareCell, impage::core::PlacementMode::Fit);
    checkNear(fit.targetRect.x(), 0.0, "fit landscape x");
    checkNear(fit.targetRect.y(), 25.0, "fit landscape is vertically centered");
    checkNear(fit.targetRect.width(), 100.0, "fit landscape width");
    checkNear(fit.targetRect.height(), 50.0, "fit landscape preserves ratio");

    const auto fill =
        engine.calculate(QSizeF(200.0, 100.0), squareCell, impage::core::PlacementMode::Fill);
    check(fill.targetRect == squareCell, "fill covers the full cell");
    checkNear(fill.sourceRect.x(), 50.0, "fill crops landscape image horizontally");
    checkNear(fill.sourceRect.width(), 100.0, "fill crop width is correct");

    const auto portraitFit =
        engine.calculate(QSizeF(100.0, 200.0), squareCell, impage::core::PlacementMode::Fit);
    checkNear(portraitFit.targetRect.x(), 25.0, "portrait fit is horizontally centered");
    checkNear(portraitFit.targetRect.height(), 100.0, "portrait fit height");

    const auto stretch =
        engine.calculate(QSizeF(200.0, 100.0), squareCell, impage::core::PlacementMode::Stretch);
    check(stretch.targetRect == squareCell, "stretch covers the cell");
    checkNear(stretch.sourceRect.width(), 200.0, "stretch uses the complete source");
}

void testUnits() {
    checkNear(impage::core::units::millimetersToPixels(25.4, 300.0), 300.0,
              "25.4 mm is 300 px at 300 dpi");
    checkNear(impage::core::units::pixelsToMillimeters(600.0, 600.0), 25.4,
              "600 px is 25.4 mm at 600 dpi");
}

void testPageRenderer() {
    QTemporaryDir directory;
    check(directory.isValid(), "temporary rendering directory is available");
    const QString imagePath = directory.filePath(QStringLiteral("source.png"));
    QImage source(200, 100, QImage::Format_ARGB32_Premultiplied);
    source.fill(QColor(210, 30, 45));
    check(source.save(imagePath), "renderer fixture can be saved");
    const QString secondImagePath = directory.filePath(QStringLiteral("second.png"));
    QImage secondSource(100, 200, QImage::Format_ARGB32_Premultiplied);
    secondSource.fill(QColor(30, 180, 80));
    check(secondSource.save(secondImagePath), "second renderer fixture can be saved");

    impage::core::DocumentModel document;
    document.grid().setRows(1);
    document.grid().setColumns(1);
    document.addImage({.source = imagePath, .pixelSize = source.size()});
    document.addImage({.source = secondImagePath, .pixelSize = secondSource.size()});

    QImage page(210, 297, QImage::Format_ARGB32_Premultiplied);
    page.fill(Qt::transparent);
    QPainter painter(&page);
    impage::core::PageRenderer renderer;
    const bool rendered =
        renderer.render(painter, document, page.rect(), impage::core::PageRenderer::Purpose::Print);
    painter.end();

    check(rendered, "renderer accepts a valid document");
    check(page.pixelColor(105, 148) == QColor(210, 30, 45),
          "renderer paints the first page image in the cell center");
    check(page.pixelColor(5, 5) == QColor(Qt::white), "renderer keeps page margins white");

    page.fill(Qt::transparent);
    painter.begin(&page);
    const bool secondPageRendered = renderer.render(painter, document, page.rect(),
                                                    impage::core::PageRenderer::Purpose::Print, 1);
    painter.end();
    check(secondPageRendered, "renderer accepts the second document page");
    check(page.pixelColor(105, 148) == QColor(30, 180, 80),
          "renderer offsets images when painting the second page");

    impage::core::DocumentModel rotatedDocument;
    rotatedDocument.grid().setRows(1);
    rotatedDocument.grid().setColumns(1);
    rotatedDocument.addImage(
        {.source = imagePath, .pixelSize = source.size(), .rotationDegrees = 90});
    page.fill(Qt::transparent);
    painter.begin(&page);
    const bool rotatedRendered = renderer.render(painter, rotatedDocument, page.rect(),
                                                 impage::core::PageRenderer::Purpose::Print);
    painter.end();
    check(rotatedRendered, "renderer accepts a non-destructive image rotation");
    check(page.pixelColor(105, 30) == QColor(210, 30, 45),
          "renderer uses rotated dimensions and pixels in the composition");
}

} // namespace

int main(int argc, char* argv[]) {
    QCoreApplication application(argc, argv);
    testLayoutOneByOne();
    testLayoutTwoByTwo();
    testLandscapeLayout();
    testPaperSizes();
    testInvalidLayout();
    testDocumentPagination();
    testImagePlacement();
    testUnits();
    testPageRenderer();

    if (failures == 0) {
        std::cout << "All core tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
