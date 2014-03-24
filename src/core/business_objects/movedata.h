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

#ifndef GKCHESS_MOVEDATA_H
#define GKCHESS_MOVEDATA_H

#include "gkchess_piece.h"
#include "gutil_vector.h"
#include <QString>

NAMESPACE_GKCHESS;

class ISquare;


/** Holds all the information we need to do and undo a move. */
struct MoveData
{
    /** The starting square.  If the move was a castle this will be null. */
    ISquare const *Source;

    /** The ending square.  If the move was a castle this will be null. */
    ISquare const *Destination;

    /** The type of castle is either 0=No Casle, 1=Castle Normal, -1=Castle Queenside. */
    enum CastleTypeEnum
    {
        NoCastle = 0,
        CastleNormal = 1,
        CastleQueenside = -1
    }
    CastleType;

    /** The piece being moved. */
    Piece PieceMoved;

    /** The captured piece, if any. If this is type NoPiece then the
     *  move did not involve a capture.
    */
    Piece PieceCaptured;

    /** The piece that was promoted, if any. If this is type NoPiece then the
     *  move did not involve a promotion. */
    Piece PiecePromoted;

    /** Stores a list of moves that could have been done instead of this.
     *  This will help support deep position exploration.
    */
    GUtil::DataObjects::Vector<MoveData> AlternateMoves;

    MoveData();

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_MOVEDATA_H
