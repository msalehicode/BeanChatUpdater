#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QQmlContext>
#include "updater/updater.h"
#include <QIcon>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/icons/BeanChat.png"));
    QCoreApplication::setOrganizationName("orgBeanChat");
    QCoreApplication::setApplicationName("appBeanChat");

    Updater updater;

    qmlRegisterUncreatableType<Updater>("BeanChatUpdater", 1, 0, "UpdaterState", "Enum Only");

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("updater", &updater);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("BeanChatUpdater", "Main");

    updater.start();

    return app.exec();
}
