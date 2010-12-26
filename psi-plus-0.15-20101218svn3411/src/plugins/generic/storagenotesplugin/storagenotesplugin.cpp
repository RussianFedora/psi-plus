/*
 * storagenotesplugin.cpp - plugin
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

#include "storagenotesplugin.h"
#include "notes.h"

#include <QLabel>

#define constVersion "0.1.4"

static const QString id = "strnotes_1";

StorageNotesPlugin::StorageNotesPlugin() {
    enabled = false;
    iconHost = 0;
    accInfo = 0;
    stanzaSender = 0;
    popup = 0;
    note = 0;
}

QString StorageNotesPlugin::name() const {
        return "Storage Notes Plugin";
}

QString StorageNotesPlugin::shortName() const {
        return "storagenotes";
}

QString StorageNotesPlugin::version() const {
        return constVersion;
}

bool StorageNotesPlugin::enable(){
                enabled = true;                
                return enabled;
}

bool StorageNotesPlugin::disable(){
    if(note) {
        delete note;
        note = 0;
    }
    enabled = false;
    return true;
}

QWidget* StorageNotesPlugin::options() {
        if (!enabled) {
		return 0;
	}
        QWidget *optionsWid = new QWidget();
        QVBoxLayout *vbox= new QVBoxLayout(optionsWid);

        QLabel *wikiLink = new QLabel(tr("<a href=\"http://psi-plus.com/wiki/plugins#storage_notes_plugin\">Wiki (Online)</a>"),optionsWid);
	wikiLink->setOpenExternalLinks(true);

        vbox->addWidget(wikiLink);
        vbox->addStretch();

        return optionsWid;
}

bool StorageNotesPlugin::incomingStanza(int/* account*/, const QDomElement& xml) {
    if(!enabled)
            return false;

    if(xml.tagName() == "iq" && xml.attribute("id") == id) {
        if(xml.attribute("type") == "error")
            error();

        if(xml.attribute("type") == "result") {
            QList<QDomElement> notes;
            QDomNodeList noteList = xml.elementsByTagName("note");
            for(int i = 0; i < noteList.size(); i++)
                notes.append(noteList.at(i).toElement());

            if(!notes.isEmpty())
                incomingNotes(notes);
        }
        return true;
    }
    return false;
}

bool StorageNotesPlugin::outgoingStanza(int account, QDomElement& xml)
{
	return false;
}

void StorageNotesPlugin::setAccountInfoAccessingHost(AccountInfoAccessingHost* host){
    accInfo = host;
}

void StorageNotesPlugin::setIconFactoryAccessingHost(IconFactoryAccessingHost* host){
        iconHost = host;
}

void StorageNotesPlugin::setStanzaSendingHost(StanzaSendingHost *host){
        stanzaSender = host;
}

void StorageNotesPlugin::setPopupAccessingHost(PopupAccessingHost* host) {
    popup = host;
}

void StorageNotesPlugin::start()
{
    if(!enabled)
        return;

    int acc = sender()->property("account").toInt();
    if(note) {
        note->setAccount(acc);
        note->load();
        note->raise();
    }
    else {
        note = new Notes(this);        
        note->setAccount(acc);
        connect(this, SIGNAL(incomingNotes(QList<QDomElement>)), note, SLOT(incomingNotes(QList<QDomElement>)));
        connect(this,SIGNAL(error()), note, SLOT(error()));
        note->load();
        note->show();
    }
}

void StorageNotesPlugin::deleteNotes() {
    delete note;
    note = 0;
}

QList < QVariantHash >* StorageNotesPlugin::getAccountMenuParam()
{
    QVariantHash hash;
        hash["icon"] = QVariant(QString("loggerplugin/openlog"));
        hash["name"] = QVariant(tr("Storage Notes"));
        hash["reciver"] = qVariantFromValue(qobject_cast<QObject *>(this));
        hash["slot"] = QVariant(SLOT(start()));
        QList< QVariantHash > * l = new QList< QVariantHash >();
        l->push_back(hash);
        return l;
}

QList < QVariantHash >* StorageNotesPlugin::getContactMenuParam()
{
    return 0;
}

QString StorageNotesPlugin::pluginInfo() {
	return tr("Author: ") +  "Dealer_WeARE\n"
			+ tr("Email: ") + "wadealer@gmail.com\n\n"
			+ trUtf8("This plugin is an implementation of XEP-0049: Private XML Storage.\n"
			 "The plugin is fully compatible with notes saved using Miranda IM.\n"
			 "The plugin is designed to keep notes on the jabber server with the ability to access them from anywhere using Psi+ or Miranda IM.");
}

Q_EXPORT_PLUGIN(StorageNotesPlugin);
