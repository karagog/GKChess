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

#include "clock.h"
#include "gkchess_globals.h"
#include "gutil_exception.h"
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;


Clock::Clock(QObject *p)
    :AbstractClock(p)
{}

Clock::~Clock(){}

void Clock::InitClock(ClockIndex, int, int)
{
    THROW_NEW_GUTIL_EXCEPTION(NotImplementedException);
}

void Clock::PushClock(ClockIndex)
{
    THROW_NEW_GUTIL_EXCEPTION(NotImplementedException);
}

void Clock::AdjustClock(ClockIndex, int)
{
    THROW_NEW_GUTIL_EXCEPTION(NotImplementedException);
}

void Clock::Pause()
{
    THROW_NEW_GUTIL_EXCEPTION(NotImplementedException);
}

QTime Clock::GetRemainingTime(ClockIndex)
{
    THROW_NEW_GUTIL_EXCEPTION(NotImplementedException);
    return QTime();
}


END_NAMESPACE_GKCHESS;
