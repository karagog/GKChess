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
#include "gkchess_chess960.h"
#include <gutil/euclideanvector.h>
#include <gutil/range.h>
#include <QStringList>
USING_NAMESPACE_GUTIL;

#ifdef DEBUG
#include <gutil/console.h>
#endif // DEBUG


#define CASTLE_A_KING_DEST  2
#define CASTLE_H_KING_DEST  6


/** Maps col-row indices to a 1-dimensional index. */
static int __map_2d_indices_to_1d(GUINT32 col, GUINT32 row)
{
    return (col << 3) | row;
}

static int __back_rank(GKChess::Piece::AllegienceEnum a)
{
    return a == GKChess::Piece::White ? 0 : 7;
}

NAMESPACE_GKCHESS;


Board::piece_index_t::piece_index_t()
{}

void Board::piece_index_t::copy_from(const piece_index_t &o, const Board &b)
{
    for(GUINT32 i = 0; i < sizeof(pieces)/sizeof(pieces[0]); ++i)
    {
        for(GUINT32 j = 0; j < sizeof(pieces[0])/sizeof(pieces[0][0]); ++j)
        {
            pieces[i][j].reserve(o.pieces[i][j].size());
            for(Square const *s : o.pieces[i][j])
                pieces[i][j].append(&b.SquareAt(s->GetColumn(), s->GetRow()));
        }
    }
}

QList<Square const *> Board::piece_index_t::all_pieces(Piece::AllegienceEnum a) const
{
    QList<Square const *> ret;
    ret.reserve(16);
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
    return 0 < find_pieces(p).size();
}

const QList<Square const *> &Board::piece_index_t::find_pieces(const Piece &p) const
{
    GASSERT(p.GetType() != Piece::NoPiece && p.GetAllegience() != Piece::AnyAllegience);
    return pieces[(int)p.GetAllegience()][(int)p.GetType()];
}

QList<Square const *> &Board::piece_index_t::find_pieces(const Piece &p)
{
    GASSERT(p.GetType() != Piece::NoPiece && p.GetAllegience() != Piece::AnyAllegience);
    return pieces[(int)p.GetAllegience()][(int)p.GetType()];
}

void Board::piece_index_t::update_piece(const Piece &p,
                                        Square const *orig_val,
                                        Square const *new_val)
{
    QList<Square const *> &vec = find_pieces(p);
    int indx = 0 == orig_val ? -1 : vec.indexOf(orig_val);
    if(-1 == indx){
        if(0 != new_val)
            vec.append(new_val);
    }
    else{
        if(0 == new_val)
            vec.removeAt(indx);
        else
            vec[indx] = new_val;
    }
}

void Board::piece_index_t::clear()
{
    for(GUINT32 i = 0; i < sizeof(pieces)/sizeof(pieces[0]); ++i){
        for(GUINT32 j = 0; j < sizeof(pieces[0])/sizeof(pieces[0][0]); ++j){
            pieces[i][j].clear();
        }
    }
}


#ifdef DEBUG

void Board::piece_index_t::show_index() const
{
    QList<Square const *> squares = all_pieces(Piece::AnyAllegience);
    for(Square const *s : squares){
        GDEBUG(String::Format("%c: %c%d", s->GetPiece().ToFEN(), 'a' + s->GetColumn(), 1 + s->GetRow()));
    }
    GDEBUG("\n");
}

#endif



