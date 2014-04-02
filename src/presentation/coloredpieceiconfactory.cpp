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

NAMESPACE_GKCHESS1(UI);


ColoredPieceIconFactory::ColoredPieceIconFactory(const QString &dirname, const QColor &light_color, const QColor &dark_color, QObject *p)
    :IFactory_PieceIcon(p),
      dir_templates(dirname),
      dir_gen(QDir::toNativeSeparators(QString("%1/%2/temp_icons/%3")
                                       .arg(QDesktopServices::storageLocation(QDesktopServices::TempLocation))
                                       .arg(qApp->applicationName())
                                       .arg(QUuid::createUuid()))),
      is_running(false),
      is_cancelled(false),
      light_progress(-1),
      dark_progress(-1)
{
    if(!QDir(dir_templates).exists())
        THROW_NEW_GUTIL_EXCEPTION2(Exception, QString("Directory does not exist: %s").arg(dirname).toUtf8().constData());

    // Make sure the template icons they gave us are valid for what we're doing
    _validate_template_icons();

    // Initialize our temp directory
    QDir dir(dir_gen);
    if(!dir.exists()){
        if(!dir.mkpath(dir_gen))
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "Temp directory does not exist, and I couldn't create one");

//        dir_gen = temp_directory;
//        GASSERT(dir_gen.exists());
    }

    //qDebug("Temp directory for icons: %s", dir_gen.toUtf8().constData());

    qRegisterMetaType<GKChess::Piece>("GKChess::Piece");
    connect(this, SIGNAL(notify_icons_updated()),
            this, SLOT(_icons_updated()),
            Qt::QueuedConnection);

    // After this point the background thread starts running
    ChangeColor(true, light_color);
    ChangeColor(false, dark_color);
}

ColoredPieceIconFactory::~ColoredPieceIconFactory()
{
    QMutexLocker lkr (&this_lock);
    if(is_running)
    {
        is_cancelled = true;
        lkr.unlock();
        bg_thread.waitForFinished();
    }

    // Remove our temp icon directory:
    // First remove all files
    QDir dir(dir_gen);
    QDirIterator iter(dir_gen);
    while(iter.hasNext()){
        dir.remove(iter.next());
    }
    QDir().rmdir(dir_gen);
}

void ColoredPieceIconFactory::_validate_template_icons()
{
    // Attempt to load the icons, skipping any files that don't match our file convention
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

QIcon ColoredPieceIconFactory::GetIcon(const Piece &p)
{
    QIcon ret;
    typename Map<int, QIcon>::iterator i(index.Search(p.UnicodeValue()));
    if(i){
        ret = i->Value();
    }
    return ret;
}

void ColoredPieceIconFactory::ChangeColor(bool light_pieces, const QColor &c)
{
    QMutexLocker lkr(&this_lock);

    if(light_pieces)
    {
        light_progress = 0;
        light_color = c;
    }
    else
    {
        dark_progress = 0;
        dark_color = c;
    }

    if(!is_running)
        bg_thread = QtConcurrent::run(this, &ColoredPieceIconFactory::_worker_thread);
}

void ColoredPieceIconFactory::_worker_thread()
{
    QMutexLocker lkr(&this_lock);
    is_running = true;

    while(!is_cancelled && (-1 != light_progress || -1 != dark_progress))
    {
        int *cur_progress;
        QColor const *cur_color;
        Piece::AllegienceEnum allegience;

        // Work on light icons first:
        if(-1 != light_progress)
        {
            cur_progress = &light_progress;
            cur_color = &light_color;
            allegience = Piece::White;
        }
        else
        {
            // Then work on dark icons
            cur_progress = &dark_progress;
            cur_color = &dark_color;
            allegience = Piece::Black;
        }

        Piece p((Piece::PieceTypeEnum)*cur_progress, allegience);
        *cur_progress += 1;
        if(6 == *cur_progress)
            *cur_progress = -1;
        lkr.unlock();

        // Generate a new icon
        QImage template_image(QDir::toNativeSeparators(QString("%1/%2.png")
                                                       .arg(dir_templates)
                                                       .arg(QChar(p.ToFEN()).toLower())));

        // Adjust the color of the image
        QVector<QRgb> colors = template_image.colorTable();
        int replace_index = colors.indexOf(0xFFFFFFFF);
        GASSERT(-1 != replace_index);  // We should have already checked for this in the constructor.
        colors[replace_index] = cur_color->rgba();
        template_image.setColorTable(colors);

        // Then save a copy to the temp directory
        QString temp_filename = _get_temp_path_for_piece(p);


        template_image.save(temp_filename);

        // For testing we can wait here to simulate a slow disk
        //GUtil::QT::Thread::sleep(1);

        lkr.relock();
    }

    if(-1 == light_progress && -1 == dark_progress)
    {
        // We are the ones listening to this signal, and we will create the QIcon object in the handler
        //  on the main thread.
        emit notify_icons_updated();
    }

    is_running = false;
}

QString ColoredPieceIconFactory::_get_temp_path_for_piece(const Piece &p)
{
    return QDir::toNativeSeparators(QString("%1/%2%3.png")
                                    .arg(dir_gen)
                                    .arg(p.GetAllegience() == Piece::White ? "w" : "b")
                                    .arg(QChar(p.ToFEN()).toLower()));
}

void ColoredPieceIconFactory::_add_pieces_icons_to_index(Piece::AllegienceEnum a)
{
    for(Piece::PieceTypeEnum t = Piece::King; t <= Piece::Pawn; t = (Piece::PieceTypeEnum)((int)t + 1))
    {
        Piece p(t, a);
        index[p.UnicodeValue()] = QIcon(_get_temp_path_for_piece(p));
    }
}

void ColoredPieceIconFactory::_icons_updated()
{
    // We have to create the QIcon on the main thread
    _add_pieces_icons_to_index(Piece::White);
    _add_pieces_icons_to_index(Piece::Black);

    // Now we tell the world the icon was updated
    emit NotifyIconsUpdated();
}


END_NAMESPACE_GKCHESS1;
