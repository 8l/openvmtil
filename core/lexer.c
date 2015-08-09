#include "../includes/cfrtil.h"
// lexer.c has been strongly influenced by the ideas in the lisp reader algorithm 
// "http://www.ai.mit.edu/projects/iiip/doc/CommonLISP/HyperSpec/Body/sec_2-2.html"
// although it doesn't fully conform to them yet the intention is to be eventually somewhat of a superset of them

#define TokenBuffer_AppendPoint( lexer ) &lexer->TokenBuffer [ lexer->TokenWriteIndex ]
#define _AppendCharacterToTokenBuffer( lex, character ) lexer->TokenBuffer [ lex->TokenWriteIndex ] = character
#define SourceCode_AppendPoint &_CfrTil_->SourceCodeScratchPad [ strlen ( ( CString ) _CfrTil_->SourceCodeScratchPad ) ]

void
CfrTil_LexerTables_Setup ( CfrTil * cfrtl )
{
    int32 i ;
    for ( i = 0 ; i < 256 ; i ++ ) cfrtl->LexerCharacterTypeTable [ i ].CharInfo = 0 ;
    cfrtl->LexerCharacterTypeTable [ 0 ].CharFunctionTableIndex = 1 ;
    cfrtl->LexerCharacterTypeTable [ '-' ].CharFunctionTableIndex = 2 ;
    cfrtl->LexerCharacterTypeTable [ '>' ].CharFunctionTableIndex = 3 ;
    cfrtl->LexerCharacterTypeTable [ '.' ].CharFunctionTableIndex = 4 ;
    cfrtl->LexerCharacterTypeTable [ '\n' ].CharFunctionTableIndex = 6 ;
    cfrtl->LexerCharacterTypeTable [ '\\' ].CharFunctionTableIndex = 7 ;
    cfrtl->LexerCharacterTypeTable [ eof ].CharFunctionTableIndex = 8 ;
    cfrtl->LexerCharacterTypeTable [ '\r' ].CharFunctionTableIndex = 9 ;
    cfrtl->LexerCharacterTypeTable [ ',' ].CharFunctionTableIndex = 10 ;

    cfrtl->LexerCharacterTypeTable [ '"' ].CharFunctionTableIndex = 5 ;
    cfrtl->LexerCharacterTypeTable [ '[' ].CharFunctionTableIndex = 11 ;
    cfrtl->LexerCharacterTypeTable [ ']' ].CharFunctionTableIndex = 11 ;
    cfrtl->LexerCharacterTypeTable [ '\'' ].CharFunctionTableIndex = 11 ;
    cfrtl->LexerCharacterTypeTable [ '`' ].CharFunctionTableIndex = 11 ;
    cfrtl->LexerCharacterTypeTable [ '(' ].CharFunctionTableIndex = 11 ;
    cfrtl->LexerCharacterTypeTable [ ')' ].CharFunctionTableIndex = 11 ;
    cfrtl->LexerCharacterTypeTable [ '%' ].CharFunctionTableIndex = 11 ;
    //cfrtl->LexerCharacterTypeTable [ '&' ].CharFunctionTableIndex = 11 ;
    //cfrtl->LexerCharacterTypeTable [ ',' ].CharFunctionTableIndex = 11 ;

    cfrtl->LexerCharacterTypeTable [ '$' ].CharFunctionTableIndex = 12 ;
    cfrtl->LexerCharacterTypeTable [ '#' ].CharFunctionTableIndex = 12 ;
    cfrtl->LexerCharacterTypeTable [ '/' ].CharFunctionTableIndex = 14 ;
    cfrtl->LexerCharacterTypeTable [ ';' ].CharFunctionTableIndex = 15 ;
    cfrtl->LexerCharacterTypeTable [ '&' ].CharFunctionTableIndex = 16 ;

    cfrtl->LexerCharacterFunctionTable [ 0 ] = Lexer_Default ;
    cfrtl->LexerCharacterFunctionTable [ 1 ] = _Zero ;
    cfrtl->LexerCharacterFunctionTable [ 2 ] = Minus ;
    cfrtl->LexerCharacterFunctionTable [ 3 ] = GreaterThan ;
    cfrtl->LexerCharacterFunctionTable [ 4 ] = Dot ;
    cfrtl->LexerCharacterFunctionTable [ 5 ] = DoubleQuote ;
    cfrtl->LexerCharacterFunctionTable [ 6 ] = NewLine ;
    cfrtl->LexerCharacterFunctionTable [ 7 ] = BackSlash ;
    cfrtl->LexerCharacterFunctionTable [ 8 ] = _EOF ;
    cfrtl->LexerCharacterFunctionTable [ 9 ] = CarriageReturn ;
    cfrtl->LexerCharacterFunctionTable [ 10 ] = Comma ;

    cfrtl->LexerCharacterFunctionTable [ 11 ] = TerminatingMacro ; //nb. MacroChar is a new approach not yet fully applied/integrated
    cfrtl->LexerCharacterFunctionTable [ 12 ] = NonTerminatingMacro ; //nb. MacroChar is a new approach not yet fully applied/integrated
    cfrtl->LexerCharacterFunctionTable [ 13 ] = SingleEscape ; //nb. MacroChar is a new approach not yet fully applied/integrated
    cfrtl->LexerCharacterFunctionTable [ 14 ] = ForwardSlash ;
    cfrtl->LexerCharacterFunctionTable [ 15 ] = Semi ; //nb. MacroChar is a new approach not yet fully applied/integrated
    cfrtl->LexerCharacterFunctionTable [ 16 ] = AddressOf ; //nb. MacroChar is a new approach not yet fully applied/integrated

    //cfrl->LexerCharacterTypeTable [ '@' ].CharFunctionTableIndex = 16 ;
    //cfrl->LexerCharacterFunctionTable [ 6 ] = Bracket ;
    //cfrl->LexerCharacterFunctionTable [ 7 ] = MacroCharacter ;
    //cfrl->LexerCharacterFunctionTable [ 12 ] = Quote ; // single quote and back quote
    //cfrl->LexerCharacterFunctionTable [ 13 ] = ShellEscape ;
    //cfrl->LexerCharacterFunctionTable [ 14 ] = LParen ;
    //cfrl->LexerCharacterFunctionTable [ 15 ] = RParen ;
    //cfrl->LexerCharacterFunctionTable [ 11 ] = AtSign ;
    //cfrl->LexerCharacterFunctionTable [ 11 ] = Ampersand ;
}

