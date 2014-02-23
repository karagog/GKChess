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

#include "gamelogic.h"
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(DataObjects);

NAMESPACE_GKCHESS;


GameLogic::GameLogic()
    :m_board(8, 8),
      m_currentMove(Piece::White),
      m_moveHistoryIndex(-1)
{}

GameLogic::~GameLogic()
{}

void GameLogic::SetupNewGame(GameLogic::SetupTypeEnum ste)
{
    if(!m_board.IsEmpty())
        m_board.Clear();

    switch(ste)
    {
    case StandardChess:
        // First set up Pawns:
        for(GUINT32 i = 0; i < m_board.ColumnCount(); ++i){
            m_board[i][1].SetPiece(new Piece(Piece::White, Piece::Pawn));
            m_board[i][6].SetPiece(new Piece(Piece::Black, Piece::Pawn));
        }

        // Then Rooks:
        m_board[0][0].SetPiece(new Piece(Piece::White, Piece::Rook));
        m_board[7][0].SetPiece(new Piece(Piece::White, Piece::Rook));
        m_board[0][7].SetPiece(new Piece(Piece::Black, Piece::Rook));
        m_board[7][7].SetPiece(new Piece(Piece::Black, Piece::Rook));

        // Then Knights:
        m_board[1][0].SetPiece(new Piece(Piece::White, Piece::Knight));
        m_board[6][0].SetPiece(new Piece(Piece::White, Piece::Knight));
        m_board[1][7].SetPiece(new Piece(Piece::Black, Piece::Knight));
        m_board[6][7].SetPiece(new Piece(Piece::Black, Piece::Knight));

        // Then Bishops:
        m_board[2][0].SetPiece(new Piece(Piece::White, Piece::Bishop));
        m_board[5][0].SetPiece(new Piece(Piece::White, Piece::Bishop));
        m_board[2][7].SetPiece(new Piece(Piece::Black, Piece::Bishop));
        m_board[5][7].SetPiece(new Piece(Piece::Black, Piece::Bishop));

        // Then Queens:
        m_board[3][0].SetPiece(new Piece(Piece::White, Piece::Queen));
        m_board[3][7].SetPiece(new Piece(Piece::Black, Piece::Queen));

        // Then Kings:
        m_board[4][0].SetPiece(new Piece(Piece::White, Piece::King));
        m_board[4][7].SetPiece(new Piece(Piece::Black, Piece::King));

        break;
    default:
        break;
    }
}


GameLogic::MoveData::MoveData()
    :SourceSquare(NULL),
      DestSquare(NULL),
      CapturedPiece(NULL)
{}

Vector<Square *> GameLogic::GetPossibleMoves(const Square &s) const
{
    Vector<Square *> ret;

    if(s.GetPiece())
    {
        switch(s.GetPiece()->GetType())
        {
        case Piece::Pawn:
        {
            // We need to know the last move to know if en passant is legal
            MoveData const *last_move( _get_last_move() );

            // The pawn can always move one or two steps forward if not capturing
            Square *temp1(NULL), *temp2(NULL);
            switch(s.GetPiece()->GetAllegience())
            {
            case Piece::White:
                temp1 = s.north;
                if(s.south && !s.south->south)
                    temp2 = s.north->north;
                break;
            case Piece::Black:
                temp1 = s.south;
                if(s.north && !s.north->north)
                    temp2 = s.south->south;
                break;
            default:
                break;
            }

            // Test to see if it's unoccupied
            if(temp1 && !temp1->GetPiece())
            {
                ret.PushBack(temp1);

                if(temp2 && !temp2->GetPiece())
                    ret.PushBack(temp1);
            }


            // If the pawn is capturing then it moves diagonally forwards and sideways
        }
            break;
        case Piece::Knight:
            break;
        case Piece::Bishop:
            break;
        case Piece::Rook:
            break;
        case Piece::Queen:
            break;
        case Piece::King:
            break;
        default:
            break;
        }
    }


    // Now we must prune the possible moves depending if they would put the moving side in Check

    return ret;
}

void GameLogic::Move(Square &source, Square &destination)
{
    if(source == destination)
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Pieces must move to a different square");

    Vector<Square *> possible_moves( GetPossibleMoves(source) );
    if(possible_moves.Contains(&destination)){
        // The move is valid, so execute it
        _execute_move(source, destination);
    }
    else{
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid Move");
    }
}

void GameLogic::_execute_move(Square &source, Square &dest)
{
    MoveData md;
    md.SourceSquare = &source;
    md.DestSquare = &dest;
    md.CapturedPiece = dest.GetPiece();

    dest.SetPiece(source.GetPiece());
    source.SetPiece(0);

    // Prune the history if necessary
    if(m_moveHistoryIndex < (GINT32)m_moveHistory.Length() - 1)
        m_moveHistory.Resize(m_moveHistoryIndex + 1);

    m_moveHistory.PushBack(md);
    ++m_moveHistoryIndex;
}

GameLogic::MoveData const *GameLogic::_get_last_move() const
{
    MoveData const *ret( NULL );
    if(0 <= m_moveHistoryIndex)
        ret = &m_moveHistory[m_moveHistoryIndex];
    return ret;
}

void GameLogic::Undo()
{
    if(0 <= m_moveHistoryIndex)
    {
        MoveData &cur( m_moveHistory[m_moveHistoryIndex] );

        cur.SourceSquare->SetPiece(cur.DestSquare->GetPiece());
        cur.DestSquare->SetPiece(cur.CapturedPiece);

        --m_moveHistoryIndex;
    }
}

void GameLogic::Redo()
{
    if((GINT32)m_moveHistory.Length() > m_moveHistoryIndex + 1)
    {
        MoveData &cur( m_moveHistory[m_moveHistoryIndex + 1] );

        cur.DestSquare->SetPiece(cur.SourceSquare->GetPiece());
        cur.SourceSquare->SetPiece(0);

        ++m_moveHistoryIndex;
    }
}


END_NAMESPACE_GKCHESS;
