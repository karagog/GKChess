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

#include "bookmodel.h"
#include "gkchess_board.h"
#include "gutil_pluginutils.h"
#include "gutil_strings.h"
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(QT);

NAMESPACE_GKCHESS1(UI);


BookModel::BookModel(ObservableBoard &b, QObject *parent)
    :QAbstractItemModel(parent),
      m_board(b),
      i_bookReader(PluginUtils::LoadPlugin<IBookReader>(m_pl, "polyglotReaderPlugin"))
{
    connect(&b, SIGNAL(NotifyPieceMoved(const GKChess::MoveData &)),
            this, SLOT(_board_position_changed()));
    connect(&b, SIGNAL(NotifySquareUpdated(const GKChess::Square &)),
            this, SLOT(_board_position_changed()));
    connect(&b, SIGNAL(NotifyBoardReset()),
            this, SLOT(_board_position_changed()));
}

bool BookModel::SetBookFile(const QString &filename)
{
    // If the filename didn't change, then return
    if(filename == i_bookReader->GetBookFilename())
        return false;

    i_bookReader->CloseBook();
    _board_position_changed();

    if(!filename.isEmpty())
    {
        try{
            i_bookReader->OpenBook(filename.toUtf8().constData());
        } catch(...) {
            return false;
        }
    }
    return true;
}

QString BookModel::GetBookFile() const
{
    return i_bookReader->GetBookFilename();
}

QModelIndexList BookModel::GetAncestry(const QModelIndex &ind) const
{
    QModelIndexList ret;
    _get_ancestry_helper(ret, ind);
    return ret;
}

void BookModel::_get_ancestry_helper(QModelIndexList &l, const QModelIndex &i) const
{
    if(!i.isValid())
        return;

    _get_ancestry_helper(l, i.parent());
    l.append(i);
}

const MoveData *BookModel::ConvertIndexToMoveData(const QModelIndex &i) const
{
    MoveData const *ret = 0;
    if(i.isValid())
        ret = &_get_data_from_index(i)->Data;
    return ret;
}

bool BookModel::hasChildren(const QModelIndex &i) const
{
    bool ret = false;
    MoveDataContainer const *c = _get_children_of_index(i);
    //if(!i.isValid() || 0 == i.column())
    if(!c->Loaded || 0 < c->Moves.Length())
        ret = true;
    return ret;
}

int BookModel::rowCount(const QModelIndex &parent) const
{
    int ret = 0;
    if(!parent.isValid() || 0 == parent.column()){
        ret = _get_children_of_index(parent)->Moves.Length();
    }
    return ret;
}

int BookModel::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant BookModel::data(const QModelIndex &index, int role) const
{
    QVariant ret;
    if(index.isValid())
    {
        int col = index.column();
        MoveDataCache *d = _get_data_from_index(index);
        switch((Qt::ItemDataRole)role)
        {
        case Qt::DisplayRole:
            if(0 == col)
                ret = d->Data.PGNData.ToString().ToQString();
            else if(1 == col)
                ret = d->BookData.Weight;
            break;
        case Qt::BackgroundRole:
        {
            int n = 0x1 & GetAncestry(index).length();
            Piece::AllegienceEnum a = m_board.GetWhoseTurn();
            if((n && a == Piece::White) || (!n && a == Piece::Black))
                ret = Qt::white;
            else
                ret = Qt::gray;
        }
            break;
        case Qt::TextAlignmentRole:
            if(0 == col)
                ret = Qt::AlignLeft;
            else if(1 == col)
                ret = Qt::AlignCenter;
            break;
        default:
            break;
        }
    }
    return ret;
}

QVariant BookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant ret;
    if(orientation == Qt::Horizontal)
    {
        switch((Qt::ItemDataRole)role)
        {
        case Qt::DisplayRole:
            if(0 == section)
                ret = tr("Move");
            else if(1 == section)
                ret = tr("Weight (%)");
            break;
        case Qt::TextAlignmentRole:
            ret = Qt::AlignCenter;
            break;
        default:
            break;
        }
    }
    return ret;
}

QModelIndex BookModel::index(int row, int column, const QModelIndex &parent) const
{
    QModelIndex ret;
    if(0 <= row && row < rowCount(parent) &&
            0 <= column && column < columnCount(parent))
    {
        ret = createIndex(row, column, (void *)&_get_children_of_index(parent)->Moves[row]);
    }
    return ret;
}

QModelIndex BookModel::parent(const QModelIndex &child) const
{
    QModelIndex ret;
    MoveDataCache *d = _get_data_from_index(child);
    if(d && d->Parent){
        int parent_row;
        if(d->Parent->Parent)
            parent_row = d->Parent->Parent->Moves.IndexOf(*d->Parent);
        else
            parent_row = m_rootContainer.Moves.IndexOf(*d->Parent);
        ret = createIndex(parent_row, 0, d->Parent);
    }
    return ret;
}

void BookModel::fetchMore(const QModelIndex &parent)
{
    MoveDataContainer *lst = _get_children_of_index(parent);
    MoveDataCache *d = _get_data_from_index(parent);
    if(lst->Loaded)
        return;

    // Need to simulate all parents' moves
    Board cpy = m_board;
    QModelIndexList parents = GetAncestry(parent);
    for(int i = 0; i < parents.length(); ++i){
        cpy.Move(_get_data_from_index(parents[i])->Data);
    }

    String s = cpy.ToFEN();
    Vector<BookMove> moves = i_bookReader->LookupMoves(s);
    lst->Loaded = true;

    if(0 < moves.Length())
    {
        beginInsertRows(parent, 0, moves.Length() - 1);
        G_FOREACH_CONST(BookMove const &m, moves){
            lst->Moves.Append(MoveDataCache(d));
            lst->Moves.Back().BookData = m;
            lst->Moves.Back().Data = cpy.GenerateMoveData(cpy.SquareAt(m.SourceCol, m.SourceRow),
                                                          cpy.SquareAt(m.DestCol, m.DestRow),
                                                          0, true);
        }
        endInsertRows();
    }
    else
    {
        // Now we know that the parent has no children,
        //  so we notify that it has changed so the view can remove the expander
        emit dataChanged(parent, parent);
    }
}

bool BookModel::canFetchMore(const QModelIndex &parent) const
{
    MoveDataContainer const *d = _get_children_of_index(parent);
    return !d->Loaded;
}

BookModel::MoveDataCache *BookModel::_get_data_from_index(const QModelIndex &i) const
{
    MoveDataCache *ret = 0;
    if(i.isValid()){
        ret = reinterpret_cast<MoveDataCache *>(i.internalPointer());
    }
    return ret;
}

BookModel::MoveDataContainer const *BookModel::_get_children_of_index(const QModelIndex &i) const
{
    MoveDataContainer const *ret;
    if(i.isValid())
        ret = reinterpret_cast<MoveDataCache *>(i.internalPointer());
    else
        ret = &m_rootContainer;
    return ret;
}

BookModel::MoveDataContainer *BookModel::_get_children_of_index(const QModelIndex &i)
{
    MoveDataContainer *ret;
    if(i.isValid())
        ret = reinterpret_cast<MoveDataCache *>(i.internalPointer());
    else
        ret = &m_rootContainer;
    return ret;
}

void BookModel::_board_position_changed()
{
    beginResetModel();
    {
        m_rootContainer.Loaded = false;
        m_rootContainer.Moves.Empty();
    }
    endResetModel();
}


END_NAMESPACE_GKCHESS1;
