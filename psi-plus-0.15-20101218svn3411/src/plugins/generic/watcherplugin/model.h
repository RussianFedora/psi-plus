/*
 * model.h - plugin
 * Copyright (C) 2010  Khryukin Evgeny
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef MODEL_H
#define MODEL_H

#include <QAbstractTableModel>
#include <QStringList>
#include <QSet>

class Model : public QAbstractTableModel
{
    Q_OBJECT

public:
        Model(QStringList watchedJids_, QStringList Sounds_, QObject *parent = 0);
        ~Model() {};
        virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
        virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
        QString jid(const QModelIndex & index) const;
        QString soundFile(const QModelIndex & index) const;
        QString tmpSoundFile(const QModelIndex & index) const;
        void apply();
        void deleteSelected();
        void reset();
        void selectAll();
        void unselectAll();
        void addRow(QString jid = "");
        void deleteRow(QString jid);
        void setStatusForJid(QString jid, QString status);
        QString statusByJid(QString jid);
        QString soundByJid(QString jid);        
        int indexByJid(QString jid);
        QStringList getWatchedJids();
        QStringList getSounds();

private:
        QStringList headers, watchedJids, tmpWatchedJids_, sounds, tmpSounds_;
        QMap<QString, QString> statuses;
        QList<bool> selected;        
};


#endif // MODEL_H
