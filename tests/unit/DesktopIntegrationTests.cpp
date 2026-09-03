#include "ImpageVersion.h"
#include "platform/desktop/CommandLineParser.h"
#include "platform/desktop/OpenRequest.h"
#include "platform/desktop/SingleInstanceService.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QUrl>

#include <functional>
#include <iostream>

namespace {

int failures = 0;

void check(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}

void drainEventsUntil(const std::function<bool()>& completed) {
    QElapsedTimer timer;
    timer.start();
    while (!completed() && timer.elapsed() < 1500) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
}

} // namespace

int main(int argc, char* argv[]) {
    QCoreApplication application(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("PurrView"));
    QCoreApplication::setApplicationVersion(QString::fromLatin1(IMPAGE_VERSION_STRING));

    QTemporaryDir directory;
    check(directory.isValid(), "temporary integration directory is available");
    const QString firstPath = directory.filePath(QStringLiteral("foto com espaço.png"));
    const QString secondPath = directory.filePath(QStringLiteral("ação.jpeg"));
    for (const QString& path : {firstPath, secondPath}) {
        QFile file(path);
        check(file.open(QIODevice::WriteOnly), "fixture path can be created");
    }

    const auto defaultResult =
        impage::desktop::parseCommandLine({QStringLiteral("purrview")}, directory.path());
    check(defaultResult.valid && defaultResult.request.mode == impage::desktop::OpenMode::Auto &&
              defaultResult.request.files.isEmpty(),
          "CLI without files selects automatic Composer startup");

    const auto viewerResult = impage::desktop::parseCommandLine(
        {QStringLiteral("purrview"), QStringLiteral("--viewer"), QFileInfo(firstPath).fileName(),
         QUrl::fromLocalFile(secondPath).toString()},
        directory.path());
    check(viewerResult.valid && viewerResult.request.mode == impage::desktop::OpenMode::Viewer &&
              viewerResult.request.files == QStringList{firstPath, secondPath},
          "CLI normalizes relative and file URL arguments while preserving order");

    const auto composeResult = impage::desktop::parseCommandLine(
        {QStringLiteral("purrview"), QStringLiteral("--compose"), firstPath}, directory.path());
    check(composeResult.valid && composeResult.request.mode == impage::desktop::OpenMode::Composer,
          "CLI recognizes explicit Composer mode");

    const auto conflictResult = impage::desktop::parseCommandLine(
        {QStringLiteral("purrview"), QStringLiteral("--viewer"), QStringLiteral("--compose")},
        directory.path());
    check(!conflictResult.valid, "CLI rejects conflicting modes");

    const auto helpResult = impage::desktop::parseCommandLine(
        {QStringLiteral("/opt/impage/bin/purrview"), QStringLiteral("--help")}, directory.path());
    check(helpResult.valid && helpResult.showHelp &&
              helpResult.output.startsWith(QStringLiteral("Usage: purrview ")),
          "CLI help has a useful executable name without a GUI application instance");

    const auto versionResult = impage::desktop::parseCommandLine(
        {QStringLiteral("purrview"), QStringLiteral("--version")}, directory.path());
    check(versionResult.valid && versionResult.showVersion &&
              versionResult.output == QStringLiteral("PurrView %1\n").arg(IMPAGE_VERSION_STRING),
          "CLI version is available without initializing the GUI");

    impage::desktop::OpenRequest encoded{.mode = impage::desktop::OpenMode::Viewer,
                                         .files = {firstPath, secondPath},
                                         .activateWindow = true};
    impage::desktop::OpenRequest decoded;
    QString protocolError;
    check(impage::desktop::deserializeOpenRequest(impage::desktop::serializeOpenRequest(encoded),
                                                  &decoded, &protocolError) &&
              decoded.mode == encoded.mode && decoded.files == encoded.files &&
              decoded.activateWindow,
          "OpenRequest JSON protocol round-trips Unicode paths");
    check(!impage::desktop::deserializeOpenRequest(QByteArrayLiteral("not-json"), &decoded,
                                                   &protocolError),
          "OpenRequest protocol rejects malformed payloads");

    const QString socketPath = directory.filePath(QStringLiteral("single-instance.sock"));
    impage::desktop::SingleInstanceService primary(socketPath);
    check(primary.startOrForward({}) ==
              impage::desktop::SingleInstanceService::StartResult::Primary,
          "first process becomes primary instance");
    QList<impage::desktop::OpenRequest> received;
    QObject::connect(&primary, &impage::desktop::SingleInstanceService::requestReceived,
                     &application,
                     [&received](const auto& request) { received.push_back(request); });

    impage::desktop::SingleInstanceService secondary(socketPath);
    check(secondary.startOrForward(encoded) ==
              impage::desktop::SingleInstanceService::StartResult::Forwarded,
          "second process forwards its request");
    drainEventsUntil([&received] { return received.size() == 1; });
    check(received.size() == 1 && received.constFirst().files == encoded.files,
          "primary receives the forwarded request");

    impage::desktop::SingleInstanceService third(socketPath);
    const impage::desktop::OpenRequest composeRequest{.mode = impage::desktop::OpenMode::Composer,
                                                      .files = {secondPath}};
    check(third.startOrForward(composeRequest) ==
              impage::desktop::SingleInstanceService::StartResult::Forwarded,
          "a later process is forwarded sequentially");
    drainEventsUntil([&received] { return received.size() == 2; });
    check(received.size() == 2 && received.constLast().mode == impage::desktop::OpenMode::Composer,
          "primary handles sequential requests in order");

    const QString staleSocketPath = directory.filePath(QStringLiteral("stale.sock"));
    QFile stale(staleSocketPath);
    check(stale.open(QIODevice::WriteOnly), "stale socket fixture can be created");
    stale.close();
    impage::desktop::SingleInstanceService staleRecovery(staleSocketPath);
    check(staleRecovery.startOrForward({}) ==
              impage::desktop::SingleInstanceService::StartResult::Primary,
          "stale socket is removed and startup recovers");

    return failures == 0 ? 0 : 1;
}