#if 0

Word *
_Lexer_PeekNextNonDebugWord ( Lexer * lexer )
{
    byte * token ;
    Word * word ;
    do
    {
        token = Lexer_ReadToken ( lexer ) ;
        word = Finder_Word_FindUsing ( lexer->OurInterpreter->Finder, token ) ;
    }
    while ( word && ( word->CType & DEBUG_WORD ) ) ;
    _CfrTil_AddTokenToTailOfPeekTokenList ( token ) ;
    return word ;
}
#endif

byte
Lexer_NextNonDelimiterChar ( Lexer * lexer )
{
    return _String_NextNonDelimiterChar ( _ReadLine_pb_NextChar ( lexer->ReadLiner ), lexer->DelimiterCharSet ) ;
}

byte *
Lexer_StrTok ( Lexer * lexer )
{
    byte * buffer = 0 ;
    byte * nextChar = _ReadLine_pb_NextChar ( lexer->ReadLiner ) ;
    if ( nextChar )
    {
        buffer = Buffer_Data ( _CfrTil_->StringB ) ;
        _StrTok ( _ReadLine_pb_NextChar ( lexer->ReadLiner ), buffer, lexer->DelimiterCharSet ) ;
    }
    return buffer ;
}

Word *
Lexer_ObjectToken_New ( Lexer * lexer, byte * token, int32 parseFlag )
{
    Word * word = 0 ;
    if ( token )
    {
        if ( parseFlag ) Lexer_ParseObject ( lexer, token ) ;
        if ( lexer->TokenType & T_RAW_STRING )
        {
            if ( GetState ( _Q_, AUTO_VAR ) ) // make it a 'variable' right here 
            {
                word = _CfrTil_Variable ( token, 0 ) ;
                _Interpret_MorphismWord_Default ( _Context_->Interpreter0, word ) ;
                goto next ;
            }
        }
        word = ConstantOrLiteral_New ( _Context_->Interpreter0, lexer->Literal ) ;
next:
        lexer->TokenWord = word ;
        _Compiler_WordStack_PushWord ( _Context_->Compiler0, word ) ;

        if ( ! Lexer_GetState ( lexer, KNOWN_OBJECT ) )
        {
            ClearLine ;
            Printf ( ( byte* ) "\n%s ?\n", ( char* ) token ) ;
            CfrTil_Exception ( NOT_A_KNOWN_OBJECT, QUIT ) ;
        }
    }
    return word ;
}

