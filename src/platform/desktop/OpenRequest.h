#pragma once

#include <QByteArray>
#include <QMetaType>
#include <QStringList>
#include <QVariantList>

namespace impage::desktop {

enum class OpenMode { Auto, Viewer, Composer };

struct OpenRequest {
    OpenMode mode = OpenMode::Auto;
    QStringList files;
    bool activateWindow = true;
};

[[nodiscard]] QByteArray serializeOpenRequest(const OpenRequest& request);
[[nodiscard]] bool deserializeOpenRequest(const QByteArray& payload, OpenRequest* request,
                                          QString* error = nullptr);
[[nodiscard]] QStringList normalizeFileArguments(const QStringList& arguments,
                                                 const QString& workingDirectory);
[[nodiscard]] QVariantList fileUrls(const QStringList& paths);

} // namespace impage::desktop

Q_DECLARE_METATYPE(impage::desktop::OpenRequest)
