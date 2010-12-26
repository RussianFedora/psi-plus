/*
 * mainwindow.h - plugin
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QPushButton>

#include "ui_mainwindow.h"
#include "boardview.h"
#include "boarddelegate.h"
#include "boardmodel.h"

class ChessWindow : public QMainWindow
{
    Q_OBJECT

    public:
        ChessWindow(Figure::GameType type, bool enableSound_, QWidget *parent = 0);
        void moveRequest(int oldX, int oldY, int newX, int newY, QString figure = "");
        void loadRequest(QString settings);
        void youWin();
        void youLose();
        void youDraw();

    protected:
        void closeEvent(QCloseEvent *e);

    private:
        Ui::ChessWindow ui_;
        BoardModel *model_;        
        QModelIndex tmpIndex_;
        bool enabledSound;
        int movesCount;
	QAction *loseAction;
	void createMenu();

    private slots:
        void figureKilled(Figure* figure);
        void needNewFigure(QModelIndex index, QString player);
        void newFigure(QString figure);
        void load();
        void save();
        void addMove(int,int,int,int);

    signals:
        void closeBoard();
        void move(int, int, int, int, QString);
        void moveAccepted();
        void error();
        void load(QString);
        void draw();
        void lose();
        void toggleEnableSound(bool);

};

class SelectFigure : public QWidget
{
    Q_OBJECT
    public:
        SelectFigure(QString player, QWidget *parent = 0);

   private slots:
        void figureSelected();

    private:
        QPushButton *tb_queen, *tb_castle, *tb_knight, *tb_bishop;

    signals:
        void newFigure(QString figure);

};

#endif // MAINWINDOW_H
