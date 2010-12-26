/*
 * figure.h - plugin
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

#ifndef FIGURE_H
#define FIGURE_H

#include <QPixmap>

class Figure
{
    public:

         enum GameType {
               NoGame = 0,
               WhitePlayer = 1,
               BlackPlayer = 2
         };

        enum FigureType {
                None = 0,
                White_Pawn = 1,
                White_Castle = 2,
                White_Bishop = 3,
                White_King = 4,
                White_Queen = 5,
                White_Knight = 6,
                Black_Pawn = 7,
                Black_Castle = 8,
                Black_Bishop = 9,
                Black_King = 10,
                Black_Queen = 11,
                Black_Knight = 12
        };

          Figure(GameType game = NoGame, FigureType type = Figure::None, int x = 0, int y = 0, QObject *parent = 0);
          QPixmap getPixmap();
          void setPosition(int x, int y);
          void setType(FigureType type);
          int positionX();
          int positionY();
          FigureType type();
          GameType gameType();
          bool isMoved;
          QString typeString();

     private:
          int positionX_, positionY_;
          FigureType type_;
          GameType gameType_;

};

#endif // FIGURE_H
