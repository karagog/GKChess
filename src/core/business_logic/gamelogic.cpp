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

void GameLogic::_init_piece(int c, int r, Piece::AllegienceEnum a, Piece::PieceTypeEnum t)
{
    (Piece::White == a ? &m_whitePieceIndex : &m_blackPieceIndex)
            ->InsertMulti(t, m_board.SetPiece(new Piece(t, a), c, r));
}

void GameLogic::SetupNewGame(GameLogic::SetupTypeEnum ste)
{
    m_board.Clear();
    m_whitePieceIndex.Clear();
    m_blackPieceIndex.Clear();

    switch(ste)
    {
    case StandardChess:
        // First set up Pawns:
        for(int i = 0; i < m_board.ColumnCount(); ++i){
            _init_piece(i, 1, Piece::White, Piece::Pawn);
            _init_piece(i, 6, Piece::Black, Piece::Pawn);
        }

        // Then Rooks:
        _init_piece(0, 0, Piece::White, Piece::Rook);
        _init_piece(7, 0, Piece::White, Piece::Rook);
        _init_piece(0, 7, Piece::Black, Piece::Rook);
        _init_piece(7, 7, Piece::Black, Piece::Rook);

        // Then Knights:
        _init_piece(1, 0, Piece::White, Piece::Knight);
        _init_piece(6, 0, Piece::White, Piece::Knight);
        _init_piece(1, 7, Piece::Black, Piece::Knight);
        _init_piece(6, 7, Piece::Black, Piece::Knight);

        // Then Bishops:
        _init_piece(2, 0, Piece::White, Piece::Bishop);
        _init_piece(5, 0, Piece::White, Piece::Bishop);
        _init_piece(2, 7, Piece::Black, Piece::Bishop);
        _init_piece(5, 7, Piece::Black, Piece::Bishop);

        // Then Queens:
        _init_piece(3, 0, Piece::White, Piece::Queen);
        _init_piece(3, 7, Piece::Black, Piece::Queen);

        // Then Kings:
        _init_piece(4, 0, Piece::White, Piece::King);
        _init_piece(4, 7, Piece::Black, Piece::King);

        break;
    default:
        break;
    }

    emit NotifyNewGame((int)ste);
}

Vector<ISquare const *> GameLogic::FindPieces(Piece::AllegienceEnum a, Piece::PieceTypeEnum t) const
{
    Vector<ISquare const *> ret( (Piece::White == a ? &m_whitePieceIndex : &m_blackPieceIndex)->Values(t) );

    // To help debug, make sure all the returned pieces are the correct type
    for(GUINT32 i = 0; i < ret.Length(); ++i){
        Piece const *p = ret[i]->GetPiece();
        GUTIL_UNUSED(p);
        GASSERT(p && t == p->GetType() && a == p->GetAllegience());
    }

    return ret;
}

