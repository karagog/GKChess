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
#include "gkchess_pgn_parser.h"
#include "gkchess_abstractboard.h"
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;

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


void StandardGameLogic::SetupNewGame(AbstractBoard &b, StandardGameLogic::SetupTypeEnum ste) const
{
    switch(ste)
    {
    case StandardChess:
        b.FromFEN(FEN_STANDARD_CHESS_STARTING_POSITION);
        break;
    default:
        break;
    }
}

StandardGameLogic::MoveValidationEnum StandardGameLogic::ValidateMove(AbstractBoard const &b, const ISquare &s, const ISquare &d) const
{
    Piece const *p(s.GetPiece());
    Piece const *dp(d.GetPiece());
    if(NULL == p)
        return InvalidEmptySquare;


    // Validate the low-level technical aspects of the move, ignoring threats to the king

    // Only the player whose turn it is can move pieces
    if(p->GetAllegience() != b.GameState().GetWhoseTurn())
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
                     (b.GameState().GetEnPassantSquare() && d == *b.GameState().GetEnPassantSquare()));
        }
    }
        break;
    case Piece::Bishop:
        if(col_diff_abs == row_diff_abs)
            technically_ok = !__is_path_blocked(b, s, d, p->GetAllegience());
        break;
    case Piece::Knight:
        technically_ok = __is_move_valid_for_knight(b, &s, &d, p->GetAllegience()) &&
                (!dp || dp->GetAllegience() == Piece::Black);
        break;
    case Piece::Rook:
        if(0 == col_diff_abs || 0 == row_diff_abs)
            technically_ok = !__is_path_blocked(b, s, d, p->GetAllegience());
        break;
    case Piece::Queen:
        if(0 == col_diff_abs || 0 == row_diff_abs || col_diff_abs == row_diff_abs)
            technically_ok = !__is_path_blocked(b, s, d, p->GetAllegience());
        break;
    case Piece::King:
        if(1 >= col_diff_abs && 1 >= row_diff_abs)
            technically_ok = !__is_path_blocked(b, s, d, p->GetAllegience());
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

Vector<ISquare const *> StandardGameLogic::GetValidMovesForSquare(AbstractBoard const &, const ISquare &) const
{
    Vector<ISquare const *> ret;
    return ret;
}

MoveData StandardGameLogic::GenerateMoveData(AbstractBoard const &b, const PGN_MoveData &m) const
{
    MoveData ret;
    Piece::AllegienceEnum turn = b.GameState().GetWhoseTurn();

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
        ISquare const *dest = &b.SquareAt(m.DestFile - 'a', m.DestRank - 1);
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
            ISquare const *s = &b.SquareAt(tmp_source_column, m.SourceRank - 1);

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

                    Vector<ISquare const *> possible_sources( b.FindPieces(Piece(m.PieceMoved, turn)) );
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

                    ISquare const *s = &b.SquareAt(ret.Destination->GetColumn(), r);
                    if(NULL == s->GetPiece())
                    {
                        // The pawn can move two squares on the first move
                        r = s->GetRow() + __allegience_to_rank_increment(turn);
                        if((turn == Piece::White && 1 != r) || (turn == Piece::Black && 6 != r) ||
                                NULL == (s = &b.SquareAt(ret.Destination->GetColumn(), r))->GetPiece())
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
                Vector<ISquare const *> possible_sources( b.FindPieces(Piece(m.PieceMoved, turn)) );
                for(GINT32 i = 0; i < possible_sources.Length(); ++i)
                {
                    ISquare const *s = possible_sources[i];

                    if(__is_move_valid_for_knight(b, s, ret.Destination, turn))
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
                Vector<ISquare const *> possible_sources( b.FindPieces(Piece(m.PieceMoved, turn)) );
                for(GINT32 i = 0; i < possible_sources.Length(); ++i)
                {
                    ISquare const *s = possible_sources[i];

                    if(__is_move_valid_for_bishop(b, s, ret.Destination, turn))
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
                Vector<ISquare const *> possible_sources( b.FindPieces(Piece(m.PieceMoved, turn)) );
                for(GINT32 i = 0; i < possible_sources.Length(); ++i)
                {
                    ISquare const *s = possible_sources[i];

                    if(__is_move_valid_for_rook(b, s, ret.Destination, turn))
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
                Vector<ISquare const *> possible_sources( b.FindPieces(Piece(m.PieceMoved, turn)) );
                for(GINT32 i = 0; i < possible_sources.Length(); ++i)
                {
                    ISquare const *s = possible_sources[i];

                    if(__is_move_valid_for_queen(b, s, ret.Destination, turn))
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
                Vector<ISquare const *> possible_sources( b.FindPieces(Piece(m.PieceMoved, turn)) );

                // There can only be one king
                GASSERT(1 == possible_sources.Length());

                ISquare const *s = possible_sources[0];
                if(__is_move_valid_for_king(b, s, ret.Destination, turn))
                    ret.Source = s;
            }
                break;
            default:
                THROW_NEW_GUTIL_EXCEPTION(Exception);
            }
        }

        if(NULL == ret.Source)
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "No such piece can reach the square");

        ret.PieceMoved = *b.SquareAt(ret.Source->GetColumn(), ret.Source->GetRow()).GetPiece();
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

MoveData StandardGameLogic::GenerateMoveData(AbstractBoard const &b,
                                                    const ISquare &s,
                                                    const ISquare &d,
                                                    IPlayerResponse *uf) const
{
    MoveData ret;
    bool ok = true;

    // Check if there is a piece promotion
    if(s.GetPiece() && s.GetPiece()->GetType() == Piece::Pawn)
    {
        int promotion_rank = Piece::White == s.GetPiece()->GetAllegience() ?
                    7 : 0;
        if(promotion_rank == d.GetRow())
        {
            ret.PiecePromoted = uf->ChoosePromotedPiece();

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

void StandardGameLogic::PieceMoved(AbstractBoard &b, const MoveData &md) const
{
    int inc;
    Piece::AllegienceEnum next_turn;
    Piece const &p( md.PieceMoved );

    if(md.IsNull() || p.IsNull())
        return;

    b.GameState().SetHalfMoveClock(b.GameState().GetHalfMoveClock() + 1);
    if(p.GetAllegience() == Piece::Black)
    {
        b.GameState().SetFullMoveNumber(b.GameState().GetFullMoveNumber() + 1);
        next_turn = Piece::White;
        inc = -1;
    }
    else
    {
        b.GameState().SetWhoseTurn(Piece::Black);
        next_turn = Piece::Black;
        inc = 1;
    }
    b.GameState().SetWhoseTurn(next_turn);

    // Set the en-passant square
    if(p.GetType() == Piece::Pawn)
    {
        int row_diff_abs = Abs(md.Destination->GetRow() - md.Source->GetRow());
        if(2 == row_diff_abs)
            b.GameState().SetEnPassantSquare(&b.SquareAt(md.Source->GetColumn(), md.Source->GetRow() + inc));
        else
            b.GameState().SetEnPassantSquare(0);
    }
    else
        b.GameState().SetEnPassantSquare(0);
}


END_NAMESPACE_GKCHESS;
