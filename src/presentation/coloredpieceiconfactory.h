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
#include "gkchess_piece.h"
#include "gutil_map.h"
#include <QDir>
#include <QReadWriteLock>
#include <QMutex>
#include <QFuture>

namespace GKChess{ namespace UI{


/** Implements a piece icon factory that dynamically generates colored icons.
 *  It expects as input to the constructor a directory of template icons with the following
 *  requirements: Each should have a color index (GIMP can do this) and when it generates the
 *  colored icons it replaces white (0xFFFFFFFF) with whatever color you want.
*/
class ColoredPieceIconFactory :
        public IFactory_PieceIcon
{
    Q_OBJECT

    QString dir_templates;
    QString dir_gen;
    GUtil::Map<int, QIcon> index;

    QMutex this_lock;
    QFuture<void> bg_thread;
    bool is_running;
    bool is_cancelled;
    int light_progress;
    int dark_progress;
    QColor light_color;
    QColor dark_color;

public:


    /** Constructs an icon factory with the given icon template path, and initializes them with
     *  the given colors.
    */
    ColoredPieceIconFactory(const QString &template_dir_path,
                              const QColor &light_color,
                              const QColor &dark_color,
                              QObject * = 0);

    virtual ~ColoredPieceIconFactory();

    void ChangeColor(bool light_pieces, const QColor &);

    virtual QIcon GetIcon(const Piece &);


signals:

    /** Internal signal used to process QIcons on the main thread. */
    void notify_icons_updated();


private slots:

    void _icons_updated();


private:

    void _validate_template_icons();

    void _worker_thread();

    QString _get_temp_path_for_piece(const Piece &);
    void _add_pieces_icons_to_index(Piece::AllegienceEnum);

};


}}

#endif // GKCHESS_COLORED_PIECE_ICON_FACTORY_H
