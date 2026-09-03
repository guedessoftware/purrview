#pragma once

#include "core/document/DocumentModel.h"
#include "core/render/PageRenderer.h"

#include <QObject>

#include <future>
#include <memory>

class QPrinter;
class QPrintDialog;

namespace impage::platform {

class PrintService : public QObject {
    Q_OBJECT

  public:
    explicit PrintService(core::PageRenderer& renderer, QObject* parent = nullptr);
    ~PrintService() override;

    void openPrintDialog(const core::DocumentModel& document);

  signals:
    void dialogOpened();
    void printFinished();
    void printFailed(const QString& message);

  private:
    void ensurePrinter();
    void configurePage(const core::DocumentModel& document);
    void handleDialogFinished(int result);
    void renderPendingDocument();

    core::PageRenderer& renderer_;
    core::DocumentModel pendingDocument_;
    std::future<std::unique_ptr<QPrinter>> printerFuture_;
    std::unique_ptr<QPrinter> printer_;
    std::unique_ptr<QPrintDialog> dialog_;
};

} // namespace impage::platform
