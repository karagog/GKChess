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

#include "gkchess_piece.h"
#include "gkchess_abstractboard.h"

NAMESPACE_GKCHESS;

class ISquare;
class PGN_MoveData;


/** Defines the game logic interface.
 *  This is the set of functions required to play chess with this library.
 *
 *  For each operation you supply a board that this class will operate on.
*/
class IGameLogic
{
public:

    /** An interface that encapsulates feedback from the user, such as choosing a promoted piece.
     *  The implementation should leverage the user interface to prompt the user for the given input.
    */
    class IPlayerResponse
    {
    public:

        /** This will be called whenever the game logic needs input from
         *  the user to decide what piece they want to promote to.
         *
         *  \note This is only relevant when generating move data by two squares. If you are
         *  generating moves from PGN then it's already in the notation (i.e. e8=Q)
        */
        virtual Piece ChoosePromotedPiece() = 0;

    };


    /** Describes different ways the board could be set up. */
    enum SetupTypeEnum
    {
        /** Causes the board to be cleared of all pieces. */
        Empty = 0,

        /** A standard game of chess. */
        StandardChess = 1,

        /** You can create your own custom board setups starting from this offset. */
        CustomSetupOffset = 100
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
    virtual void SetupNewGame(AbstractBoard &, SetupTypeEnum = StandardChess) const = 0;

    /** Validates the move. */
    virtual MoveValidationEnum ValidateMove(const AbstractBoard &,
                                            const ISquare &source,
                                            const ISquare &dest) const = 0;


    /** Returns a list of valid squares that the piece on the given square can move to. */
    virtual GUtil::Vector<ISquare const *> GetValidMovesForSquare(const AbstractBoard &,
                                                                  const ISquare &) const = 0;

    /** Creates a MoveData object from a PGN MoveData object. */
    virtual AbstractBoard::MoveData GenerateMoveData(const AbstractBoard &,
                                                     const PGN_MoveData &) const = 0;

    /** Creates a MoveData object from a source and dest square input.
     *  You must supply a user feedback object, so the game logic knows how the
     *  user wants to proceed in the event of a pawn promotion.
     *
     *  \note This function works with invalid moves, so if you care about
     *  validation you have to use ValidateMove() on the source and dest squares.
     *
     *  \returns A move data object, which is always populated except in the case of a
     *  pawn promotion that was cancelled, in which case it will be null (test with isnull())
    */
    virtual AbstractBoard::MoveData GenerateMoveData(const AbstractBoard &,
                                                     const ISquare &source,
                                                     const ISquare &dest,
                                                     IPlayerResponse *) const = 0;

    virtual void Move(AbstractBoard &, const AbstractBoard::MoveData &) = 0;

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_IGAMELOGIC_H
