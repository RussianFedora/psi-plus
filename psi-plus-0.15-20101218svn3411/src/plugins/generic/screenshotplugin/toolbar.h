/*
 * toolbar.h - plugin
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

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QToolButton>
#include <QIcon>
#include <QSpinBox>

#include "iconfactoryaccessinghost.h"

class Button;

class ToolBar : public QToolBar
{
	Q_OBJECT
public:
	enum ButtonType {
		ButtonSelect,
		ButtonPen,
		ButtonCut,
		ButtonText,
		ButtonColor,
		ButtonUndo,
		ButtonRotate,
		ButtonCopy,
		ButtonNoButton
	};

	ToolBar(QWidget *parent);
	~ToolBar();
	void setIconHost(IconFactoryAccessingHost* icoHost_);
	void init();
	ToolBar::ButtonType currentButton();
	void checkButton(ToolBar::ButtonType);
	void enableUndo(bool enable);
	void setColorForColorButton(const QColor &color);
	void setLineWidth(int width);

private slots:
	void buttonChecked(bool);
	void buttonClicked();

signals:
	void buttonClicked(ToolBar::ButtonType);
	void checkedButtonChanged(ToolBar::ButtonType);
	void newWidth(int);

private:
	QList<Button*> buttons_;
	IconFactoryAccessingHost* icoHost;
	QSpinBox *sb;

};

class Button : public QToolButton
{
	Q_OBJECT
public:
	Button(const QString &tip, const QIcon &ico, ToolBar::ButtonType type, bool checkable, QWidget *parent);
	ToolBar::ButtonType type();	

private:
	ToolBar::ButtonType type_;
};

#endif // TOOLBAR_H
