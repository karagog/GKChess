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

#include "abstractboard.h"
#include "piece.h"
#include "isquare.h"
#include "pgn_move_data.h"
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;


AbstractBoard::AbstractBoard(QObject *p)
    :QObject(p)
{}

AbstractBoard::~AbstractBoard()
{}

void AbstractBoard::SetPiece(const Piece &p, ISquare const &sqr)
{
    if(0 <= sqr.GetColumn() && sqr.GetColumn() < ColumnCount() &&
            0 <= sqr.GetRow() && sqr.GetRow() < RowCount())
    {
        emit NotifySquareAboutToBeUpdated(sqr);
        set_piece_p(p, sqr.GetColumn(), sqr.GetRow());
        emit NotifySquareUpdated(sqr);
    }
}

Piece const *AbstractBoard::GetPiece(int column, int row) const
{
    return SquareAt(column, row).GetPiece();
}

static void __update_gamestate(AbstractBoard &b, IGameState &gs, const MoveData &md)
{
    int inc;
    Piece::AllegienceEnum next_turn;
    Piece const &p( md.PieceMoved );

    if(md.IsNull() || p.IsNull())
        return;

    gs.SetHalfMoveClock(gs.GetHalfMoveClock() + 1);
    if(p.GetAllegience() == Piece::Black)
    {
        gs.SetFullMoveNumber(gs.GetFullMoveNumber() + 1);
        next_turn = Piece::White;
        inc = -1;
    }
    else
    {
        gs.SetWhoseTurn(Piece::Black);
        next_turn = Piece::Black;
        inc = 1;
    }
    gs.SetWhoseTurn(next_turn);

    // Set the en-passant square
    if(p.GetType() == Piece::Pawn)
    {
        int row_diff_abs = Abs(md.Destination->GetRow() - md.Source->GetRow());
        if(2 == row_diff_abs)
            gs.SetEnPassantSquare(&b.SquareAt(md.Source->GetColumn(), md.Source->GetRow() + inc));
        else
            gs.SetEnPassantSquare(0);
    }
    else
        gs.SetEnPassantSquare(0);
}

void AbstractBoard::Move(const MoveData &md)
{
    if(!md.IsNull())
    {
        // If you want more moves to be valid you can implement your own liberal validator
        if(IGameLogic::ValidMove == ValidateMove(*md.Source, *md.Destination))
        {
            emit NotifyPieceAboutToBeMoved(md);
            {
                // Move the piece and update the gamestate
                move_p(md);
                __update_gamestate(*this, GameState(), md);
            }
            emit NotifyPieceMoved(md);
        }
    }
}

void AbstractBoard::Resign(Piece::AllegienceEnum a)
{
    THROW_NEW_GUTIL_EXCEPTION(NotImplementedException);
    emit NotifyResignation(a);
}

void AbstractBoard::Clear()
{
    for(int i = 0; i < ColumnCount(); ++i)
        for(int j = 0; j < RowCount(); ++j)
            SetPiece(Piece(), SquareAt(i, j));
}

