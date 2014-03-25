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

#ifndef GKCHESS_PGN_MOVE_DATA_H
#define GKCHESS_PGN_MOVE_DATA_H

#include "gkchess_piece.h"
#include "gutil_flags.h"

NAMESPACE_GKCHESS;


/** A queryable data object that holds all the data parsed from a move
    in a PGN file.
*/
struct PGN_MoveData
{
    /** Enumerates the types of moves. */
    enum MoveTypeEnum
    {
        /** Indicates a move where a piece was captured (x in chess notation). */
        Capture,

        /** Indicates a King-side castle (O-O in chess notation). */
        CastleNormal,

        /** Indicates a Queen-side castle (O-O-O in chess notation). */
        CastleQueenSide,

        /** Indicates a move where the opposing king is put in check (+ in chess notation). */
        Check,

        /** Indicates a move where the opposing king is put in checkmate (# in chess notation). */
        CheckMate,

        /** Indicates that the move was a blunder (?? in chess notation). */
        Blunder,

        /** Indicates that the move was a mistake (? in chess notation). */
        Mistake,

        /** Indicates that the move was dubious (?! in chess notation). */
        Dubious,

        /** Indicates that the move was interesting (!? in chess notation). */
        Interesting,

        /** Indicates the the move was a good move (! in chess notation). */
        Good,

        /** Indicates the the move was a brilliant move (!! in chess notation). */
        Brilliant
    };

    /** An object to help flag different move types. */
    GUTIL_DECLARE_FLAGS(MoveTypeFlags, MoveTypeEnum);

    /** The type of piece being moved. */
    Piece::PieceTypeEnum PieceMoved;

    /** If the piece was promoted, then this is anything but a Pawn. */
    Piece::PieceTypeEnum PiecePromoted;

    /** The source file is not always given, but if it is this will not be 0. */
    char SourceFile;

    /** The source rank is not always given, but if it is this will not be 0. */
    int SourceRank;

    /** The destination file is always given on non-castling moves. */
    char DestFile;

    /** The destination rank is always given on non-castling moves. */
    int DestRank;

    /** Contains information about the move, like whether it puts the
        opponent king in check, or if it is a blunder, etc...
    */
    MoveTypeFlags Flags;

    /** Holds the move text directly from the PGN file. */
    GUtil::String Text;

    /** If there is a comment for the move it is stored here. */
    GUtil::String Comment;

    /** Returns a human-readable description of the move.
     *  For example: "Pawn takes e5"
    */
    GUtil::String ToString() const;

    PGN_MoveData();
};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_PGN_MOVE_DATA_H
