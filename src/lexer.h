typedef struct Number
{
    union
    {
        U64 u64;
        F64 f64;
    };
    
    U8 min_fit_bits;
    bool is_float;
} Number;

typedef struct Text_Pos
{
    File_ID file_id;
    U32 offset_to_line;
    U32 line;
    U32 column;
} Text_Pos;

typedef struct Text_Interval
{
    Text_Pos position;
    UMM size;
} Text_Interval;

enum TOKEN_KIND
{
    Token_Unknown = 0,
    Token_Error,
    Token_EndOfStream,
    
    Token_At,
    Token_Cash,
    Token_Hash,
    Token_Exclamation,
    Token_Plus,
    Token_Minus,
    Token_Asterisk,
    Token_Slash,
    Token_Percentage,
    Token_Hat,
    Token_Equals,
    Token_Greater,
    Token_Less,
    Token_Tilde,
    Token_Ampersand,
    Token_Pipe,
    Token_Underscore,
    Token_Period,
    Token_Comma,
    Token_Colon,
    Token_Semicolon,
    Token_QuestionMark,
    
    Token_OpenParen,
    Token_CloseParen,
    Token_OpenBrace,
    Token_CloseBrace,
    Token_OpenBracket,
    Token_CloseBracket,
    
    Token_Char,
    Token_String,
    Token_Identifier,
    Token_Number,
    Token_Keyword
};

enum KEYWORD_KIND
{
    Keyword_Package,
    
    Keyword_Load,
    Keyword_Import,
    Keyword_As,
    
    Keyword_If,
    Keyword_Else,
    Keyword_For,
    Keyword_Break,
    Keyword_Continue,
    
    Keyword_Proc,
    Keyword_Struct,
    Keyword_Union,
    Keyword_Enum,
    
    Keyword_Defer,
    Keyword_Return,
    Keyword_Using,
    
    Keyword_True,
    Keyword_False,
    
    KEYWORD_COUNT
};

const char* KeywordStrings[KEYWORD_COUNT] = {
    [Keyword_Package]  = "package",
    
    [Keyword_Load]     = "load",
    [Keyword_Import]   = "import",
    [Keyword_As]       = "as",
    
    [Keyword_If]       = "if",
    [Keyword_Else]     = "else",
    [Keyword_For]      = "for",
    [Keyword_Break]    = "break",
    [Keyword_Continue] = "continue",
    
    [Keyword_Proc]     = "proc",
    [Keyword_Struct]   = "struct",
    [Keyword_Union]    = "union",
    [Keyword_Enum]     = "enum",
    
    [Keyword_Using]    = "using",
    [Keyword_Defer]    = "defer",
    [Keyword_Return]   = "return",
    
    [Keyword_True]     = "true",
    [Keyword_False]    = "false",
};

typedef struct Token
{
    Enum32(TOKEN_KIND) kind;
    Text_Interval text;
    
    union
    {
        String string;
        Number number;
        Enum32(KEYWORD_KIND) keyword;
        U32 codepoint;
    };
} Token;

typedef struct Lexer
{
    Memory_Arena* arena;
    
    struct
    {
        Token token;
        bool is_valid;
    } cache;
    
    Text_Pos position;
    U8* file_start;
} Lexer;

bool
ResolveUnicodeString(Lexer* lexer, String raw, String* string)
{
    bool encountered_errors = false;
    
    NOT_IMPLEMENTED;
    
    return !encountered_errors;
}

