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

#include "boardmodel_p.h"
#include "gkchess_isquare.h"
#include <QFont>
#include <QMimeData>
#include <QStringList>
USING_NAMESPACE_GKCHESS;
USING_NAMESPACE_GUTIL;

BoardModel_p::BoardModel_p(AbstractBoard *b, QObject *parent)
    :QAbstractTableModel(parent),
      m_board(b)
{
    connect(b, SIGNAL(NotifySquareUpdated(const GKChess::ISquare &)),
            this, SLOT(_square_updated(const GKChess::ISquare &)));
    connect(b, SIGNAL(NotifyPieceMoved(const GKChess::MoveData &)),
            this, SLOT(_piece_moved(const GKChess::MoveData &)));
}

ISquare const *BoardModel_p::ConvertIndexToSquare(const QModelIndex &i) const
{
    ISquare const *ret(0);
    int r = i.row(), c = i.column();
    if(i.isValid() && this == i.model())
        ret = &m_board->SquareAt(c, r);
    return ret;
}

QModelIndex BoardModel_p::ConvertSquareToIndex(const ISquare &s) const
{
    // No need to check bounds because you cannot create a square
    //  outside of a chess board, so the indexes must be valid.
    return index(s.GetRow(), s.GetColumn());
}


int BoardModel_p::rowCount(const QModelIndex &i) const
{
    int ret(0);
    if(!i.isValid())
        ret = m_board->RowCount();
    return ret;
}

int BoardModel_p::columnCount(const QModelIndex &i) const
{
    int ret(0);
    if(!i.isValid())
        ret = m_board->ColumnCount();
    return ret;
}

QVariant BoardModel_p::data(const QModelIndex &i, int role) const
{
    QVariant ret;
    ISquare const *s = ConvertIndexToSquare(i);
    if(s)
    {
        Piece const *p = s->GetPiece();
        if(Qt::UserRole > role)
        {
            switch((Qt::ItemDataRole)role)
            {
            case Qt::DisplayRole:
                if(p)
                    ret = QString(QChar(p->UnicodeValue()));
                break;
            case Qt::EditRole:
                if(p)
                    ret = QString(QChar(p->ToFEN()));
                break;
            case Qt::ToolTipRole:
                if(p)
                    ret = p->ToString(true).ToQString();
                break;
            case Qt::DecorationRole:
                break;
            default: break;
            }
        }
        else
        {
            switch((CustomDataRoleEnum)role)
            {
            case PieceRole:
                if(p)
                    ret.setValue(*p);
                break;
            case ValidMovesRole:
            {
//                QModelIndexList il;
//                Vector<ISquare const *> tmp = GetBoard().GetValidMovesForSquare(*s);
//                G_FOREACH_CONST(ISquare const *sqr, tmp)
//                {
//                    il.append(index(sqr->GetRow(), sqr->GetColumn()));
//                }
//                ret.setValue(il);
            }
                break;
            default: break;
            }
        }
    }
    return ret;
}

bool BoardModel_p::setData(const QModelIndex &ind, const QVariant &v, int r)
{
    bool ret = false;
    if(ind.isValid())
    {
        ISquare const *sqr = ConvertIndexToSquare(ind);
        Piece const *cur_piece = sqr->GetPiece();
        if(Qt::UserRole <= r)
        {
            switch((CustomDataRoleEnum)r)
            {
            case PieceRole:
            {
                Piece new_piece = v.value<Piece>();
                if(!new_piece.IsNull()){
                    m_board->SetPiece(new_piece, *sqr);
                    ret = true;
                }
            }
                break;
            default: break;
            }
        }
        else
        {
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
                        m_board->SetPiece(Piece(), *sqr);
                        ret = true;
                    }
                }
                else if(1 == s.length())
                {
                    // Set a piece if it's valid and different
                    Piece p = Piece::FromFEN(s[0].toAscii());
                    if(!p.IsNull() && (NULL == cur_piece || p != *cur_piece))
                    {
                        m_board->SetPiece(p, *sqr);
                        ret = true;
                    }
                }
            }
                break;
            default: break;
            }
        }
    }
    return ret;
}

QVariant BoardModel_p::headerData(int indx, Qt::Orientation o, int role) const
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

