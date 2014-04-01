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
    GUTIL_DISABLE_COPY(AbstractBoard);
public:

    AbstractBoard(QObject * = 0);
    virtual ~AbstractBoard();

    /** Sets a piece on the square. If the square was occupied then
     *  it will simply be replaced by the new one. If you pass Piece::NoPiece
     *  then the space will be cleared.
     *
     *  You must emit NotifySquareUpdated() as part of the interface.
     *
     * \warning It is not the responsibility of this class to check inputs for valid bounds
    */
    virtual void SetPiece(const Piece &, int column, int row) = 0;

    /** Convenience function returns the piece on the given square. */
    Piece const *GetPiece(int column, int row) const;

    /** Moves the piece from the source index to the dest index.
     *  You must emit NotifyPieceMoved() in the implementation.
    */
    virtual void MovePiece(int source_col, int source_row, int dest_col, int dest_row) = 0;

    /** Returns the square at the given column and row.
     *  The square is valid as long as the board is.
     * \warning It is not the responsibility of this class to check inputs for valid bounds
    */
    virtual ISquare const &SquareAt(int column, int row) const = 0;


    /** Returns who has the current turn. */
    virtual Piece::AllegienceEnum GetWhoseTurn() const = 0;

    /** Sets who has the current turn. */
    virtual void SetWhoseTurn(Piece::AllegienceEnum) = 0;


    /** Returns the castle information, which is represented as the files of the rooks'
     *  initial positions.  If So in standard chess this is (0,7), but is flexible enough
     *  to support non-standard chess variants where the rooks can be elsewhere.
     *
     *  \returns A char with the first 4 bits as the first rook position, and the last 4
     *  bits as the second rook position.  The position is given as a base-1 index, so you
     *  can compare this char with 0 to see if castling is available at all before testing
     *  further.  This is done for efficiency's sake, so you can quickly query the castle info.
     *
     *  \note The order doesn't matter in the return value, you can treat it as an unordered pair
     *  of values.
    */
    virtual GUINT8 GetCastleInfo(Piece::AllegienceEnum) const = 0;

    /** Sets the castle info for the allegience.
     *
     *  \param info A char with the first 4 bits as the first rook position, and the last 4
     *  bits as the second rook position.  The position is given as a base-1 index, so you
     *  can compare this char with 0 to see if castling is available at all before testing
     *  further.  This is done for efficiency's sake, so you can quickly query the castle info.
     *
     *  \note The order doesn't matter in the parameter, you can treat it as an unordered pair
     *  of values.
    */
    virtual void SetCastleInfo(Piece::AllegienceEnum, GUINT8 info) = 0;


    /** Returns the en passant square, if there is one. If not then returns null. */
    virtual ISquare const *GetEnPassantSquare() const = 0;

    /** Sets the en passant square, or clears it if you pass null */
    virtual void SetEnPassantSquare(ISquare const *) = 0;


    /** Returns the current number of half-moves since the last capture or pawn advance.
     *  This is used for determining a draw from lack of progress.
    */
    virtual int GetHalfMoveClock() const = 0;

    /** Sets the current half-move clock. */
    virtual void SetHalfMoveClock(int) = 0;


    /** Returns the current full-move number. */
    virtual int GetFullMoveNumber() const = 0;

    /** Sets the current full-move number. */
    virtual void SetFullMoveNumber(int) = 0;


    /** Returns the number of rows. */
    virtual int RowCount() const = 0;

    /** Returns the number of columns. */
    virtual int ColumnCount() const = 0;

    /** This function has a basic implementation provided for convenience, which merely
     *  iterates through the squares and removes their pieces. Override it if you like.
    */
    virtual void Clear();
    
    /** Populates this board with the position given in X-FEN notation.
        
        This has a default implementation that should work for all board
        implementations, but it is left virtual in case you want to optimize it
        for your board implementation.
    */
    virtual void FromFEN(const GUtil::String &);
    
    /** Serializes the board object into a FEN string. 
        \note The default implementation should work for all board implementations,
        but it is left virtual in case you want to customize/optimize it. 
    */
    virtual GUtil::String ToFEN() const;


signals:

    /** The implementation needs to notify whenever a square has been updated. */
    void NotifySquareUpdated(int col, int row);

    /** The implementation needs to emit this whenever a piece is moved. */
    void NotifyPieceMoved(const Piece &, int s_col, int s_row, int d_col, int d_row);

};


}

#endif // GKCHESS_ABSTRACTBOARD_H