Board::Board(GUINT32 num_cols, GUINT32 num_rows)
    :m_columnCount(num_cols),
      m_rowCount(num_rows),
      _p_WhoseTurn(Piece::AnyAllegience),
      _p_CastleWhiteA(-1),
      _p_CastleWhiteH(-1),
      _p_CastleBlackA(-1),
      _p_CastleBlackH(-1),
      _p_EnPassantSquare(0),
      _p_HalfMoveClock(0),
      _p_FullMoveNumber(1)
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
      _p_FullMoveNumber(o.GetFullMoveNumber())
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

    SetCastleWhiteA(o.GetCastleWhiteA());
    SetCastleWhiteH(o.GetCastleWhiteH());
    SetCastleBlackA(o.GetCastleBlackA());
    SetCastleBlackH(o.GetCastleBlackH());

    SetWhoseTurn(o.GetWhoseTurn());
    SetEnPassantSquare(o.GetEnPassantSquare());
    SetHalfMoveClock(o.GetHalfMoveClock());
    SetFullMoveNumber(o.GetFullMoveNumber());
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
        int row_diff_abs = Abs(md.Destination.GetRow() - md.Source.GetRow());
        if(2 == row_diff_abs)
            SetEnPassantSquare(&SquareAt(md.Source.GetColumn(), md.Source.GetRow() + inc));
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
                if(GetCastleWhiteA() == md.Source.GetColumn() && 0 == md.Source.GetRow())
                    SetCastleWhiteA(-1);
            }
            if(-1 != GetCastleWhiteH()){
                if(GetCastleWhiteH() == md.Source.GetColumn() && 0 == md.Source.GetRow())
                    SetCastleWhiteH(-1);
            }
        }

        // If they captured a rook then it could ruin their opponent's castle
        if(Piece::Rook == md.PieceCaptured.GetType() && 7 == md.Destination.GetRow())
        {
            if(-1 != GetCastleBlackA() && GetCastleBlackA() == md.Destination.GetColumn())
                SetCastleBlackA(-1);
            else if(-1 != GetCastleBlackH() && GetCastleBlackH() == md.Destination.GetColumn())
                SetCastleBlackH(-1);
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
                if(GetCastleBlackA() == md.Source.GetColumn() && 7 == md.Source.GetRow())
                    SetCastleBlackA(-1);
            }
            if(-1 != GetCastleBlackH()){
                if(GetCastleBlackH() == md.Source.GetColumn() && 7 == md.Source.GetRow())
                    SetCastleBlackH(-1);
            }
        }

        // If they captured a rook then it could ruin their opponent's castle
        if(Piece::Rook == md.PieceCaptured.GetType() && 0 == md.Destination.GetRow())
        {
            if(-1 != GetCastleWhiteA() && GetCastleWhiteA() == md.Destination.GetColumn())
                SetCastleWhiteA(-1);
            else if(-1 != GetCastleWhiteH() && GetCastleWhiteH() == md.Destination.GetColumn())
                SetCastleWhiteH(-1);
        }
    }
}

void Board::move_p(const MoveData &md)
{
    Square &src(square_at(md.Source.GetColumn(), md.Source.GetRow()));
    Square &dest(square_at(md.Destination.GetColumn(), md.Destination.GetRow()));

    Piece piece_orig = src.GetPiece();

    if(MoveData::NoCastle == md.CastleType)
    {
        SetPiece(Piece(), src);
        SetPiece(md.PieceMoved, dest);

        if(md.PieceMoved.GetType() == Piece::Pawn)
        {
            // If it was an enpassant move
            if(GetEnPassantSquare() && md.Destination == *GetEnPassantSquare())
                SetPiece(Piece(), SquareAt(md.Destination.GetColumn(), md.Source.GetRow()));

            // If it was a pawn promotion
            else if(md.Destination.GetRow() == __back_rank(md.PieceMoved.GetOppositeAllegience())){
                SetPiece(md.PiecePromoted, dest);
                GASSERT(!md.PiecePromoted.IsNull());
            }
        }
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

        // Move the rook and king
        Square const &king_dest = square_at(king_col_dest, rank);
        Square const &rook_dest = square_at(rook_col_dest, rank);
        SetPiece(Piece(Piece::Rook, piece_orig.GetAllegience()), rook_dest);
        SetPiece(piece_orig, king_dest);

        Square const &rook_src = square_at(rook_col_src, rank);
        if(rook_src != king_dest && rook_src != rook_dest)
            SetPiece(Piece(), rook_src);
        if(src != rook_dest && src != king_dest)
            SetPiece(Piece(), src);
    }

    //m_index.show_index();

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
        ret = ValidateMove(md.Source, md.Destination);

        if(ValidMove == ret)
        {
            // Move the piece and update the gamestate
            move_p(md);
        }
    }
    return ret;
}

