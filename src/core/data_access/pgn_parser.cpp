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

#include "pgn_parser.h"
USING_NAMESPACE_GUTIL;
USING_NAMESPACE_GUTIL1(DataObjects);

NAMESPACE_GKCHESS;


PGN_Parser::MoveData::MoveData()
    :Piece(Piece::Pawn), PiecePromoted(Piece::Pawn),
     SourceFile(-1), SourceRank(-1), DestFile(-1), DestRank(-1)
{}


PGN_Parser::PGN_Parser(String const &s)
{
    if(!s.IsValidUTF8())
        THROW_NEW_GUTIL_EXCEPTION2(ValidationException,
                                   "The data contains an invalid UTF-8 sequence");

    typename String::UTF8ConstIterator move_section_start( _parse_heading(s) );
    _parse_moves(String(move_section_start, s.endUTF8()).Replace("\n", " "));
}

typename String::UTF8ConstIterator PGN_Parser::_parse_heading(const String &pgn_text)
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

void PGN_Parser::_parse_moves(const String &move_text)
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
                                       String::Format("There was a problem parsing the move number: %s", number_text.ConstData()));

        Vector<String> sl2( sl[i].Split(' ', false) );

        if(0 < sl2.Length()){
            m_moves.PushBack(sl2[0]);
        }

        if(1 < sl2.Length()){
            m_moves.PushBack(sl2[1]);
        }

        if(2 < sl2.Length()){
            number_text = sl2[2];
        }
    }
}


END_NAMESPACE_GKCHESS;
