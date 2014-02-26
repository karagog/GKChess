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

#include "gamemodel.h"
#include <QFont>

NAMESPACE_GKCHESS1(UI);


GameModel::GameModel(QObject *parent)
    :QAbstractTableModel(parent),
      m_lightColor(Qt::white),
      m_darkColor(Qt::gray),
      m_pieceColor(Qt::black),
      m_pieceSize(40)
{}

void GameModel::SetupNewGame(SetupTypeEnum s)
{
    beginResetModel();
    GameLogic::SetupNewGame(s);
    endResetModel();
}

void GameModel::Move(Square &s, Square &d)
{
    GameLogic::Move(s, d);
}

void GameModel::Undo()
{
    GameLogic::Undo();
}

void GameModel::Redo()
{
    GameLogic::Redo();
}


void GameModel::SetLightColor(const QColor &c)
{
    m_lightColor = c;

    // Cause the view to update the light squares:
    _updated_squares(1);
}

void GameModel::SetDarkColor(const QColor &c)
{
    m_darkColor = c;

    // Cause the view to update the dark squares:
    _updated_squares(0);
}

void GameModel::SetPieceColor(const QColor &c)
{
    m_pieceColor = c;

    // Update the squares with pieces on them:
    _updated_pieces();
}

void GameModel::SetPieceSize(int s)
{
    m_pieceSize = s;

    // Update the squares with pieces on them:
    _updated_pieces();
}

void GameModel::_updated_squares(int n)
{
    for(int i = 0; i < GetBoard().ColumnCount(); ++i)
    {
        for(int j = 0; j < GetBoard().RowCount(); ++j)
        {
            Square const &s( GetBoard().GetSquare(i, j) );
            if((0 == n && s.IsDarkSquare()) ||
                    (1 == n && !s.IsDarkSquare()))
            {
                QModelIndex ind( ConvertSquareToIndex(s) );
                emit dataChanged(ind, ind);
            }
        }
    }
}

void GameModel::_updated_pieces()
{
    for(int i = 0; i < GetBoard().ColumnCount(); ++i)
    {
        for(int j = 0; j < GetBoard().RowCount(); ++j)
        {
            Square const &s( GetBoard().GetSquare(i, j) );
            if(s.GetPiece())
            {
                QModelIndex ind( ConvertSquareToIndex(s) );
                emit dataChanged(ind, ind);
            }
        }
    }
}


Square const *GameModel::ConvertIndexToSquare(const QModelIndex &i) const
{
    Square const *ret(0);
    int r = i.row(), c = i.column();
    if(i.isValid() &&
            this == i.model() &&
            GetBoard().ColumnCount() > c &&
            GetBoard().RowCount() > r)
        ret = &GetBoard().GetSquare(c, r);
    return ret;
}

QModelIndex GameModel::ConvertSquareToIndex(const Square &s) const
{
    QModelIndex ret;

    // No need to check bounds because you cannot create a square
    //  outside of a chess board, so the indexes must be valid.
    ret = index(s.GetRow(), s.GetColumn());

    return ret;
}


int GameModel::rowCount(const QModelIndex &i) const
{
    int ret(0);
    if(!i.isValid())
        ret = GetBoard().RowCount();
    return ret;
}

int GameModel::columnCount(const QModelIndex &i) const
{
    int ret(0);
    if(!i.isValid())
        ret = GetBoard().ColumnCount();
    return ret;
}

QVariant GameModel::data(const QModelIndex &i, int role) const
{
    QVariant ret;
    Square const *s = ConvertIndexToSquare(i);
    if(s)
    {
        Piece const *p = s->GetPiece();

        switch((Qt::ItemDataRole)role)
        {
        case Qt::DisplayRole:
            if(p)
            {
                ret = QString(QChar(p->UnicodeValue()));
                //ret = p->ToString();
            }
            break;
        case Qt::FontRole:
        {
            QFont f;
            f.setPixelSize(m_pieceSize);
            ret = f;
        }
            break;
        case Qt::ToolTipRole:
            if(p)
            {
                ret = p->ToString(true).ToQString();
            }
            break;
        case Qt::BackgroundColorRole:
            if(s->IsDarkSquare())
                ret = GetDarkColor();
            else
                ret = GetLightColor();
            break;
        case Qt::ForegroundRole:
            ret = GetPieceColor();
            break;
        case Qt::DecorationRole:
            if(p)
            {

            }
            break;
        case Qt::TextAlignmentRole:
            ret = (int)Qt::Alignment(Qt::AlignHCenter | Qt::AlignBottom);
            break;
        default:
            break;
        }
    }
    return ret;
}

QVariant GameModel::headerData(int indx, Qt::Orientation o, int role) const
{
    QVariant ret;
    if(Qt::Horizontal == o)
    {
        if(GetBoard().RowCount() > indx)
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
        if(GetBoard().ColumnCount() > indx)
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


END_NAMESPACE_GKCHESS1;
