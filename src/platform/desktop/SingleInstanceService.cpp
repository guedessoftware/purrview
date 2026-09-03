#include "platform/desktop/SingleInstanceService.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocalSocket>
#include <QLockFile>
#include <QStandardPaths>

namespace impage::desktop {

SingleInstanceService::SingleInstanceService(QString socketPath, QObject* parent)
    : QObject(parent),
      socketPath_(socketPath.isEmpty() ? defaultSocketPath() : std::move(socketPath)) {
    server_.setSocketOptions(QLocalServer::UserAccessOption);
    connect(&server_, &QLocalServer::newConnection, this,
            &SingleInstanceService::acceptConnections);
}

SingleInstanceService::~SingleInstanceService() {
    if (primary_) {
        server_.close();
        QLocalServer::removeServer(socketPath_);
    }
}

SingleInstanceService::StartResult SingleInstanceService::startOrForward(const OpenRequest& request,
                                                                         int timeoutMilliseconds) {
    if (primary_) {
        emit requestReceived(request);
        return StartResult::Primary;
    }
    if (forwardToPrimary(request, timeoutMilliseconds)) {
        return StartResult::Forwarded;
    }

    QLockFile startupLock(socketPath_ + QStringLiteral(".lock"));
    startupLock.setStaleLockTime(10000);
    if (!startupLock.tryLock(timeoutMilliseconds)) {
        if (forwardToPrimary(request, timeoutMilliseconds)) {
            return StartResult::Forwarded;
        }
        errorString_ = QStringLiteral("Outra instância está iniciando, mas não respondeu.");
        return StartResult::Error;
    }

    if (forwardToPrimary(request, qMin(timeoutMilliseconds, 250))) {
        return StartResult::Forwarded;
    }
    QLocalServer::removeServer(socketPath_);
    if (!server_.listen(socketPath_)) {
        errorString_ = server_.errorString();
        return StartResult::Error;
    }
    primary_ = true;
    errorString_.clear();
    return StartResult::Primary;
}

bool SingleInstanceService::isPrimary() const {
    return primary_;
}

QString SingleInstanceService::socketPath() const {
    return socketPath_;
}

QString SingleInstanceService::errorString() const {
    return errorString_;
}

QString SingleInstanceService::defaultSocketPath() {
    QString runtimeDirectory = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (runtimeDirectory.isEmpty()) {
        const QByteArray userKey =
            QCryptographicHash::hash(
                QStandardPaths::writableLocation(QStandardPaths::HomeLocation).toUtf8(),
                QCryptographicHash::Sha256)
                .toHex()
                .first(12);
        runtimeDirectory = QDir::temp().filePath(
            QStringLiteral("impage-runtime-%1").arg(QString::fromLatin1(userKey)));
    }
    QDir().mkpath(runtimeDirectory);
    QFile::setPermissions(runtimeDirectory,
                          QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    return QDir(runtimeDirectory).filePath(QStringLiteral("io.github.impage.Impage.sock"));
}

bool SingleInstanceService::forwardToPrimary(const OpenRequest& request, int timeoutMilliseconds) {
    QLocalSocket socket;
    socket.connectToServer(socketPath_, QIODevice::WriteOnly);
    if (!socket.waitForConnected(timeoutMilliseconds)) {
        return false;
    }
    const QByteArray payload = serializeOpenRequest(request) + '\n';
    if (socket.write(payload) != payload.size() ||
        !socket.waitForBytesWritten(timeoutMilliseconds)) {
        errorString_ = socket.errorString();
        return false;
    }
    socket.disconnectFromServer();
    return true;
}

void SingleInstanceService::acceptConnections() {
    while (server_.hasPendingConnections()) {
        QLocalSocket* socket = server_.nextPendingConnection();
        if (socket == nullptr) {
            continue;
        }
        socket->setParent(this);
        connect(socket, &QLocalSocket::readyRead, this, [this, socket] { readRequest(socket); });
        connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
        readRequest(socket);
    }
}

void SingleInstanceService::readRequest(QLocalSocket* socket) {
    QByteArray buffer = socket->property("impageBuffer").toByteArray();
    buffer += socket->readAll();
    if (buffer.size() > 1024 * 1024) {
        emit protocolError(QStringLiteral("Mensagem de abertura excedeu o limite permitido."));
        socket->disconnectFromServer();
        return;
    }
    const qsizetype newline = buffer.indexOf('\n');
    if (newline < 0) {
        socket->setProperty("impageBuffer", buffer);
        return;
    }

    OpenRequest request;
    QString error;
    if (deserializeOpenRequest(buffer.first(newline), &request, &error)) {
        emit requestReceived(request);
    } else {
        emit protocolError(error);
    }
    socket->disconnectFromServer();
}

} // namespace impage::desktop
