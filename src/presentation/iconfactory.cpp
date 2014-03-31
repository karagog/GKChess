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
#include <QDirIterator>
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS1(UI);


DirectoryPieceIconFactory::DirectoryPieceIconFactory(const QString &dirname)
    :dir(dirname)
{
    if(!QDir().exists(dirname))
        THROW_NEW_GUTIL_EXCEPTION2(Exception, QString("Directory does not exist: %s").arg(dirname).toUtf8().constData());

    // Attempt to load the icons, skipping any files that don't match our file convention
    QDirIterator iter(dirname);
    while(iter.hasNext())
    {
        QFileInfo fi(iter.next());
        QString id = fi.baseName();

        // Check that the file name is in the expected format
        if(2 != id.length())
            continue;

        if('w' == id[0] || 'W' == id[0])
            id = id.toUpper();
        else if('b' == id[0] || 'B' == id[0])
            id = id.toLower();
        else
            continue;

        Piece tmp = Piece::FromFEN(id[1].toAscii());
        if(tmp.IsNull())
            continue;

        index.Insert(tmp.UnicodeValue(), QIcon(fi.absoluteFilePath()));

        GDEBUG(QString("Importing icon: %1").arg(fi.absoluteFilePath()).toUtf8().constData());
    }
}

DirectoryPieceIconFactory::~DirectoryPieceIconFactory()
{}

QIcon DirectoryPieceIconFactory::GetIcon(const Piece &p) const
{
    QIcon ret;
    typename Map<int, QIcon>::const_iterator i(index.Search(p.UnicodeValue()));
    if(i)
        ret = i->Value();
    return ret;
}


END_NAMESPACE_GKCHESS1;