Vector<ISquare const *> GameLogic::GetPossibleMoves(const ISquare &s, const Piece &p) const
{
    Vector<ISquare const *> ret;

    switch(p.GetType())
    {
    case Piece::Pawn:
    {
        // The pawn can always move one or two steps forward if not capturing
        ISquare *temp1(NULL), *temp2(NULL);
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
static bool __is_path_blocked(Board const &b, ISquare const &s, ISquare const &d, Piece::AllegienceEnum a)
{
    bool ret = false;
    ISquare const *cur = &s;
    int cmp_res_col = __cmp_with_zero(d.GetColumn() - cur->GetColumn());
    int cmp_res_row = __cmp_with_zero(d.GetRow() - cur->GetRow());

    while(1)
    {
        cur = &b.GetSquare(cur->GetColumn() + cmp_res_col, cur->GetRow() + cmp_res_row);

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

GameLogic::MoveValidationEnum GameLogic::ValidateMove(const ISquare &s, const ISquare &d) const
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
            technically_ok = !__is_path_blocked(m_board, s, d, p->GetAllegience());
        break;
    case Piece::Knight:
        break;
    case Piece::Rook:
        if(0 == col_diff_abs || 0 == row_diff_abs)
            technically_ok = !__is_path_blocked(m_board, s, d, p->GetAllegience());
        break;
    case Piece::Queen:
        if(0 == col_diff_abs || 0 == row_diff_abs || col_diff_abs == row_diff_abs)
            technically_ok = !__is_path_blocked(m_board, s, d, p->GetAllegience());
        break;
    case Piece::King:
        if(1 >= col_diff_abs && 1 >= row_diff_abs)
            technically_ok = !__is_path_blocked(m_board, s, d, p->GetAllegience());
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

static void __validate_square(ISquare const *s)
{
    if(NULL == s)
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Both the source and destination squares must be given");

    if(0 > s->GetRow() || 7 < s->GetRow() || 0 > s->GetColumn() || 7 < s->GetColumn())
        THROW_NEW_GUTIL_EXCEPTION2(IndexOutOfRangeException, "Invalid square");
}

void GameLogic::_move(const MoveData &md, bool reverse)
{
    ISquare *source = 0, *dest = 0;

    if(reverse)
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
    else
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
}

static int __allegience_to_rank_increment(Piece::AllegienceEnum a)
{
    return Piece::White == a ? -1 : 1;
}

static bool __is_move_valid_for_bishop(Board const *b,
                                       ISquare const *s,
                                       ISquare const *d,
                                       Piece::AllegienceEnum a)
{
    int col_diff_abs = Abs(s->GetColumn() - d->GetColumn());
    int row_diff_abs = Abs(s->GetRow() - d->GetRow());
    return col_diff_abs == row_diff_abs &&
            !__is_path_blocked(*b, *s, *d, a);
}

static bool __is_move_valid_for_knight(Board const *b,
                                       ISquare const *s,
                                       ISquare const *d,
                                       Piece::AllegienceEnum a)
{
    GUTIL_UNUSED(b);
    GUTIL_UNUSED(a);
    int col_diff_abs = Abs(s->GetColumn() - d->GetColumn());
    int row_diff_abs = Abs(s->GetRow() - d->GetRow());
    return (1 == col_diff_abs && 2 == row_diff_abs) ||
            (2 == col_diff_abs && 1 == row_diff_abs);
}

static bool __is_move_valid_for_rook(Board const *b,
                                     ISquare const *s,
                                     ISquare const *d,
                                     Piece::AllegienceEnum a)
{
    int col_diff = s->GetColumn() - d->GetColumn();
    int row_diff = s->GetRow() - d->GetRow();
    return ((0 == col_diff && 0 != row_diff) ||
            (0 == row_diff && 0 != col_diff))
            &&
            !__is_path_blocked(*b, *s, *d, a);
}

static bool __is_move_valid_for_queen(Board const *b,
                                      ISquare const *s,
                                      ISquare const *d,
                                      Piece::AllegienceEnum a)
{
    int col_diff_abs = Abs(s->GetColumn() - d->GetColumn());
    int row_diff_abs = Abs(s->GetRow() - d->GetRow());
    return (col_diff_abs == row_diff_abs || ((0 == col_diff_abs && 0 != row_diff_abs) ||
                                             (0 == row_diff_abs && 0 != col_diff_abs)))
            &&
            !__is_path_blocked(*b, *s, *d, a);
}

static bool __is_move_valid_for_king(Board const *b,
                                     ISquare const *s,
                                     ISquare const *d,
                                     Piece::AllegienceEnum a)
{
    GUTIL_UNUSED(b);
    GUTIL_UNUSED(a);
    int col_diff_abs = Abs(s->GetColumn() - d->GetColumn());
    int row_diff_abs = Abs(s->GetRow() - d->GetRow());
    return 1 >= col_diff_abs && 1 >= row_diff_abs;
}

GameLogic::MoveData GameLogic::_translate_move_data(const PGN_MoveData &m)
{
    MoveData ret;
    Piece::AllegienceEnum turn = WhoseTurn();

    if(m.Flags.TestFlag(PGN_MoveData::CastleNormal))
        ret.CastleType = MoveData::CastleNormal;
    else if(m.Flags.TestFlag(PGN_MoveData::CastleQueenSide))
        ret.CastleType = MoveData::CastleQueenside;
    else
    {
        // Validate the inputs
        if('a' > m.DestFile || m.DestFile > 'h')
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "The destination file is invalid");
        if(0 >= m.DestRank || m.DestRank > 8)
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "The destination rank is invalid");
        if(0 != m.SourceFile && ('a' > m.SourceFile || m.SourceFile > 'h'))
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "The source file is invalid");
        if(-1 != m.SourceRank && (0 >= m.SourceRank || m.SourceRank > 8))
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "The source rank is invalid");


        // Get the destination square (this is always given)
        ISquare *dest = &m_board.GetSquare(m.DestFile - 'a', m.DestRank - 1);
        ret.Destination = dest;

        if(m.Flags.TestFlag(PGN_MoveData::Capture))
        {
            if(ret.Destination->GetPiece() && ret.Destination->GetPiece()->GetAllegience() != turn)
                ret.PieceCaptured = dest->GetPiece();
            else
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid piece to capture at the destination square");
        }


        // Find the source square (this is only seldom given; normally we have to search)
        int tmp_source_column = 0 == m.SourceFile ? -1 : m.SourceFile - 'a';
        if(m.SourceFile != 0 && m.SourceRank != 0)
        {
            ISquare *s = &m_board.GetSquare(tmp_source_column, m.SourceRank - 1);

            // Do a sanity check on the square they specified
            if(NULL == s->GetPiece())
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "No piece on the square");
            if(s->GetPiece()->GetAllegience() != turn)
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "It is not your turn");
            if(s->GetPiece()->GetType() != m.PieceMoved)
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "The piece on that square is different than the piece you said to move");

            ret.Source = s;
        }
        else
        {
            // If the source was not specified explicitly then we need to search
            //  with whatever information we've got

            // The possible source squares depend on the piece being moved
            switch(m.PieceMoved)
            {
            case Piece::Pawn:
                // The pawn takes the most code, because it moves differently depending on if it's
                //  capturing or not.  It can also move two squares instead of one on the first move.
                if(m.Flags.TestFlag(PGN_MoveData::Capture))
                {
                    // If the pawn is capturing then it can only be from either of the two files
                    //  next to the destination
                    if(-1 != tmp_source_column &&
                            1 != Abs(tmp_source_column - ret.Destination->GetColumn()))
                        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid file for pawn capture");

                    Vector<ISquare const *> possible_sources( FindPieces(turn, m.PieceMoved) );
                    if(0 == possible_sources.Length())
                        THROW_NEW_GUTIL_EXCEPTION2(Exception, "There are no pieces of that type to move");

                    for(GUINT32 i = 0; i < possible_sources.Length(); ++i)
                    {
                        ISquare const *s = possible_sources[i];

                        if((1 == Abs(s->GetColumn() - ret.Destination->GetColumn())) &&
                                s->GetRow() == ret.Destination->GetRow() + __allegience_to_rank_increment(turn))
                        {
                            if(-1 != tmp_source_column)
                            {
                                if(s->GetColumn() == tmp_source_column)
                                {
                                    ret.Source = s;

                                    // We can break here, because the user specified this file, so
                                    //  there is no abiguity
                                    break;
                                }
                            }
                            else
                            {
                                if(NULL != ret.Source)
                                    THROW_NEW_GUTIL_EXCEPTION2(Exception, "Multiple pieces found which could move there");

                                ret.Source = s;

                                // Don't break here, because we need to keep searching all the pawns
                                //  for an ambiguous move
                            }
                        }
                    }
                }
                else
                {
                    // If the pawn is not capturing, it must be in the same file
                    int r = ret.Destination->GetRow() + __allegience_to_rank_increment(turn);
                    if(0 > r || r > 7)
                        THROW_NEW_GUTIL_EXCEPTION2(Exception, "No such piece can reach the square");

                    ISquare const *s = &m_board.GetSquare(ret.Destination->GetColumn(), r);
                    if(NULL == s->GetPiece())
                    {
                        // The pawn can move two squares on the first move
                        r = s->GetRow() + __allegience_to_rank_increment(turn);
                        if((turn == Piece::White && 1 != r) || (turn == Piece::Black && 6 != r) ||
                                NULL == (s = &m_board.GetSquare(ret.Destination->GetColumn(), r))->GetPiece())
                            THROW_NEW_GUTIL_EXCEPTION2(Exception, "No such piece can reach the square");
                    }

                    if(s->GetPiece()->GetAllegience() != turn ||
                            s->GetPiece()->GetType() != Piece::Pawn)
                        THROW_NEW_GUTIL_EXCEPTION2(Exception, "No such piece can reach the square");

                    if(NULL != ret.Destination->GetPiece())
                        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Destination square occupied");

                    ret.Source = s;
                }

                break;
            case Piece::Knight:
            {
                // Knights are easy, because they cannot be blocked. If they are in range of
                //  the square then it is a valid move.
                Vector<ISquare const *> possible_sources( FindPieces(turn, m.PieceMoved) );
                for(GUINT32 i = 0; i < possible_sources.Length(); ++i)
                {
                    ISquare const *s = possible_sources[i];

                    if(__is_move_valid_for_knight(&m_board, s, ret.Destination, turn))
                    {
                        if(-1 != tmp_source_column)
                        {
                            if(s->GetColumn() == tmp_source_column){
                                ret.Source = s;
                                break;
                            }
                        }
                        else
                        {
                            if(NULL != ret.Source)
                                THROW_NEW_GUTIL_EXCEPTION2(Exception, "Multiple pieces found which could move there");

                            ret.Source = s;
                        }
                    }
                }
            }
                break;
            case Piece::Bishop:
            {
                Vector<ISquare const *> possible_sources( FindPieces(turn, m.PieceMoved) );
                for(GUINT32 i = 0; i < possible_sources.Length(); ++i)
                {
                    ISquare const *s = possible_sources[i];

                    if(__is_move_valid_for_bishop(&m_board, s, ret.Destination, turn))
                    {
                        if(-1 != tmp_source_column)
                        {
                            if(s->GetColumn() == tmp_source_column){
                                ret.Source = s;
                                break;
                            }
                        }
                        else
                        {
                            if(NULL != ret.Source)
                                THROW_NEW_GUTIL_EXCEPTION2(Exception, "Multiple pieces found which could move there");

                            ret.Source = s;
                        }
                    }
                }
            }
                break;
            case Piece::Rook:
            {
                Vector<ISquare const *> possible_sources( FindPieces(turn, m.PieceMoved) );
                for(GUINT32 i = 0; i < possible_sources.Length(); ++i)
                {
                    ISquare const *s = possible_sources[i];

                    if(__is_move_valid_for_rook(&m_board, s, ret.Destination, turn))
                    {
                        if(-1 != tmp_source_column)
                        {
                            if(s->GetColumn() == tmp_source_column){
                                ret.Source = s;
                                break;
                            }
                        }
                        else
                        {
                            if(NULL != ret.Source)
                                THROW_NEW_GUTIL_EXCEPTION2(Exception, "Multiple pieces found which could move there");

                            ret.Source = s;
                        }
                    }
                }
            }
                break;
            case Piece::Queen:
            {
                Vector<ISquare const *> possible_sources( FindPieces(turn, m.PieceMoved) );
                for(GUINT32 i = 0; i < possible_sources.Length(); ++i)
                {
                    ISquare const *s = possible_sources[i];

                    if(__is_move_valid_for_queen(&m_board, s, ret.Destination, turn))
                    {
                        if(-1 != tmp_source_column)
                        {
                            if(s->GetColumn() == tmp_source_column){
                                ret.Source = s;
                                break;
                            }
                        }
                        else
                        {
                            if(NULL != ret.Source)
                                THROW_NEW_GUTIL_EXCEPTION2(Exception, "Multiple pieces found which could move there");

                            ret.Source = s;
                        }
                    }
                }
            }
                break;
            case Piece::King:
            {
                Vector<ISquare const *> possible_sources( FindPieces(turn, m.PieceMoved) );

                // There can only be one king
                GASSERT(1 == possible_sources.Length());

                ISquare const *s = possible_sources[0];
                if(__is_move_valid_for_king(&m_board, s, ret.Destination, turn))
                    ret.Source = s;
            }
                break;
            default:
                THROW_NEW_GUTIL_EXCEPTION(Exception);
            }
        }

        if(NULL == ret.Source)
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "No such piece can reach the square");

        ret.PieceMoved = m_board.GetSquare(ret.Source->GetColumn(), ret.Source->GetRow()).GetPiece();
        GASSERT(NULL != ret.PieceMoved);

        // Add a promoted piece, if necessary
        if(Piece::Pawn != m.PiecePromoted)
        {
            if(Piece::Pawn != m.PieceMoved)
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "Only pawns can be promoted");

            ret.PiecePromoted = new Piece(m.PiecePromoted, turn);
        }
    }

    return ret;
}

void GameLogic::Undo()
{
    if(0 <= m_moveHistoryIndex)
    {
        _move(m_moveHistory[m_moveHistoryIndex], true);
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
