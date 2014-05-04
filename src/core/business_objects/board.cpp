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
#include "piece.h"
#include "square.h"
#include "gkchess_pgn_parser.h"
#include "gutil_euclideanvector.h"
#include "gkchess_chess960.h"
USING_NAMESPACE_GUTIL;

#ifdef DEBUG
#include "gutil_console.h"
#endif // DEBUG


#define CASTLE_A_KING_DEST  2
#define CASTLE_H_KING_DEST  6


/** Maps col-row indices to a 1-dimensional index. */
static int __map_2d_indices_to_1d(GUINT32 col, GUINT32 row)
{
    return (col << 3) | row;
}

NAMESPACE_GKCHESS;


Board::piece_index_t::piece_index_t()
{}

void Board::piece_index_t::copy_from(const piece_index_t &o, const Board &b)
{
    int probe1 = sizeof(pieces);
    int probe2 = sizeof(pieces[0]);
    int probe3 = probe1 / probe2;

    int probe4 = sizeof(pieces[0][0]);
    int probe5 = probe2/probe4;
    for(GUINT32 i = 0; i < sizeof(pieces)/sizeof(pieces[0]); ++i)
    {
        for(GUINT32 j = 0; j < sizeof(pieces[0])/sizeof(pieces[0][0]); ++j)
        {
            pieces[i][j].Reserve(o.pieces[i][j].Capacity());
            G_FOREACH_CONST(Square const *s, o.pieces[i][j])
                pieces[i][j].PushBack(&b.SquareAt(s->GetColumn(), s->GetRow()));
        }
    }
}

Vector<Square const *> Board::piece_index_t::all_pieces(Piece::AllegienceEnum a) const
{
    Vector<Square const *> ret(16);
    if(Piece::AnyAllegience == a || Piece::White == a){
        for(GUINT32 i = 0; i < sizeof(pieces[0])/sizeof(pieces[0][0]); ++i)
            ret << pieces[Piece::White][i];
    }
    if(Piece::AnyAllegience == a || Piece::Black == a){
        for(GUINT32 i = 0; i < sizeof(pieces[0])/sizeof(pieces[0][0]); ++i)
            ret << pieces[Piece::Black][i];
    }
    return ret;
}

bool Board::piece_index_t::contains(const Piece &p) const
{
    return 0 < find_pieces(p).Length();
}

const Vector<Square const *> &Board::piece_index_t::find_pieces(const Piece &p) const
{
    GASSERT(p.GetType() != Piece::NoPiece && p.GetAllegience() != Piece::AnyAllegience);
    return pieces[(int)p.GetAllegience()][(int)p.GetType()];
}

Vector<Square const *> &Board::piece_index_t::find_pieces(const Piece &p)
{
    GASSERT(p.GetType() != Piece::NoPiece && p.GetAllegience() != Piece::AnyAllegience);
    return pieces[(int)p.GetAllegience()][(int)p.GetType()];
}

void Board::piece_index_t::update_piece(const Piece &p,
                                        Square const *orig_val,
                                        Square const *new_val)
{
    Vector<Square const *> &vec = find_pieces(p);
    int indx = 0 == orig_val ? -1 : vec.IndexOf(orig_val);
    if(-1 == indx){
        if(0 != new_val)
            vec.PushBack(new_val);
    }
    else{
        if(0 == new_val)
            vec.RemoveAt(indx);
        else
            vec[indx] = new_val;
    }
}

void Board::piece_index_t::clear()
{
    for(int i = 0; i < sizeof(pieces)/sizeof(pieces[0]); ++i){
        for(int j = 0; j < sizeof(pieces[0])/sizeof(pieces[0][0]); ++j){
            pieces[i][j].Empty();
        }
    }
}


Board::Board(int num_cols, int num_rows)
    :m_columnCount(num_cols),
      m_rowCount(num_rows),
      _p_WhoseTurn(Piece::AnyAllegience),
      _p_CastleWhiteA(-1),
      _p_CastleWhiteH(-1),
      _p_CastleBlackA(-1),
      _p_CastleBlackH(-1),
      _p_EnPassantSquare(0),
      _p_HalfMoveClock(0),
      _p_FullMoveNumber(1),
      _p_Result(Undecided)
{
    _init();
}

Board::Board(const Board &o)
    :m_columnCount(o.ColumnCount()),
      m_rowCount(o.RowCount()),
      _p_WhoseTurn(o.GetWhoseTurn()),
      _p_CastleWhiteA(o.GetCastleWhiteA()),
      _p_CastleWhiteH(o.GetCastleWhiteH()),
      _p_CastleBlackA(o.GetCastleBlackA()),
      _p_CastleBlackH(o.GetCastleBlackH()),
      _p_EnPassantSquare(o.GetEnPassantSquare()),
      _p_HalfMoveClock(o.GetHalfMoveClock()),
      _p_FullMoveNumber(o.GetFullMoveNumber()),
      _p_Result(o.GetResult())
{
    _copy_construct(o);
}

Board &Board::operator = (const Board &o)
{
    m_index.clear();
    _copy_board(o);
    return *this;
}

void Board::_init()
{
    // Initialize all the squares in the array
    m_squares = (Square *)malloc(ColumnCount() * RowCount() * sizeof(Square));
    for(int c = 0; c < ColumnCount(); ++c)
        for(int r = 0; r < RowCount(); ++r)
            new(&square_at(c, r)) Square(c, r);
}

void Board::_copy_construct(const Board &o)
{
    GASSERT(ColumnCount() == o.ColumnCount() && RowCount() == o.RowCount());
    _init();
    _copy_board(o);
}

void Board::_copy_board(const Board &o)
{
    Square *mine = m_squares;
    Square *my_end = m_squares + ColumnCount()*RowCount();
    Square const *theirs = o.m_squares;
    while(mine != my_end)
    {
        *mine = *theirs;
        ++mine, ++theirs;
    }
    m_index.copy_from(o.m_index, *this);
}

