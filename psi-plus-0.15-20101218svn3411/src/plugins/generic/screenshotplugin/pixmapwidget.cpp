/*
 * pixmapwidget.cpp - plugin
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

#include <QtGui>

#include "pixmapwidget.h"

GetTextDlg::GetTextDlg(QWidget *parent)
	: QDialog(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle(tr("Enter text"));
	QVBoxLayout *l = new QVBoxLayout(this);
	QHBoxLayout *boxLayout = new QHBoxLayout;
	QPushButton *selectFont = new QPushButton(tr("Select Font"));
	QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
						     Qt::Horizontal, this);

	boxLayout->addWidget(selectFont);
	boxLayout->addStretch();
	boxLayout->addWidget(box);

	te = new QTextEdit();

	l->addWidget(te);
	l->addLayout(boxLayout);

	connect(box, SIGNAL(accepted()), SLOT(okPressed()));
	connect(box, SIGNAL(rejected()), SLOT(close()));
	connect(selectFont, SIGNAL(released()),SIGNAL(selectFont()));

	adjustSize();
	setFixedSize(size());
	te->setFocus();
	show();
}

void GetTextDlg::okPressed()
{
	emit text(te->toPlainText());
	close();
}




//---------------PixmapWidget-----------------

PixmapWidget::PixmapWidget(QWidget *parent)
	: QWidget(parent)
	, bar_(0)
{

	p1 = QPoint(-1, -1);
	p2 = QPoint(-1, -1);
	selectionRect = QRect(-1,-1,-1,-1);

	//init(2, "#fffff", "Sans Serif,18,-1,5,50,0,0,0,0,0");

	draftPen.setColor(Qt::red);
	draftPen.setStyle(Qt::DashLine);
	draftPen.setWidth(1);
}

void PixmapWidget::init(int lineWidth, const QString &color, const QString &font)
{
	color_ = QColor(color);
	font_.fromString(font);
	pen.setColor(color_);
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(lineWidth);

	bar_->setColorForColorButton(color_);
	bar_->setLineWidth(lineWidth);
}

void PixmapWidget::setToolBar(ToolBar *bar)
{
	bar_ = bar;
	connect(bar_,SIGNAL(buttonClicked(ToolBar::ButtonType)), this, SLOT(buttonClicked(ToolBar::ButtonType)));
	connect(bar_,SIGNAL(checkedButtonChanged(ToolBar::ButtonType)),this, SLOT(checkedButtonChanged(ToolBar::ButtonType)));
	connect(bar_,SIGNAL(newWidth(int)),this,SLOT(newWidth(int)));
	bar_->checkButton(ToolBar::ButtonSelect);
}

void PixmapWidget::buttonClicked(ToolBar::ButtonType t)
{
	switch(t) {
	case ToolBar::ButtonCut:
		cut();
		return;
	case ToolBar::ButtonRotate:
		rotate();
		return;
	case ToolBar::ButtonColor:
		selectColor();
		return;
	case ToolBar::ButtonUndo:
		undo();
		return;
	case ToolBar::ButtonCopy:
		copy();
		return;
	default:
		break;
	}

	selectionRect = QRect(-1,-1,-1,-1);
	update();
}

void PixmapWidget::newWidth(int w)
{
	pen.setWidth(w);
	settingsChanged(constPenWidth, QVariant(w));
}

void PixmapWidget::setPixmap(QPixmap pix)
{
	mainPixmap = QPixmap();
	mainPixmap = pix;	
	setFixedSize(mainPixmap.size());
	selectionRect = QRect(-1,-1,-1,-1);
	update();
}

void PixmapWidget::cut()
{
	if(selectionRect.x() == -1)
		return;

	saveUndoPixmap();
	setPixmap(mainPixmap.copy(selectionRect));
	emit adjusted();
}

void PixmapWidget::copy()
{
	QClipboard *clipboard = QApplication::clipboard();
	QPixmap pix;
	if(selectionRect.width() != -1) {
		pix = mainPixmap.copy(selectionRect);
	}
	else {
		pix = mainPixmap;
	}

	clipboard->setPixmap(pix);
}

void PixmapWidget::rotate()
{
	saveUndoPixmap();
	QTransform t;
	setPixmap(mainPixmap.transformed(t.rotate(90), Qt::SmoothTransformation));
	emit adjusted();
}

void PixmapWidget::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setClipRect(rect());
	p.drawPixmap(QPoint(0, 0), mainPixmap/*.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)*/);
	if((type_ == ToolBar::ButtonSelect || type_ == ToolBar::ButtonText) && p2.x() != -1) {
		p.setPen(draftPen);
		int w = (p2.x() - p1.x());
		int h = (p2.y() - p1.y());
		p.drawRect(p1.x(), p1.y(), w, h);
	}
}

