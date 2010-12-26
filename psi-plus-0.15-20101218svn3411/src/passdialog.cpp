/*
 * passdialog.cpp
 * Copyright (C) 2009-2010 Virnik
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

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "passdialog.h"

/**  \brief PassDialog is used to display a password prompt for Jabber authentication.  Note that several return values need to be stored in prepared locations so the contents WILL be changed.  New variables are needed  for this purpose when called.
 * \param jid JID representing the connection displayed to the user to identify the password
 * \param _ok true indicates the ok button was pressed, false indicates cancel was pressed instead
 * \param _savepasswd the state of save password checkbox -- initially matches the value supplied
 * \param _passwd the password string supplied by the user
 */
PassDialog::PassDialog(QWidget *parent, QString jid, bool *_ok, bool *_savepasswd, QString *_passwd)
    : QDialog(parent, Qt::WDestructiveClose)
{    

    setCaption(tr("Wrong Account Password"));
 
    ok = _ok;
    savepasswd = _savepasswd;
    passwd = _passwd;
    
    // Layouts //
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(6);
    mainLayout->setSpacing(6);
    QHBoxLayout *topLayout = new QHBoxLayout(mainLayout);
    QHBoxLayout *midLayout = new QHBoxLayout(mainLayout);
    QHBoxLayout *botLayout = new QHBoxLayout(mainLayout);
    
    label = new QLabel(tr("Please enter your password for %1:").arg(jid),this);
    topLayout->addWidget(label);

    password = new QLineEdit(this);
    password->setEchoMode(QLineEdit::Password);
    midLayout->addWidget(password);


    savepass = new QCheckBox(tr("Save Password"), this, "savepass");
    savepass->setChecked(*savepasswd); //set the initial state of the checkbox to match supplied value
    botLayout->addWidget(savepass);
    botLayout->addStretch(1);
    
    okay = new QPushButton(tr("OK"), this, "ok");
    okay->setDefault(true);
    botLayout->addWidget(okay);

    cancel = new QPushButton(tr("Cancel"), this, "cancel");
    botLayout->addWidget(cancel);
    
    
    // Functionality //
    connect(cancel, SIGNAL(clicked()), this, SLOT(cancelSelected()));
    connect(okay, SIGNAL(clicked()), this, SLOT(okaySelected()));

}

/** Called when the user selects the Ok button */
void PassDialog::okaySelected()
{
	*ok=true;
	*passwd = password->text();
	*savepasswd = savepass->isChecked();
    
	done(0);
	close();
}

/** Called when the user selects the cancel button to abort the dialog */
void PassDialog::cancelSelected()
{
	*ok=false;
	done(-1);
}
