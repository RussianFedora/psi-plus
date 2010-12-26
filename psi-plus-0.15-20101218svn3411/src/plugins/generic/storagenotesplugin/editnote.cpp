/*
 * editnote.cpp - plugin
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

#include "editnote.h"


EditNote::EditNote( QWidget *parent, QString tags, QString title, QString text, QModelIndex index)
        :QDialog(parent)
        , index_(index)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(false);
    ui_.setupUi(this);

    ui_.le_title->setText(title);
    ui_.le_tags->setText(tags);
    ui_.pte_text->insertPlainText(text);

    connect(ui_.buttonBox, SIGNAL(accepted()), this, SLOT(ok()));
    connect(ui_.buttonBox, SIGNAL(rejected()), this, SLOT(close()));

}

EditNote::~EditNote()
{
}

void EditNote::ok()
{
    QString text = ui_.pte_text->toPlainText();
    QString title = ui_.le_title->text();
    QString tags = ui_.le_tags->text();    

   QDomDocument doc;
   QDomElement noteElem = doc.createElement("note");
   QDomElement titleElem = doc.createElement("title");
   QDomElement textElem = doc.createElement("text");
   textElem.appendChild(doc.createTextNode(text));
   titleElem.appendChild(doc.createTextNode(title));
   noteElem.setAttribute("tags", tags);
   noteElem.appendChild(titleElem);
   noteElem.appendChild(textElem);
   doc.appendChild(noteElem);

   if(!text.isEmpty() || !title.isEmpty() || !tags.isEmpty())
        newNote(doc.documentElement());

   editNote(doc.documentElement(), index_);

   close();
}

