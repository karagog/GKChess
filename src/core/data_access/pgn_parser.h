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
