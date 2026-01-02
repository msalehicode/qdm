// DownloadItem.cpp
#include "downloaditem.h"

DownloadItem::DownloadItem(const QString &id, const QUrl &url, const QString &fileName, QObject *parent)
    : QObject(parent), m_id(id), m_url(url), m_fileName(fileName) {}

QString DownloadItem::id() const { return m_id; }
QString DownloadItem::url() const { return m_url.toString(); }
QString DownloadItem::fileName() const { return m_fileName; }
qint64 DownloadItem::bytesReceived() const { return m_bytesReceived; }
qint64 DownloadItem::totalBytes() const { return m_totalBytes; }
QString DownloadItem::state() const {
    switch (m_state) {
    case Queued: return "Queued";
    case Downloading: return "Downloading";
    case Paused: return "Paused";
    case Completed: return "Completed";
    case Failed: return "Failed";
    case Canceled: return "Canceled";
    }
    return "Unknown";
}
void DownloadItem::setBytesReceived(qint64 v){ if(m_bytesReceived!=v){ m_bytesReceived=v; emit changed(); } }
void DownloadItem::setTotalBytes(qint64 v){ if(m_totalBytes!=v){ m_totalBytes=v; emit changed(); } }
void DownloadItem::setState(State s){ if(m_state!=s){ m_state=s; emit changed(); } }
