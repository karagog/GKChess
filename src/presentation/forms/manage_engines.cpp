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
#include "ui_manage_engines.h"
#include "gkchess_globals.h"
#include "gkchess_enginesettings.h"
#include "gkchess_editengine.h"
#include "gkchess_enginemanager.h"
#include "gkchess_iengine.h"
#include <QMessageBox>
#include <QGridLayout>
#include <QLabel>
USING_NAMESPACE_GUTIL1(QT);

NAMESPACE_GKCHESS1(UI);


ManageEngines::ManageEngines(EngineSettings *settings, QWidget *parent)
    :QDialog(parent),
      m_settings(settings),
      ui(new Ui::ManageEngines)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);

    connect(settings, SIGNAL(NotifyEnginesUpdated()),
            this, SLOT(_engine_list_updated()));
    connect(ui->lst_engines, SIGNAL(currentRowChanged(int)),
            this, SLOT(_current_changed(int)));

    new QGridLayout(ui->pnl_options);
    ui->pnl_options->setContentsMargins(0,0,0,0);

    _engine_list_updated();
}

void ManageEngines::_engine_list_updated()
{
    m_engineList = m_settings->GetEngineList();
    ui->lst_engines->clear();

    // Remove everything from the options panel
    QWidgetList widgets = ui->pnl_options->findChildren<QWidget *>();
    foreach(QWidget * widget, widgets){
        delete widget;
    }

    if(0 < m_engineList.length()){
        ui->lst_engines->addItems(m_engineList);
        ui->lst_engines->setCurrentRow(0);
        ui->btn_del->setEnabled(true);
    }
    else{
        ui->btn_del->setEnabled(false);
    }
}

void ManageEngines::_current_changed(int r)
{
    // Set up the options panel
    if(0 > r)
        return;

    QString cur_engine = m_engineList[r];

    QString engine_path = m_settings->GetEnginePath(cur_engine);
    if(engine_path.isEmpty()){
        GASSERT(false);
        return;
    }

    QVariantMap vals = m_settings->GetOptionsForEngine(cur_engine);

    // This starts up the engine
    m_engineManager = new EngineManager(engine_path);

    // Then we can read the options and populate the form
    IEngine &e = m_engineManager->GetEngine();
    const IEngine::EngineInfo &info = e.GetEngineInfo();

    QGridLayout *gl = static_cast<QGridLayout *>(ui->pnl_options->layout());
    gl->addWidget(new QLabel(tr("Name:"), this), 0, 0);
    gl->addWidget(new QLabel(info.Name, this), 0, 1);

    gl->addWidget(new QLabel(tr("Author:"), this), 1, 0);
    gl->addWidget(new QLabel(info.Author, this), 1, 1);
}

void ManageEngines::_add()
{
    EditEngine ne(this);
    if(QDialog::Accepted == ne.exec())
    {
        QString name = ne.GetName();
        QString path = ne.GetExePath();
        if(!name.isEmpty() && !path.isEmpty())
        {
            if(m_settings->GetEngineList().contains(name))
            {
                if(QMessageBox::Ok !=
                        QMessageBox::warning(this,
                                             tr("Engine Exists"),
                                             tr("The engine name already exists.  If you want to overwrite it with the default settings, press OK."),
                                             QMessageBox::Ok | QMessageBox::Cancel,
                                             QMessageBox::Cancel))
                    return;
            }

            m_settings->SetEnginePath(name, path);
        }
    }
}

void ManageEngines::_delete()
{
    int indx = ui->lst_engines->currentRow();
    if(0 <= indx)
        m_settings->RemoveEngine(m_engineList[indx]);
}

ManageEngines::~ManageEngines()
{
    delete ui;
}


END_NAMESPACE_GKCHESS1;
