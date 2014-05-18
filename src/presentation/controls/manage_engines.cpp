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

#include "manage_engines.h"
#include "gkchess_globals.h"
#include <QFormLayout>
USING_NAMESPACE_GUTIL1(QT);
NAMESPACE_GKCHESS1(UI);


ManageEngines::ManageEngines(PersistentData *settings, QWidget *parent)
    :QDialog(parent),
      m_settings(settings)
{
    setWindowModality(Qt::ApplicationModal);
    new QFormLayout(this);
}


END_NAMESPACE_GKCHESS1;
