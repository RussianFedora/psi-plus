/*
 * mooddlg.cpp
 * Copyright (C) 2006  Remko Troncon
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "xmpp_pubsubitem.h"
#include "xmpp_client.h"
#include "xmpp_task.h"
#include "mooddlg.h"
#include "moodcatalog.h"
#include "psiaccount.h"
#include "pepmanager.h"
#include "psiiconset.h"

MoodDlg::MoodDlg(QList<PsiAccount*> list)
	: QDialog(0), pa_(list)
{
	setAttribute(Qt::WA_DeleteOnClose);
	if(pa_.isEmpty())
		close();
	ui_.setupUi(this);
	setModal(false);
	connect(ui_.pb_cancel, SIGNAL(clicked()), SLOT(close()));
	connect(ui_.pb_ok, SIGNAL(clicked()), SLOT(setMood()));	

	ui_.cb_type->addItem(tr("<unset>"));
	PsiAccount *pa = pa_.first();
	Mood::Type mt = pa->mood().type();
	int i=1;
	foreach(MoodCatalog::Entry e, MoodCatalog::instance()->entries()) {
        	ui_.cb_type->addItem(IconsetFactory::icon("mood/"+e.value()).icon(), e.text());
		if (e.type() == mt) {
			ui_.cb_type->setCurrentIndex(i);
		}
		i++;
	}
	ui_.le_text->setText(pa->mood().text());
}


void MoodDlg::setMood()
{
	QString moodstr = ui_.cb_type->currentText();
	foreach(PsiAccount *pa, pa_) {
		if (moodstr == tr("<unset>")) {
			pa->pepManager()->retract("http://jabber.org/protocol/mood", "current");
		}
		else {
			Mood::Type type = MoodCatalog::instance()->findEntryByText(moodstr).type();
			pa->pepManager()->publish("http://jabber.org/protocol/mood", PubSubItem("current",Mood(type,ui_.le_text->text()).toXml(*pa->client()->rootTask()->doc())), PEPManager::PresenceAccess);
		}
	}
	close();
}
