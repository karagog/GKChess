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
#include "gkchess_board.h"
#include <QFont>
#include <QMimeData>
#include <QStringList>

NAMESPACE_GKCHESS1(UI);


BoardModel::BoardModel(AbstractBoard *b, bool readonly, QObject *parent)
    :QAbstractTableModel(parent),
      m_board(readonly ? 0 : b),
      m_boardC(readonly ? b : 0)
{
    _init(b);
}

BoardModel::BoardModel(AbstractBoard const *b, QObject *parent)
    :QAbstractTableModel(parent),
      m_board(0),
      m_boardC(b)
{
    _init(b);
}

void BoardModel::_init(AbstractBoard const *b)
{
    connect(b, SIGNAL(NotifySquareUpdated(int, int)),
            this, SLOT(_square_updated(int, int)));
}

bool BoardModel::IsReadOnly() const
{
    return 0 == m_board;
}



ISquare const *BoardModel::ConvertIndexToSquare(const QModelIndex &i) const
{
    ISquare const *ret(0);
    int r = i.row(), c = i.column();
    if(i.isValid() && this == i.model())
        ret = &_get_readonly_board()->SquareAt(c, r);
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
        ret = _get_readonly_board()->RowCount();
    return ret;
}

int BoardModel::columnCount(const QModelIndex &i) const
{
    int ret(0);
    if(!i.isValid())
        ret = _get_readonly_board()->ColumnCount();
    return ret;
}

QVariant BoardModel::data(const QModelIndex &i, int role) const
{
    QVariant ret;
    ISquare const *s = ConvertIndexToSquare(i);
    if(s)
    {
        Piece const *p = s->GetPiece();
        if(p)
        {
            switch((Qt::ItemDataRole)role)
            {
            case Qt::DisplayRole:
                ret = QString(QChar(p->UnicodeValue()));
                break;
            case Qt::EditRole:
                ret = QString(QChar(p->GetAsciiChar()));
                break;
            case Qt::ToolTipRole:
                ret = p->ToString(true).ToQString();
                break;
            case Qt::DecorationRole:
                break;
            default:
                break;
            }
        }
    }
    return ret;
}

bool BoardModel::setData(const QModelIndex &ind, const QVariant &v, int r)
{
    bool ret = false;
    if(!IsReadOnly() && ind.isValid())
    {
        ISquare const *sqr = ConvertIndexToSquare(ind);
        Piece const *cur_piece = sqr->GetPiece();
        switch((Qt::ItemDataRole)r)
        {
        case Qt::EditRole:
        {
            QString s = v.toString();
            if(s.length() == 0)
            {
                // Clear the square
                if(NULL != cur_piece)
                {
                    m_board->SetPiece(Piece(), sqr->GetColumn(), sqr->GetRow());
                    ret = true;
                }
            }
            else if(1 == s.length())
            {
                // Set a piece if it's valid and different
                Piece p(s[0].toAscii());
                if(!p.IsNull() && (NULL == cur_piece || p != *cur_piece))
                {
                    m_board->SetPiece(p, sqr->GetColumn(), sqr->GetRow());
                    ret = true;
                }
            }
        }
            break;
        default: break;
        }
    }
    return ret;
}

QVariant BoardModel::headerData(int indx, Qt::Orientation o, int role) const
{
    QVariant ret;
    if(Qt::Horizontal == o)
    {
        if(_get_readonly_board()->RowCount() > indx)
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
        if(_get_readonly_board()->ColumnCount() > indx)
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

AbstractBoard const *BoardModel::_get_readonly_board() const
{
    return m_board ? m_board : m_boardC;
}

Qt::ItemFlags BoardModel::flags(const QModelIndex &ind) const
{
    Qt::ItemFlags ret = Qt::ItemIsEnabled;
    if(ind.isValid())
    {
        ret |= Qt::ItemIsSelectable;
        if(!IsReadOnly())
        {
            ret |= Qt::ItemIsEditable |
                    Qt::ItemIsDragEnabled |
                    Qt::ItemIsDropEnabled;
        }
    }
    return ret;
}


QStringList BoardModel::mimeTypes() const
{
    QStringList ret;
    if(!IsReadOnly())
    {
        ret.append(MIMETYPE_GKCHESS_PIECE);
    }
    return ret;
}

bool BoardModel::dropMimeData(const QMimeData *data,
                                 Qt::DropAction action,
                                 int row,
                                 int column,
                                 const QModelIndex &parent)
{
    bool ret = false;
    if(data && parent.isValid() && -1 == row && -1 == column)
    {
        QByteArray b = data->data(MIMETYPE_GKCHESS_PIECE);
        QList<QByteArray> l = b.split(':');
        if(!l.isEmpty() && !l[0].isEmpty())
        {
            int s_col(-1), s_row(-1);
            if(1 < l.length())
            {
                // Parse the position information
                QList<QByteArray> l2 = l[1].split(',');
                if(2 == l2.length())
                {
                    bool ok1 = false, ok2 = false;
                    int tmp1 = l2[0].toInt(&ok1);
                    int tmp2 = l2[1].toInt(&ok2);
                    if(ok1 && ok2 &&
                            0 <= tmp1 && tmp1 < columnCount() &&
                            0 <= tmp2 && tmp2 < rowCount())
                    {
                        s_col = tmp1;
                        s_row = tmp2;
                    }
                }
            }

            switch(action)
            {
            case Qt::MoveAction:
                if(1 <= l[0].length() && -1 != s_col && -1 != s_row)
                {
                    ret = setData(parent, QString::fromAscii(l[0]));
                    if(ret)
                    {
                        // Clear the source square
                        setData(index(s_row, s_col), "");
                    }
                }
                break;
            default: break;
            }
        }
    }

    return ret;
}

Qt::DropActions BoardModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions BoardModel::supportedDropActions() const
{
    return Qt::MoveAction;
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
                             .arg(p->GetAsciiChar())
                             .arg(s->GetColumn())
                             .arg(s->GetRow())
                             .toAscii());
            }
        }
    }
    return ret;
}


END_NAMESPACE_GKCHESS1;
