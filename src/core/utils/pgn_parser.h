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

#ifndef PGN_PARSER_H
#define PGN_PARSER_H

#include "gutil_map.h"
#include "gutil_strings.h"
#include "gutil_flags.h"
#include "gkchess_globals.h"

NAMESPACE_GKCHESS;


/** Parses a PGN string and gives you the move text and tags.

    Simply pass a PGN string to the constructor and then access the
    data directly. An exception will throw from the constructor if there
    is a problem with parsing.
*/
class PGN_Parser
{
    GUTIL_STATIC_CLASS(PGN_Parser);
public:

    /** A queryable data object that holds all the data parsed from a move
        in a PGN file.
    */
    struct MoveData
    {
        /** Enumerates the types of moves. */
        enum MoveTypeEnum
        {
            /** Indicates a move where a piece was captured (x in chess notation). */
            Capture,

            /** Indicates a King-side castle (O-O in chess notation). */
            CastleHSide,

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

        /** Returns the full-move number for this move, which does not by itself distinguish between
         *  white and black's moves.
        */
        int MoveNumber;

        /** The type of piece being moved. */
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

        /** Holds the move text directly from the PGN file, without any comment. */
        GUtil::String MoveText;

        /** If there is a comment for the move it is stored here. */
        GUtil::String Comment;

        /** Returns a human-readable description of the move.
         *  For example: "Pawn takes e5 {This is a comment for the move}"
        */
        GUtil::String ToString() const;

        MoveData();
    };

    /** Holds all data for one game in PGN. */
    struct GameData
    {
        /** The string tags that precede the moves. */
        GUtil::Map<GUtil::String, GUtil::String> Tags;

        /** All the move data. */
        GUtil::Vector<MoveData> Moves;
    };

    /** Parses the UTF-8 string. Throws an exception on error. */
    static GUtil::List<GameData> ParseString(const GUtil::String &utf8);

    /** Parses the file with UTF-8 encoding. Throws an exception on error. */
    static GUtil::List<GameData> ParseFile(const GUtil::String &filename);

};


END_NAMESPACE_GKCHESS;

#endif // PGN_PARSER_H
