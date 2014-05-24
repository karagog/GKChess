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

#include "editengine.h"
#include "ui_editengine.h"
#include "gkchess_globals.h"
#include <QFileDialog>

NAMESPACE_GKCHESS1(UI);


EditEngine::EditEngine(QWidget *parent)
    :QDialog(parent),
      ui(new Ui::EditEngine)
{
    ui->setupUi(this);
    ui->line_name->setFocus();
}

EditEngine::~EditEngine()
{
    delete ui;
}

void EditEngine::_select_exe()
{
    QString filter;
#ifdef Q_OS_WIN
    filter = "*.exe";
#endif

    QString filename = QFileDialog::getOpenFileName(this, tr("Select Engine"), QString(), filter);
    if(!filename.isEmpty())
    {
        ui->line_path->setText(filename);
    }
}

void EditEngine::_name_changed()
{
    QString new_name = ui->line_name->text();

    // Validate and set the name
    name = new_name.trimmed();
}

void EditEngine::_path_changed()
{
    QString fn = ui->line_path->text();
    QFile f(fn);
    if(f.exists()){
        ui->lbl_warning->clear();
        path = fn;
    }
    else{
        ui->lbl_warning->setText(tr("Engine does not exist!"));
        path.clear();
    }
}


END_NAMESPACE_GKCHESS1;
