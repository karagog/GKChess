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


PGN_Parser::PGN_Parser(String const &s)
    :m_result(0)
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

static int __get_valid_rank_number(char n)
{
    int ret = -1;
    bool ok = false;
    int tmpnum = String(n).ToInt(&ok);
    if(ok && 1 <= tmpnum && tmpnum <= 8)
        ret = tmpnum;
    return ret;
}

/** Converts a string to an array index. */
static bool __validate_file_char(char c)
{
    int ret = false;
    if('a' <= c && c <= 'h')
        ret = true;
    return ret;
}

bool PGN_Parser::_new_movedata_from_string(PGN_MoveData &m, const String &s)
{
    bool ret = true;
    m.Text = s;

    // Strip the comment (if any)
    GUINT32 ind = s.IndexOf('{');
    if(UINT_MAX != ind){
        GUINT32 ind2 = s.IndexOf('}', ind + 1);

        if(UINT_MAX == ind2)
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "Unterminated comment");
        if(UINT_MAX != s.IndexOf('{', ind + 1))
            THROW_NEW_GUTIL_EXCEPTION2(Exception, "Nested comment");

        m.Comment = s.SubString(ind + 1, ind2 - ind - 1);
        m.Text.Truncate(ind);
    }

    m.Text = m.Text.Trimmed();

    printf("%s", String::Format("Parsing '%s'\n", m.Text.ConstData()).ConstData());

    if(UINT_MAX != m.Text.IndexOf("O-O-O"))
        m.Flags.SetFlag(PGN_MoveData::CastleQueenSide, true);
    else if(UINT_MAX != m.Text.IndexOf("O-O"))
        m.Flags.SetFlag(PGN_MoveData::CastleNormal, true);
    else if(UINT_MAX != m.Text.IndexOf("1-0")){
        m_result = 1;
        ret = false;
    }
    else if(UINT_MAX != m.Text.IndexOf("0-1")){
        m_result = -1;
        ret = false;
    }
    else if(UINT_MAX != m.Text.IndexOf("1/2-1/2")){
        m_result = 0;
        ret = false;
    }
    else
    {
        typename String::UTF8ConstIterator iter(m.Text.beginUTF8());

        // The first character must be a piece type, or it's a pawn
        Piece::PieceTypeEnum pt = Piece::GetTypeFromUnicodeValue(iter.UnicodeValue());
        if(Piece::InvalidPiece != pt){
            m.PieceMoved = pt;
            ++iter;
        }

        // Parse the source and destination squares
        char file1 = 0, file2 = 0;
        int rank1 = 0, rank2 = 0;
        for(; iter != m.Text.endUTF8(); ++iter)
        {
            char c = *iter.Current();
            int tmp_number = __get_valid_rank_number(c);

            if(__validate_file_char(c))
            {
                // If a file is given, it may be a source or destination file,
                //  so we just remember it until we have more information.
                if(0 == file1)
                    file1 = c;
                else if(0 == file2)
                    file2 = c;
                else
                    THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid PGN");
            }
            else if(-1 != tmp_number)
            {
                // If a number is given, it is paired with the last given file.
                if(0 != file2){
                    if(0 == rank2)
                        rank2 = tmp_number;
                    else
                        THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid PGN");
                }
                else if(0 != file1)
                    rank1 = tmp_number;
                else
                    THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid PGN");
            }
            else if('x' == c)
            {
                if(m.Flags.TestFlag(PGN_MoveData::Capture))
                    THROW_NEW_GUTIL_EXCEPTION2(Exception, "Invalid PGN");
                m.Flags.SetFlag(PGN_MoveData::Capture, true);
            }
            else if('-' == c)
            {
                // Ignore this unnecessary character
            }
            else
            {
                // Any other characters mean we have reached the end of the move info
                break;
            }
        }

        // Now we can sort out what the source and destination squares are:
        if(0 != file2){
            m.SourceFile = file1;
            m.DestFile = file2;
        }
        else if(0 != file1)
            m.DestFile = file1;

        if(0 != rank2)
            m.DestRank = rank2;
        if(0 != rank1){
            if(0 == m.SourceFile)
                m.DestRank = rank1;
            else
                m.SourceRank = rank1;
        }


        // Is there a piece promotion?
        GUINT32 ind = m.Text.IndexOfUTF8("=");
        if(UINT_MAX != ind){
            if(ind + 1 >= m.Text.LengthUTF8())
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "Promotion piece not given");
            m.PiecePromoted = Piece::GetTypeFromUnicodeValue((m.Text.beginUTF8() + (++ind)).UnicodeValue());
        }
    }

    // See if the move puts the king in check or checkmate
    if(UINT_MAX != m.Text.IndexOf('#'))
        m.Flags.SetFlag(PGN_MoveData::CheckMate, true);
    else if(UINT_MAX != m.Text.IndexOf('+'))
        m.Flags.SetFlag(PGN_MoveData::Check, true);

    // See if the annotator has an assessment of this move
    if(UINT_MAX != m.Text.IndexOf("??"))
        m.Flags.SetFlag(PGN_MoveData::Blunder, true);
    else if(UINT_MAX != m.Text.IndexOf("!!"))
        m.Flags.SetFlag(PGN_MoveData::Brilliant, true);
    else if(UINT_MAX != m.Text.IndexOf("!?"))
        m.Flags.SetFlag(PGN_MoveData::Interesting, true);
    else if(UINT_MAX != m.Text.IndexOf("?!"))
        m.Flags.SetFlag(PGN_MoveData::Dubious, true);
    else if(UINT_MAX != m.Text.IndexOf('?'))
        m.Flags.SetFlag(PGN_MoveData::Mistake, true);
    else if(UINT_MAX != m.Text.IndexOf('!'))
        m.Flags.SetFlag(PGN_MoveData::Good, true);

    return ret;
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
            {
                PGN_MoveData m;
                if(_new_movedata_from_string(m, sl2[0]))
                    m_moves.PushBack(m);
            }

            if(1 < sl2.Length()){
                PGN_MoveData m;
                if(_new_movedata_from_string(m, sl2[1]))
                    m_moves.PushBack(m);
            }
        }

        if(2 < sl2.Length()){
            number_text = sl2[2];
        }
    }
}


END_NAMESPACE_GKCHESS;
