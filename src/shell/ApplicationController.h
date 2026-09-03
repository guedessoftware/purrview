#pragma once

#include "shell/ApplicationContext.h"
#include "shell/ModuleManager.h"
#include "shell/OpenRequestHandler.h"

#include <QObject>
#include <QVariantList>

namespace impage::shell {

class ApplicationController : public QObject {
    Q_OBJECT
    Q_PROPERTY(impage::shell::ApplicationContext* context READ context CONSTANT)
    Q_PROPERTY(impage::shell::ModuleManager* modules READ modules CONSTANT)
    Q_PROPERTY(QString windowTitle READ windowTitle NOTIFY windowTitleChanged)
    Q_PROPERTY(int savedWindowWidth READ savedWindowWidth CONSTANT)
    Q_PROPERTY(int savedWindowHeight READ savedWindowHeight CONSTANT)
    Q_PROPERTY(bool savedWindowMaximized READ savedWindowMaximized CONSTANT)
    Q_PROPERTY(QString lastOpenRequestError READ lastOpenRequestError NOTIFY openRequestError)

  public:
    explicit ApplicationController(QObject* parent = nullptr);

    [[nodiscard]] ApplicationContext* context();
    [[nodiscard]] ModuleManager* modules();
    [[nodiscard]] QString windowTitle() const;
    [[nodiscard]] int savedWindowWidth() const;
    [[nodiscard]] int savedWindowHeight() const;
    [[nodiscard]] bool savedWindowMaximized() const;
    [[nodiscard]] QString lastOpenRequestError() const;
    [[nodiscard]] bool handleOpenRequest(const desktop::OpenRequest& request);
    Q_INVOKABLE void openComposer(const QVariantList& images = {});
    Q_INVOKABLE void openViewer(const QVariantList& images = {});
    Q_INVOKABLE void openDroppedInComposer(const QVariantList& images);
    Q_INVOKABLE void openDroppedInViewer(const QVariantList& images);
    Q_INVOKABLE void goBack();
    Q_INVOKABLE void saveWindowState(int width, int height, bool maximized);
    Q_INVOKABLE void clearOpenRequestError();

  signals:
    void windowActivationRequested();
    void windowTitleChanged();
    void openRequestError(const QString& message);

  private:
    [[nodiscard]] desktop::OpenRequest requestFromUrls(const QVariantList& images,
                                                       desktop::OpenMode mode) const;
    void connectViewerTitle();

    ApplicationContext context_;
    ModuleManager moduleManager_;
    OpenRequestHandler openRequestHandler_;
    QString lastOpenRequestError_;
};

} // namespace impage::shell
