#include "javahelper.h"
#include <QDebug>

JavaHelper::JavaHelper(QObject *parent)
    : QObject(parent)
{}

bool JavaHelper::writeFile(const QString &path,
                           const QString &fileName,
                           const QString &content)
{
#ifdef Q_OS_ANDROID
    bool ok = QJniObject::callStaticMethod<jboolean>(
        "org/qtproject/qmldownloadmanager/MyJavaClass",  // Java class path
        "writeTextFile",                           // method name
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z", // signature
        QJniObject::fromString(path).object<jstring>(),
        QJniObject::fromString(fileName).object<jstring>(),
        QJniObject::fromString(content).object<jstring>()
        );

    return ok;
#else
    qWarning() << "writeFile only works on Android";
    return false;
#endif
}


bool JavaHelper::makeDirectory(const QString &path)
{
#ifdef Q_OS_ANDROID
    bool ok = QJniObject::callStaticMethod<jboolean>(
        "org/qtproject/qmldownloadmanager/MyJavaClass",  // Java class
        "makeDirectory",                           // method name
        "(Ljava/lang/String;)Z",                   // signature
        QJniObject::fromString(path).object<jstring>()
        );

    return ok;
#else
    qWarning() << "makeDirectory only works on Android";
    return false;
#endif
}


QString JavaHelper::convertURL(const QString &uriString)
{
#ifdef Q_OS_ANDROID
    // Uri.parse(...)
    QJniObject uri = QJniObject::callStaticObjectMethod(
        "android/net/Uri",
        "parse",
        "(Ljava/lang/String;)Landroid/net/Uri;",
        QJniObject::fromString(uriString).object<jstring>()
        );

    // QtNative.activity()
    QJniObject activity =
        QJniObject::callStaticObjectMethod(
            "org/qtproject/qt/android/QtNative",
            "activity",
            "()Landroid/app/Activity;"
            );

    if (!activity.isValid() || !uri.isValid())
        return {};

    // activity.getContentResolver()
    QJniObject resolver = activity.callObjectMethod(
        "getContentResolver",
        "()Landroid/content/ContentResolver;"
        );

    // resolver.openInputStream(uri)
    QJniObject inputStream = resolver.callObjectMethod(
        "openInputStream",
        "(Landroid/net/Uri;)Ljava/io/InputStream;",
        uri.object()
        );

    if (!inputStream.isValid())
        return {};

    QString outPath =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + "/picked_input.bin";

    QFile out(outPath);
    out.open(QIODevice::WriteOnly);

    QJniEnvironment env;
    jbyteArray buffer = env->NewByteArray(4096);

    while (true) {
        jint read = inputStream.callMethod<jint>(
            "read", "([B)I", buffer);

        if (read <= 0)
            break;

        QByteArray chunk(read, 0);
        env->GetByteArrayRegion(
            buffer, 0, read,
            reinterpret_cast<jbyte*>(chunk.data()));

        out.write(chunk);
    }

    out.close();
    inputStream.callMethod<void>("close", "()V");

    return outPath;
#else
    return {};
#endif
}

bool JavaHelper::writeBytesFile(const QString &path, const QString &fileName, const QByteArray &data)
{
#ifdef Q_OS_ANDROID
    QJniEnvironment env;
    jbyteArray arr = env->NewByteArray(data.size());
    env->SetByteArrayRegion(arr, 0, data.size(),
                            reinterpret_cast<const jbyte*>(data.constData()));

    jboolean ok = QJniObject::callStaticMethod<jboolean>(
        "org/qtproject/qmldownloadmanager/MyJavaClass",
        "writeBytesFile",   // <-- match Java method
        "(Ljava/lang/String;Ljava/lang/String;[B)Z", // <-- signature for byte[]
        QJniObject::fromString(path).object<jstring>(),
        QJniObject::fromString(fileName).object<jstring>(),
        arr
        );

    env->DeleteLocalRef(arr);
    return ok;
#else
    return false;
#endif
}

