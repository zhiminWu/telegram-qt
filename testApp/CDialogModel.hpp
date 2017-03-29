#ifndef CDIALOGMODEL_HPP
#define CDIALOGMODEL_HPP

#include <QAbstractTableModel>

#include "TelegramNamespace.hpp"
#include "CPeerModel.hpp"

class CTelegramCore;

class CDialogModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum class Column {
        PeerType,
        PeerId,
        PeerName,
        Picture, // Photo (in terms of Telegram)
        ColumnsCount
    };

    enum class Role {
        PeerType,
        PeerId,
        PeerName,
        Picture, // Photo (in terms of Telegram)
        Invalid,
    };

    explicit CDialogModel(CTelegramCore *backend, QObject *parent = nullptr);

    void addSourceModel(CPeerModel *peerModel);

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
//    QVariant data(const QString &phone, Section section) const;
//    QVariant data(quint32 id, Section section) const;
    QVariant data(int dialogIndex, Role role) const;

    int indexOfPeer(const Telegram::Peer peer) const;

    void setDialogs(const QVector<Telegram::Peer> &dialogs);

protected slots:
    void onPeerPictureChanged(const Telegram::Peer peer);

protected:
    static Role columnToRole(Column column, int qtRole);

private:
    CTelegramCore *m_backend;
    QVector<CPeerModel*> m_sourceModels;
    QVector<Telegram::DialogInfo*> m_dialogs;

};

#endif // CDIALOGMODEL_HPP
