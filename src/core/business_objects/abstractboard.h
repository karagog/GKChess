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
#include "gkchess_igamelogic.h"
#include <QObject>

// Even though we don't need this to compile, we include it anyways for completeness of this
//  class interface.
#include "gkchess_isquare.h"

namespace GKChess{


/** Describes a chess board interface. */
class AbstractBoard :
        public QObject,
        public IGameLogic
{
    Q_OBJECT
    GUTIL_DISABLE_COPY(AbstractBoard);
public:

    /** Constructs an abstract board with the given game logic.  If null,
     *  we will default to the standard chess logic.
    */
    AbstractBoard(QObject * = 0);

    /** You can be deleted by this interface. */
    virtual ~AbstractBoard();

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



    /** \name IGameLogic interface
     *
     *  These are the default implementations of the standard chess logic. You can override them
     *  to suit your needs.
     *
     *  \{
    */
    virtual void SetupNewGame(SetupTypeEnum = SetupStandardChess);
    virtual MoveData GenerateMoveData(const ISquare &, const ISquare &, IPlayerResponse *) const;
    virtual MoveData GenerateMoveData(const PGN_MoveData &) const;
    virtual MoveValidationEnum ValidateMove(const ISquare &, const ISquare &) const;
    virtual GUtil::Vector<ISquare const *> GetValidMovesForSquare(const ISquare &) const;
    virtual void Move(const MoveData &);
    virtual void Resign(Piece::AllegienceEnum);
    /** \} */


signals:

    /** This signal is emitted to notify whenever a square is about to be updated with SetPiece(). */
    void NotifySquareAboutToBeUpdated(const GKChess::ISquare &);

    /** This signal is emitted to notify whenever a square has been updated with SetPiece(). */
    void NotifySquareUpdated(const GKChess::ISquare &);


    /** This signal is emitted whenever a piece is about to be moved with the Move() function. */
    void NotifyPieceAboutToBeMoved(const GKChess::MoveData &);

    /** This signal is emitted whenever a piece is moved with the Move() function. */
    void NotifyPieceMoved(const GKChess::MoveData &);

    /** This signal is emitted after a side resigns. */
    void NotifyResignation(Piece::AllegienceEnum);


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
