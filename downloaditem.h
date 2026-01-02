// DownloadItem.h
#pragma once
#include <QObject>
#include <QUrl>

class DownloadItem : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString url READ url CONSTANT)
    Q_PROPERTY(QString fileName READ fileName NOTIFY changed)
    Q_PROPERTY(qint64 bytesReceived READ bytesReceived NOTIFY changed)
    Q_PROPERTY(qint64 totalBytes READ totalBytes NOTIFY changed)
    Q_PROPERTY(QString state READ state NOTIFY changed)

public:
    enum State { Queued, Downloading, Paused, Completed, Failed, Canceled };
    Q_ENUM(State)

    explicit DownloadItem(const QString &id, const QUrl &url, const QString &fileName, QObject *parent = nullptr);

    QString id() const;
    QString url() const;
    QString fileName() const;
    qint64 bytesReceived() const;
    qint64 totalBytes() const;
    QString state() const;

    void setBytesReceived(qint64 v);
    void setTotalBytes(qint64 v);
    void setState(State s);

signals:
    void changed();

private:
    QString m_id;
    QUrl m_url;
    QString m_fileName;
    qint64 m_bytesReceived = 0;
    qint64 m_totalBytes = 0;
    State m_state = Queued;
};