byte *
_Lexer_NextNonDebugTokenWord ( Lexer * lexer )
{
    byte * token ;
    Word * word ;
    while ( 1 )
    {
        token = _Lexer_ParseNextToken_WithDelimiters ( lexer, 0, 0 ) ;
        word = Finder_Word_FindUsing ( lexer->OurInterpreter->Finder, token ) ;
        if ( word && ( word->CType & DEBUG_WORD ) )
        {
            //Interpret_MorphismWord_Default ( word ) ;
            _CfrTil_AddTokenToTailOfPeekTokenList ( token ) ; // TODO ; list should instead be a stack
        }
        else break ;
    }
    return token ;
}

byte *
Lexer_PeekNextNonDebugTokenWord ( Lexer * lexer )
{
    byte * token ;
    token = _Lexer_NextNonDebugTokenWord ( lexer ) ;
    _CfrTil_AddTokenToTailOfPeekTokenList ( token ) ; // TODO ; list should instead be a stack
    return token ;
}

byte *
_Lexer_ParseNextToken_WithDelimiters ( Lexer * lexer, byte * delimiters, int32 checkListFlag )
{
    ReadLiner * rl = lexer->ReadLiner ;
#if NEW    
    if ( ! GetState ( _Context_, CONTEXT_PARSING_QUALIFIED_ID ) )
        if ( Lexer_IsTokenQualifiedID ( lexer ) ) SetState ( _Context_, CONTEXT_PARSING_QUALIFIED_ID, true ) ;
    //else SetState ( _Context_, CONTEXT_PARSING_QUALIFIED_ID, false ) ;
#endif    
    if ( ( ! checkListFlag ) || ( ! _CfrTil_GetTokenFromPeekedTokenList ( ) ) ) // ( ! checkListFlag ) : allows us to peek multiple tokens ahead if we already have peeked tokens
    {
        Lexer_Init ( lexer, delimiters, lexer->State, SESSION ) ;
        lexer->TokenStart_ReadLineIndex = rl->ReadIndex ;
        while ( ( ! Lexer_CheckIfDone ( lexer, LEXER_DONE ) ) ) //&& ( ! ReadLiner_IsDone ( rl ) ) )
        {
            lexer->TokenInputCharacter = lexer->NextChar ( lexer->ReadLiner ) ;
            lexer->CurrentReadIndex = rl->ReadIndex ;
            Lexer_DoChar ( lexer ) ;
        }
        lexer->CurrentTokenDelimiter = lexer->TokenInputCharacter ;
        if ( lexer->TokenWriteIndex && ( ! GetState ( lexer, LEXER_RETURN_NULL_TOKEN ) ) )
        {
            _AppendCharacterToTokenBuffer ( lexer, 0 ) ; // null terminate TokenBuffer
            lexer->OriginalToken = TemporaryString_New ( lexer->TokenBuffer ) ; // SessionObjectsAllocate
            lexer->TokenEnd_ReadLineIndex = rl->ReadIndex ;
            _CfrTil_AddNewTokenSymbolToHeadOfTokenList ( lexer->OriginalToken ) ;
        }
        else
        {
            lexer->OriginalToken = ( byte * ) 0 ; // why not goto restartToken ? -- to allow user to hit newline and get response
        }
    }
    if ( Lexer_NextNonDelimiterChar ( _Context_->Lexer0 ) != '.' ) SetState ( _Context_, CONTEXT_PARSING_QUALIFIED_ID, false ) ;
    return lexer->OriginalToken ;
}

void
Lexer_ParseNextToken_WithDelimiters ( Lexer * lexer, byte * delimiters )
{
    _Lexer_ParseNextToken_WithDelimiters ( lexer, delimiters, 1 ) ;
}

byte *
_Lexer_ReadToken ( Lexer * lexer, byte * delimiters )
{
    Lexer_ParseNextToken_WithDelimiters ( lexer, delimiters ) ;
    return lexer->OriginalToken ;
}

byte *
Lexer_ReadToken ( Lexer * lexer )
{
    return _Lexer_ReadToken ( lexer, 0 ) ;
}

void
_Lexer_AppendCharacterToTokenBuffer ( Lexer * lexer )
{
    lexer->TokenBuffer [ lexer->TokenWriteIndex ++ ] = lexer->TokenInputCharacter ;
    lexer->TokenBuffer [ lexer->TokenWriteIndex ] = 0 ;
}

