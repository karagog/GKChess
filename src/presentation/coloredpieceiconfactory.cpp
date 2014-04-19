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

#include "coloredpieceiconfactory.h"
#include "gkchess_piece.h"
#include "gutil_flags.h"
#include "gutil_thread.h"
#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QDesktopServices>
#include <QImage>
#include <QUuid>
#include <QtConcurrentRun>
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(QT);

NAMESPACE_GKCHESS1(UI);


static QString __get_new_temp_icon_dir()
{
    return QDir::toNativeSeparators(
                QString("%1/%2/temp_icons/%3")
                .arg(QDesktopServices::storageLocation(QDesktopServices::TempLocation))
                .arg(qApp->applicationName())
                .arg(QUuid::createUuid()));
}

static QString __get_filename_for_piece(Piece const &p)
{
    return QString("%1%2.png")
            .arg(p.GetAllegience() == Piece::White ? "w" : "b")
            .arg(QChar(p.ToFEN()).toLower());
}

static void __add_pieces_icons_to_index(Map<int, QIcon> &m, const QString &dir_deploy, Piece::AllegienceEnum a)
{
    for(Piece::PieceTypeEnum t = Piece::King; t <= Piece::Pawn; t = (Piece::PieceTypeEnum)((int)t + 1))
    {
        Piece p(t, a);
        m[p.UnicodeValue()] = QIcon(
                    QDir::toNativeSeparators(dir_deploy + "/" + __get_filename_for_piece(p))
                    );
    }
}

static void __make_sure_dir_exists(const QString &dirname)
{
    QDir dir(dirname);
    if(!dir.exists()){
        if(!dir.mkpath(dirname))
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "Directory does not exist, and I couldn't create it");
    }
}

// Removes a 1-deep directory with just files in it.
// This does nothing if the directory does not exist
static void __remove_dir(const QString &d)
{
    if(!d.isEmpty()){
        QDir dir(d);
        if(dir.exists())
        {
            // First remove all files
            QDirIterator iter(d);
            while(iter.hasNext())
                dir.remove(iter.next());

            // Then remove the empty directory
            dir.rmdir(d);
        }
    }
}


ColoredPieceIconFactory::ColoredPieceIconFactory(const QString &dirname, const QColor &light_color, const QColor &dark_color, QObject *p)
    :IFactory_PieceIcon(p),
      dir_templates(dirname),
      light_progress(-1),
      dark_progress(-1),
      index_finished_updating(false),
      shutting_down(false)
{
    // Make sure the template icons they gave us are valid for what we're doing
    _validate_template_icons();

    connect(this, SIGNAL(NotifyIconsUpdated()), this, SLOT(_update_index()),
            Qt::QueuedConnection);

    // Start up the background thread
    bg_threadRef = QtConcurrent::run(this, &ColoredPieceIconFactory::_worker_thread);

    ChangeColors(light_color, dark_color);
}

ColoredPieceIconFactory::~ColoredPieceIconFactory()
{
    // Take down the background thread
    QMutexLocker lkr (&this_lock);
    shutting_down = true;
    lkr.unlock();
    something_to_do.wakeOne();
    bg_threadRef.waitForFinished();
}

void ColoredPieceIconFactory::_validate_template_icons()
{
    // Attempt to load the icons, skipping any files that don't match our file convention

    if(!QDir(dir_templates).exists())
        THROW_NEW_GUTIL_EXCEPTION2(Exception, QString("Templates directory does not exist: %s").arg(dir_templates).toUtf8());

    QDirIterator iter(dir_templates);
    Flags<> f((GUINT32)0x03F);
    while(iter.hasNext())
    {
        QFileInfo fi(iter.next());
        QString id = fi.baseName();

        // Check that the file name is in the expected format
        if(1 != id.length())
            continue;

        Piece tmp = Piece::FromFEN(id[0].toAscii());
        if(tmp.IsNull())
            continue;

        // Mark that the piece template is present and accounted for
        f.SetFlag((int)tmp.GetType(), 0);

        // Make sure the template has all the right qualities, like a color index and
        //  the color white (which we will replace with whatever color we like).
        QImage img(fi.absoluteFilePath());
        QVector<QRgb> colors = img.colorTable();
        if(0 == colors.size())
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "There is no color index for the icon template");
        if(-1 == colors.indexOf(0xFFFFFFFF))
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "I did not find the color white in the color index");

        GDEBUG(QString("Found valid icon template: %1").arg(fi.absoluteFilePath()).toUtf8().constData());
    }

    if(f)
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "The icon factory did not find all the necessary templates");
}

QIcon ColoredPieceIconFactory::GetIcon(Piece const &p)
{
    QIcon ret;
    typename Map<int, QIcon>::iterator i(index.Search(p.UnicodeValue()));
    if(i)
        ret = i->Value();
    return ret;
}