Board::MoveValidationEnum Board::Move(const Square &src, const Square &dest, IPlayerResponse *pr)
{
    MoveValidationEnum ret = ValidateMove(src, dest);
    if(ValidMove == ret)
        move_p(GenerateMoveData(src, dest, pr));
    return ret;
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
    QString cpy( s.Trimmed().ToQString() );
    QStringList sl( cpy.split(' ', QString::SkipEmptyParts) );
    QString backRank_white;
    QString backRank_black;
    if(6 != sl.size())
        throw Exception<>("FEN requires 6 fields separated by spaces");

    // First parse the position text:
    {
        QStringList sl2( sl[0].split('/', QString::KeepEmptyParts) );
        if(RowCount() != sl2.size())
            throw Exception<>("FEN position text requires 8 fields separated by /");

        backRank_white = sl2[7];
        backRank_black = sl2[0];

        // For each section of position text...
        for(int i = 0; i < sl2.size(); ++i)
        {
            int col = 0;
            int rank = 7 - i;
            auto iter = sl2[i].begin();
            auto next = iter + 1;

            // For each character in the section...
            for(;
                iter != sl2[i].end() && col < ColumnCount();
                ++iter)
            {
                int num(-1);
                QChar c = *iter;
                QChar n = (next == sl2[i].end() ? 0 : *next);
                if(c.isDigit())
                {
                    // Normally numbers are single digit, but we want to support larger boards too
                    if(n.isDigit()){
                        num = QString(c).append(n).toInt();
                    }
                    else{
                        num = QString(c).toInt();
                    }
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
                    Piece piece = Piece::FromFEN(c.toLatin1());
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
        switch(sl[1][0].toLatin1())
        {
        case 'b':
            a = Piece::Black;
            break;
        case 'w':
            a = Piece::White;
            break;
        default:
            throw Exception<>("The current turn must be either a 'w' or 'b'");
        }
        SetWhoseTurn(a);
    }


    // Then parse the castle info:
    {
        SetCastleWhiteA(-1);
        SetCastleWhiteH(-1);
        SetCastleBlackA(-1);
        SetCastleBlackH(-1);

        for(QChar c : sl[2])
        {
            Piece::AllegienceEnum a;
            char rook_char;
            char king_char;
            if(c.isUpper()){
                a = Piece::White;
                rook_char = 'R';
                king_char = 'K';
            }
            else{
                a = Piece::Black;
                rook_char = 'r';
                king_char = 'k';
                c = c.toUpper();
            }

            switch(c.toLatin1())
            {
            case '-':
                // If a dash is given, then this field is empty (nobody can castle)
                break;
            case 'K':
            {
                // Find the outermost rook on the H-side
                const QString *s = Piece::White == a ? &backRank_white : &backRank_black;
                int distance_from_h = 0;
                for(int i = s->length() - 1; i >= 0; --i){
                    QChar cur = (*s)[i];
                    if(cur.isDigit()){
                        distance_from_h += String(cur).ToInt();
                    }
                    else if(cur.toLatin1() == rook_char){
                        c = 'H' - distance_from_h;
                        break;
                    }
                    else if(cur.toLatin1() == king_char)
                        break;
                    else
                        ++distance_from_h;
                }
            }
                break;
            case 'Q':
            {
                // Find the outermost rook on the A-side
                const QString *s = Piece::White == a ? &backRank_white : &backRank_black;
                int distance_from_a = 0;
                for(int i = 0; i < s->length(); ++i){
                    QChar cur = (*s)[i];
                    if(cur.isDigit()){
                        distance_from_a += String(cur).ToInt();
                    }
                    else if(cur.toLatin1() == rook_char){
                        c = 'A' + distance_from_a;
                        break;
                    }
                    else if(cur.toLatin1() == king_char)
                        break;
                    else
                        ++distance_from_a;
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
                int file = c.toLatin1()-'A';
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
                throw Exception<>("There was an error with the castle info");
            }
        }
    }


    // Parse the en-passant square:
    {
        if(sl[3] != "-")
        {
            if(sl[3].size() != 2)
                throw Exception<>("Invalid En Passant square");

            char f = sl[3][0].toLatin1();
            char rnk = sl[3][1].toLatin1();
            if(f < 'a' || 'h' < f || rnk < '1' || '8' < rnk)
                throw Exception<>("Invalid En Passant square");

            SetEnPassantSquare(&SquareAt(f - 'a', rnk - '1'));
        }
        else
            SetEnPassantSquare(0);
    }


    // Parse the half-move clock:
    {
        bool ok(false);
        SetHalfMoveClock(sl[4].toInt(&ok));
        if(!ok)
            throw Exception<>("Invalid half-move clock");
    }


    // Parse the full-move number:
    {
        bool ok(false);
        SetFullMoveNumber(sl[5].toInt(&ok));
        if(!ok)
            throw Exception<>("Invalid full-move number");
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
        if(*cur == d)
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
                                       Square const &s,
                                       Square const &d,
                                       Piece::AllegienceEnum a)
{
    int col_diff_abs = Abs(s.GetColumn() - d.GetColumn());
    int row_diff_abs = Abs(s.GetRow() - d.GetRow());
    return col_diff_abs == row_diff_abs &&
            !__is_path_blocked(b, s, d, a);
}

static bool __is_move_valid_for_knight(Board const &b,
                                       Square const &s,
                                       Square const &d,
                                       Piece::AllegienceEnum a)
{
    GUTIL_UNUSED(b);
    GUTIL_UNUSED(a);
    int col_diff_abs = Abs(s.GetColumn() - d.GetColumn());
    int row_diff_abs = Abs(s.GetRow() - d.GetRow());
    return (1 == col_diff_abs && 2 == row_diff_abs) ||
            (2 == col_diff_abs && 1 == row_diff_abs);
}

static bool __is_move_valid_for_rook(Board const &b,
                                     Square const &s,
                                     Square const &d,
                                     Piece::AllegienceEnum a)
{
    int col_diff = s.GetColumn() - d.GetColumn();
    int row_diff = s.GetRow() - d.GetRow();
    return ((0 == col_diff && 0 != row_diff) ||
            (0 == row_diff && 0 != col_diff))
            &&
            !__is_path_blocked(b, s, d, a);
}

static bool __is_move_valid_for_queen(Board const &b,
                                      Square const &s,
                                      Square const &d,
                                      Piece::AllegienceEnum a)
{
    int col_diff_abs = Abs(s.GetColumn() - d.GetColumn());
    int row_diff_abs = Abs(s.GetRow() - d.GetRow());
    return (col_diff_abs == row_diff_abs || ((0 == col_diff_abs && 0 != row_diff_abs) ||
                                             (0 == row_diff_abs && 0 != col_diff_abs)))
            &&
            !__is_path_blocked(b, s, d, a);
}

static bool __is_move_valid_for_king(Board const &b,
                                     Square const &s,
                                     Square const &d,
                                     Piece::AllegienceEnum a)
{
    GUTIL_UNUSED(b);
    GUTIL_UNUSED(a);
    int col_diff_abs = Abs(s.GetColumn() - d.GetColumn());
    int row_diff_abs = Abs(s.GetRow() - d.GetRow());
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
        throw NotImplementedException<>();
        break;
    }

    _update_threat_counts();
}

Board::MoveValidationEnum Board::ValidateMove(const Square &s, const Square &d, bool ignore_checks) const
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
        technically_ok = __is_move_valid_for_knight(*this, s, d, p.GetAllegience()) &&
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
        // If it's a castle attempt
        if(dp == Piece(Piece::Rook, p.GetAllegience()))
        {
            int back_rank, castle_a, castle_h;
            if(Piece::White == p.GetAllegience()){
                back_rank = 0;
                castle_a = GetCastleWhiteA();
                castle_h = GetCastleWhiteH();
            }
            else{
                back_rank = 7;
                castle_a = GetCastleBlackA();
                castle_h = GetCastleBlackH();
            }

            if(s.GetRow() == back_rank)
            {
                if((castle_a != -1 && d.GetColumn() == castle_a) ||
                        (castle_h != -1 && d.GetColumn() == castle_h))
                {
                    castling = true;
                    Square const *king_src = &s;
                    Square const *king_dest;
                    Square const *rook_dest;
                    Square const *rook_src;

                    if(d.GetColumn() == castle_a){
                        king_dest = &SquareAt(2, back_rank);
                        rook_src = &SquareAt(castle_a, back_rank);
                        rook_dest = &SquareAt(3, back_rank);
                    }
                    else if(d.GetColumn() == castle_h){
                        king_dest = &SquareAt(6, back_rank);
                        rook_src = &SquareAt(castle_h, back_rank);
                        rook_dest = &SquareAt(5, back_rank);
                    }

                    // The paths for the rook and king must be unblocked, and no threats
                    //  on any squares the king passes through
                    Range<int> king_range = Range<int>::CreateDoubleBound(
                                Min(king_src->GetColumn(), king_dest->GetColumn()),
                                Max(king_src->GetColumn(), king_dest->GetColumn()));
                    Range<int> rook_range = Range<int>::CreateDoubleBound(
                                Min(rook_src->GetColumn(), rook_dest->GetColumn()),
                                Max(rook_src->GetColumn(), rook_dest->GetColumn()));
                    Range<int> sweep_range = *Region<int>(king_range).Union(rook_range).Ranges().begin();
                    bool path_blocked = false;
                    for(int i = sweep_range.LowerBound().Value();
                        !path_blocked && i <= sweep_range.UpperBound().Value();
                        ++i)
                    {
                        if(i != king_src->GetColumn() && i != rook_src->GetColumn() &&
                                !SquareAt(i, back_rank).GetPiece().IsNull())
                            path_blocked = true;
                    }

                    if(!path_blocked)
                    {
                        bool threats = false;
                        for(int i = king_src->GetColumn();
                            !threats && 0 <= i && i <= king_dest->GetColumn();
                            king_dest->GetColumn() - king_src->GetColumn() > 0 ? ++i : --i)
                        {
                            threats = SquareAt(i, back_rank).GetThreatCount(p.GetOppositeAllegience()) > 0;
                        }
                        technically_ok = !threats;
                    }
                }
            }
        }
        else if(1 >= col_diff_abs && 1 >= row_diff_abs)
        {
            // Normally the king can only move one square in any direction
            technically_ok = !__is_path_blocked(*this, s, d, p.GetAllegience());
        }
        else if(0 == row_diff)
        {

        }
        break;
    default:
        // We should not have an unkown piece type
        throw Exception<>();
    }

    if(!technically_ok)
        return InvalidTechnical;

    if(!castling && !d.GetPiece().IsNull() && d.GetPiece().GetAllegience() == p.GetAllegience())
        return InvalidTechnical;

    // Now check if the king is safe, otherwise it's an invalid move
    if(!ignore_checks)
    {
        // Copy this board to simulate the move, and see if we're in check.
        Board cpy(*this);
        cpy.move_p(cpy.GenerateMoveData(cpy.SquareAt(s.GetColumn(), s.GetRow()),
                                        cpy.SquareAt(d.GetColumn(), d.GetRow()),
                                        0,
                                        false));
        if(cpy.IsInCheck(p.GetAllegience()))
            return InvalidCheck;
    }

    return ValidMove;
}

QList<Square const *> Board::GetValidMovesForSquare(const Square &) const
{
    QList<Square const *> ret;
    return ret;
}

static Square const *__get_source_square(const Board &b,
                                         char piece_moved,
                                         Square const &dest,
                                         Piece::AllegienceEnum a,
                                         GINT8 given_file_info,
                                         GINT8 given_rank_info)
{
    Square const *ret = 0;
    QList<Square const *> possible_sources( b.FindPieces(Piece(Piece::GetTypeFromPGN(piece_moved), a)) );
    for(int i = 0; i < possible_sources.size(); ++i)
    {
        Square const *s = possible_sources[i];

        bool valid = false;
        switch(piece_moved)
        {
        case 'B':
            valid = __is_move_valid_for_bishop(b, *s, dest, a);
            break;
        case 'N':
            valid = __is_move_valid_for_knight(b, *s, dest, a);
            break;
        case 'Q':
            valid = __is_move_valid_for_queen(b, *s, dest, a);
            break;
        case 'R':
            valid = __is_move_valid_for_rook(b, *s, dest, a);
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
                    throw Exception<>("Multiple pieces found which could move there");

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
            throw Exception<>("Unable to castle on that side");

        QList<Square const *> v = FindPieces(Piece(Piece::King, turn));
        if(v.size() != 1)
            throw Exception<>();
        ret.Source = *v[0];
        ret.Destination = SquareAt(rook_loc, v[0]->GetRow());
        ret.PieceMoved = Piece(Piece::King, turn);
    }
    else if(m.Flags.TestFlag(PGN_MoveData::CastleASide))
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
            throw Exception<>("Unable to castle on that side");

        QList<Square const *> v = FindPieces(Piece(Piece::King, turn));
        if(v.size() != 1)
            throw Exception<>();
        ret.Source = *v[0];
        ret.Destination = SquareAt(rook_loc, v[0]->GetRow());
        ret.PieceMoved = Piece(Piece::King, turn);
    }
    else
    {
        // Validate the inputs
        if('a' > m.DestFile || m.DestFile > 'h')
            throw Exception<>("The destination file is invalid");
        if(0 >= m.DestRank || m.DestRank > 8)
            throw Exception<>("The destination rank is invalid");
        if(0 != m.SourceFile && ('a' > m.SourceFile || m.SourceFile > 'h'))
            throw Exception<>("The source file is invalid");
        if(0 != m.SourceRank && (0 > m.SourceRank || m.SourceRank >= 8))
            throw Exception<>("The source rank is invalid");


        // Get the destination square (this is always given)
        Square const &dest = SquareAt(m.DestFile - 'a', m.DestRank - 1);
        ret.Destination = dest;

        if(m.Flags.TestFlag(PGN_MoveData::Capture))
        {
            if(!dest.GetPiece().IsNull() && dest.GetPiece().GetAllegience() != turn)
                ret.PieceCaptured = dest.GetPiece();
            else
                throw Exception<>("Invalid piece to capture at the destination square");
        }


        // Find the source square (this is only seldom given; normally we have to search)
        int tmp_source_column = 0 == m.SourceFile ? -1 : m.SourceFile - 'a';
        if(m.SourceFile != 0 && m.SourceRank != 0)
        {
            Square const &s = SquareAt(tmp_source_column, m.SourceRank - 1);

            // Do a sanity check on the square they specified
            if(s.GetPiece().IsNull())
                throw Exception<>("No piece on the square");
            if(s.GetPiece().GetAllegience() != turn)
                throw Exception<>("It is not your turn");
            if(s.GetPiece().GetType() != m.PieceMoved)
                throw Exception<>("The piece on that square is different than the piece you said to move");

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
                            1 != Abs(tmp_source_column - ret.Destination.GetColumn()))
                        throw Exception<>("Invalid file for pawn capture");

                    QList<Square const *> possible_sources( FindPieces(Piece(Piece::GetTypeFromPGN(m.PieceMoved), turn)) );
                    if(0 == possible_sources.size())
                        throw Exception<>("There are no pieces of that type to move");

                    for(int i = 0; i < possible_sources.size(); ++i)
                    {
                        Square const *s = possible_sources[i];

                        if((1 == Abs(s->GetColumn() - ret.Destination.GetColumn())) &&
                                s->GetRow() == ret.Destination.GetRow() - __allegience_to_rank_increment(turn))
                        {
                            if(-1 != tmp_source_column)
                            {
                                if(s->GetColumn() == tmp_source_column)
                                {
                                    ret.Source = *s;

                                    // We can break here, because the user specified this file, so
                                    //  there is no abiguity
                                    break;
                                }
                            }
                            else
                            {
                                if(!ret.Source.IsNull())
                                    throw Exception<>("Multiple pieces found which could move there");

                                ret.Source = *s;

                                // Don't break here, because we need to keep searching all the pawns
                                //  for an ambiguous move
                            }
                        }
                    }
                }
                else
                {
                    // If the pawn is not capturing, it must be in the same file
                    int r = ret.Destination.GetRow() - __allegience_to_rank_increment(turn);
                    if(0 > r || r > 7)
                        throw Exception<>("No such piece can reach the square");

                    Square const *s = &SquareAt(ret.Destination.GetColumn(), r);
                    if(s->GetPiece().IsNull())
                    {
                        // The pawn can move two squares on the first move
                        r = s->GetRow() - __allegience_to_rank_increment(turn);
                        if((turn == Piece::White && 1 != r) || (turn == Piece::Black && 6 != r) ||
                                (s = &SquareAt(ret.Destination.GetColumn(), r))->GetPiece().IsNull())
                            throw Exception<>("No such piece can reach the square");
                    }

                    if(s->GetPiece().GetAllegience() != turn ||
                            s->GetPiece().GetType() != Piece::Pawn)
                        throw Exception<>("No such piece can reach the square");

                    if(!ret.Destination.GetPiece().IsNull())
                        throw Exception<>("Destination square occupied");

                    ret.Source = *s;
                }

                break;
            case 'R':
            case 'N':
            case 'B':
            case 'Q':
                ret.Source = *__get_source_square(*this, m.PieceMoved, ret.Destination, turn, tmp_source_column, tmp_source_rank);
                break;
            case 'K':
            {
                QList<Square const *> possible_sources( FindPieces(Piece(Piece::GetTypeFromPGN(m.PieceMoved), turn)) );

                // There can only be one king
                GASSERT(1 == possible_sources.size());

                Square const *s = possible_sources[0];
                if(__is_move_valid_for_king(*this, *s, ret.Destination, turn))
                    ret.Source = *s;
            }
                break;
            default:
                throw Exception<>();
            }
        }

        if(ret.Source.IsNull())
            throw Exception<>("No such piece can reach the square");

        ret.PieceMoved = SquareAt(ret.Source.GetColumn(), ret.Source.GetRow()).GetPiece();
        GASSERT(!ret.PieceMoved.IsNull());

        // Add a promoted piece, if necessary
        if(m.PiecePromoted)
        {
            if('P' != m.PieceMoved && 0 != m.PieceMoved)
                throw Exception<>("Only pawns can be promoted");

            ret.PiecePromoted = Piece(Piece::GetTypeFromPGN(m.PiecePromoted), turn);
        }
    }

    ret.PGNData = m;

    return ret;
}

