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
#include "gkchess_pgn_movedata.h"
#include "gkchess_globals.h"

NAMESPACE_GKCHESS;


/** Holds all data for one game in PGN. */
struct PGN_GameData
{
    /** The string tags that precede the moves. */
    GUtil::Map<GUtil::String, GUtil::String> Tags;

    /** All the move data. */
    GUtil::List<PGN_MoveData> Moves;

    void clear(){ Tags.Clear(); Moves.Empty(); }

};



/** Parses a PGN string and gives you the move text and tags.

    Simply pass a PGN string to the constructor and then access the
    data directly. An exception will throw from the constructor if there
    is a problem with parsing.
*/
class PGN_Parser
{
    GUTIL_STATIC_CLASS(PGN_Parser);
public:

    /** Parses the UTF-8 string. Throws an exception on error. */
    static GUtil::List<PGN_GameData> ParseString(const GUtil::String &utf8);

    /** Parses the file with UTF-8 encoding. Throws an exception on error. */
    static GUtil::List<PGN_GameData> ParseFile(const GUtil::String &filename);

    /** Parses a single PGN move into a move data object.
     *  Example move strings are:  e4 e2e4 e2-e4 O-O Nxg5+ d8=Q
    */
    static PGN_MoveData CreateMoveDataFromString(const GUtil::String &);

};


END_NAMESPACE_GKCHESS;

#endif // PGN_PARSER_H
