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
#include "gutil_file.h"
USING_NAMESPACE_GUTIL;

#define SETUP_TAG "SetUp"
#define FEN_TAG "FEN"

NAMESPACE_GKCHESS;


PGN_Parser::MoveData::MoveData()
    :MoveNumber(0),
      PieceMoved(0),
      PiecePromoted(0),
      SourceFile(0),
      SourceRank(0),
      DestFile(0),
      DestRank(0)
{}

static const char *__convert_piece_char_to_name(char c)
{
    switch(c)
    {
    case '\0':
    case 'P':
        return "Pawn";
    case 'K':
        return "King";
    case 'Q':
        return "Queen";
    case 'B':
        return "Bishop";
    case 'N':
        return "Knight";
    case 'R':
        return "Rook";
    case 'A':
        return "Archbishop";
    case 'C':
        return "Chancellor";
    default:
        return "";
    }
}

String PGN_Parser::MoveData::ToString() const
{
    String ret(25);
    if(Flags.TestFlag(CastleHSide))
        ret.Append("Castle");
    else if(Flags.TestFlag(CastleQueenSide))
        ret.Append("Castle Queenside");
    else
    {
        ret.Append(PieceMoved);

        if(0 != SourceFile){
            ret.Append(String::Format(" on %c", SourceFile));
            if(0 != SourceRank)
                ret.Append(String::FromInt(SourceRank));
        }

        if(Flags.TestFlag(Capture))
            ret.Append(" takes ");
        else
            ret.Append(" to ");

        ret.Append(String::Format("%c%d", DestFile, DestRank));

        if('P' != PiecePromoted)
            ret.Append(String::Format(" promotes to %s",
                                      __convert_piece_char_to_name(PiecePromoted)));
    }

    if(Flags.TestFlag(Check))
        ret.Append(" check");
    else if(Flags.TestFlag(CheckMate))
        ret.Append(" checkmate");

    if(Flags.TestFlag(Blunder))
        ret.Append(" (blunder)");
    else if(Flags.TestFlag(Mistake))
        ret.Append(" (mistake)");
    else if(Flags.TestFlag(Dubious))
        ret.Append(" (dubious move)");
    else if(Flags.TestFlag(Interesting))
        ret.Append(" (interesting move)");
    else if(Flags.TestFlag(Good))
        ret.Append(" (good move)");
    else if(Flags.TestFlag(Brilliant))
        ret.Append(" (brilliant move)");

    if(!Comment.IsEmpty())
        ret.Append(String::Format(" {%s}", Comment.ConstData()));

    return ret;
}