MoveData Board::GenerateMoveData(const Square &s,
                                 const Square &d,
                                 IPlayerResponse *uf,
                                 bool pgn_data) const
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

        ret.Source = s;
        ret.Destination = d;

        if(!s.GetPiece().IsNull())
            ret.PieceMoved = s.GetPiece();

        if(!d.GetPiece().IsNull() && ret.CastleType == MoveData::NoCastle)
            ret.PieceCaptured = d.GetPiece();
        else if(ret.PieceMoved.GetType() == Piece::Pawn &&
                GetEnPassantSquare() &&
                &d == GetEnPassantSquare())
        {
            ret.PieceCaptured = SquareAt(d.GetColumn(), s.GetRow()).GetPiece();
        }

        if(pgn_data)
        {
            // Create the PGN data for the move
            ret.PGNData.MoveNumber = GetFullMoveNumber();
            ret.PGNData.PieceMoved = s.GetPiece().ToPGN()[0];
            if(!ret.PiecePromoted.IsNull())
                ret.PGNData.PiecePromoted = ret.PiecePromoted.ToPGN()[0];

            if(ret.CastleType == MoveData::CastleASide)
                ret.PGNData.Flags.SetFlag(PGN_MoveData::CastleASide, true);
            else if(ret.CastleType == MoveData::CastleHSide)
                ret.PGNData.Flags.SetFlag(PGN_MoveData::CastleHSide, true);
            else if(!ret.PieceCaptured.IsNull()){
                ret.PGNData.Flags.SetFlag(PGN_MoveData::Capture, true);

                // When pawns capture we always show the source file
                if(Piece::Pawn == ret.PieceMoved.GetType())
                    ret.PGNData.SourceFile = 'a' + ret.Source.GetColumn();
            }

            // Add the source rank/file info if necessary
            if(ret.PGNData.SourceFile == 0)
            {
                // If more than one piece could reach the destination square
                QList<const Square *> pieces = m_index.find_pieces(ret.PieceMoved);
                if(pieces.size() > 1)
                {
                    QList<const Square *> possible_movers;
                    for(const Square *s : pieces){
                        if(*s != ret.Source && ValidMove == ValidateMove(*s, ret.Destination, true))
                            possible_movers.append(s);
                    }

                    if(0 < possible_movers.size())
                    {
                        bool row_match = false;
                        for(int i = 0; i < possible_movers.size(); ++i){
                            if(ret.Source.GetRow() == possible_movers[i]->GetRow()){
                                row_match = true;
                                possible_movers.removeAt(i);
                                break;
                            }
                        }

                        bool col_match = false;
                        for(int i = 0; i < possible_movers.size(); ++i){
                            if(ret.Source.GetColumn() == possible_movers[i]->GetColumn()){
                                col_match = true;
                                break;
                            }
                        }

                        if(col_match)
                            ret.PGNData.SourceRank = ret.Source.GetRow() + 1;
                        if(row_match || (!col_match && !row_match))
                            ret.PGNData.SourceFile = 'a' + ret.Source.GetColumn();
                    }
                }
            }

            ret.PGNData.DestFile = 'a' + ret.Destination.GetColumn();
            ret.PGNData.DestRank = ret.Destination.GetRow() + 1;

            Board cpy = *this;
            cpy.move_p(ret);
            if(cpy.IsInCheckMate(ret.PieceMoved.GetOppositeAllegience()))
                ret.PGNData.Flags.SetFlag(PGN_MoveData::CheckMate, true);
            else if(cpy.IsInCheck(ret.PieceMoved.GetOppositeAllegience()))
                ret.PGNData.Flags.SetFlag(PGN_MoveData::Check, true);
        }
    }

    return ret;
}

