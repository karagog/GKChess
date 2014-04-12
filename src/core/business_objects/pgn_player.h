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

#ifndef GKCHESS_PGN_PLAYER_H
#define GKCHESS_PGN_PLAYER_H

#include "gkchess_pgn_parser.h"
#include "gutil_strings.h"
#include "gutil_map.h"
#include "gutil_flags.h"

NAMESPACE_GKCHESS;


/** Plays a PGN file.
 *  Allows you to load a PGN file and step through it.
*/
class PGN_Player
{
    AbstractBoard *m_board;
    GUtil::SmartPointer<PGN_Parser::Data_t> m_pgnData;
public:

    /** Constructs a PGN player with the given game logic.  It will not take ownership. */
    PGN_Player(AbstractBoard *);

    /** Parses the PGN data from the given string. */
    void LoadFromString(const GUtil::String &);
    
    /** Parses the PGN data from the given file. */
    void LoadFromFile(const GUtil::String &);

    /** Returns true if PGN data has been successfully loaded. */
    bool IsLoaded() const{ return m_pgnData; }
    
    /** Unloads the PGN data.  After calling this, "IsLoaded()" returns false. */
    void Clear();
    
    /** After loading a PGN string, you can step through the moves with this. */
    void Next();
    
    /** After loading a PGN string, you can step back through the moves with this. */
    void Previous();

    /** Returns the game board object used by the PGN player. */
    const AbstractBoard &GetBoard() const{ return *m_board; }

    /** Returns the game board object used by the PGN player. */
    AbstractBoard &GetBoard(){ return *m_board; }

    /** Returns the moves from the file.  This will be null if data has not been loaded. */
    PGN_Parser::Data_t const *GetPGNData() const{ return m_pgnData; }

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_PGN_PLAYER_H
