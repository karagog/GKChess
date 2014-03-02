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

#include "pgn_player.h"
#include "gutil_file.h"

USING_NAMESPACE_GUTIL1(DataAccess);
USING_NAMESPACE_GUTIL1(DataObjects);
USING_NAMESPACE_GUTIL;

NAMESPACE_GKCHESS;


PGN_Player::MoveData::MoveData()
    :PlyNumber(0),
      PieceType(Piece::Pawn),
      SourceColumn(-1), SourceRow(-1),
      DestColumn(-1), DestRow(-1),
      Captures(false),
      CastleType(NoCastle)
{}


PGN_Player::PGN_Player(GameLogic *g)
    :m_game(g)
{}

void PGN_Player::Clear()
{
    m_tags.Clear();
    m_moves.Empty();
}

void PGN_Player::LoadFromString(const String &pgn_text)
{
    if(!pgn_text.IsValidUTF8())
        THROW_NEW_GUTIL_EXCEPTION2(ValidationException,
                                   "The data contains an invalid UTF-8 sequence");

    Clear();

    typename String::UTF8ConstIterator move_section_start( _parse_heading(pgn_text) );
    _parse_moves(String(move_section_start, pgn_text.endUTF8()));
}

typename String::UTF8ConstIterator PGN_Player::_parse_heading(const String &pgn_text)
{
    String tmp, tmp_key, tmp_value;
    bool escape_char = false;
    bool inside_tag = false;
    bool inside_quote = false;
    typename String::UTF8ConstIterator move_section_start(pgn_text.endUTF8());
    for(typename String::UTF8ConstIterator c( pgn_text.beginUTF8() );
        c != pgn_text.endUTF8();
        ++c)
    {
        char byt = (char)c.UnicodeValue();

        if(inside_tag)
        {
            bool skip_char = false;
            if(c.IsValidAscii())
            {
                if(String::IsNumber(byt))
                {
                    if(!inside_quote){
                        THROW_NEW_GUTIL_EXCEPTION2(ValidationException,
                                                   "Invalid number within tag section");
                    }
                }

                switch(byt)
                {
                case '[':
                    THROW_NEW_GUTIL_EXCEPTION2(ValidationException,
                                               "Invalid nested brackets");
                    break;
                case ']':
                    m_tags.Insert(tmp_key, tmp_value);
                    inside_tag = false;
                    skip_char = true;
                    break;
                case ' ':
                    if(tmp_key.IsEmpty()){
                        tmp_key = tmp;
                        tmp.Clear();
                        skip_char = true;
                    }
                    break;
                case '\"':
                    if(!escape_char){
                        if(inside_quote){
                            if(tmp_value.IsEmpty()){
                                tmp_value = tmp;
                            }
                            else
                                THROW_NEW_GUTIL_EXCEPTION2(ValidationException,
                                                           "Too many quotes in tag section");
                            inside_quote = false;
                        }
                        else{
                            inside_quote = true;
                        }
                        skip_char = true;
                    }
                    break;
                case '\\':
                    escape_char = true;
                    break;
                default:
                    break;
                }
            }

            if(!skip_char)
                tmp.Append(c.Current(), c.ByteLength());
        }
        else
        {
            if(c.IsValidAscii())
            {
                if(String::IsNumber(byt)){
                    // This is our exit condition: encountering a numeral outside of a tag
                    move_section_start = c;
                    break;
                }

                switch(byt)
                {
                case '[':
                    tmp.Clear();
                    tmp_key.Clear();
                    tmp_value.Clear();
                    inside_tag = true;
                    break;
                }
            }
        }
    }

    return move_section_start;
}

void PGN_Player::_parse_moves(const String &move_text)
{
    int move_number = -1;
    Vector<String> sl( move_text.Split('.', false) );
    String number_text;

    if(0 < sl.Length())
        number_text = sl[0];

    for(GUINT32 i = 1; i < sl.Length(); ++i)
    {
        bool ok = false;
        move_number = number_text.ToInt(&ok);
        if(!ok)
            THROW_NEW_GUTIL_EXCEPTION2(ValidationException,
                                       "There was a problem parsing the move number");

        Vector<String> sl2( sl[i].Split(' ') );

        if(0 < sl2.Length()){
            MoveData m;
            m.PlyNumber = 2 * move_number - 1;
            _parse_move(sl2[0], m);
            m_moves.PushBack(m);
        }

        if(1 < sl2.Length()){
            MoveData m;
            m.PlyNumber = 2 * move_number;
            _parse_move(sl2[1], m);
            m_moves.PushBack(m);
        }

        if(2 < sl2.Length()){
            number_text = sl2[2];
        }
    }
}

void PGN_Player::_parse_move(const String &move_text, MoveData &m)
{
    m.MoveText = move_text;

    for(typename String::UTF8ConstIterator c( move_text.beginUTF8() );
        c != move_text.endUTF8();
        ++c)
    {
        GUINT32 uc = c.UnicodeValue();
    }
}

void PGN_Player::LoadFromFile(const String &pgn_filename)
{
    File f(pgn_filename);
    f.Open(File::OpenRead);
    String s( f.Read() );
    f.Close();
    LoadFromString(s);
}


END_NAMESPACE_GKCHESS;
