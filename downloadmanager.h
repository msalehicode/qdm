// DownloadManager.h
#pragma once
#include <QObject>
#include <QAbstractListModel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QFile>

class JavaHelper;

class DownloadModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles { IdRole = Qt::UserRole + 1, UrlRole, FileNameRole, BytesReceivedRole, TotalBytesRole, StateRole };
    Q_ENUM(Roles)

    explicit DownloadModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addItem(QObject *item);
    QObject* itemAt(int i) const;
    QObject* findById(const QString &id) const;

    QList<QObject*> items() const;

    void notifyChanged(QObject *item);

private:
    QList<QObject*> m_items;
};

class DownloadManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(DownloadModel* items READ items CONSTANT)

public:
    explicit DownloadManager(JavaHelper *javaHelper, QObject *parent = nullptr);
    DownloadModel* items() const;

    Q_INVOKABLE void queueLinks(const QString &collectionName, const QString &inputText);
    Q_INVOKABLE void start();
    Q_INVOKABLE void pause(const QString &id);
    Q_INVOKABLE void resume(const QString &id);
    Q_INVOKABLE void cancel(const QString &id);
    Q_INVOKABLE void pauseAll();
    Q_INVOKABLE void resumeAll();

private:
    struct Active {
        QPointer<QNetworkReply> reply;
        QFile *file = nullptr;
        qint64 downloaded = 0;
        qint64 total = -1;
    };
    JavaHelper *m_java = nullptr;
    DownloadModel m_model;
    QNetworkAccessManager m_nam;
    QString m_collectionPath;
    int m_maxConcurrent = 3;

    QHash<QString, Active> m_active; // key: item id

    QString makeFileNameFromUrl(const QUrl &url) const;
    void ensureCollectionDir(const QString &collectionName);
    void maybeStartNext();
    void startItem(class DownloadItem *item, bool resume = false);
    void finalizeItem(class DownloadItem *item, QFile *file);
    void failItem(class DownloadItem *item, const QString &reason);
};
