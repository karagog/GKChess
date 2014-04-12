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
#include <QObject>

// Even though we don't need this to compile, we include it anyways for completeness of this
//  class interface.
#include "gkchess_isquare.h"

namespace GKChess{


class PGN_MoveData;


/** An interface that encapsulates feedback from the user, such as choosing a promoted piece.
 *  The implementation should leverage the user interface to prompt the user for the given input.
*/
class IPlayerResponse
{
public:

    /** This will be called whenever the game logic needs input from
     *  the user to decide what piece they want to promote to.
     *
     *  The allegience of the promoting side is given as a paramter, so you can
     *  show the correct colored pieces in the UI.  Also the returned piece should
     *  have the given allegience.
     *
     *  \note This is only relevant when generating move data by two squares. If you are
     *  generating moves from PGN then it's already in the notation (i.e. e8=Q)
    */
    virtual Piece ChoosePromotedPiece(Piece::AllegienceEnum) = 0;

};




/** Describes a chess board interface. */
class AbstractBoard :
        public QObject
{
    Q_OBJECT
    GUTIL_DISABLE_COPY(AbstractBoard);
public:


    /** Describes different ways the board could be set up. */
    enum SetupTypeEnum
    {
        /** Causes the board to be cleared of all pieces. */
        Empty = 0,

        /** A standard game of chess. */
        SetupStandardChess = 1,

        /** Sets up a random Chess960 board. */
        SetupChess960 = 2,

        /** You can create your own custom board setups starting from this offset. */
        SetupCustomOffset = 100
    };



    /** Encodes the different possible results of a move validation. */
    enum MoveValidationEnum
    {
        /** Means the move is valid. */
        ValidMove = 0,

        /** Invalid move because pieces can't move that way according to the rules of chess.
         *  (i.e. a pawn moving backwards).
        */
        InvalidTechnical,

        /** Invalid because you would be leaving the king in check. */
        InvalidCheck,

        /** The source square does not have a piece on it. */
        InvalidEmptySquare,

        /** Invalid because there was a problem with the parameters you gave the function.
         *  For example if you didn't specify a source and destination square.
        */
        InvalidInputError,


        /** If you are extending this class for your own custom rules then you'll base
         *  your validation types with this value.
        */
        CustomInvalidMoveOffset = 100
    };


    /** Encodes the different ways the game could end. */
    enum ResultTypeEnum
    {
        /** Means the game is still in progress; there is no result yet. */
        Undecided = 0,

        /** Game ended because of checkmate. */
        Checkmate,

        /** Game ended because a player ran out of time. */
        TimeControl,

        /** Game ended because one side resigned. */
        Resignation,

        /** The game ended in a stalemate. */
        Stalemate,

        /** The game ended in a stalemate due to the 50 moves rule. */
        Stalemate_50Moves
    };



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

    /** Executes a move.  The difference between this and Move() is that this one does not
     *  emit signals and does not validate the moves.
     *
     *  This is useful for long simulations where you're not updating a user interface and you
     *  know all moves are valid.  If this is not the case though, you should generally use Move()
    */
    void MoveQuiet(const MoveData &);





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




    /** Sets up the board for a new game. */
    virtual void SetupNewGame(SetupTypeEnum = SetupStandardChess);

    /** Returns the number of rows on the board. */
    virtual int RowCount() const = 0;

    /** Returns the number of columns on the board. */
    virtual int ColumnCount() const = 0;

    /** Returns a reference to the square at the given column and row.
     *  The square is valid as long as the game logic is, so you can safely pass around pointers to it.
     * \warning It is not the responsibility of this class to check inputs for valid bounds
     *  \sa RowCount(), ColumnCount()
    */
    virtual ISquare const &SquareAt(int column, int row) const = 0;

    /** Creates a MoveData object from a source and dest square input.
     *  You must supply a user feedback object, so the game logic knows how the
     *  user wants to proceed in the event of a pawn promotion.  If you do not
     *  we will automatically promote to queen.
     *
     *  \note This function works with invalid moves, so if you care about
     *  validation you have to use ValidateMove() on the source and dest squares.
     *
     *  \returns A move data object, which is always populated except in the case of a
     *  pawn promotion that was cancelled, in which case it will be null (test with isnull())
    */
    virtual MoveData GenerateMoveData(const ISquare &, const ISquare &, IPlayerResponse *) const;

    /** Creates a MoveData object from a PGN MoveData object. */
    virtual MoveData GenerateMoveData(const PGN_MoveData &) const;

    /** Validates the move. */
    virtual MoveValidationEnum ValidateMove(const ISquare &, const ISquare &) const;

    /** Returns a list of valid squares that the piece on the given square can move to. */
    virtual GUtil::Vector<ISquare const *> GetValidMovesForSquare(const ISquare &) const;

    /** Executes the move described by the move data object and advances the game state.
     *
     *  This should take care to call ValidateMove() to apply validation and return the result.
     *  If it was an invalid move, then the state of the board must not change.
    */
    MoveValidationEnum Move(const MoveData &);

    /** A convenience function generates move data, validates and executes the move. */
    MoveValidationEnum Move2(const ISquare &src, const ISquare &dest, IPlayerResponse *pr = 0);

    /** Indicate that the given side wants to resign. */
    virtual void Resign(Piece::AllegienceEnum);




    /** \name Game State
     *  This section describes the getters and setters of the game state variables
     *  \{
    */

    /** Whose turn it is. */
    virtual Piece::AllegienceEnum GetWhoseTurn() const = 0;
    virtual void SetWhoseTurn(Piece::AllegienceEnum) = 0;

    /** A castle column, 0 based. If this castling move has been executed, or otherwise
     *  the opportunity ruined, it will be -1
    */
    virtual int GetCastleWhite1() const = 0;
    virtual void SetCastleWhite1(int) = 0;

    /** A castle column, 0 based. If this castling move has been executed, or otherwise
     *  the opportunity ruined, it will be -1
    */
    virtual int GetCastleWhite2() const = 0;
    virtual void SetCastleWhite2(int) = 0;

    /** A castle column, 0 based. If this castling move has been executed, or otherwise
     *  the opportunity ruined, it will be -1
    */
    virtual int GetCastleBlack1() const = 0;
    virtual void SetCastleBlack1(int) = 0;

    /** A castle column, 0 based. If this castling move has been executed, or otherwise
     *  the opportunity ruined, it will be -1
    */
    virtual int GetCastleBlack2() const = 0;
    virtual void SetCastleBlack2(int) = 0;

    /** The en passant square, if there is one. If not then this is null. */
    virtual ISquare const *GetEnPassantSquare() const = 0;
    virtual void SetEnPassantSquare(ISquare const *) = 0;

    /** The current number of half-moves since the last capture or pawn advance.
     *  This is used for determining a draw from lack of progress.
    */
    virtual int GetHalfMoveClock() const = 0;
    virtual void SetHalfMoveClock(int) = 0;

    /** Returns the current full move number. */
    virtual int GetFullMoveNumber() const = 0;
    virtual void SetFullMoveNumber(int) = 0;

    /** Returns the result of the game, or Undedided if it's not over yet. */
    virtual ResultTypeEnum GetResult() const = 0;
    virtual void SetResult(ResultTypeEnum) = 0;

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


/** Defined for your convenience, a string that represents the initial chess position
 *  in Forsyth-Edwards notation.
*/
#define FEN_STANDARD_CHESS_STARTING_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#endif // GKCHESS_ABSTRACTBOARD_H
