/*
 * invitedialog.h - plugin
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

#ifndef INVITEDIALOG_H
#define INVITEDIALOG_H

#include <QCloseEvent>
#include "ui_invitedialog.h"
#include "ui_invitationdialog.h"

class InviteDialog : public QDialog
{
    Q_OBJECT
    public:
         InviteDialog(QStringList resources, QWidget *parent = 0);

    private:
         Ui::InviteDialog ui_;
         QStringList resources_;

    private slots:
         void buttonPressed();

    signals:
         void play(QString, QString);
};

class InvitationDialog : public QDialog
{
    Q_OBJECT
    public:
         InvitationDialog(QString jid, QString color, QWidget *parent = 0);

    private:
         Ui::InvitationDialog ui_;
         bool accepted;

    private slots:
         void buttonPressed();

    signals:
         void accept();
         void reject();

    protected:
         void closeEvent(QCloseEvent *e);
};


#endif // INVITEDIALOG_H
