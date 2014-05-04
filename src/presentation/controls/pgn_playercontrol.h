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

#ifndef PGN_PLAYERCONTROL_H
#define PGN_PLAYERCONTROL_H

#include "gutil_strings.h"
#include <QWidget>

namespace GKChess{
class Board;

namespace UI{


class PGN_PlayerControl : public QWidget
{
    Q_OBJECT
    void *d;
public:

    explicit PGN_PlayerControl(Board *, QWidget *parent = 0);
    ~PGN_PlayerControl();

    void LoadPGN(const GUtil::String &);


public slots:

    void GotoNext();
    void GotoPrevious();
    void GotoFirst();
    void GotoLast();
    void GotoIndex(int);

};


}}

#endif // PGN_PLAYERCONTROL_H