Board::~Board()
{
    // Destruct all squares
    Square *ptr = m_squares;
    Square *end = m_squares + ColumnCount() * RowCount();
    while(ptr != end){
        ptr->~Square();
        ++ptr;
    }

    free(m_squares);
}

int Board::ColumnCount() const
{
    return m_columnCount;
}

int Board::RowCount() const
{
    return m_rowCount;
}

Square const &Board::SquareAt(int col, int row) const
{
    return m_squares[__map_2d_indices_to_1d(col, row)];
}

Square &Board::square_at(int c, int r)
{
    return m_squares[__map_2d_indices_to_1d(c, r)];
}

Piece const &Board::GetPiece(int column, int row) const
{
    return SquareAt(column, row).GetPiece();
}

void Board::_update_gamestate(const MoveData &md)
{
    int inc;
    Piece::AllegienceEnum next_turn;
    Piece const &p( md.PieceMoved );

    if(md.IsNull() || p.IsNull())
        return;

    // Update the half-move clock
    if(Piece::Pawn != p.GetType() && md.PieceCaptured.IsNull())
        SetHalfMoveClock(GetHalfMoveClock() + 1);
    else
        SetHalfMoveClock(0);

    if(p.GetAllegience() == Piece::Black)
    {
        SetFullMoveNumber(GetFullMoveNumber() + 1);
        next_turn = Piece::White;
        inc = -1;
    }
    else
    {
        SetWhoseTurn(Piece::Black);
        next_turn = Piece::Black;
        inc = 1;
    }
    SetWhoseTurn(next_turn);

    // Set the en-passant square
    if(p.GetType() == Piece::Pawn)
    {
        int row_diff_abs = Abs(md.Destination->GetRow() - md.Source->GetRow());
        if(2 == row_diff_abs)
            SetEnPassantSquare(&SquareAt(md.Source->GetColumn(), md.Source->GetRow() + inc));
        else
            SetEnPassantSquare(0);
    }
    else
        SetEnPassantSquare(0);


    // Update the castle info

    //    Debugging:
//    int wa = GetCastleWhiteA();
//    int wh = GetCastleWhiteH();
//    int ba = GetCastleBlackA();
//    int bh = GetCastleBlackH();

    if(Piece::White == md.Whose())
    {
        if(Piece::King == p.GetType() || MoveData::NoCastle != md.CastleType)
        {
            // If the king moved, or he is castling then all castling options are set to null
            SetCastleWhiteA(-1);
            SetCastleWhiteH(-1);
        }
        else if(Piece::Rook == p.GetType())
        {
            // If a rook moved and castling was available on that side, now it's not
            if(-1 != GetCastleWhiteA()){
                if(GetCastleWhiteA() == md.Source->GetColumn() && 0 == md.Source->GetRow())
                    SetCastleWhiteA(-1);
            }
            if(-1 != GetCastleWhiteH()){
                if(GetCastleWhiteH() == md.Source->GetColumn() && 0 == md.Source->GetRow())
                    SetCastleWhiteH(-1);
            }
        }
    }
    else
    {
        if(Piece::King == p.GetType() || MoveData::NoCastle != md.CastleType)
        {
            // If the king moved, or he is castling then all castling options are set to null
            SetCastleBlackA(-1);
            SetCastleBlackH(-1);
        }
        else if(Piece::Rook == p.GetType())
        {
            // If a rook moved and castling was available on that side, now it's not
            if(-1 != GetCastleBlackA()){
                if(GetCastleBlackA() == md.Source->GetColumn() && 7 == md.Source->GetRow())
                    SetCastleBlackA(-1);
            }
            if(-1 != GetCastleBlackH()){
                if(GetCastleBlackH() == md.Source->GetColumn() && 7 == md.Source->GetRow())
                    SetCastleBlackH(-1);
            }
        }
    }
}

void Board::move_p(const MoveData &md)
{
    Square &src(square_at(md.Source->GetColumn(), md.Source->GetRow()));
    Square &dest(square_at(md.Destination->GetColumn(), md.Destination->GetRow()));

    Piece const &piece_orig = src.GetPiece();

    if(MoveData::NoCastle == md.CastleType)
    {
        SetPiece(Piece(), src);
        SetPiece(md.PieceMoved, dest);
    }
    else
    {
        int rook_col_src, rook_col_dest;
        int king_col_dest;
        int rank;
        if(Piece::White == piece_orig.GetAllegience())
        {
            rank = 0;
            if(MoveData::CastleASide == md.CastleType)
            {
                rook_col_src = GetCastleWhiteA();
                rook_col_dest = 3;
                king_col_dest = 2;
            }
            else
            {
                rook_col_src = GetCastleWhiteH();
                rook_col_dest = 5;
                king_col_dest = 6;
            }
        }
        else
        {
            rank = 7;
            if(MoveData::CastleASide == md.CastleType)
            {
                rook_col_src = GetCastleBlackA();
                rook_col_dest = 3;
                king_col_dest = 2;
            }
            else
            {
                rook_col_src = GetCastleBlackH();
                rook_col_dest = 5;
                king_col_dest = 6;
            }
        }

        // Move the rook
        SetPiece(Piece(), square_at(rook_col_src, rank));
        SetPiece(Piece(Piece::Rook, piece_orig.GetAllegience()), square_at(rook_col_dest, rank));

        // Move the King
        SetPiece(piece_orig, square_at(king_col_dest, rank));
        SetPiece(Piece(), src);
    }

    _update_gamestate(md);
    _update_threat_counts();
}

Board::MoveValidationEnum Board::Move(const MoveData &md)
{
    MoveValidationEnum ret = ValidMove;

    if(md.IsNull())
        ret = InvalidInputError;
    else
    {
        ret = ValidateMove(*md.Source, *md.Destination);

        if(ValidMove == ret)
        {
            // Move the piece and update the gamestate
            move_p(md);
        }
    }
    return ret;
}

Board::MoveValidationEnum Board::Move2(const Square &src, const Square &dest, IPlayerResponse *pr)
{
    return Move(GenerateMoveData(src, dest, pr));
}

