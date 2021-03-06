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

#ifndef GKCHESS_UIGLOBALS_H
#define GKCHESS_UIGLOBALS_H

#include <gkchess_common.h>


// Here we define some globally used string constants
#define GKCHESS_SETTING_LAST_ENGINE_USED "LastEngineUsed"


NAMESPACE_GKCHESS1(UI);


/** Your application should call this to initialize the GUI resources
 *  like fonts and icons.
*/
extern void InitializeApplicationResources();


END_NAMESPACE_GKCHESS1;

#endif // GKCHESS_UIGLOBALS_H
