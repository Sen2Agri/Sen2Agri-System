#include <iostream>

#include <unistd.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "maindialog.h"

using namespace std;

ConfigModel loadModel()
{
    QFile configFile("/etc/sen2agri.conf");
    if (!configFile.open(QIODevice::ReadOnly)) {
        return { "30", "true" };
    }

    return ConfigModel::deserialize(QJsonDocument::fromJson(configFile.readAll()));
}

int displayUI(QApplication &a)
{
    auto model = loadModel();

    MainDialog w(model);
    w.show();

    return a.exec();
}

int main(int argc, char *argv[])
{
    QCommandLineParser parser;
    QStringList argList;
    argList.reserve(argc);
    for (int i = 0; i < argc; i++) {
        argList << argv[i];
    }

    parser.addPositionalArgument(QStringLiteral("command"), QStringLiteral("command"));
    parser.addPositionalArgument(QStringLiteral("file"), QStringLiteral("file"));
    parser.process(argList);

    const auto &args = parser.positionalArguments();
    if (args.length() == 0) {
        QApplication a(argc, argv);
        return displayUI(a);
    }

    if (args.at(0) != QLatin1String("write")) {
        QTextStream(stderr) << "invalid arguments";
        return EXIT_FAILURE;
    }

    if (args.length() != 1) {
        QTextStream(stderr) << "invalid arguments";
        return EXIT_FAILURE;
    }

    QFile in;
    if (!in.open(STDIN_FILENO, QIODevice::ReadOnly)) {
        QTextStream(stderr) << "cannot open input";
        return EXIT_FAILURE;
    }

    QJsonParseError error;
    const auto &document = QJsonDocument::fromJson(in.readAll(), &error);

    QFile out("/etc/sen2agri.conf");
    if (!out.open(QIODevice::WriteOnly)) {
        QTextStream(stderr) << "cannot open output file";
        return EXIT_FAILURE;
    }
    out.write(document.toJson());
}