void Board::Resign(Piece::AllegienceEnum)
{
    THROW_NEW_GUTIL_EXCEPTION(NotImplementedException);
}

void Board::Clear()
{
    for(int i = 0; i < ColumnCount(); ++i)
        for(int j = 0; j < RowCount(); ++j)
            SetPiece(Piece(), SquareAt(i, j));
}

void Board::FromFEN(const String &s)
{
    if(s.IsNull())
        return;

    int king_col_white = -1;
    int king_col_black = -1;
    String cpy( s.Trimmed() );
    StringList sl( cpy.Split(' ', false) );
    String backRank_white;
    String backRank_black;
    if(6 != sl.Length())
        THROW_NEW_GUTIL_EXCEPTION2(Exception, "FEN requires 6 fields separated by spaces");

    // First parse the position text:
    {
        StringList sl2( sl[0].Split('/', false) );
        if(RowCount() != sl2.Length())
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "FEN position text requires 8 fields separated by /");

        backRank_white = sl2[7];
        backRank_black = sl2[0];

        // For each section of position text...
        for(int i = 0; i < sl2.Length(); ++i)
        {
            int col = 0;
            int rank = 7 - i;
            typename String::const_iterator iter(sl2[i].begin());
            typename String::const_iterator next(iter + 1);

            // For each character in the section...
            for(;
                iter != sl2[i].end() && col < ColumnCount();
                ++iter)
            {
                int num(-1);
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
                    Piece piece = Piece::FromFEN(c);
                    SetPiece(piece, SquareAt(col, rank));

                    if(Piece::King == piece.GetType())
                    {
                        if(Piece::White == piece.GetAllegience())
                            king_col_white = col;
                        else
                            king_col_black = col;
                    }

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
        SetWhoseTurn(a);
    }


    // Then parse the castle info:
    {
        SetCastleWhiteA(-1);
        SetCastleWhiteH(-1);
        SetCastleBlackA(-1);
        SetCastleBlackH(-1);

        G_FOREACH_CONST(char c, sl[2])
        {
            Piece::AllegienceEnum a = String::IsUpper(c) ? Piece::White : Piece::Black;
            char const rook_char = Piece::White == a ? 'R' : 'r';
            char const king_char = Piece::White == a ? 'K' : 'k';
            char tmps[2] = {c, '\0'};
            String::ToUpper(&c, tmps);
            switch(c)
            {
            case '-':
                // If a dash is given, then this field is empty (nobody can castle)
                break;
            case 'K':
            {
                // Find the outermost rook on the H-side
                const String *s = Piece::White == a ? &backRank_white : &backRank_black;
                for(int i = s->Length() - 1; i >= 0; --i){
                    char cur = (*s)[i];
                    if(cur == rook_char){
                        c = 'A' + i;
                        break;
                    }
                    else if(cur == king_char)
                        break;
                }
            }
                break;
            case 'Q':
            {
                // Find the outermost rook on the A-side
                const String *s = Piece::White == a ? &backRank_white : &backRank_black;
                for(int i = 0; i < s->Length(); ++i){
                    char cur = (*s)[i];
                    if(cur == rook_char){
                        c = 'A' + i;
                        break;
                    }
                    else if(cur == king_char)
                        break;
                }
            }
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
                    if(king_col_white > file)
                        SetCastleWhiteA(file);
                    else
                        SetCastleWhiteH(file);
                    break;
                case Piece::Black:
                    if(king_col_black > file)
                        SetCastleBlackA(file);
                    else
                        SetCastleBlackH(file);
                    break;
                default: break;
                }
            }
            else if('-' != c)
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

            SetEnPassantSquare(&SquareAt(f - 'a', rnk - '1'));
        }
        else
            SetEnPassantSquare(0);
    }


    // Parse the half-move clock:
    {
        bool ok(false);
        SetHalfMoveClock(sl[4].ToInt(&ok));
        if(!ok)
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid half-move clock");
    }


    // Parse the full-move number:
    {
        bool ok(false);
        SetFullMoveNumber(sl[5].ToInt(&ok));
        if(!ok)
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid full-move number");
    }

    _update_threat_counts();
}

static GUtil::String __get_castle_string_for_allegience(const Board &b,
                                                        Piece::AllegienceEnum a,
                                                        int castle_file_a_side,
                                                        int castle_file_h_side)
{
    GUtil::String ret;
    char a_side_char = Piece::White == a ? 'Q' : 'q';
    char h_side_char = Piece::White == a ? 'K' : 'k';
    char base_char = Piece::White == a ? 'A' : 'a';
    int rank = Piece::White == a ? 0 : 7;

    if(castle_file_a_side != -1 || castle_file_h_side != -1)
    {
        if(castle_file_a_side != -1 && castle_file_h_side != -1)
        {
            ret.Reserve(2);
            ret.Append(h_side_char);
            ret.Append(a_side_char);
        }
        else if(castle_file_h_side != -1)
        {
            bool no_outer_rook = true;
            for(int i = castle_file_h_side + 1; i < b.ColumnCount(); ++i)
            {
                Square const &sqr = b.SquareAt(i, rank);
                if(!sqr.GetPiece().IsNull() &&
                        sqr.GetPiece().GetType() == Piece::Rook &&
                        sqr.GetPiece().GetAllegience() == a){
                    no_outer_rook = false;
                    break;
                }
            }
            if(no_outer_rook)
                ret = h_side_char;
            else
                ret = (base_char + castle_file_h_side);
        }
        else
        {
            bool no_outer_rook = true;
            for(int i = castle_file_a_side - 1; i >= 0; --i)
            {
                Square const &sqr = b.SquareAt(i, rank);
                if(!sqr.GetPiece().IsNull() &&
                        sqr.GetPiece().GetType() == Piece::Rook &&
                        sqr.GetPiece().GetAllegience() == a){
                    no_outer_rook = false;
                    break;
                }
            }
            if(no_outer_rook)
                ret = a_side_char;
            else
                ret = (base_char + castle_file_a_side);
        }
    }
    return ret;
}