void
Lexer_Append_ConvertedCharacterToTokenBuffer ( Lexer * lexer )
{
    _String_AppendConvertCharToBackSlash ( TokenBuffer_AppendPoint ( lexer ), lexer->TokenInputCharacter ) ;
    _Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputCharacter ) ;
    lexer->TokenWriteIndex ++ ;
}

void
Lexer_AppendCharacterToTokenBuffer ( Lexer * lexer )
{
    _Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
    _Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputCharacter ) ;
}

byte
Lexer_UnAppendCharacterToTokenBuffer ( Lexer * lexer )
{
    return lexer->TokenBuffer [ -- lexer->TokenWriteIndex ] ;
}

byte
Lexer_LastChar ( Lexer * lexer )
{
    return lexer->TokenBuffer [ lexer->TokenWriteIndex - 1 ] ;
}

void
Lexer_SetTokenDelimiters ( Lexer * lexer, byte * delimiters, int32 allocType )
{
    if ( lexer->DelimiterCharSet ) CharSet_Init ( lexer->DelimiterCharSet, 128, delimiters ) ;
    else lexer->DelimiterCharSet = CharSet_New ( delimiters, allocType ) ;
    lexer->TokenDelimiters = delimiters ;
}

void
Lexer_SetBasicTokenDelimiters ( Lexer * lexer, byte * delimiters, int32 allocType )
{
    lexer->BasicDelimiterCharSet = CharSet_New ( delimiters, allocType ) ;
    lexer->BasicTokenDelimiters = delimiters ;
}

void
Lexer_Init ( Lexer * lexer, byte * delimiters, int32 state, int32 allocType )
{
    lexer->TokenBuffer = _CfrTil_->TokenBuffer ;
    Mem_Clear ( lexer->TokenBuffer, BUFFER_SIZE ) ;
    lexer->OriginalToken = 0 ;
    lexer->Literal = 0 ;
    if ( delimiters ) Lexer_SetTokenDelimiters ( lexer, delimiters, allocType ) ;
    else
    {
        lexer->DelimiterCharSet = lexer->BasicDelimiterCharSet ; //Lexer_SetTokenDelimiters ( lexer, " \n\r\t", allocType ) ;
        lexer->TokenDelimiters = lexer->BasicTokenDelimiters ;
    }
    lexer->State = state & ( ~ LEXER_RETURN_NULL_TOKEN ) ;
    Lexer_SetState ( lexer, KNOWN_OBJECT | LEXER_DONE | END_OF_FILE | END_OF_STRING | END_OF_LINE, false ) ;
    lexer->TokenDelimitersAndDot = ( byte* ) " .\n\r\t" ;
    RestartToken ( lexer ) ;
}

Lexer *
Lexer_New ( int32 allocType )
{
    Lexer * lexer = ( Lexer * ) _Mem_Allocate ( sizeof (Lexer ), allocType ) ;
    Lexer_Init ( lexer, 0, 0, allocType ) ;
    lexer->DelimiterOrDotCharSet = CharSet_New ( lexer->TokenDelimitersAndDot, allocType ) ;
    Lexer_SetBasicTokenDelimiters ( lexer, " \n\r\t", allocType ) ;
    lexer->ReadLiner = ReadLine_New ( allocType ) ;
    lexer->NextChar = _Lexer_NextChar ;
    return lexer ;
}

void
_Lexer_Copy ( Lexer * lexer, Lexer * lexer0, int32 allocType )
{
    memcpy ( lexer, lexer0, sizeof (Lexer ) ) ;
    Lexer_Init ( lexer, 0, 0, allocType ) ;
    lexer->NextChar = _Lexer_NextChar ;
}

Lexer *
Lexer_Copy ( Lexer * lexer0, int32 allocType )
{
    Lexer * lexer = ( Lexer * ) _Mem_Allocate ( sizeof (Lexer ), allocType ) ;
    _Lexer_Copy ( lexer, lexer0, allocType ) ;
    return lexer ;
}

#if 0

void
Lexer_Delete ( Lexer * lexer )
{
    Mem_FreeItem ( _Q_->PermanentMemList, ( byte* ) lexer ) ;
}
#endif

void
RestartToken ( Lexer * lexer )
{
    lexer->TokenWriteIndex = 0 ;
}

// special case here is quoted Strings - "String literals"
// use lexer->ReadLinePosition = 0 to cause a new Token read
// or lexer->Token_ReadLineIndex = BUFFER_SIZE