void AbstractBoard::FromFEN(const String &s)
{
    String cpy( s.Trimmed() );
    StringList sl( cpy.Split(' ', false) );
    if(6 != sl.Length())
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "FEN requires 6 fields separated by spaces");

    // First parse the position text:
    {
        StringList sl2( sl[0].Split('/', false) );
        if(8 != sl2.Length())
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "FEN position text requires 8 fields separated by /");

        // For each section of position text...
        for(int i = 0; i < sl2.Length(); ++i)
        {
            int col = 0;
            typename String::const_iterator iter(sl2[i].begin());
            typename String::const_iterator next(iter + 1);

            // For each character in the section...
            for(;
                iter != sl2[i].end() && col < ColumnCount();
                ++iter)
            {
                int num(-1);
                int rank = 7 - i;
                char c = *iter.Current();
                char n = (next == sl2[i].end() ? 0 : *next.Current());
                if(String::IsNumber(c))
                {
                    // Normally numbers are single digit, but we want to support larger boards too
                    if(String::IsNumber(n))
                        num = String(iter, next + 1).ToInt();
                    else
                        num = String(c).ToInt();
                }

                if(-1 != num)
                {
                    // Numbers define empty space. Set all empty spaces null
                    for(int h = col; h < col + num; ++h)
                        SetPiece(Piece(), SquareAt(h, rank));
                    col += num;
                }
                else
                {
                    SetPiece(Piece::FromFEN(c), SquareAt(col, rank));
                    col++;
                }

                if(next != sl2[i].end())
                    ++next;
            }
        }
    }


    // Then parse whose turn it is:
    {
        Piece::AllegienceEnum a;
        switch(sl[1][0])
        {
        case 'b':
            a = Piece::Black;
            break;
        case 'w':
            a = Piece::White;
            break;
        default:
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "The current turn must be either a 'w' or 'b'");
        }
        GameState().SetWhoseTurn(a);
    }


    // Then parse the castle info:
    {
        GameState().SetCastleWhite1(-1);
        GameState().SetCastleWhite2(-1);
        GameState().SetCastleBlack1(-1);
        GameState().SetCastleBlack2(-1);

        G_FOREACH_CONST(char c, sl[2])
        {
            Piece::AllegienceEnum a = String::IsUpper(c) ? Piece::White : Piece::Black;
            char tmps[2] = {c, '\0'};
            String::ToUpper(&c, tmps);
            switch(c)
            {
            case '-':
                // If a dash is given, then this field is empty (nobody can castle)
                break;
            case 'K':
                // Standard FEN - translate to X-FEN
                c = 'H';
                break;
            case 'Q':
                // Standard FEN - translate to X-FEN
                c = 'A';
                break;
            default:
                break;
            }

            // X-FEN specifies the castle files occupied by the rooks, the char must fall in
            //  the range from a-h
            if('A' <= c && c <= 'H')
            {
                int file = c-'A';
                switch(a)
                {
                case Piece::White:
                    if(-1 == GameState().GetCastleWhite1())
                        GameState().SetCastleWhite1(file);
                    else
                        GameState().SetCastleWhite2(file);
                    break;
                case Piece::Black:
                    if(-1 == GameState().GetCastleBlack1())
                        GameState().SetCastleBlack1(file);
                    else
                        GameState().SetCastleBlack2(file);
                    break;
                default: break;
                }
            }
            else
            {
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "There was an error with the castle info");
            }
        }
    }


    // Parse the en-passant square:
    {
        if(sl[3] != "-")
        {
            if(sl[3].Length() != 2)
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid En Passant square");

            char f = sl[3][0];
            char rnk = sl[3][1];
            if(f < 'a' || 'h' < f || rnk < '1' || '8' < rnk)
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid En Passant square");

            GameState().SetEnPassantSquare(&SquareAt(f - 'a', rnk - '1'));
        }
    }


    // Parse the half-move clock:
    {
        bool ok(false);
        GameState().SetHalfMoveClock(sl[4].ToInt(&ok));
        if(!ok)
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid half-move clock");
    }


    // Parse the full-move number:
    {
        bool ok(false);
        GameState().SetFullMoveNumber(sl[5].ToInt(&ok));
        if(!ok)
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid full-move number");
    }
}

String AbstractBoard::ToFEN() const
{
    THROW_NEW_GUTIL_EXCEPTION(NotImplementedException);
}







/**  Below is the chess logic implementation  */






