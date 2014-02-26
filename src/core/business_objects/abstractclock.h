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

#ifndef GKCHESS_ABSTRACTCLOCK_H
#define GKCHESS_ABSTRACTCLOCK_H

#include <QTime>

namespace GKChess{


/** Describes a chess clock interface. */
class AbstractClock :
    public QObject
{
    Q_OBJECT
public:

    AbstractClock(QObject * = 0);
    virtual ~AbstractClock();
    
    /** Describes the clock indexes for the chess clock. */
    enum ClockIndex
    {
        One = 1,
        Two = 2
    };

    /** Initializes the clock at the index to the given time.
        You may optionally pass a delay parameter, which will
        unilaterally apply a delay (in seconds) to one of the clocks.
    */
    virtual void InitClock(ClockIndex, int time_in_minutes, int delay_in_secs = 0) = 0;
    
    /** Pushes the clock at the given index. This is equivalent to pressing
        the button on a chess clock. i.e. Pushing index One starts the clock
        on index Two and vice versa.
    */
    virtual void PushClock(ClockIndex) = 0;
    
    /** Adds the given number of milliseconds to the clock
        at the given index. If it is negative then it will 
        reduce the time on the clock.
    */
    virtual void AdjustClock(ClockIndex, int milliseconds) = 0;
    
    /** Pauses the clock. Call PushClock() to make it run again.
        Call InitClock() to start a new game.
    */
    virtual void Pause() = 0;
    
    /** Returns the amount of time remaining on the clock at the given index. */
    virtual QTime GetRemainingTime(ClockIndex) = 0;
    
    
signals:

    /** This signal is emitted when the time runs out for one of the clocks. 
        \param clock_index Refers to a value from ClockIndex enum.
    */
    void TimeIsUp(int clock_index);

};


}

#endif // GKCHESS_ABSTRACTCLOCK_H
