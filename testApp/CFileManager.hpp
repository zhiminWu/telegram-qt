#ifndef FILETRANSFERMANAGER_HPP
#define FILETRANSFERMANAGER_HPP

#include <QObject>
#include <QHash>
#include <QPixmap>

#include <TelegramNamespace.hpp>

class CTelegramCore;

struct FileInfo
{
    struct PeerPictureData
    {
        PeerPictureData() { }
        PeerPictureData(const Telegram::Peer &p, Telegram::PeerPictureSize s);
        Telegram::Peer peer;
        Telegram::PeerPictureSize size;
    };

    FileInfo() :
        m_peerPicture(nullptr)
    {
    }

    FileInfo(const FileInfo &fileInfo) :
        m_data(fileInfo.m_data),
        m_picture(fileInfo.m_picture),
        m_peerPicture(nullptr)
    {
        if (fileInfo.m_peerPicture) {
            m_peerPicture = new PeerPictureData(*fileInfo.m_peerPicture);
        }
    }

    ~FileInfo()
    {
        if (m_peerPicture) {
            delete m_peerPicture;
        }
    }

    FileInfo &operator=(const FileInfo &fileInfo);

    bool hasPeerPicture() const { return m_peerPicture; }
    Telegram::Peer peer() const;
    QPixmap picture() const { return m_picture; }
    Telegram::PeerPictureSize pictureSize() const;

    void setPeerPictureRequestData(const Telegram::Peer &peer, Telegram::PeerPictureSize size);


    void addData(const QByteArray &newData);
    void completeDownload(const Telegram::RemoteFile &result);

private:
    QByteArray m_data;
    QPixmap m_picture;
    PeerPictureData *m_peerPicture;
};

class CFileManager : public QObject
{
    Q_OBJECT
public:
    explicit CFileManager(CTelegramCore *backend, QObject *parent = nullptr);

    QString requestFile(const Telegram::RemoteFile &file);
    QString requestPeerPicture(const Telegram::Peer &peer, Telegram::PeerPictureSize size = Telegram::PeerPictureSize::Small);

    QPixmap getPicture(const QString &uniqueId) const;

signals:
    void requestComplete(const QString &uniqueId);

protected slots:
    void onFilePartReceived(quint32 requestId, const QByteArray &data, const QString &mimeType, quint32 offset, quint32 totalSize);
    void onFileRequestFinished(quint32 requestId, const Telegram::RemoteFile &requestResult);

protected:
    void unqueuePendingRequest();

    CTelegramCore *m_backend;
    QHash<QString,FileInfo> m_files; // UniqueId to file info
    QHash<quint32,QString> m_requestToStringId; // Request number to UniqueId

    QHash<QString,FileInfo> m_pendingRequests;

};

#endif // FILETRANSFERMANAGER_HPP
