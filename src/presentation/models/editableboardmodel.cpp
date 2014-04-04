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

#include "editableboardmodel.h"
#include "gkchess_abstractboard.h"
#include "gkchess_isquare.h"
#include <QMimeData>
NAMESPACE_GKCHESS1(UI);


EditableBoardModel::EditableBoardModel(AbstractBoard *b, QObject *parent)
    :BoardModel(b, parent),
      m_board(b)
{}

bool EditableBoardModel::dropMimeData(const QMimeData *data,
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

QVariant EditableBoardModel::data(const QModelIndex &i, int r) const
{
    QVariant ret;
    ISquare const *s = ConvertIndexToSquare(i);
    if(s)
    {
        Piece const *p = s->GetPiece();
        switch((Qt::ItemDataRole)r)
        {
        case Qt::EditRole:
            if(p)
                ret = QString(QChar(p->ToFEN()));
            break;
        default:
            ret = BoardModel::data(i, r);
            break;
        }
    }
    return ret;
}

bool EditableBoardModel::setData(const QModelIndex &ind, const QVariant &v, int r)
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
                    m_board->SetPiece(new_piece, ind.column(), ind.row());
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
                        m_board->SetPiece(Piece(), sqr->GetColumn(), sqr->GetRow());
                        ret = true;
                    }
                }
                else if(1 == s.length())
                {
                    // Set a piece if it's valid and different
                    Piece p = Piece::FromFEN(s[0].toAscii());
                    if(!p.IsNull() && (NULL == cur_piece || p != *cur_piece))
                    {
                        m_board->SetPiece(p, sqr->GetColumn(), sqr->GetRow());
                        ret = true;
                    }
                }
            }
                break;
            default:
                ret = BoardModel::setData(ind, v, r);
                break;
            }
        }
    }
    return ret;
}

Qt::ItemFlags EditableBoardModel::flags(const QModelIndex &ind) const
{
    Qt::ItemFlags ret;
    if(ind.isValid())
    {
        ret = BoardModel::flags(ind) |
                Qt::ItemIsEditable;
    }
    return ret;
}

Qt::DropActions EditableBoardModel::supportedDropActions() const
{
    return Qt::CopyAction;
}

Qt::DropActions EditableBoardModel::supportedDragActions() const
{
    return Qt::CopyAction;
}

void EditableBoardModel::MovePiece(const QModelIndex &source, const QModelIndex &dest)
{
    if(source.isValid() && dest.isValid())
        m_board->MovePiece(source.column(), source.row(), dest.column(), dest.row());
}

bool EditableBoardModel::ValidateMove(const QModelIndex &, const QModelIndex &)
{
    return true;
}


END_NAMESPACE_GKCHESS1;