// If there is a piece at the destination, dest_piece will be updated with the information
//  Returns true if the square was appended
static bool __append_if_within_bounds(QList<Square const *> &res, const Board &b, int col, int row, Piece *dest_piece)
{
    bool ret = false;
    if(0 <= col && 0 <= row && col < b.ColumnCount() && row < b.RowCount()){
        Square const *sqr = &b.SquareAt(col, row);
        res.append(sqr);
        ret = true;

        if(!sqr->GetPiece().IsNull())
            *dest_piece = sqr->GetPiece();
    }
    return ret;
}


static void __get_threatened_squares_helper(QList<Square const *> &ret, const Board &b, const Square &s, int col_inc, int row_inc, int max_distance = -1)
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
        forever
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
static QList<Square const *> __get_threatened_squares(const Board &b, Piece const &p, const Square &s)
{
    QList<Square const *> ret;
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
                ret.append(&b.SquareAt(col, rank));

            col = s.GetColumn() + 1;
            if(7 >= col)
                ret.append(&b.SquareAt(col, rank));
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

    QList<Square const *> all_pieces(FindPieces(Piece()));
    for(Square const *s_c : all_pieces)
    {
        GASSERT(!s_c->GetPiece().IsNull());

        Piece const &p = s_c->GetPiece();

        QList<Square const *> squares(__get_threatened_squares(*this, p, *s_c));
        for(Square const *s_c : squares){
            Square &s = square_at(s_c->GetColumn(), s_c->GetRow());
            s.SetThreatCount(p.GetAllegience(), s.GetThreatCount(p.GetAllegience()) + 1);
        }
    }
}

