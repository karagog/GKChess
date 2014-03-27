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

#include "boardmodel.h"
#include "gkchess_abstractboard.h"
#include <QFont>
#include <QMimeData>
#include <QStringList>
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS1(UI);


BoardModel::BoardModel(AbstractBoard const *b, QObject *parent)
    :QAbstractTableModel(parent),
      m_board(b)
{
    connect(b, SIGNAL(NotifySquareUpdated(int, int)),
            this, SLOT(_square_updated(int, int)));
}



ISquare const *BoardModel::ConvertIndexToSquare(const QModelIndex &i) const
{
    ISquare const *ret(0);
    int r = i.row(), c = i.column();
    if(i.isValid() && this == i.model())
        ret = &m_board->SquareAt(c, r);
    return ret;
}

QModelIndex BoardModel::ConvertSquareToIndex(const ISquare &s) const
{
    // No need to check bounds because you cannot create a square
    //  outside of a chess board, so the indexes must be valid.
    return index(s.GetRow(), s.GetColumn());
}


int BoardModel::rowCount(const QModelIndex &i) const
{
    int ret(0);
    if(!i.isValid())
        ret = m_board->RowCount();
    return ret;
}

int BoardModel::columnCount(const QModelIndex &i) const
{
    int ret(0);
    if(!i.isValid())
        ret = m_board->ColumnCount();
    return ret;
}

QVariant BoardModel::data(const QModelIndex &i, int role) const
{
    QVariant ret;
    ISquare const *s = ConvertIndexToSquare(i);
    if(s)
    {
        Piece const *p = s->GetPiece();
        switch((Qt::ItemDataRole)role)
        {
        case Qt::DisplayRole:
            if(p)
                ret = QString(QChar(p->UnicodeValue()));
            break;
        case Qt::ToolTipRole:
            if(p)
                ret = p->ToString(true).ToQString();
            break;
        case Qt::DecorationRole:
            break;
        case Qt::BackgroundColorRole:
            if(m_formattingOptions.Contains(s))
                ret = m_formattingOptions.At(s).BackgroundColor;
            break;
        default:
            break;
        }
    }
    return ret;
}

bool BoardModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool ret = false;
    ISquare const *s = ConvertIndexToSquare(index);
    if(s)
    {
        switch((Qt::ItemDataRole)role)
        {
        case Qt::BackgroundColorRole:
            HighlightSquare(s, value.value<QColor>());
            ret = true;
            break;
        default:
            break;
        }
    }
    return ret;
}

QVariant BoardModel::headerData(int indx, Qt::Orientation o, int role) const
{
    QVariant ret;
    if(Qt::Horizontal == o)
    {
        if(m_board->RowCount() > indx)
        {
            switch((Qt::ItemDataRole)role)
            {
            case Qt::DisplayRole:
                ret = QChar('a' + indx);
                break;
            default:
                break;
            }
        }
    }
    else if(Qt::Vertical == o)
    {
        if(m_board->ColumnCount() > indx)
        {
            switch((Qt::ItemDataRole)role)
            {
            case Qt::DisplayRole:
                ret = 1 + indx;
                break;
            default:
                break;
            }
        }
    }
    return ret;
}

void BoardModel::_square_updated(int c, int r)
{
    QModelIndex i( index(r, c) );
    emit dataChanged(i, i);
}

Qt::ItemFlags BoardModel::flags(const QModelIndex &ind) const
{
    Qt::ItemFlags ret;
    if(ind.isValid())
    {
        ret |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    return ret;
}


QStringList BoardModel::mimeTypes() const
{
    return QStringList(MIMETYPE_GKCHESS_PIECE);
}

QMimeData *BoardModel::mimeData(const QModelIndexList &l) const
{
    QMimeData *ret(0);
    if(1 == l.length())
    {
        ISquare const *s = ConvertIndexToSquare(l[0]);
        if(s)
        {
            Piece const *p = s->GetPiece();
            if(p)
            {
                ret = new QMimeData;
                ret->setData(MIMETYPE_GKCHESS_PIECE,
                             QString("%1:%2,%3")
                             .arg(p->ToFEN())
                             .arg(s->GetColumn())
                             .arg(s->GetRow())
                             .toAscii());
            }
        }
    }
    return ret;
}

void BoardModel::HighlightSquare(ISquare const *s, const QColor &c)
{
    m_formattingOptions[s].BackgroundColor = c;
    _square_updated(s->GetColumn(), s->GetRow());
}

void BoardModel::ClearSquareHighlighting()
{
    Vector<ISquare const *> squares = m_formattingOptions.Keys();
    m_formattingOptions.Clear();
    G_FOREACH_CONST(ISquare const *s, squares){
        _square_updated(s->GetColumn(), s->GetRow());
    }
}

Piece::AllegienceEnum BoardModel::GetWhoseTurn() const
{
    return m_board->GetWhoseTurn();
}


END_NAMESPACE_GKCHESS1;