void
Lexer_SourceCodeOn ( Lexer * lexer )
{
    Lexer_SetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ), true ) ;
}

void
Lexer_SourceCodeOff ( Lexer * lexer )
{
    Lexer_SetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ), false ) ;
}

void
_Lexer_AppendCharToSourceCode ( Lexer * lexer, byte c )
{
    if ( Lexer_GetState ( lexer, ADD_CHAR_TO_SOURCE ) )
    {
        _CfrTil_AppendCharToSourceCode ( c ) ;
    }
}

void
Lexer_DoDelimiter ( Lexer * lexer )
{
    _Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputCharacter == '\n' ? ' ' : lexer->TokenInputCharacter ) ;
    // must have at least one non-delimiter to make a token
    // else keep going we just have multiple delimiters ( maybe just spaces ) in a row
    if ( lexer->TokenWriteIndex )
    {
        Lexer_SetState ( lexer, LEXER_DONE, true ) ;
        return ;
    }
    else
    {
        RestartToken ( lexer ) ; //prevent null token which complicates lexers
        return ;
    }
}

Boolean
Lexer_IsCurrentInputCharADelimiter ( Lexer * lexer )
{
    return ( Boolean ) _Lexer_IsCharDelimiter ( lexer, lexer->TokenInputCharacter ) ;
}

void
Lexer_Default ( Lexer * lexer )
{
    if ( Lexer_IsCurrentInputCharADelimiter ( lexer ) ) //_IsChar_Delimiter ( lexer->TokenDelimiters, lexer->TokenInputCharacter ) )
    {
        _Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputCharacter == '\n' ? ' ' : lexer->TokenInputCharacter ) ;
        //_Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputCharacter ) ; //== '\n' ? ' ' : lexer->TokenInputCharacter ) ;
        // must have at least one non-delimiter to make a token
        // else keep going we just have multiple delimiters ( maybe just spaces ) in a row
        if ( lexer->TokenWriteIndex )
        {
            Lexer_SetState ( lexer, LEXER_DONE, true ) ;
            return ;
        }
        else
        {
            RestartToken ( lexer ) ; //prevent null token which complicates lexers
            return ;
        }
    }
    Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
}

void
TerminatingMacro ( Lexer * lexer )
{
    if ( ! lexer->TokenWriteIndex ) Lexer_Default ( lexer ) ;
    else ReadLine_UnGetChar ( lexer->ReadLiner ) ; // so NextChar will have this TokenInputCharacter for the next token
    Lexer_FinishTokenHere ( lexer ) ;
    return ;
}

void
NonTerminatingMacro ( Lexer * lexer )
{
    Lexer_Default ( lexer ) ;
    if ( lexer->TokenWriteIndex == 1 )
    {
        byte chr = ReadLine_PeekNextChar ( lexer->ReadLiner ) ;
        if ( ( chr != 'x' ) && ( chr != 'X' ) && ( chr != 'b' ) && ( chr != 'o' ) && ( chr != 'd' ) ) Lexer_FinishTokenHere ( lexer ) ; // x/X : check for hexidecimal marker
    }
    return ;
}

int32
_Lexer_MacroChar_NamespaceCheck ( Lexer * lexer, byte * namespace )
{
    byte buffer [2] ;
    buffer [0] = lexer->TokenInputCharacter ;
    buffer [1] = 0 ;
    return _CfrTil_IsContainingNamespace ( buffer, namespace ) ;
}

void
Lexer_FinishTokenHere ( Lexer * lexer )
{
    _AppendCharacterToTokenBuffer ( lexer, 0 ) ; // null terminate TokenBuffer making LParen a token
    Lexer_SetState ( lexer, LEXER_DONE, true ) ;
    return ; // return "(" as the token
}

void
SingleEscape ( Lexer * lexer )
{
    lexer->TokenInputCharacter = ReadLine_NextChar ( lexer->ReadLiner ) ;
    Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
}

void
_MultipleEscape ( Lexer * lexer )
{
    byte multipleEscapeChar = lexer->TokenInputCharacter ;
    while ( 1 )
    {
        lexer->TokenInputCharacter = ReadLine_NextChar ( lexer->ReadLiner ) ;
        if ( lexer->TokenInputCharacter == multipleEscapeChar ) break ;
        Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
    }
    Lexer_SetState ( lexer, LEXER_DONE, true ) ;
}

// '"'

