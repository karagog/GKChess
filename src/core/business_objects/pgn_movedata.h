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

#ifndef GKCHESS_PGN_MOVEDATA_H
#define GKCHESS_PGN_MOVEDATA_H

#include <gutil/string.h>
#include <gutil/flags.h>
#include <gkchess_common.h>
#include <QList>

NAMESPACE_GKCHESS;


/** A queryable data object that holds all the data parsed from a move
    in a PGN file.
*/
class PGN_MoveData
{
public:

    /** Enumerates the types of moves. */
    enum MoveTypeEnum
    {
        /** Indicates a move where a piece was captured (x in chess notation). */
        Capture,

        /** Indicates a King-side castle (O-O in chess notation). */
        CastleHSide,

        /** Indicates a Queen-side castle (O-O-O in chess notation). */
        CastleASide,

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

    /** Returns the full-move number for this move, which does not by itself distinguish between
     *  white and black's moves.
    */
    int MoveNumber;

    /** The type of piece being moved, in PGN notation ('K' for king, 'Q' for queen, etc...) */
    char PieceMoved;

    /** If the piece was promoted, then this is anything but 0. */
    char PiecePromoted;

    /** The source file is not always given, but if it is this will not be 0. */
    GINT8 SourceFile;

    /** The source rank is not always given, but if it is this will not be 0. */
    GINT8 SourceRank;

    /** The destination file is always given on non-castling moves. */
    GINT8 DestFile;

    /** The destination rank is always given on non-castling moves. */
    GINT8 DestRank;

    /** Contains information about the move, like whether it puts the
        opponent king in check, or if it is a blunder, etc...
    */
    MoveTypeFlags Flags;

    /** If there is a comment for the move it is stored here. */
    GUtil::String Comment;

    /** Stores any variants for this move from the PGN file.
     *  A variant is interpreted as if this move was unplayed and the variant move was played instead.
    */
    QList<PGN_MoveData> Variants;

    /** Returns the normal PGN form of the move.
     *  For example, any of: e4 e5 Nf3 O-O
    */
    GUtil::String ToString() const;

    /** Returns a human-readable description of the move.
     *  For example: "Pawn takes e5 {This is a comment for the move}"
    */
    GUtil::String ToPrettyString() const;

    PGN_MoveData();

};


END_NAMESPACE_GKCHESS;


#endif // GKCHESS_PGN_MOVEDATA_H
