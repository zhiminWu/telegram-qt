#include "CFileManager.hpp"

#include <CTelegramCore.hpp>
#include "Debug.hpp"

#include <QDebug>
#include <QDir>
#include <QFile>

static const int c_maxConcurrentDownloads = 8;

CFileManager::CFileManager(CTelegramCore *backend, QObject *parent) :
    QObject(parent),
    m_backend(backend)
{
    connect(m_backend, SIGNAL(filePartReceived(quint32,QByteArray,QString,quint32,quint32)),
            SLOT(onFilePartReceived(quint32,QByteArray,QString,quint32,quint32)));
    connect(m_backend, SIGNAL(fileRequestFinished(quint32,Telegram::RemoteFile)),
            this, SLOT(onFileRequestFinished(quint32,Telegram::RemoteFile)));
}

QString CFileManager::requestFile(const Telegram::RemoteFile &file)
{
    const QString key = file.getUniqueId();
    if (m_files.contains(key)) {
        return key; // Already requested
    }
    FileInfo requestFileInfo;
    m_files.insert(key, requestFileInfo);

    if (m_requestToStringId.count() >= c_maxConcurrentDownloads) {
        m_pendingRequests.insert(key, file);
        qDebug() << Q_FUNC_INFO << "Request delayed" << key;
        return key;
    }

    const quint32 requestId = m_backend->requestFile(&file);
    if (!requestId) {
        qDebug() << Q_FUNC_INFO << "File is not available" << key;
        return QString();
    }
    m_requestToStringId.insert(requestId, key);
    return key;
}

QString CFileManager::requestPeerPicture(const Telegram::Peer &peer, Telegram::PeerPictureSize size)
{
    Telegram::RemoteFile file;
    if (!getPeerPictureFileInfo(peer, &file, size)) {
        return QString();
    }

    const QString key = requestFile(file);
    qDebug() << Q_FUNC_INFO << peer << key;
    if (key.isEmpty()) {
        return QString();
    }

    m_files[key].setPeerPictureRequestData(peer, size);
    return key;
}

QByteArray CFileManager::getData(const QString &uniqueId) const
{
    if (!m_files.contains(uniqueId)) {
        return QByteArray();
    }
    const FileInfo &info = m_files.value(uniqueId);
    return info.data();
}

QPixmap CFileManager::getPicture(const QString &uniqueId) const
{
    if (!m_files.contains(uniqueId)) {
        return QPixmap();
    }
    const FileInfo &info = m_files.value(uniqueId);
    return info.picture();
}

bool CFileManager::getPeerPictureFileInfo(const Telegram::Peer &peer, Telegram::RemoteFile *file, Telegram::PeerPictureSize size) const
{
    switch (peer.type) {
    case Telegram::Peer::User:
    {
        Telegram::UserInfo info;
        m_backend->getUserInfo(&info, peer.id);
        return info.getPeerPicture(file, size);
    }
    case Telegram::Peer::Chat:
    case Telegram::Peer::Channel:
    {
        Telegram::ChatInfo info;
        m_backend->getChatInfo(&info, peer);
        return info.getPeerPicture(file, size);
    }
    default:
        break;
    }
    return false;
}

void CFileManager::onFilePartReceived(quint32 requestId, const QByteArray &data, const QString &mimeType, quint32 offset, quint32 totalSize)
{
    Q_UNUSED(mimeType)
    Q_UNUSED(offset)
    Q_UNUSED(totalSize)

    const QString key = m_requestToStringId.value(requestId);
    if (key.isEmpty()) {
        // Unknown requestId
        return;
    }
    if (!m_files.contains(key)) {
        // Unknown key. Assert?
        return;
    }

    m_files[key].addData(data);
}

void CFileManager::onFileRequestFinished(quint32 requestId, const Telegram::RemoteFile &requestResult)
{
    Q_UNUSED(requestResult)

    const QString key = m_requestToStringId.take(requestId);
    if (key.isEmpty()) {
        // Unknown requestId
        return;
    }
    if (!m_files.contains(key)) {
        // Unknown key. Assert?
        return;
    }

    m_files[key].completeDownload(requestResult);
    qDebug() << Q_FUNC_INFO << "Request complete:" << key << requestId;
    emit requestComplete(key);

    unqueuePendingRequest();
}

QString CFileManager::unqueuePendingRequest()
{
    if (m_pendingRequests.isEmpty()) {
        return QString();
    }

    qDebug() << Q_FUNC_INFO << "remains:" << m_pendingRequests.count() << m_pendingRequests.keys();
    const QString key = *m_pendingRequests.keyBegin();
    const Telegram::RemoteFile info = m_pendingRequests.take(key);
    qDebug() << Q_FUNC_INFO << "took key:" << key;

    const quint32 requestId = m_backend->requestFile(&info);
    if (!requestId) {
        qDebug() << Q_FUNC_INFO << "File is not available" << key;
        return QString();
    }
    m_requestToStringId.insert(requestId, key);
    return key;
}

FileInfo &FileInfo::operator=(const FileInfo &fileInfo)
{
    m_data = fileInfo.m_data;
    m_picture = fileInfo.m_picture;
    if (fileInfo.m_peerPicture) {
        setPeerPictureRequestData(fileInfo.peer(), fileInfo.pictureSize());
    }
    return *this;
}

Telegram::Peer FileInfo::peer() const
{
    if (!m_peerPicture) {
        return Telegram::Peer();
    }
    return m_peerPicture->peer;
}

Telegram::PeerPictureSize FileInfo::pictureSize() const
{
    if (!m_peerPicture) {
        return Telegram::PeerPictureSize::Small; // Incorrect size
    }
    return m_peerPicture->size;
}

void FileInfo::setPeerPictureRequestData(const Telegram::Peer &peer, Telegram::PeerPictureSize size)
{
    if (!m_peerPicture) {
        m_peerPicture = new PeerPictureData(peer, size);
    }
}

void FileInfo::addData(const QByteArray &newData)
{
    m_data += newData;
}

void FileInfo::completeDownload(const Telegram::RemoteFile &result)
{
    Q_UNUSED(result)
    if (m_peerPicture) {
        m_picture = QPixmap::fromImage(QImage::fromData(m_data));
    }
}

FileInfo::PeerPictureData::PeerPictureData(const Telegram::Peer &p, Telegram::PeerPictureSize s) :
    peer(p),
    size(s)
{
}
