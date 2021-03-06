/*
   Copyright (C) 2014-2015 Alexandr Akulich <akulichalexander@gmail.com>

   This file is a part of TelegramQt library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

 */

#include "CMessageModel.hpp"
#include "CContactModel.hpp"

#include "CTelegramCore.hpp"

#include <QDateTime>
#include <QDebug>

QString messageDeliveryStatusStr(CMessageModel::SMessage::Status status)
{
    switch (status) {
    case CMessageModel::SMessage::StatusUnknown:
        return QLatin1String("Unknown");
    case CMessageModel::SMessage::StatusSent:
        return QLatin1String("Sent");
    case CMessageModel::SMessage::StatusReceived:
        return QLatin1String("Received");
    case CMessageModel::SMessage::StatusRead:
        return QLatin1String("Read");
    case CMessageModel::SMessage::StatusDeleted:
        return QLatin1String("Deleted");
    default:
        return QString();
    }
}

CMessageModel::CMessageModel(CTelegramCore *backend, QObject *parent) :
    QAbstractTableModel(parent),
    m_backend(backend),
    m_contactsModel(nullptr)
{
    connect(m_backend, SIGNAL(sentMessageIdReceived(quint64,quint32)),
            SLOT(setResolvedMessageId(quint64,quint32)));

    connect(m_backend, SIGNAL(messageReadInbox(Telegram::Peer,quint32)),
            SLOT(setMessageInboxRead(Telegram::Peer,quint32)));
    connect(m_backend, SIGNAL(messageReadOutbox(Telegram::Peer,quint32)),
            SLOT(setMessageOutboxRead(Telegram::Peer,quint32)));
}

void CMessageModel::setContactsModel(CContactModel *model)
{
    m_contactsModel = model;
}

QString CMessageModel::peerToText(const Telegram::Peer &peer) const
{
    if (peer.type == Telegram::Peer::User) {
        Telegram::UserInfo info;
        m_backend->getUserInfo(&info, peer.id);
        if (!info.id()) {
            qDebug() << Q_FUNC_INFO << "Unknown contact..";
            return QStringLiteral("user%1").arg(peer.id);
        }
        return CContactModel::formatName(info);
    }
    Telegram::GroupChat info;
    m_backend->getChatInfo(&info, peer.id);
    if (!info.title.isEmpty()) {
        return info.title;
    }
    return QStringLiteral("chat%1").arg(peer.id);
}

QVariant CMessageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    case Peer:
        return tr("Peer");
    case Contact:
        return tr("Phone");
    case Direction:
        return tr("Direction");
    case Timestamp:
        return tr("Timestamp");
        break;
    case MessageId:
        return tr("Message Id");
    case Message:
        return tr("Message text");
    case Status:
        return tr("Status");
    case ForwardFrom:
        return tr("Forward from");
    case ForwardTimestamp:
        return tr("Forward timestamp");
    default:
        break;
    }

    return QVariant();
}

QVariant CMessageModel::data(const QModelIndex &index, int role) const
{
    int section = index.column();
    int messageIndex = index.row();

    if (messageIndex >= rowCount()) {
        return QVariant();
    }

    if (role == Qt::DecorationRole) {
        switch (section) {
        case Message:
            if (m_messages.at(messageIndex).mediaData.isValid()) {
                return m_messages.at(messageIndex).mediaData;
            }
            break;
        case Contact:
            if (m_contactsModel) {
                const SContact *contact = m_contactsModel->getContact(m_messages.at(messageIndex).fromId);
                if (contact) {
                    return contact->avatar;
                }
            }
            break;
        default:
            break;
        }

        return QVariant();
    }

    if ((role != Qt::DisplayRole) && (role != Qt::EditRole)) {
        return QVariant();
    }

    switch (section) {
    case Peer:
        section = PeerText;
        break;
    case ForwardFrom:
        section = ForwardFromText;
        break;
    default:
        break;
    }

    return rowData(messageIndex, section);
}

