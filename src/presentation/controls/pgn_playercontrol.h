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

#include <QWidget>

namespace Ui {
class PGN_PlayerControl;
}

namespace GKChess{ namespace UI{


class PGN_PlayerControl : public QWidget
{
    Q_OBJECT

public:
    explicit PGN_PlayerControl(QWidget *parent = 0);
    ~PGN_PlayerControl();

private:
    Ui::PGN_PlayerControl *ui;
};


}}

#endif // PGN_PLAYERCONTROL_H
