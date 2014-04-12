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
#include "gamelogic.h"
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;


/** Allocate one for all instances to use, since it's immutable. */
static StandardGameLogic s_standardLogic;


AbstractBoard::AbstractBoard(IGameLogic const *g, QObject *p)
    :QObject(p),
      i_gameLogic(NULL == g ? &s_standardLogic : g)
{}

AbstractBoard::AbstractBoard(const AbstractBoard &o)
    :QObject(o.parent()),
      i_gameLogic(o.i_gameLogic)
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

void AbstractBoard::MovePiece(ISquare const &s, ISquare const &d, IGameLogic::IPlayerResponse *pr)
{
    // If you want more moves to be valid you can implement your own liberal validator
    if(IGameLogic::ValidMove == GameLogic().ValidateMove(*this, s, d))
    {
        MoveData md( GameLogic().GenerateMoveData(*this, s, d, pr) );
        emit NotifyPieceAboutToBeMoved(md);
        {
            // Move the piece
            move_p(md);
            GameLogic().PieceMoved(*this, md);
        }
        emit NotifyPieceMoved(md);
    }
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


END_NAMESPACE_GKCHESS;