void
DoubleQuote ( Lexer * lexer )
{
#if 0
    if ( ( ! CompileMode ) && ( ! GetState ( _Context_->Compiler0, LC_ARG_PARSING ) ) && ( ! GetState ( lexer, ( ADD_TOKEN_TO_SOURCE | ADD_CHAR_TO_SOURCE ) ) ) )
    {
        CfrTil_InitSourceCode_WithCurrentInputChar ( ) ;
    }
#endif    
#if 1    
    TerminatingMacro ( lexer ) ;
#else    
    _Context_DoubleQuoteMacro ( _Context_ ) ;
#endif    
}


// '->' for pointers within a string and without surrounding spaces 

void
Minus ( Lexer * lexer ) // '-':
{
    //if ( ! Lexer_GetState ( lexer, PARSING_STRING ) ) // if we are not parsing a String ?
    {
        if ( lexer->TokenWriteIndex )
        {
            if ( ReadLine_PeekNextChar ( lexer->ReadLiner ) == '>' )
            {
                ReadLine_UnGetChar ( lexer->ReadLiner ) ; // allow to read '[' or ']' as next token
                Lexer_SetState ( lexer, LEXER_DONE, true ) ;

                return ;
            }
        }
    }
    Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
}

void
ForwardSlash ( Lexer * lexer ) // '/':
{
    Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
    if ( ReadLine_PeekNextChar ( lexer->ReadLiner ) == '/' )
    {
        lexer->NextChar ( lexer->ReadLiner ) ;
        Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
        Lexer_SetState ( lexer, LEXER_DONE, true ) ;
    }
}

#if 1

void
AddressOf ( Lexer * lexer ) // ';':
{
    //if ( GetState ( _Context_, C_SYNTAX ) && ( ReadLine_PeekNextChar ( lexer->ReadLiner ) != '&' ) ) TerminatingMacro ( lexer ) ;
    //if ( ( CharTable_IsCharType ( ReadLine_PeekNextChar ( lexer->ReadLiner ), CHAR_ALPHA ) && ( ReadLine_LastChar ( lexer->ReadLiner ) != '&' ) ) ) TerminatingMacro ( lexer ) ;
    if ( GetState ( _Context_, C_SYNTAX ) && CharTable_IsCharType ( ReadLine_PeekNextChar ( lexer->ReadLiner ), CHAR_ALPHA ) ) TerminatingMacro ( lexer ) ;
    else Lexer_Default ( lexer ) ;
}
#endif

void
Semi ( Lexer * lexer ) // ';':
{
    //if ( _CfrTil_AreWeInThisNamespace ( "C_Syntax" ) ) 
    if ( GetState ( _Context_, C_SYNTAX ) ) TerminatingMacro ( lexer ) ;
    else Lexer_Default ( lexer ) ;
}

void
GreaterThan ( Lexer * lexer ) // '>':
{
    if ( lexer->TokenWriteIndex )
    {
        if ( Lexer_LastChar ( lexer ) == '-' )
        {
            Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
            Lexer_SetState ( lexer, LEXER_DONE, true ) ;

            return ;
        }
    }
    Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
}

// package the dot to be lexed as a token
#if 1

void
Dot ( Lexer * lexer ) //  '.':
{
    if ( Lexer_LastChar ( lexer ) != '/' ) //allow for lisp special char sequence "/." as a substitution for lambda
    {
        int32 i ;
        if ( ( ! Lexer_GetState ( lexer, PARSING_STRING ) ) && ( ! GetState ( _Context_->Compiler0, PARSING_QUALIFIED_ID ) ) ) // if we are not parsing a String ?
        {
            if ( lexer->TokenWriteIndex )
            {
                for ( i = lexer->TokenWriteIndex - 1 ; i >= 0 ; i -- ) // go back into previous chars read, check if it is a number
                {
                    if ( ! isdigit ( lexer->TokenBuffer [ i ] ) )
                    {
                        ReadLine_UnGetChar ( lexer->ReadLiner ) ; // allow to read '.' as next token
                        //_CfrTil_UnAppendFromSourceCode ( 1 ) ;
                        Lexer_SetState ( lexer, LEXER_DONE, true ) ;
                        return ;
                    }
                }
            }
            else if ( ! isdigit ( ReadLine_PeekNextChar ( lexer->ReadLiner ) ) )
            {
                Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
                Lexer_SetState ( lexer, LEXER_DONE, true ) ;
                return ;
            }
        }
    }
    Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
}
#else