Token
GetToken(Lexer* lexer)
{
    Token token = {.kind = Token_Unknown};
    
    if (lexer->cache.is_valid) token = lexer->cache.token;
    else
    {
        { // Skip junk
            bool in_single_line_comment = false;
            U32 comment_nesting_level   = 0;
            
            for (;;)
            {
                U8* scan = lexer->file_start + lexer->position.offset_to_line + lexer->position.column;
                
                if (*scan == 0) break;
                
                if (scan[0] == ' ' || scan[0] == '\t' || scan[0] == '\v' || scan[0] == '\r')
                {
                    lexer->position.column += 1;
                }
                
                else if (scan[0] == '\n')
                {
                    lexer->position.line += 1;
                    
                    lexer->position.offset_to_line = lexer->position.column + 1;
                    lexer->position.column         = 0;
                    
                    in_single_line_comment = false;
                }
                
                else if (scan[0] == '/' && scan[1] == '/' && comment_nesting_level == 0)
                {
                    lexer->position.column += 2;
                    in_single_line_comment  = true;
                }
                
                else if (scan[0] == '/' && scan[1] == '*' && !in_single_line_comment)
                {
                    lexer->position.column += 2;
                    comment_nesting_level  += 1;
                }
                
                else if (scan[0] == '*' && scan[1] == '/' && comment_nesting_level != 0)
                {
                    lexer->position.column += 2;
                    comment_nesting_level  -= 1;
                }
                
                else if (in_single_line_comment || comment_nesting_level != 0) lexer->position.column += 1;
                
                else break;
            }
            
            if (comment_nesting_level != 0)
            {
                //// ERROR: Unterminated multiline comment
                token.kind = Token_Error;
            }
        }
        
        if (token.kind != Token_Error)
        {
            U8* start = lexer->file_start + lexer->position.offset_to_line + lexer->position.column;
            U8* peek  = start + 1;
            
            switch (peek[-1])
            {
                case  0:  token.kind = Token_EndOfStream;  break;
                case '@': token.kind = Token_At;           break;
                case '$': token.kind = Token_Cash;         break;
                case '#': token.kind = Token_Hash;         break;
                case '!': token.kind = Token_Exclamation;  break;
                case '+': token.kind = Token_Plus;         break;
                case '-': token.kind = Token_Minus;        break;
                case '*': token.kind = Token_Asterisk;     break;
                case '/': token.kind = Token_Slash;        break;
                case '%': token.kind = Token_Percentage;   break;
                case '^': token.kind = Token_Hat;          break;
                case '=': token.kind = Token_Equals;       break;
                case '>': token.kind = Token_Greater;      break;
                case '<': token.kind = Token_Less;         break;
                case '~': token.kind = Token_Tilde;        break;
                case '&': token.kind = Token_Ampersand;    break;
                case '|': token.kind = Token_Pipe;         break;
                case '.': token.kind = Token_Period;       break;
                case ',': token.kind = Token_Comma;        break;
                case ':': token.kind = Token_Colon;        break;
                case ';': token.kind = Token_Semicolon;    break;
                case '?': token.kind = Token_QuestionMark; break;
                case '(': token.kind = Token_OpenParen;    break;
                case ')': token.kind = Token_CloseParen;   break;
                case '{': token.kind = Token_OpenBrace;    break;
                case '}': token.kind = Token_CloseBrace;   break;
                case '[': token.kind = Token_OpenBracket;  break;
                case ']': token.kind = Token_CloseBracket; break;
                
                default:
                {
                    if (peek[-1] == '_' && !(IsAlpha(peek[0]) | IsDigit(peek[0])))
                    {
                        token.kind = Token_Underscore;
                    }
                    
                    else if (IsAlpha(peek[-1]) || peek[-1] == '_')
                    {
                        token.kind = Token_Identifier;
                        
                        token.string.data = (U8*)peek - 1;
                        
                        while (IsAlpha(peek[0]) || IsDigit(peek[0]) || peek[0] == '_')
                        {
                            ++peek;
                        }
                        
                        token.string.size = (U8*)peek - token.string.data;
                        
                        for (uint i = 0; i < KEYWORD_COUNT; ++i)
                        {
                            if (StringCStringCompare(token.string, KeywordStrings[i]))
                            {
                                token.kind    = Token_Keyword;
                                token.keyword = i;
                                
                                break;
                            }
                        }
                    }
                    
                    else if (peek[-1] == '"')
                    {
                        token.kind = Token_String;
                        
                        String raw_string = { .data = (U8*)peek };
                        
                        while (peek[0] != 0 && peek[0] != '"')
                        {
                            ++peek;
                        }
                        
                        if (peek[0] != '"')
                        {
                            //// ERROR: Unterminated string literal
                            token.kind = Token_Error;
                        }
                        
                        else
                        {
                            raw_string.size = (U8*)peek - token.string.data;
                            ++peek;
                            
                            if (!ResolveUnicodeString(lexer, raw_string, &token.string))
                            {
                                token.kind = Token_Error;
                            }
                        }
                    }
                    
                    else if (peek[-1] == '\'')
                    {
                        token.kind = Token_Char;
                        
                        String raw_string = { .data = (U8*)peek };
                        
                        while (peek[0] != 0 && peek[0] != '\'')
                        {
                            ++peek;
                        }
                        
                        if (peek[0] != '\'')
                        {
                            //// ERROR: Unterminated character literal
                            token.kind = Token_Error;
                        }
                        
                        else
                        {
                            raw_string.size = (U8*)peek - token.string.data;
                            ++peek;
                            
                            String resolved_string;
                            if (ResolveUnicodeString(lexer, raw_string, &resolved_string))
                            {
                                if (resolved_string.data != 0)
                                {
                                    U8* bytes = resolved_string.data;
                                    
                                    token.codepoint = 0;
                                    if ((bytes[0] & 0xF0) == 0xF0)
                                    {
                                        token.codepoint |= ((U32)bytes[0] & 0x07) << 18;
                                        token.codepoint |= ((U32)bytes[1] & 0x3F) << 12;
                                        token.codepoint |= ((U32)bytes[2] & 0x3F) << 6;
                                        token.codepoint |= ((U32)bytes[3] & 0x3F) << 0;
                                        
                                        resolved_string.size -= 4;
                                    }
                                    
                                    else if ((bytes[0] & 0xE0) == 0xE0)
                                    {
                                        token.codepoint |= ((U32)bytes[0] & 0x0F) << 12;
                                        token.codepoint |= ((U32)bytes[1] & 0x3F) << 6;
                                        token.codepoint |= ((U32)bytes[2] & 0x3F) << 0;
                                        
                                        resolved_string.size -= 3;
                                    }
                                    
                                    else if ((bytes[0] & 0xC0) == 0xC0)
                                    {
                                        token.codepoint |= ((U32)bytes[0] & 0x1F) << 6;
                                        token.codepoint |= ((U32)bytes[1] & 0x3F) << 0;
                                        
                                        resolved_string.size -= 2;
                                    }
                                    
                                    else
                                    {
                                        token.codepoint |= ((U32)bytes[0] & 0x7F);
                                        
                                        resolved_string.size -= 1;
                                    }
                                    
                                    if (resolved_string.size != 0)
                                    {
                                        //// ERROR: Character literal contains more than one codepoint
                                        token.kind = Token_Error;
                                    }
                                }
                                
                                else
                                {
                                    //// ERROR: Empty character literal
                                    token.kind = Token_Error;
                                }
                            }
                            
                            else
                            {
                                token.kind = Token_Error;
                            }
                        }
                    }
                    
                    else if (IsDigit(peek[-1]))
                    {
                        token.kind = Token_Number;
                        
                        peek -= 1;
                        
                        bool is_hex    = false;
                        bool is_float  = false;
                        bool is_binary = false;
                        
                        if (peek[0] == '0')
                        {
                            if (peek[1] == 'x')
                            {
                                is_hex = true;
                                peek  += 2;
                            }
                            
                            else if (peek[1] == 'h')
                            {
                                is_hex   = true;
                                is_float = true;
                                peek    += 2;
                            }
                            
                            else if (peek[1] == 'b')
                            {
                                is_binary = true;
                                peek     += 2;
                            }
                        }
                        
                        U8 base;
                        if (is_hex)         base = 16;
                        else if (is_binary) base = 2;
                        else                base = 10;
                        
                        U64 u64 = 0;
                        F64 f64 = 0;
                        
                        F64 digit_mod     = 1;
                        bool u64_overflow = false;
                        uint num_digits   = 0;
                        
                        bool encountered_errors = false;
                        for (; !encountered_errors; ++peek)
                        {
                            U8 digit = 0;
                            
                            if (IsDigit(peek[0]))
                            {
                                if (is_binary && peek[0] > '1')
                                {
                                    //// ERROR: Invalid digit in binary literal
                                    encountered_errors = true;
                                }
                                
                                else
                                {
                                    digit = peek[0] - 0;
                                }
                            }
                            
                            else if (is_hex && ToUpper(peek[0]) >= 'A' && ToUpper(peek[0]) <= 'F')
                            {
                                digit = (ToUpper(peek[0]) - 'A') + 10;
                            }
                            
                            else if (!is_float && peek[0] == '.')
                            {
                                is_float = true;
                                digit_mod = 0.1;
                                continue;
                            }
                            
                            else if (peek[0] == '_') continue;
                            
                            else break;
                            
                            
                            U64 new_u64 = u64 * base + digit;
                            
                            if (!is_float && new_u64 < u64) u64_overflow = true;
                            else u64 = new_u64;
                            
                            f64 = f64 * (digit_mod == 1 ? base : 1) + digit * digit_mod;
                            if (digit_mod != 1) digit_mod /= 10;
                            
                            num_digits += 1;
                        }
                        
                        if (peek[-1] == '_')
                        {
                            //// ERROR: Numeric literal cannot end with digit separator
                            encountered_errors = true;
                        }
                        
                        if (!encountered_errors && !is_hex && !is_binary && ToUpper(peek[0]) == 'E')
                        {
                            ++peek;
                            
                            I8 sign = 1;
                            if (peek[0] == '+') ++peek;
                            else if (peek[0] == '-')
                            {
                                sign = -1;
                                ++peek;
                            }
                            
                            if (IsDigit(peek[0]))
                            {
                                U64 exponent = 0;
                                for (; !encountered_errors; ++peek)
                                {
                                    if (IsDigit(peek[0]))
                                    {
                                        U64 new_exponent = exponent * 10 + (peek[0] - '0');
                                        if (new_exponent < exponent)
                                        {
                                            //// ERROR: Exponent too large to be represented by any integral type
                                            encountered_errors = true;
                                        }
                                        
                                        else exponent = new_exponent;
                                    }
                                    
                                    else if (peek[0] == '_') continue;
                                    
                                    else break;
                                }
                                
                                is_float = true;
                                
                                if (sign == 1) for (U64 i = 0; i < exponent; ++i) f64 *= 10;
                                else           for (U64 i = 0; i < exponent; ++i) f64 /= 10;
                            }
                            
                            else
                            {
                                //// ERROR: Missing exponent value
                                encountered_errors = true;
                            }
                        }
                        
                        if (!encountered_errors)
                        {
                            if (!is_float)
                            {
                                if (u64_overflow)
                                {
                                    //// ERROR: Interger literal too large to be representable by any integral type
                                    encountered_errors = true;
                                }
                                
                                else
                                {
                                    token.number.u64      = u64;
                                    token.number.is_float = false;
                                    
                                    if      (u64 <= U8_MAX)  token.number.min_fit_bits = 8;
                                    else if (u64 <= U16_MAX) token.number.min_fit_bits = 16;
                                    else if (u64 <= U32_MAX) token.number.min_fit_bits = 32;
                                    else                     token.number.min_fit_bits = 64;
                                }
                            }
                            
                            else
                            {
                                // IMPORTANT TODO(soimn): Check if the value is "valid"
                                if (is_hex)
                                {
                                    if (num_digits != 8 && num_digits != 16)
                                    {
                                        //// ERROR: Hexadecimal floating point literal must be of either length 8 or 16
                                        encountered_errors = true;
                                    }
                                    
                                    else
                                    {
                                        // HACK(soimn): Find a better way of converting the hex value to float
                                        if (num_digits == 8)
                                        {
                                            U32 u32 = (U32)u64;
                                            
                                            token.number.f64          = *(F32*)&u32;
                                            token.number.is_float     = true;
                                            token.number.min_fit_bits = 32;
                                        }
                                        
                                        else
                                        {
                                            token.number.f64          = *(F64*)&u64;
                                            token.number.is_float     = true;
                                            token.number.min_fit_bits = 32;
                                        }
                                    }
                                }
                                
                                else
                                {
                                    token.number.f64      = f64;
                                    token.number.is_float = true;
                                    
                                    // HACK(soimn): Find a better way of checking the minimum precision required
                                    if ((F64)(F32)f64 == f64) token.number.min_fit_bits = 32;
                                    else                      token.number.min_fit_bits = 64;
                                }
                            }
                        }
                        
                        if (encountered_errors)
                        {
                            token.kind = Token_Error;
                        }
                    }
                    
                    else token.kind = Token_Unknown;
                } break;
            }
            
            token.text.position = lexer->position;
            token.text.size     = peek - start;
            
            lexer->cache.token = token;
        }
    }
    
    return token;
}

