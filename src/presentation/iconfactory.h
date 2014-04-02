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

#ifndef GKCHESS_DIRECTORY_PIECE_ICON_FACTORY_H
#define GKCHESS_DIRECTORY_PIECE_ICON_FACTORY_H

#include "gkchess_ifactory_pieceicon.h"
#include "gkchess_piece.h"
#include "gutil_map.h"
#include <QDir>
#include <QThread>
#include <QUuid>
#include <QReadWriteLock>
#include <QMutex>

namespace GKChess{ namespace UI{


/** Implements a piece icon factory that gets icons from a directory. */
class DirectoryPieceIconFactory :
        private QThread,
        public IFactory_PieceIcon
{
    Q_OBJECT

    QUuid id;
    QString dir_templates;
    QString dir_gen;

    struct index_item_t{
        QIcon icon;
        QReadWriteLock lock;
        index_item_t(){}
        index_item_t(const index_item_t &o)
            :icon(o.icon), lock() {}
        index_item_t &operator = (const index_item_t &o){
            icon = o.icon;
            return *this;
        }
    };

    // There is no need to lock protect the index, because after the constructor it is
    //  read-only (only the contained items may change, and they are all individually lock protected)
    GUtil::Map<int, index_item_t> index;

    QMutex this_lock;
    bool is_cancelled;
    int light_progress;
    int dark_progress;
    QColor light_color;
    QColor dark_color;

public:


    /** Constructs an icon factory with the given icon template path, and initializes them with
     *  the given colors.
    */
    DirectoryPieceIconFactory(const QString &template_dir_path,
                              const QColor &light_color,
                              const QColor &dark_color,
                              QObject * = 0);

    virtual ~DirectoryPieceIconFactory();

    void ChangeColor(bool light_pieces, const QColor &);

    virtual QIcon GetIcon(const Piece &);


signals:

    /** Notifies that there is an updated icon for the given piece. */
    void NotifyIconUpdated(const GKChess::Piece &, const QString &icon_path);


protected:

    virtual void run();


private slots:

    void _icon_updated(const GKChess::Piece &, const QString &);


private:

    void _validate_template_icons();

};


}}

#endif // GKCHESS_DIRECTORY_PIECE_ICON_FACTORY_H
