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
#include "gkchess_globals.h"
#include "gutil_map.h"
#include <QDir>

NAMESPACE_GKCHESS1(UI);


/** Implements a piece icon factory that gets icons from a directory. */
class DirectoryPieceIconFactory :
        public IFactory_PieceIcon
{
    QDir dir;
    GUtil::Map<int, QIcon> index;
public:
    /** Constructs an icon factory that searches the directory for icons of a specific file
     *  format.  Each piece has an icon with their allegience and PGN name, so the white king
     *  is "wk.png" and the black rook is "br.png", etc...
    */
    DirectoryPieceIconFactory(const QString &dir_root);
    virtual ~DirectoryPieceIconFactory();

    virtual QIcon GetIcon(const Piece &) const;

};


END_NAMESPACE_GKCHESS1;

#endif // GKCHESS_DIRECTORY_PIECE_ICON_FACTORY_H
