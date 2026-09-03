#pragma once

#include "platform/desktop/OpenRequest.h"

#include <QString>
#include <QStringList>

namespace impage::desktop {

struct CommandLineResult {
    OpenRequest request;
    bool valid = true;
    bool showHelp = false;
    bool showVersion = false;
    QString output;
};

[[nodiscard]] CommandLineResult parseCommandLine(const QStringList& arguments,
                                                 const QString& workingDirectory);

} // namespace impage::desktop
