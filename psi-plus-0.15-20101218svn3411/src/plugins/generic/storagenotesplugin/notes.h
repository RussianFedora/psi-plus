/*
 * notes.h - plugin
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

#ifndef NOTES_H
#define NOTES_H

#include "ui_notes.h"
#include "storagenotesplugin.h"
#include "tagsmodel.h"

#include <QKeyEvent>

class Notes : public QDialog
{
        Q_OBJECT
public:
        Notes(StorageNotesPlugin *storageNotes, QWidget *parent = 0);
        ~Notes();
        void setAccount(int acc);

private:
        Ui::Notes ui_;
        int account_;
        StorageNotesPlugin *storageNotes_;
        TagModel *tagModel_;
        NoteModel *noteModel_;
        ProxyModel *proxyModel_;
        void updateTags();
        QString replaceSymbols(QString str);
        bool newNotes;

public slots:
        void incomingNotes(QList<QDomElement> notes);
        void load();
        void error();

private slots:
        void save();
        void add();
        void del();
        void edit();
        void addNote(QDomElement note);
        void noteEdited(QDomElement note, QModelIndex index);
        void selectTag();

protected:
        void closeEvent(QCloseEvent * event);
        void keyPressEvent(QKeyEvent *e);
};


#endif // NOTES_H
