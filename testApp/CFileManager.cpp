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
    if (m_requestToStringId.count() >= c_maxConcurrentDownloads) {
        m_pendingRequests.insert(key, requestFileInfo);
        // Delayed request
        return key;
    }

    const quint32 requestId = m_backend->requestFile(&file);
    if (!requestId) {
        // File is not available
        return QString();
    }
    m_requestToStringId.insert(requestId, key);
    m_files.insert(key, requestFileInfo);
    return key;
}

QString CFileManager::requestPeerPicture(const Telegram::Peer &peer, Telegram::PeerPictureSize size)
{
    const QString key = m_backend->peerPictureToken(peer, size);
    qDebug() << Q_FUNC_INFO << peer << key;
    if (m_files.contains(key)) {
        return key; // Already requested
    }
    // TODO: Work via requestFile(Telegram::RemoteFile)

    FileInfo requestFileInfo;
    requestFileInfo.setPeerPictureRequestData(peer, size);
    if (m_requestToStringId.count() >= c_maxConcurrentDownloads) {
        m_pendingRequests.insert(key, requestFileInfo);
        qDebug() << Q_FUNC_INFO << "Request delayed" << key;
        return key;
    }

    const quint32 requestId = m_backend->requestPeerPicture(peer, size);
    if (!requestId) {
        qDebug() << Q_FUNC_INFO << "File is not available" << key;
        return QString();
    }
    m_requestToStringId.insert(requestId, key);
    m_files.insert(key, requestFileInfo);
    qDebug() << Q_FUNC_INFO << "Request added" << key << requestId;
    return key;
}

QPixmap CFileManager::getPicture(const QString &uniqueId) const
{
    if (!m_files.contains(uniqueId)) {
        return QPixmap();
    }
    const FileInfo &info = m_files.value(uniqueId);
    return info.picture();
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

void CFileManager::unqueuePendingRequest()
{
    if (m_pendingRequests.isEmpty()) {
        return;
    }

    qDebug() << Q_FUNC_INFO << "remains:" << m_pendingRequests.count() << m_pendingRequests.keys();
    const QString key = m_pendingRequests.keys().first();
    qDebug() << Q_FUNC_INFO << "took key:" << key;
    const FileInfo info = m_pendingRequests.take(key);

    quint32 requestId = 0;
    if (info.hasPeerPicture()) {
        requestId = m_backend->requestPeerPicture(info.peer(), info.pictureSize());
    }
    if (!requestId) {
        qWarning() << Q_FUNC_INFO << "File is not available" << key;
        return unqueuePendingRequest();
    }
    qDebug() << Q_FUNC_INFO << "pending request id:" << key << requestId;
    m_requestToStringId.insert(requestId, key);
    m_files.insert(key, info);
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
