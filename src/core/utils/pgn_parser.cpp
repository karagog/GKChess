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
#include <gutil/file.h>
USING_NAMESPACE_GUTIL;

#define TAG_RESULT "result"

NAMESPACE_GKCHESS;


/** Populates the heading tags and updates the iterator to the start of the move data section. */
static void __parse_heading(PGN_GameData &gm,
                            typename String::const_iterator &iter,
                            const typename String::const_iterator &end)
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
            if(0 < *iter)
            {
                char c = (char)*iter;

                if(String::IsNumber(c))
                {
                    if(!inside_quote){
                        throw ValidationException<>("Invalid number within tag section");
                    }
                }

                switch(c)
                {
                case '[':
                    throw ValidationException<>("Invalid nested brackets");
                    break;
                case ']':
                    gm.Tags.insert(tmp_key.ToLower(), tmp_value);
                    inside_tag = false;
                    skip_char = true;
                    //GDEBUG(String::Format("Found tag: %s-%s", tmp_key.ConstData(), tmp_value.ConstData()));
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
                                throw ValidationException<>("Too many quotes in tag section");
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
                default: break;
                }
            }

            if(!skip_char)
                tmp.Append(*iter);
        }
        else
        {
            if(0 <= *iter)
            {
                char c = (char)*iter;
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
                    //GDEBUG("Found Tag Start");
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

static bool __is_invalid_promotion_piece(char c)
{
    return c != 'Q' && c != 'N' && c != 'R' && c != 'B';
}

PGN_MoveData PGN_Parser::CreateMoveDataFromString(const String &s)
{
    PGN_MoveData ret;

    //GDEBUG(String::Format("Parsing '%s'", s.ConstData()));

    if(GUINT32_MAX != s.ToUpper().IndexOf("O-O-O"))
        ret.Flags.SetFlag(PGN_MoveData::CastleASide, true);
    else if(GUINT32_MAX != s.ToUpper().IndexOf("O-O"))
        ret.Flags.SetFlag(PGN_MoveData::CastleHSide, true);
    else
    {
        typename String::const_iterator iter(s.begin());

        // The first character must be a piece type, or we don't record it
        if(String::IsUpper(*iter)){
            ret.PieceMoved = *iter;
            ++iter;
        }

        // Parse the source and destination squares
        QList<char> files;
        QList<int> ranks;
        files.reserve(2);
        ranks.reserve(2);
        for(; iter != s.end(); ++iter)
        {
            char c = *iter.Current();
            int tmp_number = __get_valid_rank_number(c);

            if(__validate_file_char(c))
            {
                // If a file is given, it may be a source or destination file,
                //  so we just remember it until we have more information.
                files.append(c);
            }
            else if(-1 != tmp_number)
            {
                ranks.append(tmp_number);
            }
            else if('x' == c)
            {
                ret.Flags.SetFlag(PGN_MoveData::Capture, true);
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
        if(files.size() == 2){
            ret.SourceFile = files[0];
            ret.DestFile = files[1];
        }
        else if(files.size() == 1)
            ret.DestFile = files[0];
        else
            throw Exception<>("Invalid file info");

        if(ranks.size() == 2){
            ret.SourceRank = ranks[0];
            ret.DestRank = ranks[1];
        }
        else if(ranks.size() == 1)
            ret.DestRank = ranks[0];
        else
            throw Exception<>("Invalid rank info");

        // Is there a piece promotion?
        GUINT32 ind = s.IndexOf("=");
        if(GUINT32_MAX != ind){
            if(ind + 1 >= s.Length() ||
                    __is_invalid_promotion_piece(s[ind+1]))
                throw Exception<>("Invalid promotion piece");
            ret.PiecePromoted = s[ind+1];
        }
    }

    // See if the move puts the king in check or checkmate
    if(GUINT32_MAX != s.IndexOf('#'))
        ret.Flags.SetFlag(PGN_MoveData::CheckMate, true);
    else if(GUINT32_MAX != s.IndexOf('+'))
        ret.Flags.SetFlag(PGN_MoveData::Check, true);

    // See if the annotator has an assessment of this move
    if(GUINT32_MAX != s.IndexOf("??"))
        ret.Flags.SetFlag(PGN_MoveData::Blunder, true);
    else if(GUINT32_MAX != s.IndexOf("!!"))
        ret.Flags.SetFlag(PGN_MoveData::Brilliant, true);
    else if(GUINT32_MAX != s.IndexOf("!?"))
        ret.Flags.SetFlag(PGN_MoveData::Interesting, true);
    else if(GUINT32_MAX != s.IndexOf("?!"))
        ret.Flags.SetFlag(PGN_MoveData::Dubious, true);
    else if(GUINT32_MAX != s.IndexOf('?'))
        ret.Flags.SetFlag(PGN_MoveData::Mistake, true);
    else if(GUINT32_MAX != s.IndexOf('!'))
        ret.Flags.SetFlag(PGN_MoveData::Good, true);

    return ret;
}

/** Populates the move data and updates the iterator to the start of the next game, or the end of the string. */
static void __parse_moves(PGN_GameData &gm,
                          typename String::const_iterator &iter,
                          const typename String::const_iterator &end)
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
    };

    state_enum cur_state = ground;
    state_enum last_state = ground;

    int first_move_number = 1;  // This is only not 1 in the case of a partial game
    int prev_move_number = 0;
    int next_move_number = 0;
    int whose_turn = 0;      // 0 for white, 1 for black
    bool has_move_data = false;

    int dot_count = 0;

    String tmps;
    PGN_MoveData md;

    for(; iter != end; ++iter)
    {
        int uc = *iter;
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
                continue;
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
            next_move_number = tmps.ToInt(&ok);
            tmps.Empty();
            if(0 == gm.Moves.size()){
                first_move_number = next_move_number;
                prev_move_number = next_move_number;
            }
            GASSERT(ok);
        }
        if(prev_state == parsing_movetext && cur_state != parsing_movetext)
        {
            // Add the movetext to the move data
            md = PGN_Parser::CreateMoveDataFromString(tmps);
            has_move_data = true;
            tmps.Empty();
        }
        if(prev_state != parsing_movetext && cur_state == parsing_movetext && has_move_data)
        {
            // Add the last move data to the list when we encounter the next move data. That way we ensure
            //  the comments get tagged to the right moves

            md.MoveNumber = prev_move_number;
            prev_move_number = next_move_number;

            if((gm.Moves.size() >> 1) + 1 != md.MoveNumber){
                throw Exception<>(String::Format("Invalid move number: '%d'", md.MoveNumber));
            }
            gm.Moves.append(md);
        }
        if((prev_state == parsing_comment_curly_braces && cur_state != parsing_comment_curly_braces) ||
                (prev_state == parsing_comment_semicolon && cur_state != parsing_comment_semicolon))
        {
            // Add the comment to the move data
            md.Comment = tmps;
            tmps.Empty();
        }

        // Remember every time we leave an important state
        if(prev_state != ground && cur_state == ground)
            last_state = prev_state;


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

    // append the last move data
    if(has_move_data){
        md.MoveNumber = prev_move_number;
        gm.Moves.append(md);
    }
}



QList<PGN_GameData> PGN_Parser::ParseFile(const String &filename)
{
    String s;
    {
        File f(filename);
        f.Open(File::OpenRead);
        s = f.Read();
    }
    return ParseString(s);
}

QList<PGN_GameData> PGN_Parser::ParseString(String const &s)
{
    if(!s.IsValidUTF8())
        throw ValidationException<>("The data contains an invalid UTF-8 sequence");

    QList<PGN_GameData> ret;
    typename String::const_iterator iter(s.begin());
    typename String::const_iterator end(s.end());
    while(iter != end)
    {
        ret.append(PGN_GameData());
        PGN_GameData &gd = ret.back();

        // Parse the heading section for tags-value pairs
        __parse_heading(gd, iter, end);

        // Validate the heading to make sure it has the required tags
        if(!gd.Tags.contains(TAG_RESULT))
            throw Exception<>(String::Format("Tag section is missing '%s'", TAG_RESULT));

        // The result is the game termination marker
        String result_val = gd.Tags[TAG_RESULT];
        if(result_val != "1-0" && result_val != "0-1" && result_val != "1/2-1/2" && result_val != "*")
            throw Exception<>(String::Format("Invalid Result: %s", result_val.ConstData()));

        GUINT32 last_index = s.IndexOf(result_val, iter - s.begin());
        if(GUINT32_MAX == last_index)
            throw Exception<>("Move section not terminated by result");

        __parse_moves(gd, iter, s.begin() + last_index);

        // Seek to the start of the next PGN
        while(iter != end && '[' != *iter)
            ++iter;
    }
    return ret;
}


END_NAMESPACE_GKCHESS;
