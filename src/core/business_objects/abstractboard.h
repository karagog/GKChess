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

#include "gutil_strings.h"
#include "gkchess_piece.h"
#include "gkchess_movedata.h"

// Even though we don't need this to compile, we include it anyways for completeness of this
//  class interface.
#include "gkchess_square.h"

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

    /** This is used in Chess960 for situations in which it is not clear if you wish to castle
     *  or simply move your king.
     *
     *  \returns True if the player wants to castle
    */
    virtual bool CastleYesNo() = 0;

};




/** Describes a chess board interface, with move logic and game state and everything.

    By default the board automatically emits signals before and after a piece
    is moved or otherwise placed on the board, so views can be updated. Additionally,
    the board supports a simulation mode which suppresses all signals and validation for
    optimum performance.
*/
class AbstractBoard
{
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
    AbstractBoard();

    /** You can be deleted by this interface. */
    virtual ~AbstractBoard();

    /** Populates this board with the position given in X-FEN notation.

        This has a default implementation that should work for all board
        implementations, but it is left virtual in case you want to optimize it
        for your board implementation.
    */
    void FromFEN(const GUtil::String &);

    /** Serializes the board object into a FEN string.
        \note The default implementation should work for all board implementations,
        but it is left virtual in case you want to customize/optimize it.
    */
    GUtil::String ToFEN() const;

    /** Sets a piece on the square. If the square was occupied then
     *  it will simply be replaced by the new one. If you pass Piece::NoPiece
     *  then the space will be cleared.
     *
     *  The signal NotifySquareUpdated will be emitted.
     *
     *  \warning It is not the responsibility of this class to check inputs for valid bounds
    */
    virtual void SetPiece(Piece const &, Square const &);

    /** Convenience function returns the piece on the given square. */
    Piece const &GetPiece(int column, int row) const;



    /** Returns a list of squares occupied by the type of piece specified.
     *
     *  The list will be empty if there are no such pieces on the board.
     *
     *  To find the locations of all pieces for a given allegience, pass
     *  a piece with no type but with the allegience you're interested in.
     *
     *  This lookup should be done at least as good as O(log(N)) time, where N is the
     *  number of different types of pieces.
    */
    virtual GUtil::Vector<Square const *> FindPieces(Piece const &) const = 0;

    /** This function has a basic implementation provided for convenience, which merely
     *  iterates through the squares and removes their pieces. Override it if you like.
    */
    virtual void Clear();

    /** Causes the board to update the threat counts for all squares. */
    virtual void UpdateThreatCounts();




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
    virtual Square const &SquareAt(int column, int row) const = 0;

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
    virtual MoveData GenerateMoveData(const Square &, const Square &, IPlayerResponse *) const;

    /** Creates a MoveData object from a PGN MoveData object. */
    virtual MoveData GenerateMoveData(const PGN_MoveData &) const;

    /** Validates the move. */
    virtual MoveValidationEnum ValidateMove(const Square &, const Square &) const;

    /** Returns a list of valid squares that the piece on the given square can move to. */
    virtual GUtil::Vector<Square const *> GetValidMovesForSquare(const Square &) const;

    /** Executes the move described by the move data object and advances the game state.
     *
     *  This should take care to call ValidateMove() to apply validation and return the result.
     *  If it was an invalid move, then the state of the board must not change.
    */
    MoveValidationEnum Move(const MoveData &);

    /** A convenience function generates move data, validates and executes the move. */
    MoveValidationEnum Move2(const Square &src, const Square &dest, IPlayerResponse *pr = 0);

    /** Indicate that the given side wants to resign. */
    virtual void Resign(Piece::AllegienceEnum);




    /** \name Game State
     *  This section describes the getters and setters of the game state variables
     *  \{
    */

    /** Whose turn it is. */
    PROPERTY(WhoseTurn, Piece::AllegienceEnum);


    /** The castle column on the white king's A-side, 0 based. If this castling move has been executed,
     *  or was otherwise spoiled, it will be -1.
    */
    PROPERTY(CastleWhiteA, int);

    /** The castle column on the white king's H-side, 0 based. If this castling move has been executed,
     *  or was otherwise spoiled, it will be -1.
    */
    PROPERTY(CastleWhiteH, int);

    /** The castle column on the black king's A-side, 0 based. If this castling move has been executed,
     *  or was otherwise spoiled, it will be -1.
    */
    PROPERTY(CastleBlackA, int);

    /** The castle column on the black king's H-side, 0 based. If this castling move has been executed,
     *  or was otherwise spoiled, it will be -1.
    */
    PROPERTY(CastleBlackH, int);

    /** The en passant square, if there is one. If not then this is null. */
    PROPERTY(EnPassantSquare, SquarePointerConst);

    /** The current number of half-moves since the last capture or pawn advance.
     *  This is used for determining a draw from lack of progress.
    */
    PROPERTY(HalfMoveClock, int);

    /** Returns the current full move number. */
    PROPERTY(FullMoveNumber, int);

    /** Returns the result of the game, or Undedided if it's not over yet. */
    PROPERTY(Result, ResultTypeEnum);

    /** \} */


protected:

    /** This is called when a piece is moved. */
    virtual void move_p(const MoveData &);

    /** Returns a modifiable reference to the square at the given row and column. */
    virtual Square &square_at(int col, int row) = 0;


private:

    void _update_gamestate(const MoveData &);
    void _set_all_threat_counts(int);

};


}


/** Defined for your convenience, a string that represents the initial chess position
 *  in Forsyth-Edwards notation.
*/
#define FEN_STANDARD_CHESS_STARTING_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#endif // GKCHESS_ABSTRACTBOARD_H
