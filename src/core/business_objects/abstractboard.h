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

#include "gkchess_piece.h"
#include "gkchess_movedata.h"
#include "gkchess_igamestate.h"
#include "gkchess_igamelogic.h"
#include <QObject>

// Even though we don't need this to compile, we include it anyways for completeness of this
//  class interface.
#include "gkchess_isquare.h"

namespace GKChess{


/** Describes a chess board interface. */
class AbstractBoard :
        public QObject
{
    Q_OBJECT
    IGameLogic const *const i_gameLogic;
public:

    /** Constructs an abstract board with the given game logic.  If null,
     *  we will default to the standard chess logic.
    */
    AbstractBoard(IGameLogic const * = 0, QObject * = 0);
    AbstractBoard(const AbstractBoard &);

    /** You can be deleted by this interface. */
    virtual ~AbstractBoard();

    /** Returns the game logic.  You will use this to validate moves and
     *  generate move data.
    */
    IGameLogic const &GameLogic() const{ return *i_gameLogic; }

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

    /** Sets a piece on the square. If the square was occupied then
     *  it will simply be replaced by the new one. If you pass Piece::NoPiece
     *  then the space will be cleared.
     *
     *  The signal NotifySquareUpdated will be emitted.
     *
     *  \warning It is not the responsibility of this class to check inputs for valid bounds
    */
    void SetPiece(const Piece &, ISquare const &);

    /** Convenience function returns the piece on the given square. */
    Piece const *GetPiece(int column, int row) const;

    /** Moves based the piece from src to dest.
     *
     *  This validates moves using the game logic object.
     *
     *  You should pass a player response object so we can ask what piece to promote to.
    */
    void MovePiece(ISquare const &src, ISquare const &dest, IGameLogic::IPlayerResponse *);

    /** Returns a reference to the square at the given column and row.
     *  The square is valid as long as the board is, so you can safely pass around pointers to it.
     * \warning It is not the responsibility of this class to check inputs for valid bounds
    */
    virtual ISquare const &SquareAt(int column, int row) const = 0;

    /** Returns the number of rows. */
    virtual int RowCount() const = 0;

    /** Returns the number of columns. */
    virtual int ColumnCount() const = 0;

    /** Returns the current game state, as a reference for best performance. */
    virtual IGameState const &GameState() const = 0;

    /** Returns the current game state, as a reference for best performance. You
     *  modify it directly.
    */
    virtual IGameState &GameState() = 0;

    /** Returns a list of squares occupied by the type of piece specified.
     *  The list will be empty if there are no such pieces on the board.
     *
     *  This lookup should be done at least as good as O(log(N)) time, where N is the
     *  number of different types of pieces.
    */
    virtual GUtil::Vector<ISquare const *> FindPieces(const Piece &) const = 0;

    /** This function has a basic implementation provided for convenience, which merely
     *  iterates through the squares and removes their pieces. Override it if you like.
    */
    virtual void Clear();


signals:

    /** This signal is emitted to notify whenever a square is about to be updated with SetPiece(). */
    void NotifySquareAboutToBeUpdated(const GKChess::ISquare &);

    /** This signal is emitted to notify whenever a square has been updated with SetPiece(). */
    void NotifySquareUpdated(const GKChess::ISquare &);


    /** This signal is emitted whenever a piece is about to be moved with the Move() function. */
    void NotifyPieceAboutToBeMoved(const GKChess::MoveData &);

    /** This signal is emitted whenever a piece is moved with the Move() function. */
    void NotifyPieceMoved(const GKChess::MoveData &);


protected:

    /** You must implement moving pieces, but don't emit any signals.  The move has already
     *  been validated at this point.
    */
    virtual void move_p(const MoveData &) = 0;

    /** You must implement setting pieces on the board, but don't emit any signals. */
    virtual void set_piece_p(const Piece &, int col, int row) = 0;

};


}

#endif // GKCHESS_ABSTRACTBOARD_H
