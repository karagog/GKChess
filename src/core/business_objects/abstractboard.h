/*Copyright 2014 George Karagoulis

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#ifndef GKCHESS_ABSTRACTBOARD_H
#define GKCHESS_ABSTRACTBOARD_H

#include "gkchess_isquare.h"
#include <QObject>

namespace GKChess{


/** Describes a chess board interface. */
class AbstractBoard :
        public QObject
{
    Q_OBJECT
public:

    AbstractBoard(QObject * = 0);
    virtual ~AbstractBoard();

    /** Sets a piece on the square. If the square was occupied then
     *  it will simply be replaced by the new one. If you pass Piece::NoPiece
     *  then the space will be cleared.
     *
     *  Since all moves can be represented as a series of SetPiece()'s,
     *  this is the main thing you have to implement.
     *
     *  You must emit NotifySquareUpdated() as part of the interface.
     *
     * \warning It is not the responsibility of this class to check inputs for valid bounds
    */
    virtual void SetPiece(const Piece &, int column, int row) = 0;

    /** Convenience function returns the piece on the given square. */
    Piece const *GetPiece(int column, int row) const{ return GetSquare(column, row).GetPiece(); }

    /** Returns the square at the given column and row.
     *  The square is valid as long as the board is.
     * \warning It is not the responsibility of this class to check inputs for valid bounds
    */
    virtual ISquare const &GetSquare(int column, int row) const = 0;


    /** Returns the number of rows. */
    int RowCount() const{ return 8; }

    /** Returns the number of columns. */
    int ColumnCount() const{ return 8; }

    /** This function is provided for convenience, and merely iterates through the
     *  squares and removes their pieces
     *
     * \note At the time of implementation, I couldn't see a reason to make this virtual,
     * but I may think of a compelling reason later.
    */
    void Clear();


signals:

    /** The implementation needs to notify whenever a square has been updated. */
    void NotifySquareUpdated(int col, int row);

};


}

#endif // GKCHESS_ABSTRACTBOARD_H
