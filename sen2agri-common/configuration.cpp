#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QProcessEnvironment>

#include "configuration.hpp"

QString getConfigurationFile(const QCoreApplication &app)
{
    const auto &appName = QCoreApplication::applicationName();
    if (appName.isEmpty()) {
        throw std::runtime_error(
            "Please call QCoreApplication::setApplicationName() with a valid name.");
    }

    QCommandLineParser parser;
    QCommandLineOption configFileOption(QStringLiteral("f"), QStringLiteral("Use this config file"),
                                        QStringLiteral("config file"));
    parser.addOption(configFileOption);
    parser.addHelpOption();
    parser.process(app);

    QString configFile;
    if (parser.isSet(configFileOption)) {
        configFile = parser.value(configFileOption);
    } else {
        configFile = QProcessEnvironment::systemEnvironment().value(
            QStringLiteral("SEN2AGRI_CONFIG_DIR"), QStringLiteral("/etc/sen2agri"));
        configFile += QStringLiteral("/%1.conf").arg(appName);
    }

    return configFile;
}
