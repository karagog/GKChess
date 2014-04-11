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

#ifndef GKCHESS_IGAMESTATE_H
#define GKCHESS_IGAMESTATE_H

#include "gkchess_piece.h"

NAMESPACE_GKCHESS;


/** Other than the positions of the pieces, this stores the rest of the information
 *  needed to represent the complete state of a chess game.
*/
class IGameState
{
public:

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

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_IGAMESTATE_H
