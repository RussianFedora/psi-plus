/*
 * boardmodel.h - plugin
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

#ifndef BOARDMODEL_H
#define BOARDMODEL_H

#include <QAbstractTableModel>
#include <QStringList>

#include "figure.h"

class BoardModel : public QAbstractTableModel
{
    Q_OBJECT

public:    
        BoardModel(Figure::GameType type, QObject *parent = 0);
        ~BoardModel() {};
        virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
        virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;

        void setGameType(Figure::GameType type);
        bool isYourFigure(const QModelIndex &index) const;
        bool moveRequested(QModelIndex oldIndex, QModelIndex newIndex);
        bool moveRequested(int oldX, int oldY, int newX, int newY);
        QModelIndex kingIndex() const;
        QModelIndex invert(QModelIndex index) const; //for black player
        Figure* findFigure(QModelIndex index) const;
        void reset();
        void updateFigure(QModelIndex index, QString newFigure);
        QString saveString();
        void loadSettings(QString settings, bool myLoad = true);
        void updateView();
        int checkGameState(); //0 - in progress; 1 - draw; 2 - lose;

        bool myMove;
        bool waitForFigure;
        bool check;
        Figure::GameType gameType_;
        int gameState_; //0 - in progress, 1 - draw, 2 - win, 3 - lose;

private:
        QStringList hHeader, vHeader;
        QList<Figure*> whiteFigures_, blackFigures_;
        QModelIndex tempIndex_;

        int canMove(Figure *figure, int newX, int newY) const;
        QModelIndex findFigure(Figure::FigureType type, Figure::GameType game) const;
        QMultiMap<QModelIndex, int> availableMoves(Figure* figure);
        bool doTestMove(Figure *figure, QModelIndex newIndex, int move);        
        bool isCheck() const;
        void moveTransfer();
        void setHeaders();

        struct Move {
            QModelIndex oldIndex;
            QModelIndex newIndex;
            Figure *figure;
            Figure *killedFigure;
        };
        Move lastMove;

signals:
        void move(int, int, int, int, QString);
        void figureKilled(Figure*);
        void needNewFigure(QModelIndex index, QString player);
};

#endif // BOARDMODEL_H
