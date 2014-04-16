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

#ifndef GKCHESS_CHESS960_H
#define GKCHESS_CHESS960_H

#include "gutil_strings.h"
#include "gkchess_globals.h"

NAMESPACE_GKCHESS;


/** Static class that describes the utility functions needed for Chess960. */
class Chess960
{
public:

    /** Returns a list of all possible starting positions for Chess960 in X-FEN notation.
        The first time you call this it will take some time to generate the
        positions, but afterwards they will be cached in a static variable.
        
        \note The order of the return list is technically undefined, but in
        reality they will be sorted alphanumerically.
    */
    static GUtil::Vector<GUtil::String> const &GetAllStartingPositions();
    
    /** Returns a random Chess960 starting position.
        \note Each position is guaranteed to be equally likely.
    */
    static GUtil::String const &GetRandomStartingPosition();
    
};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_CHESS960_H
