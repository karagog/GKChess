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

#include "iconfactory.h"
#include "gkchess_piece.h"
#include "gutil_flags.h"
#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QDesktopServices>
#include <QImage>
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS1(UI);


DirectoryPieceIconFactory::DirectoryPieceIconFactory(const QString &dirname, const QColor &light_color, const QColor &dark_color, QObject *p)
    :QThread(p),
      id(QUuid::createUuid()),
      dir_templates(dirname),
      dir_gen(QDir::toNativeSeparators(QString("%1/%2/temp_icons/%3")
                                       .arg(QDesktopServices::storageLocation(QDesktopServices::TempLocation))
                                       .arg(qApp->applicationName())
                                       .arg(id))),
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
    connect(this, SIGNAL(NotifyIconUpdated(const GKChess::Piece &, const QString &)),
            this, SLOT(_icon_updated(const GKChess::Piece &, const QString &)),
            Qt::QueuedConnection);

    // After this point the background thread starts running
    ChangeColor(true, light_color);
    ChangeColor(false, dark_color);
}

DirectoryPieceIconFactory::~DirectoryPieceIconFactory()
{
    if(isRunning())
    {
        this_lock.lock();
        is_cancelled = true;
        this_lock.unlock();
        wait();
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

void DirectoryPieceIconFactory::_validate_template_icons()
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

        // Insert some null entries into our index for the piece type we found
        index.Insert(Piece(tmp.GetType(), Piece::White).UnicodeValue(), index_item_t());
        index.Insert(Piece(tmp.GetType(), Piece::Black).UnicodeValue(), index_item_t());

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

QIcon DirectoryPieceIconFactory::GetIcon(const Piece &p)
{
    QIcon ret;
    typename Map<int, index_item_t>::iterator i(index.Search(p.UnicodeValue()));
    if(i){
        index_item_t &item(i->Value());

        item.lock.lockForRead();
        ret = item.icon;
        item.lock.unlock();
    }
    return ret;
}

void DirectoryPieceIconFactory::ChangeColor(bool light_pieces, const QColor &c)
{
    this_lock.lock();

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

    // This does nothing if the thread is already running
    start();

    this_lock.unlock();
}

void DirectoryPieceIconFactory::run()
{
    QMutexLocker lkr(&this_lock);

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
        index_item_t &item(index[p.UnicodeValue()]);
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
        QString temp_filename = QDir::toNativeSeparators(QString("%1/%2%3.png")
                                                         .arg(dir_gen)
                                                         .arg(allegience == Piece::White ? "w" : "b")
                                                         .arg(QChar(p.ToFEN()).toLower()));

        // We lock here because we are overwriting a file that is protected, even though we're not modifying our memory structures
        item.lock.lockForWrite();
        template_image.save(temp_filename);
        item.lock.unlock();

        // We are also listening to this signal, and we will create the QIcon object in the handler
        //  on the main thread.
        emit NotifyIconUpdated(p, temp_filename);

        lkr.relock();
    }

    this_lock.unlock();
}

void DirectoryPieceIconFactory::_icon_updated(const Piece &p, const QString &path)
{
    // We have to create the QIcon on the main thread
    index_item_t &item(index[p.UnicodeValue()]);
    item.lock.lockForWrite();
    item.icon = QIcon(path);
    item.lock.unlock();
}


END_NAMESPACE_GKCHESS1;
