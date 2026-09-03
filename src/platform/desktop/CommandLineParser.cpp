#include "platform/desktop/CommandLineParser.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <QRegularExpression>

namespace impage::desktop {

namespace {

QString standaloneHelpText(QCommandLineParser& parser, const QStringList& arguments) {
    QString help = parser.helpText();
    const QString executable = arguments.isEmpty() ? QStringLiteral("purrview")
                                                   : QFileInfo(arguments.constFirst()).fileName();
    const QString displayName = executable.isEmpty() ? QStringLiteral("purrview") : executable;
    help.replace(QRegularExpression(QStringLiteral("\\A([^\\n:]+:\\s*)\\S+")),
                 QStringLiteral("\\1") + displayName);
    return help;
}

} // namespace

CommandLineResult parseCommandLine(const QStringList& arguments, const QString& workingDirectory) {
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Visualize imagens ou monte páginas prontas para impressão."));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption viewer(QStringLiteral("viewer"),
                                    QStringLiteral("Abrir os arquivos no visualizador."));
    const QCommandLineOption compose(QStringLiteral("compose"),
                                     QStringLiteral("Adicionar os arquivos ao compositor."));
    parser.addOption(viewer);
    parser.addOption(compose);
    parser.addPositionalArgument(QStringLiteral("imagens"),
                                 QStringLiteral("Imagens PNG, JPEG, WebP, BMP, GIF, TIFF, AVIF, "
                                                "HEIF/HEIC ou ICNS."),
                                 QStringLiteral("[imagens...]"));

    CommandLineResult result;
    if (!parser.parse(arguments)) {
        result.valid = false;
        result.output =
            parser.errorText() + QLatin1Char('\n') + standaloneHelpText(parser, arguments);
        return result;
    }
    if (parser.isSet(viewer) && parser.isSet(compose)) {
        result.valid = false;
        result.output =
            QStringLiteral("As opções --viewer e --compose não podem ser usadas juntas.\n") +
            standaloneHelpText(parser, arguments);
        return result;
    }

    result.showHelp = parser.isSet(QStringLiteral("help"));
    result.showVersion = parser.isSet(QStringLiteral("version"));
    if (result.showHelp) {
        result.output = standaloneHelpText(parser, arguments);
        return result;
    }
    if (result.showVersion) {
        result.output = QStringLiteral("%1 %2\n").arg(QCoreApplication::applicationName(),
                                                      QCoreApplication::applicationVersion());
        return result;
    }

    result.request.mode = parser.isSet(viewer)    ? OpenMode::Viewer
                          : parser.isSet(compose) ? OpenMode::Composer
                                                  : OpenMode::Auto;
    result.request.files = normalizeFileArguments(parser.positionalArguments(), workingDirectory);
    return result;
}

} // namespace impage::desktop
