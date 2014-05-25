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
#include "gkchess_uiglobals.h"
#include <QMessageBox>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(QT);

NAMESPACE_GKCHESS1(UI);


ManageEngines::ManageEngines(EngineSettings *settings, PersistentData *app_settings, QWidget *parent)
    :QDialog(parent),
      m_settings(settings),
      ui(new Ui::ManageEngines)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);

    new QGridLayout(ui->pnl_options);
    ui->pnl_options->setContentsMargins(0,0,0,0);

    _engine_list_updated();

    connect(ui->lst_engines, SIGNAL(currentRowChanged(int)),
            this, SLOT(_current_changed(int)));

    const QString last_engine = app_settings->Value(GKCHESS_SETTING_LAST_ENGINE_USED).toString();
    QList<QListWidgetItem *> lst = ui->lst_engines->findItems(last_engine, Qt::MatchExactly);
    if(0 < lst.length())
        ui->lst_engines->setCurrentItem(lst[0]);
}

ManageEngines::~ManageEngines()
{
    delete ui;
}

void ManageEngines::_clear_options_panel()
{
    // Remove everything from the options panel
    QLayoutItem *item;
    while((item = ui->pnl_options->layout()->takeAt(0)) != 0)
        delete item;
}

void ManageEngines::_engine_list_updated()
{
    m_engineList = m_settings->GetEngineList();
    ui->lst_engines->clear();

    _clear_options_panel();

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

    _clear_options_panel();

    // This starts up the engine
    m_engineManager = new EngineManager(m_engineList[r], m_settings);

    // Then we can read the options and populate the form
    IEngine &e = m_engineManager->GetEngine();
    const IEngine::EngineInfo &info = e.GetEngineInfo();

    QGridLayout *gl = static_cast<QGridLayout *>(ui->pnl_options->layout());
    gl->addWidget(new QLabel(tr("Name:"), this), 0, 0);
    gl->addWidget(new QLabel(info.Name, this), 0, 1);

    gl->addWidget(new QLabel(tr("Author:"), this), 1, 0);
    gl->addWidget(new QLabel(info.Author, this), 1, 1);


    const QString default_tooltip = tr("Restore default value");
    QActionGroup *edited_actions = new QActionGroup(this);
    QActionGroup *default_actions = new QActionGroup(this);
    connect(edited_actions, SIGNAL(triggered(QAction*)), this, SLOT(_edited_action(QAction*)));
    connect(default_actions, SIGNAL(triggered(QAction*)), this, SLOT(_default_action(QAction *)));

    // Iterate through all the options and add labels and widgets to configure them:
    for(int i = 0; i < info.OptionNames.length(); ++i)
    {
        bool is_default_val = true;
        bool show_option = true;
        QWidget *editing_widget = 0;
        QAction *edited_action = 0;
        QString cur_name = info.OptionNames[i];
        IEngine::Option_t *opt = info.Options.At(cur_name);

        switch(opt->GetType())
        {
        case IEngine::Option_t::Check:
        {
            IEngine::CheckOption *co = static_cast<IEngine::CheckOption *>(opt);
            QCheckBox *cb = new QCheckBox(this);
            cb->setChecked(co->Value);
            editing_widget = cb;

            edited_action = new QAction(this);
            connect(cb, SIGNAL(toggled(bool)), edited_action, SLOT(trigger()));

            is_default_val = co->Value == co->Default;
        }
            break;
        case IEngine::Option_t::Spin:
        {
            IEngine::SpinOption *so = static_cast<IEngine::SpinOption *>(opt);
            QSpinBox *sb = new QSpinBox(this);
            sb->setRange(so->Min, so->Max);
            sb->setValue(so->Value);
            editing_widget = sb;

            edited_action = new QAction(this);
            connect(sb, SIGNAL(valueChanged(int)), edited_action, SLOT(trigger()));

            is_default_val = so->Value == so->Default;
        }
            break;
        case IEngine::Option_t::String:
        {
            IEngine::StringOption *so = static_cast<IEngine::StringOption *>(opt);
            QLineEdit *le = new QLineEdit(so->Value, this);
            editing_widget = le;

            edited_action = new QAction(this);
            connect(le, SIGNAL(textChanged(QString)), edited_action, SLOT(trigger()));

            is_default_val = so->Value == so->Default;
        }
            break;
        case IEngine::Option_t::Combo:
        {
            IEngine::ComboOption *co = static_cast<IEngine::ComboOption *>(opt);
            QComboBox *cb = new QComboBox(this);
            cb->addItems(co->Values);
            cb->setCurrentIndex(co->Values.indexOf(co->Value));
            editing_widget = cb;

            edited_action = new QAction(this);
            connect(cb, SIGNAL(currentIndexChanged(int)), edited_action, SLOT(trigger()));

            is_default_val = co->Value == co->Default;
        }
            break;
        default:
            show_option = false;
            break;
        }

        if(show_option)
        {
            gl->addWidget(new QLabel(cur_name, this), i + 2, 0);

            if(editing_widget)
                gl->addWidget(editing_widget, i + 2, 1);

            if(edited_action){
                edited_action->setData(i);
                edited_actions->addAction(edited_action);
            }

            QPushButton *pb = new QPushButton(tr("<-"), this);
            pb->setToolTip(default_tooltip);
            QAction *act = new QAction(this);
            act->setData(i);
            default_actions->addAction(act);

            if(is_default_val)
                pb->setEnabled(false);

            connect(pb, SIGNAL(released()), act, SLOT(trigger()));
            gl->addWidget(pb, i + 2, 2);
        }
    }
}

