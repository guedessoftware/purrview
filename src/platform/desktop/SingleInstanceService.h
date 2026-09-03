#pragma once

#include "platform/desktop/OpenRequest.h"

#include <QLocalServer>
#include <QObject>
#include <QString>

namespace impage::desktop {

class SingleInstanceService final : public QObject {
    Q_OBJECT

  public:
    enum class StartResult { Primary, Forwarded, Error };

    explicit SingleInstanceService(QString socketPath = {}, QObject* parent = nullptr);
    ~SingleInstanceService() override;

    [[nodiscard]] StartResult startOrForward(const OpenRequest& request,
                                             int timeoutMilliseconds = 1500);
    [[nodiscard]] bool isPrimary() const;
    [[nodiscard]] QString socketPath() const;
    [[nodiscard]] QString errorString() const;
    [[nodiscard]] static QString defaultSocketPath();

  signals:
    void requestReceived(const impage::desktop::OpenRequest& request);
    void protocolError(const QString& message);

  private:
    [[nodiscard]] bool forwardToPrimary(const OpenRequest& request, int timeoutMilliseconds);
    void acceptConnections();
    void readRequest(QLocalSocket* socket);

    QString socketPath_;
    QString errorString_;
    QLocalServer server_;
    bool primary_ = false;
};

} // namespace impage::desktop
