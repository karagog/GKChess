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

#include "board.h"

NAMESPACE_GKCHESS;


Board::Board()
{}

Board::Board(const AbstractBoard &o)
    :Board_Imp<8>(o)
{}

Board &Board::operator = (const AbstractBoard &o)
{
    return static_cast<Board &>(Board_Imp<8>::operator = (o));
}

Board::~Board()
{}


IObservableBoard::IObservableBoard(QObject *p)
    :QObject(p)
{}


ObservableBoard::ObservableBoard(QObject *p)
    :IObservableBoard(p)
{}

ObservableBoard::ObservableBoard(const AbstractBoard &o)
    :IObservableBoard(0),
      Board(o)
{}

ObservableBoard &ObservableBoard::operator = (const AbstractBoard &o)
{
    return static_cast<ObservableBoard &>(Board::operator = (o));
}

ObservableBoard::~ObservableBoard()
{}

void ObservableBoard::SetPiece(const Piece &p, const Square &s)
{
    emit NotifySquareAboutToBeUpdated(s);
    Board::SetPiece(p, s);
    emit NotifySquareUpdated(s);
}

void ObservableBoard::move_p(const MoveData &md)
{
    emit NotifyPieceAboutToBeMoved(md);
    Board::move_p(md);
    emit NotifyPieceMoved(md);
}


END_NAMESPACE_GKCHESS;
