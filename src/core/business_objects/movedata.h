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

NAMESPACE_GKCHESS;

class Square;


/** Holds all the information we need to do a move on a Board object. */
struct MoveData
{
    /** The half-move number for the move. */
    int PlyNumber;

    /** The starting square.  If the move was a castle this will be null. */
    Square const *Source;

    /** The ending square.  If the move was a castle this will be null. */
    Square const *Destination;

    /** The type of castle. */
    enum CastleTypeEnum
    {
        NoCastle = 0,

        /** In standard chess this indicates a king-side castle. */
        CastleHSide,

        /** In standard chess this indicates a queen-side castle. */
        CastleASide
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

    /** Returns true if this is a null move data (default constructed). */
    bool IsNull() const{ return -1 == PlyNumber; }

    /** Returns whose move this is, according to the ply number. */
    Piece::AllegienceEnum Whose() const{ return 0 == ((PlyNumber - 1) & 0x1) ? Piece::White : Piece::Black; }

    MoveData()
        :PlyNumber(-1),
          Source(0),
          Destination(0),
          CastleType(NoCastle)
    {}

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_MOVEDATA_H
