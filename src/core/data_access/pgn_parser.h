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

#include "gkchess_piece.h"
#include "gkchess_pgn_move_data.h"
#include "gkchess_board.h"
#include "gutil_map.h"

NAMESPACE_GKCHESS;


/** Parses a PGN string and gives you the move text and tags. */
class PGN_Parser
{
public:

    /** Parses the UTF-8 string. Throws an exception on error. */
    PGN_Parser(const GUtil::DataObjects::String &utf8);

    /** Holds all data parsed by the PGN_Parser. */
    struct Data
    {
        /** The string tags that precede the moves. */
        GUtil::DataObjects::Map<GUtil::DataObjects::String, GUtil::DataObjects::String> Tags;

        /** Stores the initial position given in the PGN file, in the SetUp tag in X-FEN notation.
         *  If the board is null, then you can assume the standard chess starting position.
        */
        GUtil::Utils::SmartPointer<Board> InitialPosition;
        
        /** All the move data. */
        GUtil::DataObjects::Vector<PGN_MoveData> Moves;
        
        /** The result is 1 if White won, -1 if Black won and 0 if it was a draw.
         *  It is MAX_INT if the result was not given.
        */
        int Result;

        Data();
        Data(const Data &);
    };
    
    /** Returns the data that was parsed from the file. */
    Data const &GetData() const{ return m_data; }

    /** Returns a board object with the position given in X-FEN notation.
     *
     *  Since X-FEN is backwards compatible with FEN (Forsyth-Edwards Notation) you
     *  can also pass FEN strings to it.
    */
    static Board FromX_FEN(const GUtil::DataObjects::String &);

private:

    Data m_data;

    /** Populates the heading tags and returns an iterator to the start of the move data section. */
    typename GUtil::DataObjects::String::UTF8ConstIterator
        _parse_heading(const GUtil::DataObjects::String &);

    void _parse_moves(const GUtil::DataObjects::String &);

    bool _new_movedata_from_string(PGN_MoveData &, const GUtil::DataObjects::String &);

};


END_NAMESPACE_GKCHESS;

#endif // PGN_PARSER_H
