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

#ifndef GKCHESS_IGAMELOGIC_H
#define GKCHESS_IGAMELOGIC_H

#include "gkchess_movedata.h"
#include "gkchess_igamestate.h"
#include "gutil_vector.h"

NAMESPACE_GKCHESS;


class AbstractBoard;
class ISquare;
class Piece;
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



/** Defines the game logic interface.
 *
 *  This is the set of functions required to play chess.  It is meant to be used with your
 *  chess board implementation.
*/
class IGameLogic
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



    /** Sets up the board for a new game. */
    virtual void SetupNewGame(SetupTypeEnum = SetupStandardChess) = 0;


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

    /** Returns the current game state, as a reference for best performance. */
    virtual IGameState const &GameState() const = 0;

    /** Returns the current game state, as a reference for best performance. You modify it directly. */
    virtual IGameState &GameState() = 0;


    /** Validates the move. */
    virtual MoveValidationEnum ValidateMove(const ISquare &source,
                                            const ISquare &dest) const = 0;


    /** Returns a list of valid squares that the piece on the given square can move to. */
    virtual GUtil::Vector<ISquare const *> GetValidMovesForSquare(const ISquare &) const = 0;

    /** Creates a MoveData object from a PGN MoveData object. */
    virtual MoveData GenerateMoveData(const PGN_MoveData &) const = 0;

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
    virtual MoveData GenerateMoveData(const ISquare &source,
                                      const ISquare &dest,
                                      IPlayerResponse * = 0) const = 0;

    /** Executes the move described by the move data object and advances the game state.
     *
     *  This should take care to call ValidateMove() to apply validation and return the result.
     *  If it was an invalid move, then the state of the board must not change.
    */
    virtual MoveValidationEnum Move(const MoveData &) = 0;

    /** A convenience function generates move data, validates and executes the move. */
    MoveValidationEnum Move2(const ISquare &src, const ISquare &dest, IPlayerResponse *pr = 0){
        return Move(GenerateMoveData(src, dest, pr));
    }

    /** Indicate that the given side wants to resign. */
    virtual void Resign(Piece::AllegienceEnum) = 0;

    virtual ~IGameLogic(){}

};


END_NAMESPACE_GKCHESS;


/** Defined for your convenience, a string that represents the initial chess position
 *  in Forsyth-Edwards notation. You can pass this to the PGN parser to generate a board
 *  object.
*/
#define FEN_STANDARD_CHESS_STARTING_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


#endif // GKCHESS_IGAMELOGIC_H