void
SkipPastToken(Lexer* lexer, Token token)
{
    ASSERT(token.kind != Token_EndOfStream);
    
    lexer->position         = token.text.position;
    lexer->position.column += (U32)token.text.size;
    
    lexer->cache.is_valid = false;
}

Token
PeekNextToken(Lexer* lexer, Token token)
{
    Lexer tmp_lexer = *lexer;
    SkipPastToken(&tmp_lexer, token);
    return GetToken(lexer);
}

Text_Interval
TextInterval(Text_Pos position, UMM size)
{
    Text_Interval interval = {position, size};
    return interval;
}

Text_Interval
TextInterval_Merge(Text_Interval i0, Text_Interval i1)
{
    ASSERT(i0.position.file_id == i1.position.file_id);
    
    Text_Pos start_pos = i0.position;
    
    if (i1.position.line < i0.position.line ||
        i1.position.line == i0.position.line && i1.position.column < i0.position.column)
    {
        start_pos = i1.position;
    }
    
    UMM i0_max = i0.position.offset_to_line + i0.position.column + i0.size;
    UMM i1_max = i1.position.offset_to_line + i1.position.column + i1.size;
    
    UMM size = MAX(i0_max, i1_max) - (start_pos.offset_to_line + start_pos.column);
    
    Text_Interval interval = {start_pos, size};
    return interval;
}

Text_Interval
TextInterval_FromEndPoints(Text_Pos p0, Text_Pos p1)
{
    if (p1.offset_to_line + p1.column < p0.offset_to_line + p0.column)
    {
        Text_Pos tmp = p0;
        p0 = p1;
        p1 = tmp;
    }
    
    UMM size = (p1.offset_to_line + p1.column) - (p0.offset_to_line + p0.column);
    
    Text_Interval interval = {p0, size};
    return interval;
}