String Board::ToFEN() const
{
    String ret;
    for(int r = RowCount() - 1; r >= 0; --r)
    {
        int empty_cnt = 0;
        for(int c = 0; c < ColumnCount(); ++c)
        {
            Square const &cur = SquareAt(c, r);
            if(!cur.GetPiece().IsNull())
            {
                if(0 < empty_cnt)
                {
                    ret.Append(String::FromInt(empty_cnt));
                    empty_cnt = 0;
                }

                ret.Append(cur.GetPiece().ToFEN());
            }
            else
            {
                ++empty_cnt;
            }
        }

        if(0 < empty_cnt)
        {
            ret.Append(String::FromInt(empty_cnt));
        }

        if(r != 0)
            ret.Append('/');
    }

    // Generate the castle string
    String castle_string("-");
    if(GetCastleBlackA() != -1 || GetCastleBlackH() != -1 ||
            GetCastleWhiteA() != -1 || GetCastleWhiteH() != -1)
    {
        castle_string = "";

        castle_string.Append(__get_castle_string_for_allegience(
                                 *this,
                                 Piece::White,
                                 GetCastleWhiteA(),
                                 GetCastleWhiteH()));

        castle_string.Append(__get_castle_string_for_allegience(
                                 *this,
                                 Piece::Black,
                                 GetCastleBlackA(),
                                 GetCastleBlackH()));
    }

    return String::Format("%s %s %s %s %d %d",
                          ret.ConstData(),
                          Piece::White == GetWhoseTurn() ? "w" : "b",
                          castle_string.ConstData(),
                          GetEnPassantSquare() ? GetEnPassantSquare()->ToString().ConstData() : "-",
                          GetHalfMoveClock(),
                          GetFullMoveNumber()
                          );
}







/**  Below is the chess logic implementation  */






