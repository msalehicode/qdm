// DownloadManager.cpp
#include "downloadmanager.h"
#include "downloaditem.h"
#include "javahelper.h"
#include <QUrl>
#include <QFileInfo>
#include <QSaveFile>
#include <QUuid>
#include <QDir>
#include <QRegularExpression>

DownloadModel::DownloadModel(QObject *p):QAbstractListModel(p){}
int DownloadModel::rowCount(const QModelIndex &parent) const{ return parent.isValid()?0:m_items.size(); }
QVariant DownloadModel::data(const QModelIndex &index,int role) const {
    if (!index.isValid()) return {};
    auto *it = qobject_cast<DownloadItem*>(m_items.at(index.row()));
    if (!it) return {};
    switch(role){
    case IdRole: return it->id();
    case UrlRole: return it->url();
    case FileNameRole: return it->fileName();
    case BytesReceivedRole: return it->bytesReceived();
    case TotalBytesRole: return it->totalBytes();
    case StateRole: return it->state();
    }
    return {};
}
QHash<int,QByteArray> DownloadModel::roleNames() const {
    return {{IdRole,"id"},{UrlRole,"url"},{FileNameRole,"fileName"},{BytesReceivedRole,"bytesReceived"},{TotalBytesRole,"totalBytes"},{StateRole,"state"}};
}
void DownloadModel::addItem(QObject *item){ beginInsertRows({}, m_items.size(), m_items.size()); m_items.append(item); endInsertRows(); }
QObject* DownloadModel::itemAt(int i) const { return m_items.value(i); }
QObject* DownloadModel::findById(const QString &id) const {
    for (auto *o : m_items) if (auto *it=qobject_cast<DownloadItem*>(o); it && it->id()==id) return it;
    return nullptr;
}
QList<QObject*> DownloadModel::items() const { return m_items; }
void DownloadModel::notifyChanged(QObject *item) {
    int idx = m_items.indexOf(item);
    if (idx>=0) emit dataChanged(index(idx,0), index(idx,0));
}

DownloadManager::DownloadManager(JavaHelper *javaHelper, QObject *parent)
    : QObject(parent), m_java(javaHelper) {}

DownloadModel* DownloadManager::items() const { return const_cast<DownloadModel*>(&m_model); }

QString DownloadManager::makeFileNameFromUrl(const QUrl &url) const {
    QString base = QFileInfo(url.path()).fileName();
    if (base.isEmpty()) base = "download.bin";
    // sanitize filename
    base.replace(QRegularExpression("[\\\\/:*?\"<>|]"), "_");
    return base;
}

void DownloadManager::ensureCollectionDir(const QString &collectionName) {
#ifdef Q_OS_ANDROID
    m_collectionPath = "/storage/emulated/0/Download/" + collectionName;
    m_java->makeDirectory(m_collectionPath);
#else
    m_collectionPath = QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).filePath(collectionName);
    QDir().mkpath(m_collectionPath);
#endif
}

void DownloadManager::queueLinks(const QString &collectionName, const QString &inputText) {
    ensureCollectionDir(collectionName);
    QStringList tokens = inputText.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    for (const QString &t : tokens) {
        QUrl url(t);
        if (!url.isValid() || url.scheme().isEmpty()) continue;
        QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QString fn = makeFileNameFromUrl(url);
        auto *item = new DownloadItem(id, url, fn, &m_model);
        m_model.addItem(item);
    }
}

void DownloadManager::maybeStartNext() {
    // Count active
    int activeCount = 0;
    for (auto *o : m_model.items()) {
        auto *it = qobject_cast<DownloadItem*>(o);
        if (it && it->state()=="Downloading") activeCount++;
    }
    if (activeCount >= m_maxConcurrent) return;

    for (auto *o : m_model.items()) {
        auto *it = qobject_cast<DownloadItem*>(o);
        if (!it) continue;
        if (it->state()=="Queued") { startItem(it, false); break; }
    }
}

void DownloadManager::start() { maybeStartNext(); }
void DownloadManager::startItem(DownloadItem *item, bool resume)
{
    item->setState(DownloadItem::Downloading);
    m_model.notifyChanged(item);

    QNetworkRequest req(item->url());

    // If you want resume support, you need to track how many bytes were already written.
    // For simplicity, we start fresh each time here.
    auto *reply = m_nam.get(req);

    Active a;
    a.reply = reply;
    a.downloaded = 0;
    a.total = -1;
    m_active.insert(item->id(), a);

    connect(reply, &QNetworkReply::downloadProgress, this, [this, item](qint64 rec, qint64 tot){
        item->setTotalBytes(tot);
        item->setBytesReceived(rec);
        m_model.notifyChanged(item);
    });

    connect(reply, &QIODevice::readyRead, this, [this, item](){
        auto &a = m_active[item->id()];
        if (!a.reply) return;
        QByteArray chunk = a.reply->readAll();
        if (!chunk.isEmpty()) {
#ifdef Q_OS_ANDROID
            // Write chunk via JavaHelper into public Downloads/<collection>/<filename>
            m_java->writeBytesFile(m_collectionPath, item->fileName(), chunk);
#else
            // On desktop, just use QFile
            QFile file(m_collectionPath + "/" + item->fileName());
            if (file.open(QIODevice::Append)) {
                file.write(chunk);
                file.close();
            }
#endif
            a.downloaded += chunk.size();
            item->setBytesReceived(a.downloaded);
            m_model.notifyChanged(item);
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, item](){
        auto a = m_active.take(item->id());
        if (!a.reply) return;

        if (a.reply->error() == QNetworkReply::NoError) {
            item->setState(DownloadItem::Completed);
        } else {
            item->setState(DownloadItem::Failed);
        }
        a.reply->deleteLater();
        m_model.notifyChanged(item);
        maybeStartNext();
    });
}

void DownloadManager::pause(const QString &id) {
    auto *obj = qobject_cast<DownloadItem*>(m_model.findById(id));
    if (!obj) return;
    auto a = m_active.take(id);
    if (a.reply) { a.reply->abort(); a.reply->deleteLater(); }
    if (a.file) { a.file->flush(); a.file->close(); a.file->deleteLater(); }
    obj->setState(DownloadItem::Paused);
    m_model.notifyChanged(obj);
    maybeStartNext();
}

void DownloadManager::resume(const QString &id) {
    auto *obj = qobject_cast<DownloadItem*>(m_model.findById(id));
    if (!obj) return;
    startItem(obj, true);
}

void DownloadManager::cancel(const QString &id) {
    auto *obj = qobject_cast<DownloadItem*>(m_model.findById(id));
    if (!obj) return;
    auto a = m_active.take(id);
    if (a.reply) { a.reply->abort(); a.reply->deleteLater(); }
    if (a.file) { a.file->close(); a.file->remove(); a.file->deleteLater(); }
    obj->setState(DownloadItem::Canceled);
    m_model.notifyChanged(obj);
    maybeStartNext();
}

void DownloadManager::pauseAll() {
    for (auto *o : m_model.items())
        if (auto *it = qobject_cast<DownloadItem*>(o); it && it->state()=="Downloading")
            pause(it->id());
}
void DownloadManager::resumeAll() {
    for (auto *o : m_model.items())
        if (auto *it = qobject_cast<DownloadItem*>(o); it && it->state()=="Paused")
            resume(it->id());
}
