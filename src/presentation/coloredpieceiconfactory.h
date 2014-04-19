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

#ifndef GKCHESS_COLORED_PIECE_ICON_FACTORY_H
#define GKCHESS_COLORED_PIECE_ICON_FACTORY_H

#include "gkchess_ifactory_pieceicon.h"
#include "gutil_map.h"
#include <QDir>
#include <QMutex>
#include <QFuture>
#include <QWaitCondition>

namespace GKChess{ namespace UI{


/** Implements a piece icon factory that dynamically generates colored icons.
 *  It expects as input to the constructor a directory of template icons with the following
 *  requirements: Each should have a color index (GIMP can do this) and when it generates the
 *  colored icons it replaces white (0xFFFFFFFF) with whatever color you want.
 *
 *  \note This is not a thread-safe implementation (but it does use threads in the implementation)
*/
class ColoredPieceIconFactory :
        public IFactory_PieceIcon
{
    Q_OBJECT
    GUTIL_DISABLE_COPY(ColoredPieceIconFactory);

    // Readonly members, unprotected, can be safely shared between threads
    QString const dir_templates;

    // Main GUI thread members, unprotected
    GUtil::Map<int, QIcon> index;
    QFuture<void> bg_threadRef;

    // Shared members between main and background thread, protected by this_lock
    QString dir_deploy;
    QColor light_color;
    QColor dark_color;
    int light_progress;
    int dark_progress;
    bool index_finished_updating;
    bool shutting_down;

    QMutex this_lock;
    QWaitCondition something_to_do;

public:


    /** Constructs an icon factory with the given icon template path, and initializes them with
     *  the given colors.
    */
    ColoredPieceIconFactory(const QString &template_dir_path,
                              const QColor &light_color,
                              const QColor &dark_color,
                              QObject * = 0);

    virtual ~ColoredPieceIconFactory();

    /** Changes the colors of the icons.  It may take a second or two before the icons are updated. */
    void ChangeColors(const QColor &light_color, const QColor &dark_color);

    /** Returns the light piece color. */
    QColor GetLightColor() const{ return light_color; }

    /** Returns the dark piece color. */
    QColor GetDarkColor() const{ return dark_color; }


    /** \name IFactory_PieceIcon interface
     *  \{
    */
    virtual QIcon GetIcon(Piece const &);
    /** \} */


private slots:

    void _update_index();


private:

    void _worker_thread();

    void _validate_template_icons();

};


}}

#endif // GKCHESS_COLORED_PIECE_ICON_FACTORY_H
