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
   
    /** Returns the starting position at the given index.
    
        The index must fall in the range [0, 959], and corresponds to
        Scharnagl's ordering with a 0-based index.
    */
    static GUtil::String GetStartingPosition(int);
    
    /** Returns a random Chess960 starting position.

        Each position is guaranteed to be equally likely.
        
        \param indx A pointer to an integer that, if non-zero, will be updated with the
        index of the starting position so you can find it again later.
    */
    static GUtil::String GetRandomStartingPosition(int *indx = 0);
    
};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_CHESS960_H
