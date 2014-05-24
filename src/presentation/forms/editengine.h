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

#ifndef NEWENGINE_H
#define NEWENGINE_H

#include <QDialog>

namespace Ui {
class EditEngine;
}

namespace GKChess{ namespace UI{


class EditEngine : public QDialog
{
    Q_OBJECT
    Ui::EditEngine *ui;
    QString name, path;
public:

    explicit EditEngine(QWidget *parent = 0);
    ~EditEngine();

    QString GetName() const{ return name; }
    QString GetExePath() const{ return path; }


private slots:

    void _select_exe();
    void _name_changed();
    void _path_changed();

};


}}

#endif // NEWENGINE_H
