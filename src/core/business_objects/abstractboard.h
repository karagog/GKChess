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
#include "gkchess_piece.h"
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
    Piece const *GetPiece(int column, int row) const;

    /** Returns the square at the given column and row.
     *  The square is valid as long as the board is.
     * \warning It is not the responsibility of this class to check inputs for valid bounds
    */
    virtual ISquare const &SquareAt(int column, int row) const = 0;

    /** Returns the en passant square, if there is one. If not then returns null. */
    virtual ISquare const *GetEnPassantSquare() const = 0;

    /** Sets the en passant square, or clears it if you pass null */
    virtual void SetEnPassantSquare(ISquare const *) = 0;

    /** Returns the castle information, which is represented as the files of the rooks'
     *  initial positions.  If So in standard chess this is (0,7), but is flexible enough
     *  to support non-standard chess variants where the rooks can be elsewhere.
     *
     *  \returns A char with the first 4 bits as the first rook position, and the last 4
     *  bits as the second rook position.  The position is given as a base-1 index, so you
     *  can compare this char with 0 to see if castling is available at all before testing
     *  further.  This is done for efficiency's sake, so you can quickly query the castle info.
    */
    virtual GUINT8 GetCastleInfo(Piece::AllegienceEnum) const = 0;

    /** Sets the castle info for the allegience.
     *
     *  \param info A char with the first 4 bits as the first rook position, and the last 4
     *  bits as the second rook position.  The position is given as a base-1 index, so you
     *  can compare this char with 0 to see if castling is available at all before testing
     *  further.  This is done for efficiency's sake, so you can quickly query the castle info.
    */
    virtual void SetCastleInfo(Piece::AllegienceEnum, GUINT8 info) = 0;


    /** Returns the number of rows. */
    virtual int RowCount() const = 0;

    /** Returns the number of columns. */
    virtual int ColumnCount() const = 0;

    /** This function has a basic implementation provided for convenience, which merely
     *  iterates through the squares and removes their pieces. Override it if you like.
    */
    virtual void Clear();


signals:

    /** The implementation needs to notify whenever a square has been updated. */
    void NotifySquareUpdated(int col, int row);

};


}

#endif // GKCHESS_ABSTRACTBOARD_H