/** Populates the heading tags and updates the iterator to the start of the move data section. */
static void __parse_heading(PGN_Parser::GameData &gm,
                            typename String::UTF8ConstIterator &iter,
                            const typename String::UTF8ConstIterator &end)
{
    String tmp, tmp_key, tmp_value;
    bool escape_char = false;
    bool inside_tag = false;
    bool inside_quote = false;
    for(; iter != end; ++iter)
    {
        if(inside_tag)
        {
            bool skip_char = false;
            if(iter.IsValidAscii())
            {
                char c = (char)iter.UnicodeValue();

                if(String::IsNumber(c))
                {
                    if(!inside_quote){
                        THROW_NEW_GUTIL_EXCEPTION2(ValidationException,
                                                   "Invalid number within tag section");
                    }
                }

                switch(c)
                {
                case '[':
                    THROW_NEW_GUTIL_EXCEPTION2(ValidationException,
                                               "Invalid nested brackets");
                    break;
                case ']':
                    gm.Tags.Insert(tmp_key, tmp_value);
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
                tmp.Append(iter.Current(), iter.ByteLength());
        }
        else
        {
            if(iter.IsValidAscii())
            {
                char c = (char)iter.UnicodeValue();
                if(String::IsNumber(c))
                {
                    // This is our exit condition: encountering a numeral outside of a tag
                    break;
                }

                switch(c)
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

static bool __new_movedata_from_string(PGN_Parser::MoveData &m, const String &s)
{
    bool ret = true;
    m.MoveText = s;

    GDEBUG(String::Format("Parsing '%s'", m.MoveText.ConstData()));

    if(-1 != m.MoveText.ToUpper().IndexOf("O-O-O"))
        m.Flags.SetFlag(PGN_Parser::MoveData::CastleQueenSide, true);
    else if(-1 != m.MoveText.ToUpper().IndexOf("O-O"))
        m.Flags.SetFlag(PGN_Parser::MoveData::CastleHSide, true);
    else if(-1 != m.MoveText.IndexOf("1-0")){
        ret = false;
    }
    else if(-1 != m.MoveText.IndexOf("0-1")){
        ret = false;
    }
    else if(-1 != m.MoveText.IndexOf("1/2-1/2")){
        ret = false;
    }
    else
    {
        typename String::const_iterator iter(m.MoveText.begin());

        // The first character must be a piece type, or it's a pawn
        if(String::IsUpper(*iter)){
            m.PieceMoved = *iter;
            ++iter;
        }
        else{
            m.PieceMoved = 'P';
        }

        // Parse the source and destination squares
        char file1 = 0, file2 = 0;
        int rank1 = 0, rank2 = 0;
        for(; iter != m.MoveText.endUTF8(); ++iter)
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
                m.Flags.SetFlag(PGN_Parser::MoveData::Capture, true);
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
        GINT32 ind = m.MoveText.IndexOf("=");
        if(-1 != ind){
            if(ind + 1 >= m.MoveText.Length())
                THROW_NEW_GUTIL_EXCEPTION2(Exception, "Promotion piece not given");
            m.PiecePromoted = m.MoveText[ind+1];
        }
    }

    // See if the move puts the king in check or checkmate
    if(-1 != m.MoveText.IndexOf('#'))
        m.Flags.SetFlag(PGN_Parser::MoveData::CheckMate, true);
    else if(-1 != m.MoveText.IndexOf('+'))
        m.Flags.SetFlag(PGN_Parser::MoveData::Check, true);

    // See if the annotator has an assessment of this move
    if(-1 != m.MoveText.IndexOf("??"))
        m.Flags.SetFlag(PGN_Parser::MoveData::Blunder, true);
    else if(-1 != m.MoveText.IndexOf("!!"))
        m.Flags.SetFlag(PGN_Parser::MoveData::Brilliant, true);
    else if(-1 != m.MoveText.IndexOf("!?"))
        m.Flags.SetFlag(PGN_Parser::MoveData::Interesting, true);
    else if(-1 != m.MoveText.IndexOf("?!"))
        m.Flags.SetFlag(PGN_Parser::MoveData::Dubious, true);
    else if(-1 != m.MoveText.IndexOf('?'))
        m.Flags.SetFlag(PGN_Parser::MoveData::Mistake, true);
    else if(-1 != m.MoveText.IndexOf('!'))
        m.Flags.SetFlag(PGN_Parser::MoveData::Good, true);

    return ret;
}

/** Populates the move data and updates the iterator to the start of the next game, or the end of the string. */
static void __parse_moves(PGN_Parser::GameData &gm,
                          typename String::UTF8ConstIterator &iter,
                          const typename String::UTF8ConstIterator &end)
{
    // The states of our parsing function as it traverses the string
    enum state_enum
    {
        ground = 0,

        // Parsing the move number
        parsing_movenumber = 1,

        // Parsing the dots after the move number, either . or ...
        parsing_dots = 2,

        // Parsing the move text
        parsing_movetext = 3,

        // Parsing a comment enclosed in curly braces
        parsing_comment_curly_braces = 4,

        // Parsing an "end of line" comment starting with a semicolon
        parsing_comment_semicolon = 5
    }
    cur_state = ground;

    int first_move_number = 1;  // This is only not 1 in the case of a partial game
    int whose_turn = 0;      // 0 for white, 1 for black

    int dot_count = 0;

    String tmps;
    PGN_Parser::MoveData md;

    for(; iter != end; ++iter)
    {
        int uc = iter.UnicodeValue();
        char c = (char)uc;
        bool ok;
        state_enum prev_state = cur_state;

        //GDEBUG(String::Format("%c", c));

        // This first switch leaves our current state and enters 'ground'
        switch(cur_state)
        {
        case parsing_movenumber:
            // As soon as you encounter the first non-number character, the movenumber text is finished
            if(!String::IsNumber(c))
                cur_state = ground;
            break;
        case parsing_dots:
            // Dot sequences are self-delimiting
            if('.' != c)
                cur_state = ground;
            break;
        case parsing_movetext:
            // Movetext is delimited by whitespace
            if(String::IsWhitespace(c))
                cur_state = ground;
            break;
        case parsing_comment_curly_braces:
            // Curly braces end at the next right brace
            if(cur_state == parsing_comment_curly_braces && '}' == c)
                cur_state = ground;
            break;
        case parsing_comment_semicolon:
            // The semicolon comment ends at the next newline
            if(cur_state == parsing_comment_semicolon && '\n' == c)
                cur_state = ground;
            break;
        default: break;
        }


        // Debug info
//        if(prev_state != cur_state){
//            GDEBUG(String::Format("Leaving state %d for %d", (int)prev_state, (int)cur_state));
//        }


        // Next we provide criteria for entering a new state from ground
        if(ground == cur_state)
        {
            if(String::IsNumber(c)){
                cur_state = parsing_movenumber;
            }
            else if(String::IsRoman(c)){
                cur_state = parsing_movetext;
            }
            else if('.' == c){
                cur_state = parsing_dots;
            }
            else if('{' == c){
                cur_state = parsing_comment_curly_braces;
            }
            else if(';' == c){
                cur_state = parsing_comment_semicolon;
            }
        }


//        if(prev_state != cur_state){
//            GDEBUG(String::Format("Entering state %d", (int)cur_state));
//        }


        // Here we do some processing upon certain state transitions:
        if(prev_state == parsing_dots && cur_state != parsing_dots)
        {
            if(1 == dot_count)
                whose_turn = 0;   // white's turn
            else if(1 < dot_count)
                whose_turn = 1;   // black's turn
            dot_count = 0;
            GASSERT(0 == tmps.Length());
        }

        if(prev_state == parsing_movenumber && cur_state != parsing_movenumber)
        {
            md.MoveNumber = tmps.ToInt(&ok);
            tmps.Empty();
            if(0 == gm.Moves.Length())
                first_move_number = md.MoveNumber;
            GASSERT(ok);
        }
        if(prev_state == parsing_movetext && cur_state != parsing_movetext)
        {
            // Add the movetext to the move data
            __new_movedata_from_string(md, tmps);
            tmps.Empty();
        }
        if(prev_state != parsing_movetext && cur_state == parsing_movetext && gm.Moves != 0)
        {
            // Add the last move data to the list when we encounter the next move data. That way we ensure
            //  the comments get tagged to the right moves

            if((gm.Moves.Length() >> 1) + 1 != md.MoveNumber){
                THROW_NEW_GUTIL_EXCEPTION2(Exception, String::Format("Invalid move number: '%d'", md.MoveNumber));
            }
            gm.Moves.PushBack(md);
            md = PGN_Parser::MoveData();
        }
        if((prev_state == parsing_comment_curly_braces && cur_state != parsing_comment_curly_braces) ||
                (prev_state == parsing_comment_semicolon && cur_state != parsing_comment_semicolon))
        {
            // Add the comment to the move data
            md.Comment = tmps;
            tmps.Empty();
        }


        // The third switch does the appropriate things with the latest character and new state
        switch(cur_state)
        {
        case parsing_movenumber:
            tmps.Append(c);
            GASSERT(String::IsNumber(c));
            break;
        case parsing_dots:
            ++dot_count;
            GASSERT('.' == c);
            break;
        case parsing_movetext:
            tmps.Append(c);
            break;
        case parsing_comment_curly_braces:
        case parsing_comment_semicolon:
            tmps.Append(c);
            break;
        case ground:
            // If we're in the ground state at this point we ignore all characters (as in whitespace)
        default: break;
        }
    }
}



Vector<PGN_Parser::GameData> PGN_Parser::ParseFile(const String &filename)
{
    String s;
    {
        File f(filename);
        f.Open(File::OpenRead);
        s = f.Read();
    }
    return ParseString(s);
}

Vector<PGN_Parser::GameData> PGN_Parser::ParseString(String const &s)
{
    if(!s.IsValidUTF8())
        THROW_NEW_GUTIL_EXCEPTION2(ValidationException,
                                   "The data contains an invalid UTF-8 sequence");

    Vector<GameData> ret;
    typename String::UTF8ConstIterator iter(s.beginUTF8());
    typename String::UTF8ConstIterator end(s.endUTF8());
    while(iter != end)
    {
        ret.PushBack(GameData());
        __parse_heading(ret.Back(), iter, end);
        __parse_moves(ret.Back(), iter, end);
    }
    return ret;
}


END_NAMESPACE_GKCHESS;