void ColoredPieceIconFactory::ChangeColors(const QColor &col_light, const QColor &col_dark)
{
    QMutexLocker lkr(&this_lock);
    light_progress = 0;
    light_color = col_light;
    dark_progress = 0;
    dark_color = col_dark;
    index_finished_updating = false;

    something_to_do.wakeOne();
}

static void __generate_icon(Piece const &p, const QColor &color, const QString &dir_templates, const QString &dir_gen)
{
    QImage template_image(QDir::toNativeSeparators(QString("%1/%2.png")
                                                   .arg(dir_templates)
                                                   .arg(QChar(p.ToFEN()).toLower())));

    // Adjust the color of the image
    QVector<QRgb> colors = template_image.colorTable();
    int replace_index = colors.indexOf(0xFFFFFFFF);
    GASSERT(-1 != replace_index);  // We should have already checked for this in the constructor.
    colors[replace_index] = color.rgba();
    template_image.setColorTable(colors);

    // Then save a copy to the temp directory
    template_image.save(QDir::toNativeSeparators(dir_gen + "/" + __get_filename_for_piece(p)));
}

void ColoredPieceIconFactory::_worker_thread()
{
    // Initialize our background thread members
    QString dir_gen(__get_new_temp_icon_dir());
    QString old_deploy_dir;

    QMutexLocker lkr(&this_lock);

    // Loop until we are shut down
    while(!shutting_down)
    {
        // Wait here for something to do
        while(!shutting_down &&
              -1 == light_progress && -1 == dark_progress &&
              (old_deploy_dir.isEmpty() || !index_finished_updating))
        {
            something_to_do.wait(&this_lock);
        }

        // If we are in the process of generating icons...
        if(!shutting_down && (-1 != light_progress || -1 != dark_progress))
        {
            // Work on light icons before the dark:
            Piece p;
            QColor c;
            const int cur_light_progress = light_progress;
            const int cur_dark_progress = dark_progress;
            if(-1 != light_progress)
            {
                p = Piece((Piece::PieceTypeEnum)light_progress, Piece::White);
                c = light_color;
                if(++light_progress > (int)Piece::Pawn)
                    light_progress = -1;
            }
            else
            {
                p = Piece((Piece::PieceTypeEnum)dark_progress, Piece::Black);
                c = dark_color;
                if(++dark_progress > (int)Piece::Pawn)
                    dark_progress = -1;
            }

            // Unlock while we generate the icon
            lkr.unlock();
            {
                // Expensive task:

                // The first time we need to make sure our temp directory exists
                if(0 == cur_light_progress && 0 == cur_dark_progress)
                    __make_sure_dir_exists(dir_gen);

                __generate_icon(p, c, dir_templates, dir_gen);

                // For testing we can wait here to simulate a really slow operation
                //Thread::msleep(200);
            }
            lkr.relock();

            // When we're finished with a complete set of icons we do some extra work
            if(-1 == light_progress && -1 == dark_progress)
            {
                // Point to the new deploy directory, but remember the old so we can clean it up
                old_deploy_dir = dir_deploy;
                dir_deploy = dir_gen;
                index_finished_updating = false;

                // Unlock while we notify, so we're not hogging the resources others need
                lkr.unlock();
                {
                    // Unprotected section:
                    emit NotifyIconsUpdated();

                    // Generate a new icon dir name, but don't mkdir until we need it
                    dir_gen = __get_new_temp_icon_dir();
                }
                lkr.relock();
            }
        }


        // If the index is done updating and we can clean up the old icon directory...
        if(!old_deploy_dir.isEmpty() && index_finished_updating)
        {
            // Unlock while we remove files and directories
            lkr.unlock();
            __remove_dir(old_deploy_dir);
            old_deploy_dir = QString();
            lkr.relock();
        }
    }

    // Clean up after ourselves
    __remove_dir(dir_gen);
    __remove_dir(old_deploy_dir);
    __remove_dir(dir_deploy);
}

void ColoredPieceIconFactory::_update_index()
{
    // This is an expensive operation, loading the icons, but it's necessary to
    //  do this on the main GUI thread because of the implementation of QIcon
    //
    // It is not necessary to hold the lock while we do this, because it is not possible
    //  for the user to cause an icon update unless they are on a different thread, and since
    //  we are not thread safe that violates our conditions of use.  Normally this variable
    //  should be protected by the lock, however, so be careful!
    __add_pieces_icons_to_index(index, dir_deploy, Piece::White);
    __add_pieces_icons_to_index(index, dir_deploy, Piece::Black);

    // Let the background thread know that we're done updating the index
    QMutexLocker lkr(&this_lock);
    index_finished_updating = true;
    something_to_do.wakeOne();
}


END_NAMESPACE_GKCHESS1;
