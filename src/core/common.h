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

#ifndef GKCHESS_COMMON_H
#define GKCHESS_COMMON_H

#include <gutil/macros.h>

#define NAMESPACE_GKCHESS1( n )           namespace GKChess{ namespace n { enum{}
#define NAMESPACE_GKCHESS                 namespace GKChess{ enum{}

#define END_NAMESPACE_GKCHESS1            }} enum{}
#define END_NAMESPACE_GKCHESS             } enum{}

#define USING_NAMESPACE_GKCHESS1( ns )            using namespace GKChess::ns
#define USING_NAMESPACE_GKCHESS                   using namespace GKChess

#endif // GKCHESS_COMMON_H