QVariant CMessageModel::rowData(quint32 messageIndex, int column) const
{
    if (int(messageIndex) >= m_messages.count()) {
        return QVariant();
    }

    const SMessage &message = m_messages.at(messageIndex);
    switch (column) {
    case Peer:
        return QVariant::fromValue(message.peer());
    case PeerText:
        return peerToText(message.peer());
    case Contact:
        if (m_contactsModel) {
            const SContact *contact = m_contactsModel->getContact(message.fromId);
            if (contact) {
                return CContactModel::getContactName(*contact);
            }
        }
        return message.fromId;
    case Direction:
        return (message.flags & TelegramNamespace::MessageFlagOut) ? tr("out") : tr("in");
    case Timestamp:
        return QDateTime::fromMSecsSinceEpoch(quint64(message.timestamp) * 1000);
    case MessageId:
        return message.id ? message.id : message.id64;
    case Message:
        return message.text;
    case Status:
        return messageDeliveryStatusStr(message.status);
    case ForwardFrom:
        return QVariant::fromValue(message.forwardFromPeer());
    case ForwardFromText:
        if (message.forwardFromPeer().isValid()) {
            return peerToText(message.forwardFromPeer());
        }
        return QString();
    case ForwardTimestamp:
        if (message.fwdTimestamp) {
            return QDateTime::fromMSecsSinceEpoch(quint64(message.fwdTimestamp) * 1000);
        }
        break;
    default:
        break;
    }

    return QVariant();
}

const CMessageModel::SMessage *CMessageModel::messageAt(quint32 messageIndex) const
{
    if (int(messageIndex) >= m_messages.count()) {
        return 0;
    }

    return &m_messages[messageIndex];
}

int CMessageModel::messageIndex(quint64 messageId) const
{
    for (int i = 0; i < m_messages.count(); ++i) {
        if (m_messages.at(i).id == messageId) {
            return i;
        }
        if (m_messages.at(i).id64 == messageId) {
            return i;
        }
    }

    return -1;
}

void CMessageModel::addMessage(const SMessage &message)
{
    for (int i = 0; i < m_messages.count(); ++i) {
        if ((m_messages.at(i).id64 && (m_messages.at(i).id64 == message.id64))
                || (!m_messages.at(i).id64 && (m_messages.at(i).id == message.id))) {
            m_messages.replace(i, message);

            emit dataChanged(index(i, 0), index(i, ColumnsCount - 1));
            return;
        }
    }
    beginInsertRows(QModelIndex(), m_messages.count(), m_messages.count());
    m_messages.append(message);
    if (!m_messages.last().timestamp) {
        m_messages.last().timestamp = QDateTime::currentMSecsSinceEpoch() / 1000;
    }
    endInsertRows();
}

int CMessageModel::setMessageMediaData(quint64 messageId, const QVariant &data)
{
    int i = messageIndex(messageId);
    if (i < 0) {
        return i;
    }

    m_messages[i].mediaData = data;
    emit dataChanged(index(i, Message), index(i, Message), QVector<int>() << Qt::DecorationRole);
    return i;
}

void CMessageModel::setMessageRead(Telegram::Peer peer, quint32 messageId, bool out)
{
    int from = -1;
    for (int i = 0; i < m_messages.count(); ++i) {
        SMessage &message = m_messages[i];

        const bool hasFlagOut = message.flags & TelegramNamespace::MessageFlagOut;
        const bool hasRightDirection = hasFlagOut == out;
        const bool idIsFitInRange = message.id <= messageId;
        const bool isNotAlreadyRead = message.status != CMessageModel::SMessage::StatusRead;
        const bool haveTargetPeer = message.peer() == peer;

        if (hasRightDirection && idIsFitInRange && isNotAlreadyRead && haveTargetPeer) {
            if (from < 0) {
                from = i;
            }

            message.status = CMessageModel::SMessage::StatusRead;
        } else {
            if (from >= 0) {
                QModelIndex firstIndex = index(from, Status);
                QModelIndex lastIndex = index(i - 1, Status);
                emit dataChanged(firstIndex, lastIndex);
                from = -1;
            }
        }
    }

    if (from >= 0) {
        QModelIndex firstIndex = index(from, Status);
        QModelIndex lastIndex = index(m_messages.count() - 1, Status);
        emit dataChanged(firstIndex, lastIndex);
    }
}

void CMessageModel::setMessageInboxRead(Telegram::Peer peer, quint32 messageId)
{
    setMessageRead(peer, messageId, /* out */ false);
}

void CMessageModel::setMessageOutboxRead(Telegram::Peer peer, quint32 messageId)
{
    setMessageRead(peer, messageId, /* out */ true);
}

void CMessageModel::setResolvedMessageId(quint64 randomId, quint32 resolvedId)
{
    for (int i = 0; i < m_messages.count(); ++i) {
        SMessage &message = m_messages[i];
        if (message.id64 == randomId) {
            message.id = resolvedId;
            message.status = CMessageModel::SMessage::StatusSent;

            QModelIndex firstIndex = index(i, MessageId);
            QModelIndex lastIndex = index(i, Status);
            emit dataChanged(firstIndex, lastIndex);
            break;
        }
    }
}

void CMessageModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
    m_messages.clear();
    endRemoveRows();
}