/** Converts an allegience into a positive or negative 1, so we know what direction is forward. */
static int __allegience_to_rank_increment(Piece::AllegienceEnum a)
{
    return -2 * (int)a + 1;
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
static bool __is_path_blocked(Board const &b, Square const &s, Square const &d, Piece::AllegienceEnum a)
{
    bool ret = false;
    Square const *cur = &s;
    int cmp_res_col = __cmp_with_zero(d.GetColumn() - cur->GetColumn());
    int cmp_res_row = __cmp_with_zero(d.GetRow() - cur->GetRow());

    while(1)
    {
        cur = &b.SquareAt(cur->GetColumn() + cmp_res_col, cur->GetRow() + cmp_res_row);

        Piece const &p = cur->GetPiece();
        if(cur == &d)
        {
            // The destination square is only blocked if there is a piece that belongs
            //  to the same allegience as the piece being moved.
            if(!p.IsNull())
                ret = a == p.GetAllegience();
            break;
        }
        else if(!p.IsNull())
        {
            ret = true;
            break;
        }
    };
    return ret;
}

static bool __is_move_valid_for_bishop(Board const &b,
                                       Square const *s,
                                       Square const *d,
                                       Piece::AllegienceEnum a)
{
    int col_diff_abs = Abs(s->GetColumn() - d->GetColumn());
    int row_diff_abs = Abs(s->GetRow() - d->GetRow());
    return col_diff_abs == row_diff_abs &&
            !__is_path_blocked(b, *s, *d, a);
}

static bool __is_move_valid_for_knight(Board const &b,
                                       Square const *s,
                                       Square const *d,
                                       Piece::AllegienceEnum a)
{
    GUTIL_UNUSED(b);
    GUTIL_UNUSED(a);
    int col_diff_abs = Abs(s->GetColumn() - d->GetColumn());
    int row_diff_abs = Abs(s->GetRow() - d->GetRow());
    return (1 == col_diff_abs && 2 == row_diff_abs) ||
            (2 == col_diff_abs && 1 == row_diff_abs);
}

static bool __is_move_valid_for_rook(Board const &b,
                                     Square const *s,
                                     Square const *d,
                                     Piece::AllegienceEnum a)
{
    int col_diff = s->GetColumn() - d->GetColumn();
    int row_diff = s->GetRow() - d->GetRow();
    return ((0 == col_diff && 0 != row_diff) ||
            (0 == row_diff && 0 != col_diff))
            &&
            !__is_path_blocked(b, *s, *d, a);
}

static bool __is_move_valid_for_queen(Board const &b,
                                      Square const *s,
                                      Square const *d,
                                      Piece::AllegienceEnum a)
{
    int col_diff_abs = Abs(s->GetColumn() - d->GetColumn());
    int row_diff_abs = Abs(s->GetRow() - d->GetRow());
    return (col_diff_abs == row_diff_abs || ((0 == col_diff_abs && 0 != row_diff_abs) ||
                                             (0 == row_diff_abs && 0 != col_diff_abs)))
            &&
            !__is_path_blocked(b, *s, *d, a);
}

static bool __is_move_valid_for_king(Board const &b,
                                     Square const *s,
                                     Square const *d,
                                     Piece::AllegienceEnum a)
{
    GUTIL_UNUSED(b);
    GUTIL_UNUSED(a);
    int col_diff_abs = Abs(s->GetColumn() - d->GetColumn());
    int row_diff_abs = Abs(s->GetRow() - d->GetRow());
    return 1 >= col_diff_abs && 1 >= row_diff_abs;
}


void Board::SetupNewGame(Board::SetupTypeEnum ste)
{
    switch(ste)
    {
    case SetupStandardChess:
        FromFEN(FEN_STANDARD_CHESS_STARTING_POSITION);
        break;
    case SetupChess960:
        FromFEN(Chess960::GetRandomStartingPosition());
        break;
    default:
        THROW_NEW_GUTIL_EXCEPTION(NotImplementedException);
        break;
    }

    _update_threat_counts();
}

Board::MoveValidationEnum Board::ValidateMove(const Square &s, const Square &d) const
{
    Piece const &p(s.GetPiece());
    Piece const &dp(d.GetPiece());
    if(p.IsNull())
        return InvalidEmptySquare;


    // Validate the low-level technical aspects of the move, ignoring threats to the king

    // Only the player whose turn it is can move pieces
    if(p.GetAllegience() != GetWhoseTurn())
        return InvalidTechnical;

    // A piece cannot stay in the same place if it is moving
    if(s == d)
        return InvalidTechnical;

    bool technically_ok = false;
    bool castling = false;
    int col_diff = d.GetColumn() - s.GetColumn();
    int row_diff = d.GetRow() - s.GetRow();
    int col_diff_abs = Abs(col_diff);
    int row_diff_abs = Abs(row_diff);
    switch(p.GetType())
    {
    case Piece::Pawn:
    {
        int sign;
        int startRank;
        if(Piece::White == p.GetAllegience()){
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
            technically_ok = dp.IsNull() &&
                    (row_diff == 1*sign || (startRank == s.GetRow() && row_diff == 2*sign));
        }

        // capture move
        else if(col_diff_abs == 1)
        {
            technically_ok =

                    // must be one diagonal forward
                    row_diff == 1*sign && 1 == col_diff_abs &&

                    // there must be a captured piece of the other color on the dest square
                    ((!dp.IsNull() && dp.GetAllegience() != p.GetAllegience()) ||

                     //  or the square being captured to must be an en passant square
                     (GetEnPassantSquare() && d == *GetEnPassantSquare()));
        }
    }
        break;
    case Piece::Bishop:
        if(col_diff_abs == row_diff_abs)
            technically_ok = !__is_path_blocked(*this, s, d, p.GetAllegience());
        break;
    case Piece::Knight:
        technically_ok = __is_move_valid_for_knight(*this, &s, &d, p.GetAllegience()) &&
                (dp.IsNull() || dp.GetAllegience() != p.GetAllegience());
        break;
    case Piece::Rook:
        if(0 == col_diff_abs || 0 == row_diff_abs)
            technically_ok = !__is_path_blocked(*this, s, d, p.GetAllegience());
        break;
    case Piece::Queen:
        if(0 == col_diff_abs || 0 == row_diff_abs || col_diff_abs == row_diff_abs)
            technically_ok = !__is_path_blocked(*this, s, d, p.GetAllegience());
        break;
    case Piece::King:
        if(1 >= col_diff_abs && 1 >= row_diff_abs)
        {
            // Normally the king can only move one square in any direction
            technically_ok = !__is_path_blocked(*this, s, d, p.GetAllegience());
        }
        else if(0 == row_diff)
        {
            // The king can move more than one square if he's castling
            if(Piece::White == p.GetAllegience()){
                if(s.GetRow() == 0){
                    if((GetCastleWhiteA() != -1 && d.GetColumn() == GetCastleWhiteA()) ||
                            (GetCastleWhiteH() != -1 && d.GetColumn() == GetCastleWhiteH()))
                    {
                        Square const *king_src = &s;
                        Square const *king_dest;
                        Square const *rook_dest;
                        Square const *rook_src;
                        if(d.GetColumn() == GetCastleWhiteA()){
                            king_dest = &SquareAt(2, 0);
                            rook_src = &SquareAt(GetCastleWhiteA(), 0);
                            rook_dest = &SquareAt(3, 0);
                        }
                        else if(d.GetColumn() == GetCastleWhiteH()){
                            king_dest = &SquareAt(6, 0);
                            rook_src = &SquareAt(GetCastleWhiteH(), 0);
                            rook_dest = &SquareAt(5, 0);
                        }

                        bool threats = false;
                        for(int i = king_src->GetColumn(); !threats && 0 <= i && i <= king_dest->GetColumn();
                            king_dest->GetColumn() - king_src->GetColumn() > 0 ? ++i : --i)
                        {
                            threats = SquareAt(i, 0).GetThreatCount(Piece::Black) > 0;
                        }
                        technically_ok = !threats &&
                                !__is_path_blocked(*this, *king_src, *king_dest, Piece::White) &&
                                !__is_path_blocked(*this, *rook_src, *rook_dest, Piece::White);
                        castling = true;
                    }
                }
            }
            else{
                if(s.GetRow() == 7){
                    if((GetCastleBlackA() != -1 && d.GetColumn() == GetCastleBlackA()) ||
                            (GetCastleBlackH() != -1 && d.GetColumn() == GetCastleBlackH()))
                    {
                        Square const *king_src = &s;
                        Square const *king_dest;
                        Square const *rook_dest;
                        Square const *rook_src;
                        if(d.GetColumn() == GetCastleBlackA()){
                            king_dest = &SquareAt(2, 7);
                            rook_src = &SquareAt(GetCastleBlackA(), 7);
                            rook_dest = &SquareAt(3, 7);
                        }
                        else if(d.GetColumn() == GetCastleBlackH()){
                            king_dest = &SquareAt(6, 7);
                            rook_src = &SquareAt(GetCastleBlackH(), 7);
                            rook_dest = &SquareAt(5, 7);
                        }

                        bool threats = false;
                        for(int i = king_src->GetColumn(); !threats && 0 <= i && i <= king_dest->GetColumn();
                            king_dest->GetColumn() - king_src->GetColumn() > 0 ? ++i : --i)
                        {
                            threats = SquareAt(i, 7).GetThreatCount(Piece::White) > 0;
                        }
                        technically_ok = !threats &&
                                !__is_path_blocked(*this, *king_src, *king_dest, Piece::Black) &&
                                !__is_path_blocked(*this, *rook_src, *rook_dest, Piece::Black);
                        castling = true;
                    }
                }
            }
        }
        break;
    default:
        // We should not have an unkown piece type
        THROW_NEW_GUTIL_EXCEPTION(Exception);
    }

    if(!technically_ok)
        return InvalidTechnical;

    if(!castling && !d.GetPiece().IsNull() && d.GetPiece().GetAllegience() == p.GetAllegience())
        return InvalidTechnical;

    // Now check if the king is safe, otherwise it's an invalid move
    {
        // Copy this board to simulate the move, and see if we're in check.
        Board cpy(*this);
        cpy.move_p(cpy.GenerateMoveData(cpy.SquareAt(s.GetColumn(), s.GetRow()),
                                        cpy.SquareAt(d.GetColumn(), d.GetRow())));
        if(cpy.IsInCheck(p.GetAllegience()))
            return InvalidCheck;
    }

    return ValidMove;
}

Vector<Square const *> Board::GetValidMovesForSquare(const Square &) const
{
    Vector<Square const *> ret;
    return ret;
}

static Square const *__get_source_square(const Board &b,
                                         char piece_moved,
                                         Square const *dest,
                                         Piece::AllegienceEnum a,
                                         GINT8 given_file_info,
                                         GINT8 given_rank_info)
{
    Square const *ret = 0;
    Vector<Square const *> possible_sources( b.FindPieces(Piece(Piece::GetTypeFromPGN(piece_moved), a)) );
    for(GINT32 i = 0; i < possible_sources.Length(); ++i)
    {
        Square const *s = possible_sources[i];

        bool valid = false;
        switch(piece_moved)
        {
        case 'B':
            valid = __is_move_valid_for_bishop(b, s, dest, a);
            break;
        case 'N':
            valid = __is_move_valid_for_knight(b, s, dest, a);
            break;
        case 'Q':
            valid = __is_move_valid_for_queen(b, s, dest, a);
            break;
        case 'R':
            valid = __is_move_valid_for_rook(b, s, dest, a);
            break;
        default:
            GASSERT(false);
        }

        if(valid)
        {
            if(-1 != given_file_info && -1 != given_rank_info)
            {
                if(s->GetColumn() == given_file_info && s->GetRow() == given_rank_info){
                    ret = s;
                    break;
                }
            }
            else if(-1 != given_file_info)
            {
                if(s->GetColumn() == given_file_info){
                    ret = s;
                    break;
                }
            }
            else if(-1 != given_rank_info)
            {
                if(s->GetRow() == given_rank_info){
                    ret = s;
                    break;
                }
            }
            else
            {
                if(NULL != ret)
                    THROW_NEW_GUTIL_EXCEPTION2(Exception, "Multiple pieces found which could move there");

                ret = s;
            }
        }
    }
    return ret;
}

MoveData Board::GenerateMoveData(const PGN_MoveData &m) const
{
    MoveData ret;
    Piece::AllegienceEnum turn = GetWhoseTurn();

    ret.Position = ToFEN();
    ret.PlyNumber = m.MoveNumber * 2 - 1;
    if(Piece::Black == turn)
        ++ret.PlyNumber;

    if(m.Flags.TestFlag(PGN_MoveData::CastleHSide))
    {
        ret.CastleType = MoveData::CastleHSide;

        int rook_loc = -1;
        switch(turn)
        {
        case Piece::White:
            rook_loc = GetCastleWhiteH();
            break;
        case Piece::Black:
            rook_loc = GetCastleBlackH();
            break;
        default: break;
        }

        if(-1 == rook_loc)
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "Unable to castle on that side");

        Vector<Square const *> v = FindPieces(Piece(Piece::King, turn));
        if(v.Length() != 1)
            THROW_NEW_GUTIL_EXCEPTION(Exception);
        ret.Source = v[0];
        ret.Destination = &SquareAt(rook_loc, v[0]->GetRow());
        ret.PieceMoved = Piece(Piece::King, turn);
    }
    else if(m.Flags.TestFlag(PGN_MoveData::CastleQueenSide))
    {
        ret.CastleType = MoveData::CastleASide;

        int rook_loc = -1;
        switch(turn)
        {
        case Piece::White:
            rook_loc = GetCastleWhiteA();
            break;
        case Piece::Black:
            rook_loc = GetCastleBlackA();
            break;
        default: break;
        }

        if(-1 == rook_loc)
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "Unable to castle on that side");

        Vector<Square const *> v = FindPieces(Piece(Piece::King, turn));
        if(v.Length() != 1)
            THROW_NEW_GUTIL_EXCEPTION(Exception);
        ret.Source = v[0];
        ret.Destination = &SquareAt(rook_loc, v[0]->GetRow());
        ret.PieceMoved = Piece(Piece::King, turn);
    }
    else
    {
        // Validate the inputs
        if('a' > m.DestFile || m.DestFile > 'h')
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "The destination file is invalid");
        if(0 >= m.DestRank || m.DestRank > 8)
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "The destination rank is invalid");
        if(0 != m.SourceFile && ('a' > m.SourceFile || m.SourceFile > 'h'))
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "The source file is invalid");
        if(0 != m.SourceRank && (0 > m.SourceRank || m.SourceRank >= 8))
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "The source rank is invalid");


        // Get the destination square (this is always given)
        Square const *dest = &SquareAt(m.DestFile - 'a', m.DestRank - 1);
        ret.Destination = dest;

        if(m.Flags.TestFlag(PGN_MoveData::Capture))
        {
            if(!dest->GetPiece().IsNull() && dest->GetPiece().GetAllegience() != turn)
                ret.PieceCaptured = dest->GetPiece();
            else
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid piece to capture at the destination square");
        }


        // Find the source square (this is only seldom given; normally we have to search)
        int tmp_source_column = 0 == m.SourceFile ? -1 : m.SourceFile - 'a';
        if(m.SourceFile != 0 && m.SourceRank != 0)
        {
            Square const *s = &SquareAt(tmp_source_column, m.SourceRank - 1);

            // Do a sanity check on the square they specified
            if(s->GetPiece().IsNull())
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "No piece on the square");
            if(s->GetPiece().GetAllegience() != turn)
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "It is not your turn");
            if(s->GetPiece().GetType() != m.PieceMoved)
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "The piece on that square is different than the piece you said to move");

            ret.Source = s;
        }
        else
        {
            int tmp_source_rank = 0 == m.SourceRank ? -1 : m.SourceRank - 1;

            // If the source was not specified explicitly then we need to search
            //  with whatever information we've got

            // The possible source squares depend on the piece being moved
            switch(m.PieceMoved)
            {
            case 0:
            case 'P':
                // The pawn takes the most code, because it moves differently depending on if it's
                //  capturing or not.  It can also move two squares instead of one on the first move.
                if(m.Flags.TestFlag(PGN_MoveData::Capture))
                {
                    // If the pawn is capturing then it can only be from either of the two files
                    //  next to the destination
                    if(-1 != tmp_source_column &&
                            1 != Abs(tmp_source_column - ret.Destination->GetColumn()))
                        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid file for pawn capture");

                    Vector<Square const *> possible_sources( FindPieces(Piece(Piece::GetTypeFromPGN(m.PieceMoved), turn)) );
                    if(0 == possible_sources.Length())
                        THROW_NEW_GUTIL_EXCEPTION2(Exception, "There are no pieces of that type to move");

                    for(GINT32 i = 0; i < possible_sources.Length(); ++i)
                    {
                        Square const *s = possible_sources[i];

                        if((1 == Abs(s->GetColumn() - ret.Destination->GetColumn())) &&
                                s->GetRow() == ret.Destination->GetRow() - __allegience_to_rank_increment(turn))
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
                    int r = ret.Destination->GetRow() - __allegience_to_rank_increment(turn);
                    if(0 > r || r > 7)
                        THROW_NEW_GUTIL_EXCEPTION2(Exception, "No such piece can reach the square");

                    Square const *s = &SquareAt(ret.Destination->GetColumn(), r);
                    if(s->GetPiece().IsNull())
                    {
                        // The pawn can move two squares on the first move
                        r = s->GetRow() - __allegience_to_rank_increment(turn);
                        if((turn == Piece::White && 1 != r) || (turn == Piece::Black && 6 != r) ||
                                (s = &SquareAt(ret.Destination->GetColumn(), r))->GetPiece().IsNull())
                            THROW_NEW_GUTIL_EXCEPTION2(Exception, "No such piece can reach the square");
                    }

                    if(s->GetPiece().GetAllegience() != turn ||
                            s->GetPiece().GetType() != Piece::Pawn)
                        THROW_NEW_GUTIL_EXCEPTION2(Exception, "No such piece can reach the square");

                    if(!ret.Destination->GetPiece().IsNull())
                        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Destination square occupied");

                    ret.Source = s;
                }

                break;
            case 'R':
            case 'N':
            case 'B':
            case 'Q':
                ret.Source = __get_source_square(*this, m.PieceMoved, ret.Destination, turn, tmp_source_column, tmp_source_rank);
                break;
            case 'K':
            {
                Vector<Square const *> possible_sources( FindPieces(Piece(Piece::GetTypeFromPGN(m.PieceMoved), turn)) );

                // There can only be one king
                GASSERT(1 == possible_sources.Length());

                Square const *s = possible_sources[0];
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

        ret.PieceMoved = SquareAt(ret.Source->GetColumn(), ret.Source->GetRow()).GetPiece();
        GASSERT(!ret.PieceMoved.IsNull());

        // Add a promoted piece, if necessary
        if(m.PiecePromoted)
        {
            if(Piece::Pawn != m.PieceMoved)
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "Only pawns can be promoted");

            ret.PiecePromoted = Piece(Piece::GetTypeFromPGN(m.PiecePromoted), turn);
        }
    }

    return ret;
}

MoveData Board::GenerateMoveData(const Square &s,
                                 const Square &d,
                                 IPlayerResponse *uf) const
{
    MoveData ret;
    bool ok = true;

    if(!s.GetPiece().IsNull())
    {
        // Check if there is a piece promotion
        if(s.GetPiece().GetType() == Piece::Pawn)
        {
            Piece::AllegienceEnum a = s.GetPiece().GetAllegience();
            int promotion_rank = Piece::White == a ? 7 : 0;
            if(promotion_rank == d.GetRow())
            {
                ret.PiecePromoted = NULL == uf ? Piece(Piece::Queen, a) : uf->ChoosePromotedPiece(a);

                // If the user cancelled then we return a null move data
                if(ret.PiecePromoted.IsNull())
                    ok = false;
            }
        }
        else if(s.GetPiece().GetType() == Piece::King && d.GetPiece().GetType() == Piece::Rook &&
                s.GetPiece().GetAllegience() == d.GetPiece().GetAllegience())
        {
            ret.CastleType = d.GetColumn() - s.GetColumn() > 0 ? MoveData::CastleHSide : MoveData::CastleASide;
        }
    }

    if(ok)
    {
        ret.PlyNumber = GetFullMoveNumber() * 2 - 1;
        if(Piece::Black == GetWhoseTurn())
            ret.PlyNumber++;

        ret.Source = &s;
        ret.Destination = &d;

        if(!s.GetPiece().IsNull())
            ret.PieceMoved = s.GetPiece();

        if(!d.GetPiece().IsNull() && ret.CastleType == MoveData::NoCastle)
            ret.PieceCaptured = d.GetPiece();
    }

    return ret;
}

// If there is a piece at the destination, dest_piece will be updated with the information
//  Returns true if the square was appended
static bool __append_if_within_bounds(Vector<Square const *> &res, const Board &b, int col, int row, Piece *dest_piece)
{
    bool ret = false;
    if(0 <= col && col < b.ColumnCount() && 0 <= row && row < b.RowCount()){
        Square const *sqr = &b.SquareAt(col, row);
        res.PushBack(sqr);
        ret = true;

        if(!sqr->GetPiece().IsNull())
            *dest_piece = sqr->GetPiece();
    }
    return ret;
}


static void __get_threatened_squares_helper(Vector<Square const *> &ret, const Board &b, const Square &s, int col_inc, int row_inc, int max_distance = -1)
{
    int distance = 1;
    int measuring_cnt = 4;
    bool measuring[4] = {true, true, true, true};
    while((-1 == max_distance || distance <= max_distance) &&
           0 < measuring_cnt)
    {
        int col = s.GetColumn() + (distance * col_inc);
        int row = s.GetRow() + (distance * row_inc);
        int i = 0;

        // The increments get rotated 4 times for each distance
        G_FOREVER
        {
            if(measuring[i])
            {
                Piece dp;
                if(!__append_if_within_bounds(ret, b, col, row, &dp) ||
                   !dp.IsNull())
                {
                    measuring[i] = false;
                    --measuring_cnt;
                }
            }

            if(i < 3)
            {
                // Rotate the increments 90 degrees each time:
                EuclideanVector2<GINT8> vec = EuclideanVector2<GINT8>(s.GetColumn(), s.GetRow(), col, row).Orthogonal();
                col = vec.GetX() + s.GetColumn();
                row = vec.GetY() + s.GetRow();
                ++i;
            }
            else{
                // To avoid checking the exit condition twice, just break here
                break;
            }
        }

        ++distance;
    }
}

// Returns the squares that a piece with given type on the given square can capture
static Vector<Square const *> __get_threatened_squares(const Board &b, Piece const &p, const Square &s)
{
    Vector<Square const *> ret;
    int max_distance = -1;
    switch(p.GetType())
    {
    case Piece::Pawn:
    {
        // A pawn threatens the two squares diagonally in front of it
        int rank = s.GetRow() + __allegience_to_rank_increment(p.GetAllegience());
        if(0 < rank && rank < 7)
        {
            int col = s.GetColumn() - 1;
            if(0 <= col)
                ret.PushBack(&b.SquareAt(col, rank));

            col = s.GetColumn() + 1;
            if(7 >= col)
                ret.PushBack(&b.SquareAt(col, rank));
        }
    }
        break;
    case Piece::Bishop:
        __get_threatened_squares_helper(ret, b, s, 1, 1);
        break;
    case Piece::Rook:
        __get_threatened_squares_helper(ret, b, s, 0, 1);
        break;
    case Piece::Knight:
        __get_threatened_squares_helper(ret, b, s, 1, 2, 1);
        __get_threatened_squares_helper(ret, b, s, 2, 1, 1);
        break;
    case Piece::King:
        max_distance = 1;
        // King falls through to Queen, because they move the same way, just a different distance
    case Piece::Queen:
        __get_threatened_squares_helper(ret, b, s, 0, 1, max_distance);
        __get_threatened_squares_helper(ret, b, s, 1, 1, max_distance);
        break;
    default:break;
    }
    return ret;
}

void Board::_update_threat_counts()
{
    // Set all threats to 0 and then increment them as we find threats
    _set_all_threat_counts(0);

    Vector<Square const *> all_pieces(FindPieces(Piece()));
    G_FOREACH(Square const *s_c, all_pieces)
    {
        GASSERT(!s_c->GetPiece().IsNull());

        Piece const &p = s_c->GetPiece();

        Vector<Square const *> squares(__get_threatened_squares(*this, p, *s_c));
        G_FOREACH_CONST(Square const *s_c, squares){
            Square &s = square_at(s_c->GetColumn(), s_c->GetRow());
            s.SetThreatCount(p.GetAllegience(), s.GetThreatCount(p.GetAllegience()) + 1);
        }
    }
}

void Board::_set_all_threat_counts(int c)
{
    for(int i = 0; i < ColumnCount(); ++i)
    {
        for(int j = 0; j < RowCount(); ++j)
        {
            square_at(i, j).SetThreatCount(Piece::White, c);
            square_at(i, j).SetThreatCount(Piece::Black, c);
        }
    }
}

void Board::SetPiece(Piece const &p, const Square &s)
{
    Piece orig = s.GetPiece();
    square_at(s.GetColumn(), s.GetRow()).SetPiece(p);

    // Remove the old piece from the index
    if(!orig.IsNull())
        m_index.update_piece(orig, &s, 0);
    // Add the new piece to the index
    if(!p.IsNull())
        m_index.update_piece(p, 0, &s);
}

Vector<Square const *> Board::FindPieces(Piece const &pc) const
{
    Vector<Square const *> ret;
    if(Piece::NoPiece == pc.GetType()){
        ret << m_index.all_pieces(pc.GetAllegience());
    }
    else{
        ret = m_index.find_pieces(pc);
    }
    return ret;
}

bool Board::IsInCheck(Piece::AllegienceEnum a) const
{
    bool ret = false;
    Piece king(Piece::King, a);
    Vector<Square const *> king_loc = FindPieces(king);
    if(1 == king_loc.Length())
    {
        if(0 < king_loc[0]->GetThreatCount(king.GetOppositeAllegience()))
            ret = true;
    }
    return ret;
}




#ifdef DEBUG

void Board::ShowIndex() const
{
    Console::WriteLine("White Pieces:");
    Vector<Square const *> v = m_index.all_pieces(Piece::White);
    G_FOREACH_CONST(const Square *s, v){
        Console::WriteLine(String::Format("%s: %s",
                                          s->GetPiece().ToString(true).ConstData(),
                                          s->ToString().ConstData()));
    }


    Console::WriteLine("\nBlack Pieces:");
    v = m_index.all_pieces(Piece::Black);
    G_FOREACH_CONST(const Square *s, v){
        Console::WriteLine(String::Format("%s: %s",
                                          s->GetPiece().ToString(true).ConstData(),
                                          s->ToString().ConstData()));
    }
    Console::FlushOutput();
}

#endif









ObservableBoard::ObservableBoard(int num_cols)
    :QObject(0),
      Board(num_cols)
{}

ObservableBoard::ObservableBoard(const Board &o)
    :QObject(0),
      Board(o)
{}

ObservableBoard &ObservableBoard::operator = (const Board &o)
{
    Board::operator = (o);
    emit NotifyBoardReset();
    return *this;
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
