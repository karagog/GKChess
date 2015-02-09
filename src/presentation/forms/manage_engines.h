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

#ifndef MANAGE_ENGINES_H
#define MANAGE_ENGINES_H

#include <gutil/smartpointer.h>
#include <gutil/qt_settings.h>
#include <QDialog>

namespace Ui{
class ManageEngines;
}

namespace GKChess{
class EngineSettings;
class EngineManager;
namespace UI{


class ManageEngines : public QDialog
{
    Q_OBJECT
    EngineSettings *m_settings;
    Ui::ManageEngines *ui;

    GUtil::StringList m_engineList;
    GUtil::SmartPointer<EngineManager> m_engineManager;

    QWidgetList m_optionItems;

public:

    explicit ManageEngines(EngineSettings *engine_settings, GUtil::Qt::Settings *app_settings, QWidget *parent = 0);
    ~ManageEngines();


public slots:

    virtual void accept();
    virtual void reject();


private slots:

    void _engine_list_updated();
    void _current_changed(int);
    void _add();
    void _edit();
    void _delete();

    void _edited_action(QAction *);
    void _default_action(QAction *);

    void _clear_options_panel();

};


}}

#endif // MANAGE_ENGINES_H
