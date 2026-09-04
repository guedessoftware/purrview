#include "platform/printing/PrintService.h"

#include <QDialog>
#include <QElapsedTimer>
#include <QLoggingCategory>
#include <QMarginsF>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>

#include <exception>

Q_LOGGING_CATEGORY(logPrint, "purrview.print")

namespace purrview::platform {

namespace {
QPageSize pageSizeFor(const core::PageModel& page) {
    switch (page.paperSize()) {
    case core::PageModel::PaperSize::A4:
        return QPageSize(QPageSize::A4);
    case core::PageModel::PaperSize::A3:
        return QPageSize(QPageSize::A3);
    case core::PageModel::PaperSize::A5:
        return QPageSize(QPageSize::A5);
    case core::PageModel::PaperSize::Letter:
        return QPageSize(QPageSize::Letter);
    case core::PageModel::PaperSize::Legal:
        return QPageSize(QPageSize::Legal);
    case core::PageModel::PaperSize::Photo10x15:
        return QPageSize(QSizeF(100.0, 150.0), QPageSize::Millimeter,
                         QStringLiteral("Foto 10 × 15 cm"), QPageSize::ExactMatch);
    }
    return QPageSize(QPageSize::A4);
}
} // namespace

PrintService::PrintService(core::PageRenderer& renderer, QObject* parent)
    : QObject(parent), renderer_(renderer) {}

PrintService::~PrintService() = default;

void PrintService::openPrintDialog(const core::DocumentModel& document) {
    pendingDocument_ = document;

    QElapsedTimer timer;
    timer.start();
    try {
        ensurePrinter();
    } catch (const std::exception&) {
        emit printFailed(QStringLiteral("Não foi possível consultar as impressoras disponíveis."));
        return;
    }

    configurePage(pendingDocument_);

    if (!dialog_) {
        dialog_ = std::make_unique<QPrintDialog>(printer_.get());
        dialog_->setWindowTitle(QStringLiteral("Imprimir composição"));
        dialog_->setWindowModality(Qt::ApplicationModal);
        connect(dialog_.get(), &QDialog::finished, this, &PrintService::handleDialogFinished);
    }

    qCDebug(logPrint) << "Print dialog prepared in" << timer.elapsed() << "ms";
    dialog_->open();
    emit dialogOpened();
}

void PrintService::ensurePrinter() {
    if (!printer_) {
        // Construct and use the platform print backend on the GUI thread. This
        // avoids eagerly initializing CUPS for users who only view images.
        printer_ = std::make_unique<QPrinter>(QPrinter::HighResolution);
    }
}

void PrintService::configurePage(const core::DocumentModel& document) {
    const QPageLayout::Orientation orientation =
        document.page().orientation() == core::PageModel::Orientation::Portrait
            ? QPageLayout::Portrait
            : QPageLayout::Landscape;
    const QPageLayout pageLayout(pageSizeFor(document.page()), orientation, QMarginsF(),
                                 QPageLayout::Millimeter);
    printer_->setPageLayout(pageLayout);
    printer_->setFullPage(true);
}

void PrintService::handleDialogFinished(int result) {
    if (result == QDialog::Accepted) {
        renderPendingDocument();
    }
}

void PrintService::renderPendingDocument() {
    // Keep the document geometry authoritative if printer properties changed it.
    configurePage(pendingDocument_);

    QPainter painter;
    if (!painter.begin(printer_.get())) {
        emit printFailed(QStringLiteral("Não foi possível iniciar a impressão selecionada."));
        return;
    }

    const QRectF pageRect(0.0, 0.0, printer_->width(), printer_->height());
    const int pageCount = pendingDocument_.pageCount();

    bool rendered = true;
    for (int pageIndex = 0; pageIndex < pageCount; ++pageIndex) {
        if (pageIndex > 0 && !printer_->newPage()) {
            rendered = false;
            break;
        }
        if (!renderer_.render(painter, pendingDocument_, pageRect,
                              core::PageRenderer::Purpose::Print, pageIndex)) {
            rendered = false;
            break;
        }
    }
    painter.end();

    if (!rendered) {
        emit printFailed(QStringLiteral("Não foi possível montar a página para impressão."));
        return;
    }
    emit printFinished();
}

} // namespace purrview::platform
