#include "platform/desktop/OpenRequest.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

#include <utility>

namespace impage::desktop {
namespace {

QString modeName(OpenMode mode) {
    switch (mode) {
    case OpenMode::Viewer:
        return QStringLiteral("viewer");
    case OpenMode::Composer:
        return QStringLiteral("composer");
    case OpenMode::Auto:
        return QStringLiteral("auto");
    }
    return QStringLiteral("auto");
}

bool parseMode(const QString& value, OpenMode* mode) {
    if (value == QStringLiteral("auto")) {
        *mode = OpenMode::Auto;
        return true;
    }
    if (value == QStringLiteral("viewer")) {
        *mode = OpenMode::Viewer;
        return true;
    }
    if (value == QStringLiteral("composer")) {
        *mode = OpenMode::Composer;
        return true;
    }
    return false;
}

} // namespace

QByteArray serializeOpenRequest(const OpenRequest& request) {
    QJsonArray files;
    for (const QString& file : request.files) {
        files.push_back(file);
    }
    return QJsonDocument(QJsonObject{{QStringLiteral("version"), 1},
                                     {QStringLiteral("mode"), modeName(request.mode)},
                                     {QStringLiteral("files"), files},
                                     {QStringLiteral("activateWindow"), request.activateWindow}})
        .toJson(QJsonDocument::Compact);
}

bool deserializeOpenRequest(const QByteArray& payload, OpenRequest* request, QString* error) {
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error != nullptr) {
            *error =
                QStringLiteral("Mensagem de abertura inválida: %1").arg(parseError.errorString());
        }
        return false;
    }

    const QJsonObject object = document.object();
    if (object.value(QStringLiteral("version")).toInt() != 1 ||
        !object.value(QStringLiteral("files")).isArray()) {
        if (error != nullptr) {
            *error = QStringLiteral("Versão ou campos da mensagem de abertura inválidos.");
        }
        return false;
    }

    OpenRequest decoded;
    if (!parseMode(object.value(QStringLiteral("mode")).toString(), &decoded.mode)) {
        if (error != nullptr) {
            *error = QStringLiteral("Modo de abertura inválido.");
        }
        return false;
    }
    for (const QJsonValue& value : object.value(QStringLiteral("files")).toArray()) {
        if (!value.isString()) {
            if (error != nullptr) {
                *error = QStringLiteral("A lista de arquivos contém um item inválido.");
            }
            return false;
        }
        decoded.files.push_back(value.toString());
    }
    decoded.activateWindow = object.value(QStringLiteral("activateWindow")).toBool(true);
    *request = std::move(decoded);
    return true;
}

QStringList normalizeFileArguments(const QStringList& arguments, const QString& workingDirectory) {
    QStringList files;
    files.reserve(arguments.size());
    for (const QString& argument : arguments) {
        const QUrl url = QUrl::fromUserInput(argument, workingDirectory, QUrl::AssumeLocalFile);
        if (url.isLocalFile()) {
            files.push_back(QDir::cleanPath(QFileInfo(url.toLocalFile()).absoluteFilePath()));
        } else {
            files.push_back(argument);
        }
    }
    return files;
}

QVariantList fileUrls(const QStringList& paths) {
    QVariantList urls;
    urls.reserve(paths.size());
    for (const QString& path : paths) {
        urls.push_back(QUrl::fromLocalFile(path));
    }
    return urls;
}

} // namespace impage::desktop
