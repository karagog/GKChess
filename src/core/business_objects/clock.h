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

#ifndef GKCHESS_CLOCK_H
#define GKCHESS_CLOCK_H

#include "gkchess_abstractclock.h"

namespace GKChess{


/** Describes a chess clock. */
class Clock :
        public AbstractClock
{
    Q_OBJECT
public:

    Clock(QObject * = 0);
    virtual ~Clock();


    /** \name AbstractClock interface
     *  \{
    */
    virtual void InitClock(ClockIndex, int, int);
    virtual void PushClock(ClockIndex);
    virtual void AdjustClock(ClockIndex, int);
    virtual void Pause();
    virtual QTime GetRemainingTime(ClockIndex);
    /** \} */

};


}

#endif // GKCHESS_CLOCK_H