void ManageEngines::_edited_action(QAction *a)
{
    bool ok = false;
    int option_index = a->data().toInt(&ok);
    if(!ok || 0 > option_index)
        return;

    IEngine &engine = m_engineManager->GetEngine();
    IEngine::EngineInfo const &info = engine.GetEngineInfo();
    IEngine::Option_t *opt = info.Options.At(info.OptionNames[option_index]);

    bool default_value = true;
    QVariant value;
    QGridLayout *gl = static_cast<QGridLayout*>(ui->pnl_options->layout());
    QPushButton *btn_default = static_cast<QPushButton *>(gl->itemAtPosition(option_index + 2, 2)->widget());
    QWidget *editor_widget = gl->itemAtPosition(option_index + 2, 1)->widget();
    GASSERT(editor_widget);
    switch(opt->GetType())
    {
    case IEngine::Option_t::Check:
    {
        IEngine::CheckOption *co = static_cast<IEngine::CheckOption *>(opt);
        QCheckBox *cb = static_cast<QCheckBox *>(editor_widget);
        value = cb->isChecked();
        default_value = cb->isChecked() == co->Default;
    }
        break;
    case IEngine::Option_t::Spin:
    {
        IEngine::SpinOption *so = static_cast<IEngine::SpinOption *>(opt);
        QSpinBox *sb = static_cast<QSpinBox *>(editor_widget);
        value = sb->value();
        default_value = sb->value() == so->Default;
    }
        break;
    case IEngine::Option_t::String:
    {
        IEngine::StringOption *so = static_cast<IEngine::StringOption *>(opt);
        QLineEdit *le = static_cast<QLineEdit *>(editor_widget);
        value = le->text();
        default_value = le->text() == so->Default;
    }
        break;
    case IEngine::Option_t::Combo:
    {
        IEngine::ComboOption *co = static_cast<IEngine::ComboOption *>(opt);
        QComboBox *cb = static_cast<QComboBox *>(editor_widget);
        value = cb->currentText();
        default_value = cb->currentText() == co->Default;
    }
        break;
    default:
        break;
    }

    if(default_value){
        // Remove default settings from the config
        m_settings->RemoveOptionForEngine(m_engineManager->GetEngineName(), opt->Name);
    }
    else{
        // Add non-default values to the settings config
        m_settings->SetOptionForEngine(m_engineManager->GetEngineName(), opt->Name, value);
    }

    if(!value.isNull())
        engine.SetOption(opt->Name, value);

    btn_default->setEnabled(!default_value);
}

void ManageEngines::_default_action(QAction *a)
{
    bool ok = false;
    int option_index = a->data().toInt(&ok);
    if(!ok || 0 > option_index)
        return;

    IEngine::EngineInfo const &info = m_engineManager->GetEngine().GetEngineInfo();
    IEngine::Option_t *opt = info.Options.At(info.OptionNames[option_index]);
    switch(opt->GetType())
    {
    case IEngine::Option_t::Check:
    {
        IEngine::CheckOption *co = static_cast<IEngine::CheckOption *>(opt);
    }
        break;
    case IEngine::Option_t::Spin:
    {
        IEngine::SpinOption *so = static_cast<IEngine::SpinOption *>(opt);
    }
        break;
    case IEngine::Option_t::String:
    {
        IEngine::StringOption *so = static_cast<IEngine::StringOption *>(opt);
    }
        break;
    case IEngine::Option_t::Combo:
    {
        IEngine::ComboOption *co = static_cast<IEngine::ComboOption *>(opt);
    }
        break;
    default:
        break;
    }
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
            _engine_list_updated();
        }
    }
}

void ManageEngines::_edit()
{

}

void ManageEngines::_delete()
{
    int indx = ui->lst_engines->currentRow();
    if(0 <= indx){
        m_settings->RemoveEngine(m_engineList[indx]);
        _engine_list_updated();
    }
}

void ManageEngines::accept()
{
    m_settings->CommitChanges();
    QDialog::accept();
}

void ManageEngines::reject()
{
    m_settings->RejectChanges();
    QDialog::reject();
}


END_NAMESPACE_GKCHESS1;
