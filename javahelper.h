#ifndef JAVAHELPER_H
#define JAVAHELPER_H

#include <QObject>
#include <QJniObject>

#include <QJniEnvironment>
#include <QStandardPaths>
#include <QFile>
class JavaHelper : public QObject
{
    Q_OBJECT
public:
    explicit JavaHelper(QObject *parent = nullptr);

    Q_INVOKABLE bool writeFile(const QString &path,
                               const QString &fileName,
                               const QString &content);


    Q_INVOKABLE bool makeDirectory(const QString &path);

    Q_INVOKABLE QString convertURL(const QString& uriString);

    bool writeBytesFile(const QString &path,
                                    const QString &fileName,
                                    const QByteArray &data);

};

#endif // JAVAHELPER_H
