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
    :m_board(),
      m_currentTurn(Piece::White),
      m_moveHistoryIndex(-1)
{}

GameLogic::~GameLogic()
{}

void GameLogic::SetupNewGame(GameLogic::SetupTypeEnum ste)
{
    m_board.Clear();

    switch(ste)
    {
    case StandardChess:
        // First set up Pawns:
        for(int i = 0; i < m_board.ColumnCount(); ++i){
            m_board.InitPiece(new Piece(Piece::White, Piece::Pawn), i, 1);
            m_board.InitPiece(new Piece(Piece::Black, Piece::Pawn), i, 6);
        }

        // Then Rooks:
        m_board.InitPiece(new Piece(Piece::White, Piece::Rook), 0, 0);
        m_board.InitPiece(new Piece(Piece::White, Piece::Rook), 7, 0);
        m_board.InitPiece(new Piece(Piece::Black, Piece::Rook), 0, 7);
        m_board.InitPiece(new Piece(Piece::Black, Piece::Rook), 7, 7);

        // Then Knights:
        m_board.InitPiece(new Piece(Piece::White, Piece::Knight), 1, 0);
        m_board.InitPiece(new Piece(Piece::White, Piece::Knight), 6, 0);
        m_board.InitPiece(new Piece(Piece::Black, Piece::Knight), 1, 7);
        m_board.InitPiece(new Piece(Piece::Black, Piece::Knight), 6, 7);

        // Then Bishops:
        m_board.InitPiece(new Piece(Piece::White, Piece::Bishop), 2, 0);
        m_board.InitPiece(new Piece(Piece::White, Piece::Bishop), 5, 0);
        m_board.InitPiece(new Piece(Piece::Black, Piece::Bishop), 2, 7);
        m_board.InitPiece(new Piece(Piece::Black, Piece::Bishop), 5, 7);

        // Then Queens:
        m_board.InitPiece(new Piece(Piece::White, Piece::Queen), 3, 0);
        m_board.InitPiece(new Piece(Piece::Black, Piece::Queen), 3, 7);

        // Then Kings:
        m_board.InitPiece(new Piece(Piece::White, Piece::King), 4, 0);
        m_board.InitPiece(new Piece(Piece::Black, Piece::King), 4, 7);

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
                break;
            case Piece::Black:
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


int __cmp_with_zero(int n)
{
    int ret = 0;
    if(0 < n)
        ret = 1;
    else if(0 > n)
        ret = -1;
    return ret;
}

// This function assumes that source and destination are in a straight line away from each
//  other, horizontally, vertically or diagonally. Any other inputs will probably seg fault.
// It is this way intentionally as an optimization to make it as fast as possible.
static bool __is_path_blocked(Square const &s, Square const &d, Piece::AllegienceEnum a)
{
    bool ret = false;
    Square const *cur = &s;
    int cmp_res_col = __cmp_with_zero(d.GetColumn() - cur->GetColumn());
    int cmp_res_row = __cmp_with_zero(d.GetRow() - cur->GetRow());

    while(1)
    {
        cur = &cur->GetBoard()->GetSquare(cur->GetColumn() + cmp_res_col, cur->GetRow() + cmp_res_row);

        Piece const *p = cur->GetPiece();
        if(cur == &d)
        {
            // The destination square is only blocked if there is a piece that belongs
            //  to the same allegience as the piece being moved.
            if(p)
                ret = a == p->GetAllegience();
            break;
        }
        else if(p)
        {
            ret = true;
            break;
        }
    };
    return ret;
}

GameLogic::MoveValidationEnum GameLogic::ValidateMove(const Square &s, const Square &d) const
{
    if(!(&m_board == s.GetBoard() && &m_board == d.GetBoard()))
        return InvalidUnknownSquare;

    Piece const *p( s.GetPiece() );
    if(!p)
        return InvalidEmptySquare;


    // Validate the low-level technical aspects of the move, ignoring threats to the king

    // A piece cannot stay in the same place if it is moving
    if(&s == &d)
        return InvalidTechnical;

    bool technically_ok = false;
    int col_diff = d.GetColumn() - s.GetColumn();
    int row_diff = d.GetRow() - s.GetRow();
    int col_diff_abs = Abs(col_diff);
    int row_diff_abs = Abs(row_diff);
    switch(p->GetType())
    {
    case Piece::Pawn:
        break;
    case Piece::Bishop:
        if(col_diff_abs == row_diff_abs)
            technically_ok = !__is_path_blocked(s, d, p->GetAllegience());
        break;
    case Piece::Knight:
        break;
    case Piece::Rook:
        if(0 == col_diff_abs || 0 == row_diff_abs)
            technically_ok = !__is_path_blocked(s, d, p->GetAllegience());
        break;
    case Piece::Queen:
        if(0 == col_diff_abs || 0 == row_diff_abs || col_diff_abs == row_diff_abs)
            technically_ok = !__is_path_blocked(s, d, p->GetAllegience());
        break;
    case Piece::King:
        if(1 >= col_diff_abs && 1 >= row_diff_abs)
            technically_ok = !__is_path_blocked(s, d, p->GetAllegience());
        break;
    default:
        // We should not have an unkown piece type
        THROW_NEW_GUTIL_EXCEPTION(Exception);
    }

    return ValidMove;
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
