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

#ifndef PASSDIALOG_H
#define PASSDIALOG_H

#include <QDialog>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;

class PassDialog : public QDialog
{
    Q_OBJECT
public:
    PassDialog(QWidget *parent = 0, QString jid = "", bool *ok = false, bool *savepassword = false, QString *password = NULL);

private slots:
    void okaySelected();
    void cancelSelected();

private:
    bool *savepasswd;
    QString *passwd;
    bool *ok;

    QLabel *label;
    QPushButton *okay;
    QPushButton *cancel;
    QCheckBox *savepass;
    QLineEdit *password;

};

#endif