/** Converts an allegience into a positive or negative 1, so we know what direction is forward. */
static int __allegience_to_rank_increment(Piece::AllegienceEnum a)
{
    return Piece::White == a ? -1 : 1;
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
static bool __is_path_blocked(AbstractBoard const &b, ISquare const &s, ISquare const &d, Piece::AllegienceEnum a)
{
    bool ret = false;
    ISquare const *cur = &s;
    int cmp_res_col = __cmp_with_zero(d.GetColumn() - cur->GetColumn());
    int cmp_res_row = __cmp_with_zero(d.GetRow() - cur->GetRow());

    while(1)
    {
        cur = &b.SquareAt(cur->GetColumn() + cmp_res_col, cur->GetRow() + cmp_res_row);

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

static bool __is_move_valid_for_bishop(AbstractBoard const &b,
                                       ISquare const *s,
                                       ISquare const *d,
                                       Piece::AllegienceEnum a)
{
    int col_diff_abs = Abs(s->GetColumn() - d->GetColumn());
    int row_diff_abs = Abs(s->GetRow() - d->GetRow());
    return col_diff_abs == row_diff_abs &&
            !__is_path_blocked(b, *s, *d, a);
}

static bool __is_move_valid_for_knight(AbstractBoard const &b,
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

static bool __is_move_valid_for_rook(AbstractBoard const &b,
                                     ISquare const *s,
                                     ISquare const *d,
                                     Piece::AllegienceEnum a)
{
    int col_diff = s->GetColumn() - d->GetColumn();
    int row_diff = s->GetRow() - d->GetRow();
    return ((0 == col_diff && 0 != row_diff) ||
            (0 == row_diff && 0 != col_diff))
            &&
            !__is_path_blocked(b, *s, *d, a);
}

static bool __is_move_valid_for_queen(AbstractBoard const &b,
                                      ISquare const *s,
                                      ISquare const *d,
                                      Piece::AllegienceEnum a)
{
    int col_diff_abs = Abs(s->GetColumn() - d->GetColumn());
    int row_diff_abs = Abs(s->GetRow() - d->GetRow());
    return (col_diff_abs == row_diff_abs || ((0 == col_diff_abs && 0 != row_diff_abs) ||
                                             (0 == row_diff_abs && 0 != col_diff_abs)))
            &&
            !__is_path_blocked(b, *s, *d, a);
}

static bool __is_move_valid_for_king(AbstractBoard const &b,
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


void AbstractBoard::SetupNewGame(AbstractBoard::SetupTypeEnum ste)
{
    switch(ste)
    {
    case SetupStandardChess:
        FromFEN(FEN_STANDARD_CHESS_STARTING_POSITION);
        break;
    case SetupChess960:
    default:
        THROW_NEW_GUTIL_EXCEPTION(NotImplementedException);
        break;
    }
}

AbstractBoard::MoveValidationEnum AbstractBoard::ValidateMove(const ISquare &s, const ISquare &d) const
{
    Piece const *p(s.GetPiece());
    Piece const *dp(d.GetPiece());
    if(NULL == p)
        return InvalidEmptySquare;


    // Validate the low-level technical aspects of the move, ignoring threats to the king

    // Only the player whose turn it is can move pieces
    if(p->GetAllegience() != GameState().GetWhoseTurn())
        return InvalidTechnical;

    // A piece cannot stay in the same place if it is moving
    if(s == d)
        return InvalidTechnical;

    bool technically_ok = false;
    int col_diff = d.GetColumn() - s.GetColumn();
    int row_diff = d.GetRow() - s.GetRow();
    int col_diff_abs = Abs(col_diff);
    int row_diff_abs = Abs(row_diff);
    switch(p->GetType())
    {
    case Piece::Pawn:
    {
        int sign;
        int startRank;
        if(Piece::White == p->GetAllegience()){
            sign = 1;
            startRank = 1;
        }
        else{
            sign = -1;
            startRank = 6;
        }

        // non-capture
        if(col_diff == 0)
        {
            technically_ok = row_diff == 1*sign || (startRank == s.GetRow() && row_diff == 2*sign);
        }

        // capture move
        else if(col_diff_abs == 1)
        {
            technically_ok =

                    // must be one diagonal forward
                    row_diff == 1*sign && 1 == col_diff_abs &&

                    // there must be a captured piece of the other color on the dest square
                    ((dp && dp->GetAllegience() != p->GetAllegience()) ||

                     //  or the square being captured to must be an en passant square
                     (GameState().GetEnPassantSquare() && d == *GameState().GetEnPassantSquare()));
        }
    }
        break;
    case Piece::Bishop:
        if(col_diff_abs == row_diff_abs)
            technically_ok = !__is_path_blocked(*this, s, d, p->GetAllegience());
        break;
    case Piece::Knight:
        technically_ok = __is_move_valid_for_knight(*this, &s, &d, p->GetAllegience()) &&
                (!dp || dp->GetAllegience() == Piece::Black);
        break;
    case Piece::Rook:
        if(0 == col_diff_abs || 0 == row_diff_abs)
            technically_ok = !__is_path_blocked(*this, s, d, p->GetAllegience());
        break;
    case Piece::Queen:
        if(0 == col_diff_abs || 0 == row_diff_abs || col_diff_abs == row_diff_abs)
            technically_ok = !__is_path_blocked(*this, s, d, p->GetAllegience());
        break;
    case Piece::King:
        if(1 >= col_diff_abs && 1 >= row_diff_abs)
            technically_ok = !__is_path_blocked(*this, s, d, p->GetAllegience());
        break;
    default:
        // We should not have an unkown piece type
        THROW_NEW_GUTIL_EXCEPTION(Exception);
    }

    if(!technically_ok)
        return InvalidTechnical;

    if(d.GetPiece() && d.GetPiece()->GetAllegience() == p->GetAllegience())
        return InvalidTechnical;

    return ValidMove;
}

Vector<ISquare const *> AbstractBoard::GetValidMovesForSquare(const ISquare &) const
{
    Vector<ISquare const *> ret;
    return ret;
}

MoveData AbstractBoard::GenerateMoveData(const PGN_MoveData &m) const
{
    MoveData ret;
    Piece::AllegienceEnum turn = GameState().GetWhoseTurn();

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
        ISquare const *dest = &SquareAt(m.DestFile - 'a', m.DestRank - 1);
        ret.Destination = dest;

        if(m.Flags.TestFlag(PGN_MoveData::Capture))
        {
            if(dest->GetPiece() && dest->GetPiece()->GetAllegience() != turn)
                ret.PieceCaptured = *dest->GetPiece();
            else
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid piece to capture at the destination square");
        }


        // Find the source square (this is only seldom given; normally we have to search)
        int tmp_source_column = 0 == m.SourceFile ? -1 : m.SourceFile - 'a';
        if(m.SourceFile != 0 && m.SourceRank != 0)
        {
            ISquare const *s = &SquareAt(tmp_source_column, m.SourceRank - 1);

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

                    Vector<ISquare const *> possible_sources( FindPieces(Piece(m.PieceMoved, turn)) );
                    if(0 == possible_sources.Length())
                        THROW_NEW_GUTIL_EXCEPTION2(Exception, "There are no pieces of that type to move");

                    for(GINT32 i = 0; i < possible_sources.Length(); ++i)
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

                    ISquare const *s = &SquareAt(ret.Destination->GetColumn(), r);
                    if(NULL == s->GetPiece())
                    {
                        // The pawn can move two squares on the first move
                        r = s->GetRow() + __allegience_to_rank_increment(turn);
                        if((turn == Piece::White && 1 != r) || (turn == Piece::Black && 6 != r) ||
                                NULL == (s = &SquareAt(ret.Destination->GetColumn(), r))->GetPiece())
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
                Vector<ISquare const *> possible_sources( FindPieces(Piece(m.PieceMoved, turn)) );
                for(GINT32 i = 0; i < possible_sources.Length(); ++i)
                {
                    ISquare const *s = possible_sources[i];

                    if(__is_move_valid_for_knight(*this, s, ret.Destination, turn))
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
                Vector<ISquare const *> possible_sources( FindPieces(Piece(m.PieceMoved, turn)) );
                for(GINT32 i = 0; i < possible_sources.Length(); ++i)
                {
                    ISquare const *s = possible_sources[i];

                    if(__is_move_valid_for_bishop(*this, s, ret.Destination, turn))
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
                Vector<ISquare const *> possible_sources( FindPieces(Piece(m.PieceMoved, turn)) );
                for(GINT32 i = 0; i < possible_sources.Length(); ++i)
                {
                    ISquare const *s = possible_sources[i];

                    if(__is_move_valid_for_rook(*this, s, ret.Destination, turn))
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
                Vector<ISquare const *> possible_sources( FindPieces(Piece(m.PieceMoved, turn)) );
                for(GINT32 i = 0; i < possible_sources.Length(); ++i)
                {
                    ISquare const *s = possible_sources[i];

                    if(__is_move_valid_for_queen(*this, s, ret.Destination, turn))
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
                Vector<ISquare const *> possible_sources( FindPieces(Piece(m.PieceMoved, turn)) );

                // There can only be one king
                GASSERT(1 == possible_sources.Length());

                ISquare const *s = possible_sources[0];
                if(__is_move_valid_for_king(*this, s, ret.Destination, turn))
                    ret.Source = s;
            }
                break;
            default:
                THROW_NEW_GUTIL_EXCEPTION(Exception);
            }
        }

        if(NULL == ret.Source)
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "No such piece can reach the square");

        ret.PieceMoved = *SquareAt(ret.Source->GetColumn(), ret.Source->GetRow()).GetPiece();
        GASSERT(NULL != ret.PieceMoved);

        // Add a promoted piece, if necessary
        if(Piece::Pawn != m.PiecePromoted)
        {
            if(Piece::Pawn != m.PieceMoved)
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "Only pawns can be promoted");

            ret.PiecePromoted = Piece(m.PiecePromoted, turn);
        }
    }

    return ret;
}

MoveData AbstractBoard::GenerateMoveData(const ISquare &s,
                                         const ISquare &d,
                                         IPlayerResponse *uf) const
{
    MoveData ret;
    bool ok = true;

    // Check if there is a piece promotion
    if(s.GetPiece() && s.GetPiece()->GetType() == Piece::Pawn)
    {
        Piece::AllegienceEnum a = s.GetPiece()->GetAllegience();
        int promotion_rank = Piece::White == a ? 7 : 0;
        if(promotion_rank == d.GetRow())
        {
            ret.PiecePromoted = NULL == uf ? Piece(Piece::Queen, a) : uf->ChoosePromotedPiece(a);

            // If the user cancelled then we return a null move data
            if(ret.PiecePromoted.IsNull())
                ok = false;
        }
    }

    if(ok)
    {
        ret.PlyNumber = 0;
        ret.Source = &s;
        ret.Destination = &d;

        if(s.GetPiece())
            ret.PieceMoved = *s.GetPiece();

        if(d.GetPiece())
            ret.PieceCaptured = *d.GetPiece();

        //ret.CurrentPosition_FEN = b.ToFEN();
    }

    return ret;
}


END_NAMESPACE_GKCHESS;
