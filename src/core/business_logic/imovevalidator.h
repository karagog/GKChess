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

#ifndef GKCHESS_IMOVEVALIDATOR_H
#define GKCHESS_IMOVEVALIDATOR_H

#include "gkchess_globals.h"
#include "gutil_vector.h"

/** An interface for a class that validates chess moves. */
NAMESPACE_GKCHESS;

class AbstractBoard;
class ISquare;


class IMoveValidator
{
public:

    /** Encodes the different possible results of a move validation. */
    enum MoveValidationEnum
    {
        /** Means the move is valid. */
        ValidMove = 0,

        /** Invalid move because pieces can't move that way according to the rules of chess.
         *  (i.e. a pawn moving backwards).
        */
        InvalidTechnical = 1,

        /** Invalid because you would be leaving the king in check. */
        InvalidCheck = 2,

        /** The source square does not have a piece on it. */
        InvalidEmptySquare = 3,

        /** Invalid because there was a problem with the parameters you gave the function.
         *  For example if you didn't specify a source and destination square.
        */
        InvalidInputError = 4,


        /** If you are extending this class for your own custom rules then you'll base
         *  your validation types with this value.
        */
        CustomInvalidMoveOffset = 100
    };


    /** Validates the move.  For the best possible performance there are constraints on
     *  the valid inputs, otherwise the results are undefined.  The constraints are as follows:
     *
     *  * The board and squares must be valid.
     *  * The squares must belong to the board.
    */
    virtual MoveValidationEnum ValidateMove(const AbstractBoard &board,
                                            const ISquare &source,
                                            const ISquare &dest) const = 0;


    /** Returns a list of valid squares that the piece on the given square can move to. */
    virtual GUtil::Vector<ISquare const *> GetValidMovesForSquare(const AbstractBoard &,
                                                                  const ISquare &) const = 0;


};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_IMOVEVALIDATOR_H
