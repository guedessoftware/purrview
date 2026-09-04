#include "platform/desktop/SingleInstanceService.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocalSocket>
#include <QLockFile>
#include <QStandardPaths>

#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

namespace {

bool preparePrivateRuntimeDirectory(const QString& path) {
    QFileInfo info(path);
    if (info.exists()) {
        if (!info.isDir() || info.isSymbolicLink()) {
            return false;
        }
#if defined(Q_OS_UNIX)
        if (info.ownerId() != static_cast<uint>(geteuid())) {
            return false;
        }
#endif
    } else if (!QDir().mkpath(path)) {
        return false;
    }

    if (!QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                                         QFileDevice::ExeOwner)) {
        return false;
    }
    info.refresh();
    if (!info.exists() || !info.isDir() || info.isSymbolicLink()) {
        return false;
    }
#if defined(Q_OS_UNIX)
    if (info.ownerId() != static_cast<uint>(geteuid())) {
        return false;
    }
#endif
    return true;
}

} // namespace

namespace purrview::desktop {

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
    if (socketPath_.isEmpty()) {
        errorString_ = QStringLiteral("Não foi possível preparar o diretório privado de execução.");
        return StartResult::Error;
    }
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
#if defined(Q_OS_UNIX)
        const QByteArray userKey = QByteArray::number(static_cast<qulonglong>(geteuid()));
#else
        const QByteArray userKey =
            QCryptographicHash::hash(
                QStandardPaths::writableLocation(QStandardPaths::HomeLocation).toUtf8(),
                QCryptographicHash::Sha256)
                .toHex()
                .first(12);
#endif
        runtimeDirectory = QDir::temp().filePath(
            QStringLiteral("purrview-runtime-%1").arg(QString::fromLatin1(userKey)));
        if (!preparePrivateRuntimeDirectory(runtimeDirectory)) {
            return {};
        }
    }
    return QDir(runtimeDirectory)
        .filePath(QStringLiteral("io.github.guedessoftware.PurrView.sock"));
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
    constexpr qsizetype maximumMessageBytes = qsizetype{1024} * 1024;
    QByteArray buffer = socket->property("purrviewBuffer").toByteArray();
    buffer += socket->readAll();
    if (buffer.size() > maximumMessageBytes) {
        emit protocolError(QStringLiteral("Mensagem de abertura excedeu o limite permitido."));
        socket->disconnectFromServer();
        return;
    }
    const qsizetype newline = buffer.indexOf('\n');
    if (newline < 0) {
        socket->setProperty("purrviewBuffer", buffer);
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

} // namespace purrview::desktop
