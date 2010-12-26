/*
 * pixmapwidget.h - plugin
 * Copyright (C) 2009-2010  Khryukin Evgeny
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

#ifndef PIXMAPWIDGET_H
#define PIXMAPWIDGET_H

#include <QPixmap>
#include <QPen>
#include <QTextEdit>
#include <QDialog>
#include <QPointer>
#include "toolbar.h"

#define constPenWidth "penwidth"
#define constColor "color"
#define constFont "font"

class PixmapWidget : public QWidget
{
	Q_OBJECT
public:
	PixmapWidget(QWidget *parent);
	void init(int lineWidth, const QString& color, const QString& font);
	void setToolBar(ToolBar *bar);
	void setPixmap(QPixmap pix);
	QPixmap getPixmap() { return mainPixmap; };	

private slots:	
	void checkedButtonChanged(ToolBar::ButtonType type);
	void paintToPixmap(QString text = "");
	void newWidth(int w);
	void buttonClicked(ToolBar::ButtonType);
	void cut();
	void copy();
	void selectFont();

protected:
	void mousePressEvent(QMouseEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void paintEvent(QPaintEvent *);

signals:
	void adjusted();
	void settingsChanged(QString, QVariant);

private:
	ToolBar *bar_;
	QColor color_;
	QList<QPixmap> undoList_;
	QPixmap mainPixmap;
	ToolBar::ButtonType type_;
	QPoint p1;
	QPoint p2;
	QPen pen;
	QPen draftPen;
	QFont font_;
	QRect selectionRect;

	void saveUndoPixmap();
	void selectColor();
	void undo();	
	void rotate();
};

class GetTextDlg : public QDialog
{
	Q_OBJECT
public:
	GetTextDlg(QWidget *parent);

signals:
	void text(QString);
	void selectFont();

private slots:
	void okPressed();

private:
	QTextEdit *te;
};


#endif // PIXMAPWIDGET_H
