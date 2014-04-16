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

#ifndef GKCHESS_RNG_H
#define GKCHESS_RNG_H

#include "gkchess_globals.h"

NAMESPACE_GKCHESS;


/** Describes the Random Number Generator interface used by the library. */
class RNG
{
public:

    /** Returns a uniformly distributed random integer between the two bounds, inclusive. */
    static int RandInt(int min, int max);

};


END_NAMESPACE_GKCHESS;

#endif // GKCHESS_RNG_H
