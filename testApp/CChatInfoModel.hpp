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

#ifndef CCHATINFOMODEL_HPP
#define CCHATINFOMODEL_HPP

#include <QAbstractTableModel>

#include "TelegramNamespace.hpp"

class CTelegramCore;

class CChatInfoModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Columns {
        Id,
        Title,
        ParticipantsCount,
        ColumnsCount
    };

    explicit CChatInfoModel(CTelegramCore *backend, QObject *parent = 0);

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    int indexOfChat(quint32 id) const;
    bool haveChat(quint32 id) const;
    const Telegram::GroupChat *chatById(quint32 id) const;

public slots:
    void addChat(quint32 id);

signals:
    void chatAdded(quint32 id);
    void chatChanged(quint32 id);

protected slots:
    void onChatChanged(quint32 id);

private:
    CTelegramCore *m_backend;

    QList<Telegram::GroupChat> m_chats;

};

inline int CChatInfoModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return ColumnsCount;
}

inline int CChatInfoModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_chats.count();
}

#endif // CCHATINFOMODEL_HPP
