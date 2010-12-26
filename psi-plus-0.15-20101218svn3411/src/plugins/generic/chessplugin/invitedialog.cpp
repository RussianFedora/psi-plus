/*
 * invitedialog.cpp - plugin
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

#include "invitedialog.h"

InviteDialog::InviteDialog(QStringList resources, QWidget *parent)
        :  QDialog(parent)
        , resources_(resources)
{
    setAttribute(Qt::WA_DeleteOnClose);
    ui_.setupUi(this);
    ui_.cb_resource->setEditable(true);
    if(!resources.isEmpty()) {
        ui_.cb_resource->addItems(resources_);
    } else {
        ui_.cb_resource->addItem("Enter resource");
    }

    connect(ui_.pb_black, SIGNAL(pressed()), this, SLOT(buttonPressed()));
    connect(ui_.pb_white, SIGNAL(pressed()), this, SLOT(buttonPressed()));

    adjustSize();
    setFixedSize(size());
}

void InviteDialog::buttonPressed()
{
    if(ui_.pb_white->isDown()) {
        emit play(ui_.cb_resource->currentText(), "white");
        close();
    }
    else if(ui_.pb_black->isDown()) {
        emit play(ui_.cb_resource->currentText(), "black");
        close();
    }
}



InvitationDialog::InvitationDialog(QString jid, QString color,  QWidget *parent)
        :  QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(false);
    ui_.setupUi(this);
    accepted = false;

    if(color == "white")
	    color = tr("white");
    else
	    color = tr("black");

    ui_.lbl_text->setText(tr("Player %1 invites you \nto play chess. He wants to play %2.")
                          .arg(jid).arg(color));

    connect(ui_.pb_accept, SIGNAL(pressed()), this, SLOT(buttonPressed()));
    connect(ui_.pb_reject, SIGNAL(pressed()), this, SLOT(close()));

    adjustSize();
    setFixedSize(size());
}

void InvitationDialog::buttonPressed()
{
        emit accept();
        accepted = true;
        close();
}

void InvitationDialog::closeEvent(QCloseEvent *e)
{
    if(!accepted)
        emit reject();
    e->accept();
    close();
}
