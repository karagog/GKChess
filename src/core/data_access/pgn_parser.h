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

#include "gkchess_globals.h"
#include "gutil_strings.h"
#include "gutil_map.h"

NAMESPACE_GKCHESS;


/** Parses a PGN string and gives you the move text and tags. */
class PGN_Parser
{
    GUtil::DataObjects::Map<GUtil::DataObjects::String, GUtil::DataObjects::String> m_tags;
    GUtil::DataObjects::Vector<GUtil::DataObjects::String> m_moves;
public:

    /** Enumerates the types of moves. */
    enum MoveTypeEnum
    {
        /** Indicates a move where a piece was captured. */
        Capture,
        
        /** Indicates a King-side castle. */
        CastleNormal,
        
        /** Indicates a Queen-side castle. */
        CastleQueenSide,
        
        /** Indicates a move where the opposing king is put in check. */
        Check,
        
        /** Indicates a move where the opposing king is put in checkmate. */
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

    /** A queryable data object that holds all the data parsed from a move
        in a PGN file.
    */
    struct MoveData
    {
        /** The type of piece being moved. */
        Piece::PieceTypeEnum Piece;
        
        /** If the piece was promoted, then this is anything but a Pawn. */
        Piece::PieceTypeEnum PiecePromoted;
        
        /** The source file is not always given, but if it is this will not be -1. */
        int SourceFile;
        
        /** The source rank is not always given, but if it is this will not be -1. */
        int SourceRank;
        
        /** The destination file is always given on non-castling moves. */
        int DestFile;
        
        /** The destination rank is always given on non-castling moves. */
        int DestRank;
        
        /** Contains information about the move, like whether it puts the
            opponent king in check, or if it is a blunder, etc...
        */
        MoveTypeFlags Flags;
        
        /** Holds the move text directly from the PGN file. */
        String Text;
        
        /** If there is a comment for the move it is stored here. */
        String Comment;
        
        MoveData();
    };
    
    /** Holds a pair of moves. */
    struct MoveDataPair
    {
        /** This is white's move. */
        MoveData WhitesMove;
        
        /** This is black's move. */
        MoveData BlacksMove;
    };

    /** Parses the UTF-8 string. Throws an exception on error. */
    PGN_Parser(const GUtil::DataObjects::String &utf8);

    /** Returns the tag pairs. */
    const GUtil::DataObjects::Map<GUtil::DataObjects::String, GUtil::DataObjects::String> GetTags() const
    { return m_tags; }

    /** Returns the move texts. */
    GUtil::DataObjects::Vector<GUtil::DataObjects::String> const &GetMoves() const
    { return m_moves;  }


private:

    /** Populates the heading tags and returns an iterator to the start of the move data section. */
    typename GUtil::DataObjects::String::UTF8ConstIterator
        _parse_heading(const GUtil::DataObjects::String &);

    void _parse_moves(const GUtil::DataObjects::String &);

};


END_NAMESPACE_GKCHESS;

#endif // PGN_PARSER_H
