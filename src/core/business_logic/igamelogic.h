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

#include "gkchess_imovevalidator.h"

NAMESPACE_GKCHESS;

class PGN_MoveData;
class MoveData;


/** Defines the game logic interface.
 *  This is the set of functions required to play chess with this library.
*/
class IGameLogic :
        public IMoveValidator
{
public:

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


    /** Returns the game board. */
    virtual AbstractBoard const &GetBoard() const = 0;

    /** Sets up the board for a new game. */
    virtual void SetupNewGame(SetupTypeEnum = StandardChess) = 0;

    /** Moves based on a PGN_MoveData object, which is created from the PGN parser. */
    virtual void Move(const PGN_MoveData &) = 0;

    /** Moves based on a MoveData object. */
    virtual void Move(const MoveData &) = 0;


    /** Convenience function clears the board. */
    void Clear(){ SetupNewGame(Empty); }

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_IGAMELOGIC_H
