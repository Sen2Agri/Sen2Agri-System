#include <QFileInfo>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QProcessEnvironment>
#include <QDir>

#include "configuration.hpp"
#include "logger.hpp"

QString getConfigurationFile(const QCoreApplication &app)
{
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
        configFile = QDir::cleanPath(
            QProcessEnvironment::systemEnvironment().value(QStringLiteral("SEN2AGRI_CONFIG_DIR"),
                                                           QStringLiteral("/etc/sen2agri")) +
            QDir::separator() + QStringLiteral("sen2agri.conf"));
    }

    Logger::info(QStringLiteral("Reading settings from %1").arg(configFile));

    if (!QFileInfo::exists(configFile)) {
        throw std::runtime_error(QStringLiteral("Configuration file %1 does not exist, exiting.")
                                     .arg(configFile)
                                     .toStdString());
    }

    return configFile;
}