void Board::_set_all_threat_counts(int c)
{
    for(int i = 0; i < ColumnCount(); ++i){
        for(int j = 0; j < RowCount(); ++j){
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

QList<Square const *> Board::FindPieces(Piece const &pc) const
{
    QList<Square const *> ret;
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
    QList<Square const *> king_loc = FindPieces(king);
    if(1 == king_loc.size())
    {
        if(0 < king_loc[0]->GetThreatCount(king.GetOppositeAllegience()))
            ret = true;
    }
    return ret;
}

bool Board::IsInCheckMate(Piece::AllegienceEnum) const
{
    /** \todo Implement this */
    return false;
}




#ifdef DEBUG

void Board::ShowIndex() const
{
    Console::WriteLine("White Pieces:");
    QList<Square const *> v = m_index.all_pieces(Piece::White);
    for(const Square *s : v){
        Console::WriteLine(String::Format("%s: %s",
                                          s->GetPiece().ToString(true).ConstData(),
                                          s->ToString().ConstData()));
    }


    Console::WriteLine("\nBlack Pieces:");
    v = m_index.all_pieces(Piece::Black);
    for(const Square *s : v){
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

void ObservableBoard::FromFEN(const String &s)
{
    // This way we don't emit a signal every time a piece is placed
    Board newboard;
    newboard.FromFEN(s);
    *this = newboard;
}

void ObservableBoard::move_p(const MoveData &md)
{
    emit NotifyPieceAboutToBeMoved(md);
    Board::move_p(md);
    emit NotifyPieceMoved(md);
}


END_NAMESPACE_GKCHESS;
