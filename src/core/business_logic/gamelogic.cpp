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


GameLogic::MoveData::MoveData()
    :Source(0),
      Destination(0),
      CastleType(NoCastle),
      PieceMoved(),
      PieceCaptured(),
      PiecePromoted()
{}

GameLogic::GameLogic(QObject *p)
    :QObject(p),
      m_currentTurn(Piece::White),
      m_moveHistoryIndex(-1)
{}

GameLogic::~GameLogic()
{}

Piece::AllegienceEnum GameLogic::WhoseTurn() const
{
    return (0x1 & m_moveHistoryIndex) ? Piece::White : Piece::Black;
}

void GameLogic::SetupNewGame(GameLogic::SetupTypeEnum ste)
{
    m_board.Clear();

    switch(ste)
    {
    case StandardChess:
        // First set up Pawns:
        for(int i = 0; i < m_board.ColumnCount(); ++i){
            m_board.InitPiece(new Piece(Piece::Pawn, Piece::White), i, 1);
            m_board.InitPiece(new Piece(Piece::Pawn, Piece::Black), i, 6);
        }

        // Then Rooks:
        m_board.InitPiece(new Piece(Piece::Rook, Piece::White), 0, 0);
        m_board.InitPiece(new Piece(Piece::Rook, Piece::White), 7, 0);
        m_board.InitPiece(new Piece(Piece::Rook, Piece::Black), 0, 7);
        m_board.InitPiece(new Piece(Piece::Rook, Piece::Black), 7, 7);

        // Then Knights:
        m_board.InitPiece(new Piece(Piece::Knight, Piece::White), 1, 0);
        m_board.InitPiece(new Piece(Piece::Knight, Piece::White), 6, 0);
        m_board.InitPiece(new Piece(Piece::Knight, Piece::Black), 1, 7);
        m_board.InitPiece(new Piece(Piece::Knight, Piece::Black), 6, 7);

        // Then Bishops:
        m_board.InitPiece(new Piece(Piece::Bishop, Piece::White), 2, 0);
        m_board.InitPiece(new Piece(Piece::Bishop, Piece::White), 5, 0);
        m_board.InitPiece(new Piece(Piece::Bishop, Piece::Black), 2, 7);
        m_board.InitPiece(new Piece(Piece::Bishop, Piece::Black), 5, 7);

        // Then Queens:
        m_board.InitPiece(new Piece(Piece::Queen, Piece::White), 3, 0);
        m_board.InitPiece(new Piece(Piece::Queen, Piece::Black), 3, 7);

        // Then Kings:
        m_board.InitPiece(new Piece(Piece::King, Piece::White), 4, 0);
        m_board.InitPiece(new Piece(Piece::King, Piece::Black), 4, 7);

        break;
    default:
        break;
    }
}