void BoardModel_p::_square_updated(const ISquare &s)
{
    QModelIndex i( index(s.GetRow(), s.GetColumn()) );
    emit dataChanged(i, i);
}

void BoardModel_p::_piece_moved(const MoveData &md)
{
    QModelIndex i = index(md.Destination->GetRow(), md.Destination->GetColumn());

    // We need to notify views that the source and dest squares will be updated
    // As a simplification, we notify that the entire source row was updated, to simplify
    //  special cases like enpassant and castling, which result in other squares to need updating,
    //  but they are always along the rank of the source square.
    emit dataChanged(index(md.Source->GetRow(), 0), index(md.Source->GetRow(), columnCount() - 1));
    emit dataChanged(i, i);
}

Qt::ItemFlags BoardModel_p::flags(const QModelIndex &ind) const
{
    Qt::ItemFlags ret;
    if(ind.isValid())
    {
        ret |= Qt::ItemIsEnabled
                | Qt::ItemIsSelectable
                | Qt::ItemIsEditable
                ;
    }
    return ret;
}


QStringList BoardModel_p::mimeTypes() const
{
    //return QStringList(MIMETYPE_GKCHESS_PIECE);
    return QStringList();
}

QMimeData *BoardModel_p::mimeData(const QModelIndexList &l) const
{
    GUTIL_UNUSED(l);
    QMimeData *ret(0);
//    if(1 == l.length())
//    {
//        ISquare const *s = ConvertIndexToSquare(l[0]);
//        if(s)
//        {
//            Piece const *p = s->GetPiece();
//            if(p)
//            {
//                ret = new QMimeData;
//                ret->setData(MIMETYPE_GKCHESS_PIECE,
//                             QString("%1:%2,%3")
//                             .arg(p->ToFEN())
//                             .arg(s->GetColumn())
//                             .arg(s->GetRow())
//                             .toAscii());
//            }
//        }
//    }
    return ret;
}

bool BoardModel_p::dropMimeData(const QMimeData *data,
                              Qt::DropAction action,
                              int row,
                              int column,
                              const QModelIndex &parent)
{
    GUTIL_UNUSED(data);
    GUTIL_UNUSED(action);
    GUTIL_UNUSED(row);
    GUTIL_UNUSED(column);
    GUTIL_UNUSED(parent);
    bool ret = false;
//    if(data && parent.isValid() && -1 == row && -1 == column)
//    {
//        QByteArray b = data->data(MIMETYPE_GKCHESS_PIECE);
//        QList<QByteArray> l = b.split(':');
//        if(!l.isEmpty() && !l[0].isEmpty())
//        {
//            int s_col(-1), s_row(-1);
//            if(1 < l.length())
//            {
//                // Parse the position information
//                QList<QByteArray> l2 = l[1].split(',');
//                if(2 == l2.length())
//                {
//                    bool ok1 = false, ok2 = false;
//                    int tmp1 = l2[0].toInt(&ok1);
//                    int tmp2 = l2[1].toInt(&ok2);
//                    if(ok1 && ok2 &&
//                            0 <= tmp1 && tmp1 < columnCount() &&
//                            0 <= tmp2 && tmp2 < rowCount())
//                    {
//                        s_col = tmp1;
//                        s_row = tmp2;
//                    }
//                }
//            }

//            switch(action)
//            {
//            case Qt::MoveAction:
//                if(1 <= l[0].length() && -1 != s_col && -1 != s_row)
//                {
//                    ret = setData(parent, QString::fromAscii(l[0]), Qt::EditRole);
//                    if(ret)
//                    {
//                        // Clear the source square
//                        setData(index(s_row, s_col), "", Qt::EditRole);
//                    }
//                }
//                break;
//            default: break;
//            }
//        }
//    }

    return ret;
}

AbstractBoard::MoveValidationEnum BoardModel_p::ValidateMove(const QModelIndex &s, const QModelIndex &d) const
{
    return GetBoard().ValidateMove(*ConvertIndexToSquare(s),
                                    *ConvertIndexToSquare(d));
}

AbstractBoard::MoveValidationEnum BoardModel_p::Move(const QModelIndex &s, const QModelIndex &d, IPlayerResponse *pr)
{
    return m_board->Move2(*ConvertIndexToSquare(s),
                          *ConvertIndexToSquare(d),
                          pr);
}
