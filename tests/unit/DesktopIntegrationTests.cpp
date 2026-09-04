#include "PurrViewVersion.h"
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
    QCoreApplication::setApplicationVersion(QString::fromLatin1(PURRVIEW_VERSION_STRING));

    QTemporaryDir directory;
    check(directory.isValid(), "temporary integration directory is available");
    const QString firstPath = directory.filePath(QStringLiteral("foto com espaço.png"));
    const QString secondPath = directory.filePath(QStringLiteral("ação.jpeg"));
    for (const QString& path : {firstPath, secondPath}) {
        QFile file(path);
        check(file.open(QIODevice::WriteOnly), "fixture path can be created");
    }

    const auto defaultResult =
        purrview::desktop::parseCommandLine({QStringLiteral("purrview")}, directory.path());
    check(defaultResult.valid && defaultResult.request.mode == purrview::desktop::OpenMode::Auto &&
              defaultResult.request.files.isEmpty(),
          "CLI without files selects automatic Composer startup");

    const auto viewerResult = purrview::desktop::parseCommandLine(
        {QStringLiteral("purrview"), QStringLiteral("--viewer"), QFileInfo(firstPath).fileName(),
         QUrl::fromLocalFile(secondPath).toString()},
        directory.path());
    check(viewerResult.valid && viewerResult.request.mode == purrview::desktop::OpenMode::Viewer &&
              viewerResult.request.files == QStringList{firstPath, secondPath},
          "CLI normalizes relative and file URL arguments while preserving order");

    const auto composeResult = purrview::desktop::parseCommandLine(
        {QStringLiteral("purrview"), QStringLiteral("--compose"), firstPath}, directory.path());
    check(composeResult.valid &&
              composeResult.request.mode == purrview::desktop::OpenMode::Composer,
          "CLI recognizes explicit Composer mode");

    const auto conflictResult = purrview::desktop::parseCommandLine(
        {QStringLiteral("purrview"), QStringLiteral("--viewer"), QStringLiteral("--compose")},
        directory.path());
    check(!conflictResult.valid, "CLI rejects conflicting modes");

    const auto helpResult = purrview::desktop::parseCommandLine(
        {QStringLiteral("/opt/purrview/bin/purrview"), QStringLiteral("--help")}, directory.path());
    check(helpResult.valid && helpResult.showHelp &&
              helpResult.output.startsWith(QStringLiteral("Usage: purrview ")),
          "CLI help has a useful executable name without a GUI application instance");

    const auto versionResult = purrview::desktop::parseCommandLine(
        {QStringLiteral("purrview"), QStringLiteral("--version")}, directory.path());
    check(versionResult.valid && versionResult.showVersion &&
              versionResult.output == QStringLiteral("PurrView %1\n").arg(PURRVIEW_VERSION_STRING),
          "CLI version is available without initializing the GUI");

    purrview::desktop::OpenRequest encoded{.mode = purrview::desktop::OpenMode::Viewer,
                                           .files = {firstPath, secondPath},
                                           .activateWindow = true};
    purrview::desktop::OpenRequest decoded;
    QString protocolError;
    check(purrview::desktop::deserializeOpenRequest(
              purrview::desktop::serializeOpenRequest(encoded), &decoded, &protocolError) &&
              decoded.mode == encoded.mode && decoded.files == encoded.files &&
              decoded.activateWindow,
          "OpenRequest JSON protocol round-trips Unicode paths");
    check(!purrview::desktop::deserializeOpenRequest(QByteArrayLiteral("not-json"), &decoded,
                                                     &protocolError),
          "OpenRequest protocol rejects malformed payloads");
    purrview::desktop::OpenRequest tooManyFiles;
    tooManyFiles.files.fill(QStringLiteral("image.png"),
                            purrview::desktop::MaximumOpenRequestFiles + 1);
    check(!purrview::desktop::deserializeOpenRequest(
              purrview::desktop::serializeOpenRequest(tooManyFiles), &decoded, &protocolError),
          "OpenRequest protocol rejects excessive file counts");
    purrview::desktop::OpenRequest longPath;
    longPath.files = {
        QString(purrview::desktop::MaximumOpenRequestPathLength + 1, QLatin1Char('a'))};
    check(!purrview::desktop::deserializeOpenRequest(
              purrview::desktop::serializeOpenRequest(longPath), &decoded, &protocolError),
          "OpenRequest protocol rejects excessive path lengths");
    check(!purrview::desktop::deserializeOpenRequest(
              purrview::desktop::serializeOpenRequest(encoded), nullptr, &protocolError),
          "OpenRequest protocol rejects a missing destination safely");

    const QString socketPath = directory.filePath(QStringLiteral("single-instance.sock"));
    purrview::desktop::SingleInstanceService primary(socketPath);
    check(primary.startOrForward({}) ==
              purrview::desktop::SingleInstanceService::StartResult::Primary,
          "first process becomes primary instance");
    QList<purrview::desktop::OpenRequest> received;
    QObject::connect(&primary, &purrview::desktop::SingleInstanceService::requestReceived,
                     &application,
                     [&received](const auto& request) { received.push_back(request); });

    purrview::desktop::SingleInstanceService secondary(socketPath);
    check(secondary.startOrForward(encoded) ==
              purrview::desktop::SingleInstanceService::StartResult::Forwarded,
          "second process forwards its request");
    drainEventsUntil([&received] { return received.size() == 1; });
    check(received.size() == 1 && received.constFirst().files == encoded.files,
          "primary receives the forwarded request");

    purrview::desktop::SingleInstanceService third(socketPath);
    const purrview::desktop::OpenRequest composeRequest{
        .mode = purrview::desktop::OpenMode::Composer, .files = {secondPath}};
    check(third.startOrForward(composeRequest) ==
              purrview::desktop::SingleInstanceService::StartResult::Forwarded,
          "a later process is forwarded sequentially");
    drainEventsUntil([&received] { return received.size() == 2; });
    check(received.size() == 2 &&
              received.constLast().mode == purrview::desktop::OpenMode::Composer,
          "primary handles sequential requests in order");

    const QString staleSocketPath = directory.filePath(QStringLiteral("stale.sock"));
    QFile stale(staleSocketPath);
    check(stale.open(QIODevice::WriteOnly), "stale socket fixture can be created");
    stale.close();
    purrview::desktop::SingleInstanceService staleRecovery(staleSocketPath);
    check(staleRecovery.startOrForward({}) ==
              purrview::desktop::SingleInstanceService::StartResult::Primary,
          "stale socket is removed and startup recovers");

    return failures == 0 ? 0 : 1;
}