Vector<Square const *> GameLogic::GetPossibleMoves(const Square &s, const Piece &p) const
{
    Vector<Square const *> ret;

    switch(p.GetType())
    {
    case Piece::Pawn:
    {
        // The pawn can always move one or two steps forward if not capturing
        Square *temp1(NULL), *temp2(NULL);
        switch(p.GetAllegience())
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


    // Now we must prune the possible moves depending if they would put the moving side in Check

    return ret;
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
static bool __is_path_blocked(Board const *b, Square const &s, Square const &d, Piece::AllegienceEnum a)
{
    bool ret = false;
    Square const *cur = &s;
    int cmp_res_col = __cmp_with_zero(d.GetColumn() - cur->GetColumn());
    int cmp_res_row = __cmp_with_zero(d.GetRow() - cur->GetRow());

    while(1)
    {
        cur = &b->GetSquare(cur->GetColumn() + cmp_res_col, cur->GetRow() + cmp_res_row);

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
            technically_ok = !__is_path_blocked(&m_board, s, d, p->GetAllegience());
        break;
    case Piece::Knight:
        break;
    case Piece::Rook:
        if(0 == col_diff_abs || 0 == row_diff_abs)
            technically_ok = !__is_path_blocked(&m_board, s, d, p->GetAllegience());
        break;
    case Piece::Queen:
        if(0 == col_diff_abs || 0 == row_diff_abs || col_diff_abs == row_diff_abs)
            technically_ok = !__is_path_blocked(&m_board, s, d, p->GetAllegience());
        break;
    case Piece::King:
        if(1 >= col_diff_abs && 1 >= row_diff_abs)
            technically_ok = !__is_path_blocked(&m_board, s, d, p->GetAllegience());
        break;
    default:
        // We should not have an unkown piece type
        THROW_NEW_GUTIL_EXCEPTION(Exception);
    }

    return ValidMove;
}

void GameLogic::Move(const PGN_MoveData &md)
{
    Move(_translate_move_data(md));
}

void GameLogic::Move(const MoveData &m)
{
    _move(m);
}

static void __validate_square(Square const *s)
{
    if(NULL == s)
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Both the source and destination squares must be given");

    if(0 > s->GetRow() || 7 < s->GetRow() || 0 > s->GetColumn() || 7 < s->GetColumn())
        THROW_NEW_GUTIL_EXCEPTION2(IndexOutOfRangeException, "Invalid square");
}

void GameLogic::_move(const MoveData &md, bool direction)
{
    Square *source = 0, *dest = 0;

    if(direction)
    {
        if(MoveData::CastleNormal == md.CastleType)
        {

        }
        else if(MoveData::CastleQueenside == md.CastleType)
        {
            // Queenside castle
        }
        else
        {
            __validate_square(md.Source);
            __validate_square(md.Destination);

            // Get the non-const squares
            source = &m_board.GetSquare(md.Source->GetColumn(), md.Source->GetRow());
            dest = &m_board.GetSquare(md.Destination->GetColumn(), md.Destination->GetRow());

            if(md.PiecePromoted)
                dest->SetPiece(md.PiecePromoted);
            else
                dest->SetPiece(md.PieceMoved);

            source->SetPiece(0);
        }
    }
    else
    {
        if(MoveData::CastleNormal == md.CastleType)
        {

        }
        else if(MoveData::CastleQueenside == md.CastleType)
        {

        }
        else
        {
            __validate_square(md.Source);
            __validate_square(md.Destination);

            // Get the non-const squares
            source = &m_board.GetSquare(md.Source->GetColumn(), md.Source->GetRow());
            dest = &m_board.GetSquare(md.Destination->GetColumn(), md.Destination->GetRow());

            source->SetPiece(md.PieceMoved);
            if(md.PieceCaptured)
                dest->SetPiece(md.PieceCaptured);
            else
                dest->SetPiece(0);
        }
    }
}

GameLogic::MoveData GameLogic::_translate_move_data(const PGN_MoveData &m)
{
    MoveData ret;

    if(m.Flags.TestFlag(PGN_MoveData::CastleNormal))
        ret.CastleType = MoveData::CastleNormal;
    else if(m.Flags.TestFlag(PGN_MoveData::CastleQueenSide))
        ret.CastleType = MoveData::CastleQueenside;
    else
    {
        // Get the destination square (this is always given)
        Square *dest = &m_board.GetSquare(m.DestFile - 'a', m.DestRank - 1);
        ret.Destination = dest;

        if(m.Flags.TestFlag(PGN_MoveData::Capture))
        {
            if(ret.Destination->GetPiece())
                ret.PieceCaptured = dest->GetPiece();
            else
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "I was told to capture but there is no piece");
        }

        // Find the source square (this is only seldom given; normally we have to search)
        int tmp_file = -1;
        if(m.SourceFile != 0){
            tmp_file = m.SourceFile - 'a';
            if(m.SourceRank != 0)
            {
                // The source was specified explicitly, so no search needed
                ret.Source = &m_board.GetSquare(tmp_file, m.SourceRank - 1);
            }
            else
            {
                // Only the file was specified, so we have to find the piece
            }
        }
        else
        {
            // No source information was given, except for the type of piece
        }
    }

    return ret;
}

void GameLogic::Undo()
{
    if(0 <= m_moveHistoryIndex)
    {
        _move(m_moveHistory[m_moveHistoryIndex]);
        --m_moveHistoryIndex;
    }
}

void GameLogic::Redo()
{
    if((GINT32)m_moveHistory.Length() > m_moveHistoryIndex + 1)
    {
        _move(m_moveHistory[m_moveHistoryIndex + 1]);
        ++m_moveHistoryIndex;
    }
}


END_NAMESPACE_GKCHESS;