void
Dot ( Lexer * lexer ) //  '.':
{
    if ( lexer->TokenWriteIndex && ( isdigit ( lexer->TokenBuffer [ lexer->TokenWriteIndex ] ) ) ) Lexer_Default ( lexer ) ;
    else TerminatingMacro ( lexer ) ;
}
#endif

void
Lexer_DoReplMacro ( Lexer * lexer )
{
    ReadLine_UnGetChar ( lexer->ReadLiner ) ; // let the repl re-get the char 
    Lexer_FinishTokenHere ( lexer ) ;
    LO_ReadEvalPrint ( ) ;
    SetState ( lexer, LEXER_RETURN_NULL_TOKEN, true ) ;
    return ;
}

void
Lexer_CheckMacroRepl ( Lexer * lexer )
{
    //if ( _Lexer_MacroChar_Check ( _Context_->Lexer0, "Lisp" ) )
    {
        byte nextChar = ReadLine_PeekNextNonWhitespaceChar ( lexer->ReadLiner ) ;
        if ( ( nextChar == '(' ) || ( nextChar == ',' ) )
        {
            Lexer_DoReplMacro ( lexer ) ;
            return ;
        }
    }
}

void
Comma ( Lexer * lexer )
{
    if ( ! GetState ( _Context_->Compiler0, LC_ARG_PARSING ) )
    {
        if ( _Lexer_MacroChar_NamespaceCheck ( lexer, "Lisp" ) )
        {
            if ( _LC_ )
            {
                byte nextChar = ReadLine_PeekNextNonWhitespaceChar ( lexer->ReadLiner ) ;
                if ( nextChar == '@' )
                {
                    Lexer_AppendCharacterToTokenBuffer ( lexer ) ; // the comma
                    byte chr = _ReadLine_GetNextChar ( lexer->ReadLiner ) ; // the '@'
                    lexer->TokenInputCharacter = chr ;
                }
                Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
                Lexer_FinishTokenHere ( lexer ) ;

                return ;
            }
        }
    }
    Lexer_Default ( lexer ) ;
}

void
BackSlash ( Lexer * lexer )
{
    if ( ReadLine_PeekNextChar ( lexer->ReadLiner ) == 'n' )
    {
        _ReadLine_GetNextChar ( lexer->ReadLiner ) ;
        lexer->TokenInputCharacter = '\n' ;
    }
    else if ( ReadLine_PeekNextChar ( lexer->ReadLiner ) == 'r' )
    {
        _ReadLine_GetNextChar ( lexer->ReadLiner ) ;
        lexer->TokenInputCharacter = '\r' ;
    }
    else if ( ReadLine_PeekNextChar ( lexer->ReadLiner ) == 't' )
    {
        _ReadLine_GetNextChar ( lexer->ReadLiner ) ;
        lexer->TokenInputCharacter = '\t' ;
    }
    else if ( ReadLine_PeekNextChar ( lexer->ReadLiner ) == '\\' ) // current lisp lambda abbreviation "/\"
    {

        Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
    }
    //else lexer->TokenInputCharacter = _ReadLine_GetNextChar ( lexer->OurReadLiner ) ; //lexer->NextChar ( ) ;
    Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
}

void
CarriageReturn ( Lexer * lexer )
{

    NewLine ( lexer ) ;
}

void
NewLine ( Lexer * lexer )
{
    if ( ( ! _Context_->System0->IncludeFileStackNumber ) || GetState ( _CfrTil_->Debugger0, DBG_COMMAND_LINE ) )
    {
        Lexer_SetState ( lexer, LEXER_DONE | END_OF_LINE, true ) ;
        if ( lexer->OurInterpreter ) Interpreter_SetState ( lexer->OurInterpreter, INTERPRETER_DONE | END_OF_LINE, true ) ;
    }
    else
    {
        Lexer_SetState ( lexer, END_OF_LINE, true ) ;
        Lexer_Default ( lexer ) ;
    }
}

void
_EOF ( Lexer * lexer ) // case eof:
{

    if ( lexer->OurInterpreter ) Interpreter_SetState ( lexer->OurInterpreter, END_OF_FILE, true ) ;
    Lexer_SetState ( lexer, LEXER_DONE | END_OF_FILE, true ) ;
}

void
_Zero ( Lexer * lexer ) // case 0
{

    if ( lexer->OurInterpreter ) Interpreter_SetState ( lexer->OurInterpreter, END_OF_STRING, true ) ;
    Lexer_SetState ( lexer, LEXER_DONE | END_OF_STRING, true ) ;
}