void PixmapWidget::mousePressEvent(QMouseEvent *e)
{
	if(e->buttons() == Qt::LeftButton) {
		p1 = e->pos();
		p2 = QPoint(-1, -1);

		if(type_ == ToolBar::ButtonPen) {
			setAttribute(Qt::WA_OpaquePaintEvent, true);
			p2 = p1;
			saveUndoPixmap();
			paintToPixmap();
		}
	}
	else if(e->buttons() == Qt::RightButton && selectionRect.contains(e->pos())) {
		QMenu *m = new QMenu(this);
		m->addAction(tr("Cut"), this, SLOT(cut()));
		m->addAction(tr("Copy"), this, SLOT(copy()));
		m->exec(e->globalPos());
	}
	e->accept();
}

void PixmapWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
	if(selectionRect.contains(e->pos())
		&& e->buttons() == Qt::LeftButton)
		cut();

	e->accept();
}

void PixmapWidget::mouseReleaseEvent(QMouseEvent *e)
{

	if(e->buttons() != !Qt::LeftButton ) {
		e->accept();
		return;
	}

	if(type_ == ToolBar::ButtonPen) {
		setAttribute(Qt::WA_OpaquePaintEvent, false);
		paintToPixmap();
	}
	else if(type_ == ToolBar::ButtonText) {
		selectionRect = QRect(QPoint(qMin(p1.x(), p2.x()), qMin(p1.y(), p2.y())),
				      QPoint(qMax(p1.x(), p2.x()), qMax(p1.y(), p2.y())));
		GetTextDlg *gtd = new GetTextDlg(this);
		connect(gtd, SIGNAL(text(QString)), SLOT(paintToPixmap(QString)));
		connect(gtd, SIGNAL(selectFont()), SLOT(selectFont()));
	}
	else if(type_ == ToolBar::ButtonSelect && p1 != e->pos() && p1.x() != -1) {
		selectionRect = QRect(QPoint(qMin(p1.x(), p2.x()), qMin(p1.y(), p2.y())),
				      QPoint(qMax(p1.x(), p2.x()), qMax(p1.y(), p2.y())));
	}

	p1 = QPoint(-1, -1);
	p2 = QPoint(-1, -1);
	e->accept();
}

void PixmapWidget::mouseMoveEvent(QMouseEvent *e)
{
	if(e->buttons() != Qt::LeftButton) {
		e->accept();
		return;
	}

	if(type_ == ToolBar::ButtonPen) {
		p1 = p2;
		p2 = e->pos();
		paintToPixmap();
	}
	else if(type_ == ToolBar::ButtonSelect || type_ == ToolBar::ButtonText){
		if(e->pos().x() >= 0 && e->pos().y() >= 0)
			p2 = e->pos();
		update();
	}

	e->accept();
}

void PixmapWidget::paintToPixmap(QString text)
{
    QPainter painter;
    painter.begin(&mainPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen);

    if(type_ == ToolBar::ButtonPen) {
	    if(p1.x() != -1 && p2.x() != -1) {
		    if(p1 != p2)
			    painter.drawLine(p1, p2);
		    else
			    painter.drawPoint(p1);
	    }
    }
    else if(type_ == ToolBar::ButtonText && !text.isEmpty() && selectionRect.x() != -1) {
	    saveUndoPixmap();
	    painter.setFont(font_);
	    painter.drawText(selectionRect, text);
	    selectionRect = QRect(-1,-1,-1,-1);
    }

    painter.end();
    update();
}

void PixmapWidget::selectColor()
{
	color_ = QColorDialog::getColor(color_, this);
	pen.setColor(color_);
	bar_->setColorForColorButton(color_);
	settingsChanged(constColor, QVariant(color_.name()));
}

void PixmapWidget::selectFont()
{
	bool *ok = 0;
	font_ = QFontDialog::getFont(ok, font_, this);
	settingsChanged(constFont, QVariant(font_.toString()));
}

void PixmapWidget::undo()
{
	if(!undoList_.isEmpty()) {
		setPixmap(undoList_.takeLast());
		emit adjusted();
	}
	bar_->enableUndo(!undoList_.isEmpty());
}

void PixmapWidget::saveUndoPixmap()
{
	undoList_.append(mainPixmap);
	bar_->enableUndo(true);
}

void PixmapWidget::checkedButtonChanged(ToolBar::ButtonType type)
{
	switch(type) {
	case(ToolBar::ButtonPen):
		this->setCursor(QCursor(QPixmap(":/screenshotplugin/draw.png"), 2,15));
		break;
	case(ToolBar::ButtonSelect):
		this->setCursor(QCursor(QCursor(Qt::CrossCursor)));
		break;
	case(ToolBar::ButtonText):
		this->setCursor(QCursor(QCursor(Qt::CrossCursor)));
		break;
	default:
		this->setCursor(QCursor(QCursor(Qt::ArrowCursor)));
	}

	selectionRect = QRect(-1,-1,-1,-1);
	type_ = type;

	update();
}
