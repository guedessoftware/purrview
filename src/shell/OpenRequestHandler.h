#pragma once

#include "platform/desktop/OpenRequest.h"

#include <QObject>

namespace impage::shell {

class ModuleManager;

class OpenRequestHandler final : public QObject {
    Q_OBJECT

  public:
    explicit OpenRequestHandler(ModuleManager& modules, QObject* parent = nullptr);

    [[nodiscard]] bool handle(const desktop::OpenRequest& request);

  signals:
    void errorOccurred(const QString& message);
    void requestHandled(const impage::desktop::OpenRequest& request);

  private:
    ModuleManager& modules_;
};

} // namespace impage::shell