#if 0

void
Bracket ( Lexer * lexer ) // ']':   case '[':
{
    if ( ! Lexer_GetState ( lexer, PARSING_STRING ) ) // if we are not parsing a String ?
    {
        if ( lexer->TokenWriteIndex )
        {
            ReadLine_UnGetChar ( lexer->OurReadLiner ) ; // allow to read '[' or ']' as next token
            Lexer_SetState ( lexer, LEXER_DONE, true ) ;
            return ;
        }
        else
        {
            Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
            //Lexer_SetState ( lexer, ADD_CHAR_TO_SOURCE, false ) ; // for lexer words
            Lexer_SetState ( lexer, LEXER_DONE, true ) ;

            return ;
        }
    }
    Lexer_AppendCharacterToTokenBuffer ( lexer ) ;
}
#endif

int32
Lexer_CheckIfDone ( Lexer * lexer, int32 flags )
{
    return lexer->State & flags ;
}

// the non-string lexer char input function

byte
_Lexer_NextChar ( ReadLiner * rl )
{
    return ReadLine_NextChar ( rl ) ;
}

void
Lexer_SetInputFunction ( Lexer * lexer, byte ( *lipf ) ( ) )
{
    lexer->NextChar = lipf ;
}

void
Lexer_DoChar ( Lexer * lexer )
{
#if 0    
    if ( lexer->TokenInputCharacter )
    {
        if ( Lexer_IsCurrentInputCharADelimiter ( lexer ) )
        {
            Lexer_DoDelimiter ( lexer ) ;

            return ;
        }
    }
#endif    
    _CfrTil_->LexerCharacterFunctionTable [ _CfrTil_->LexerCharacterTypeTable [ lexer->TokenInputCharacter ].CharInfo ] ( lexer ) ;
}

Boolean
Lexer_IsTokenQualifiedID ( Lexer * lexer )
{
    ReadLiner * rl = lexer->ReadLiner ;
    int32 i, end = rl->ReadIndex ; //lexer->TokenEnd_ReadLineIndex ;
    for ( i = end ; i < rl->MaxEndPosition ; i ++ )
    {
        if ( ! _Lexer_IsCharDelimiter ( lexer, rl->InputLine [ i ] ) ) break ;
    }
    for ( i ++ ; i < rl->MaxEndPosition ; i ++ )
    {
        if ( _Lexer_IsCharDelimiter ( lexer, rl->InputLine [ i ] ) ) break ;
        if ( rl->InputLine [ i ] == '.' )
        {
            for ( i ++ ; i < rl->MaxEndPosition ; i ++ )
            {
                if ( _Lexer_IsCharDelimiter ( lexer, rl->InputLine [ i ] ) ) return true ;
            }
        }
    }
    return false ;
}

Boolean
Lexer_IsTokenForwardDotted ( Lexer * lexer )
{
    ReadLiner * rl = lexer->ReadLiner ;
    int32 i, end = lexer->TokenEnd_ReadLineIndex ;
    for ( i = end ; i < rl->MaxEndPosition ; i ++ )
    {
        if ( rl->InputLine [ i ] == '.' ) return true ;
        if ( rl->InputLine [ i ] == '[' ) return true ;

        if ( isgraph ( rl->InputLine [ i ] ) ) break ;
    }
    return false ;
}

#if 0

Boolean
Lexer_IsDottedNamespace ( Lexer * lexer )
{
    ReadLiner * rl = lexer->OurReadLiner ;
    int32 i, start = lexer->TokenStart_ReadLineIndex, end = lexer->TokenEnd_ReadLineIndex ;
    for ( i = end ; i < rl->MaxEndPosition ; i ++ )
    {
        if ( rl->InputLine [ i ] == '.' ) return true ;
        if ( isgraph ( rl->InputLine [ i ] ) ) break ;
    }
    for ( i = start - 1 ; i > 0 ; i -- )
    {
        if ( rl->InputLine [ i ] == '.' ) return true ;
        if ( isgraph ( rl->InputLine [ i ] ) ) break ;
    }
    return false ;
}
#endif
/*
//    !, ", #, $, %, &, ', (, ), *, +, ,, -, ., /, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, :, ;, <, =, >, ?,
// @, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, _CfrTil, R, S, T, U, V, W, X, Y, Z, [, \, ], ^, _,
// `, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z, {, |, }, ~,
 */

