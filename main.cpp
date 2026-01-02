#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "javahelper.h"
#include "downloadmanager.h"

#include <QSslSocket>
#include <QDebug>
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);



    QQmlApplicationEngine engine;

    auto *javaHelper = new JavaHelper(&engine);
    auto *backend = new DownloadManager(javaHelper, &engine);

    engine.rootContext()->setContextProperty("backend", backend);




    qDebug() << "Supports SSL:" << QSslSocket::supportsSsl();
    qDebug() << "Available backends:" << QSslSocket::availableBackends();
    qDebug() << "version" <<QSslSocket::sslLibraryVersionString();


    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("qmldownloadManager", "Main");

    return app.exec();
}


