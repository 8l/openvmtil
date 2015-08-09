
#include "../includes/cfrtil.h"

#define ReadLine_AppendPoint( rl ) &rl->InputBuffer [ rl->EndPosition ]

void
_ReadLine_NullDelimitInputBuffer ( ReadLiner * rl )
{
    rl->InputLine [ rl->EndPosition ] = 0 ;
}

void
_ReadLine_QuickAppendCharacter ( ReadLiner * rl, byte chr )
{
    rl->InputLine [ rl->EndPosition ++ ] = chr ;
    rl->InputLine [ rl->EndPosition ] = 0 ;
}

void
_ReadLine_SetOutputLineCharacterNumber ( ReadLiner * rl )
{
    rl->OutputLineCharacterNumber = strlen ( ( char* ) rl->Prompt ) + rl->EndPosition ;
}

void
__ReadLine_AppendCharacter ( ReadLiner * rl, byte chr )
{
    _ReadLine_QuickAppendCharacter ( rl, chr ) ;
    rl->EndPosition ++ ;
    _ReadLine_SetOutputLineCharacterNumber ( rl ) ;
    _ReadLine_SetMaxEndPosition ( rl ) ;
}

void
__ReadLine_AppendCharacter_Actual ( ReadLiner * rl, byte chr )
{
    _ReadLine_QuickAppendCharacter ( rl, chr ) ;
    _ReadLine_SetOutputLineCharacterNumber ( rl ) ;
    _ReadLine_SetMaxEndPosition ( rl ) ;
}

void
_ReadLine_AppendCharacter ( ReadLiner * rl )
{
    __ReadLine_AppendCharacter ( rl, rl->InputKeyedCharacter ) ;
}

void
_ReadLine_AppendCharacter_Actual ( ReadLiner * rl )
{
    __ReadLine_AppendCharacter_Actual ( rl, rl->InputKeyedCharacter ) ;
}

void
ReadLine_DoCursorMoveInput ( ReadLiner * rl, int32 newCursorPosition )
{
    ReadLine_ClearCurrentTerminalLine ( rl, rl->i32_CursorPosition ) ;
    ReadLine_SetCursorPosition ( rl, newCursorPosition ) ;
    ReadLine_ShowPad ( rl ) ;
    ReadLine_ShowCursor ( rl ) ;
}

#if 0
int32
ReadLine_PositionCursor ( ReadLiner * rl )
{
    int32 pos = rl->i32_CursorPosition ;
    while ( ( pos >= 0 ) && ( rl->InputLine [ pos ] == 0 ) ) 
        rl->InputLine [ --pos ] = ' ' ;
    return rl->i32_CursorPosition ; //= pos >= 0 ? pos : 0 ;
}
#endif

void
ReadLine_SetCursorPosition ( ReadLiner * rl, int32 pos )
{
    if ( pos < 0 ) pos = 0 ; // prompt width 
    rl->i32_CursorPosition = pos ;
}

void
ReadLiner_CommentToEndOfLine ( ReadLiner * rl )
{
    rl->ReadIndex = BUFFER_SIZE ; // cf. _ReadLine_GetNextChar
    ReadLiner_Done ( rl ) ;
}

void
ReadLiner_Done ( ReadLiner * rl )
{
    ReadLiner_SetState ( rl, READLINER_DONE, true ) ;
}

Boolean
ReadLiner_IsDone ( ReadLiner * rl )
{
    return ( ( ReadLiner_GetState ( rl, READLINER_DONE ) ) || ( rl->EndPosition >= BUFFER_SIZE ) || ( rl->ReadIndex >= BUFFER_SIZE ) ) ;
}

#define Cursor_Up( n ) _Printf ( "\r%c[%dA", ESC, n )

void
_ReadLine_MoveInputStartToLineStart ( int32 fromPosition )
{
    // nb. this is *necessary* when user scrolls up with scrollbar in eg. konsole and then hits up/down arrow
    int32 n, columns = GetTerminalWidth ( ) ;
    if ( fromPosition && columns )
    {
        n = ( fromPosition ) / ( columns ) ;
        if ( ( fromPosition % columns ) < 2 ) n -- ; // nb : ?? -- i don't understand this but it works
        if ( n ) Cursor_Up ( n ) ;//_Printf ( "\r%c[%dA", ESC, n ) ; // move n lines up 
    }
    else _Printf ( "\r" ) ; // nb -- a workaround : ?? second sequence ( clear 2 eol ) not necessary but seems to reset things to work -- ??
    //_Printf ( "\r%c[2K", ESC ) ; // nb -- a workaround : ?? second sequence ( clear 2 eol ) not necessary but seems to reset things to work -- ??
}

void
_ReadLine_PrintfClearTerminalLine ( )
{
    _Printf ( "\r%c[J", ESC ) ; // clear from cursor to end of screen -- important if we have (mistakenly) gone up an extra line
}

void
ReadLine_ClearCurrentTerminalLine ( ReadLiner * rl, int32 fromPosition )
{
    _ReadLine_MoveInputStartToLineStart ( fromPosition + PROMPT_LENGTH + 1 ) ; // 1 : zero array indexing
    _ReadLine_PrintfClearTerminalLine ( ) ;
}

void
ReadLine_SetInputLine ( ReadLiner * rl, byte * buffer )
{
    rl->InputLine = buffer ;
}

void
ReadLine_InputLine_Clear ( ReadLiner * rl )
{
    Mem_Clear ( rl->InputLine, BUFFER_SIZE ) ;
}

void
ReadLine_InputLine_Init ( ReadLiner * rl )
{
    ReadLine_InputLine_Clear ( rl ) ;
    rl->InputLineCharacterNumber = 0 ;
    rl->ReadIndex = 0 ;
    ReadLiner_SetState ( rl, READLINER_DONE, false ) ;
}

void
ReadLine_RunInit ( ReadLiner * rl )
{
    rl->HistoryNode = 0 ;
    rl->EscapeModeFlag = 0 ;
    _ReadLine_CursorToStart ( rl ) ;
    rl->EndPosition = 0 ;
    rl->MaxEndPosition = 0 ;
    ReadLiner_SetState ( rl, END_OF_INPUT | END_OF_LINE | TAB_WORD_COMPLETION, false ) ; // ?? here ??
    ReadLine_InputLine_Init ( rl ) ;
}

void
ReadLine_Init ( ReadLiner * rl, ReadLiner_KeyFunction ipf, int32 type )
{
    ReadLine_SetInputLine ( rl, Buffer_Data (_CfrTil_->InputLineB) ) ; // set where the actual memory buffer is located
    ReadLine_RunInit ( rl ) ;
    ReadLiner_SetState ( rl, CHAR_ECHO, true ) ; // this is how we see our input at the command line!
    rl->i32_LineNumber = 0 ;
    rl->InputFile = stdin ;
    rl->OutputFile = stdout ;
    rl->bp_Filename = 0 ;
    rl->FileCharacterNumber = 0 ;
    rl->NormalPrompt = ( byte* ) "<: " ;
    rl->AltPrompt = ( byte* ) ":> " ;
    rl->DebugPrompt = ( byte* ) "dbg=> " ;
    rl->Prompt = rl->NormalPrompt ;
    rl->InputStringOriginal = 0 ;
    if ( ipf ) ReadLine_SetRawInputFunction ( rl, ipf ) ;
}

ReadLiner *
ReadLine_New ( int32 type )
{
    ReadLiner * rl = ( ReadLiner * ) _Mem_Allocate ( sizeof (ReadLiner ), type ) ;
    rl->TabCompletionInfo0 = TabCompletionInfo_New ( type ) ;
    rl->TciNamespaceStack = Stack_New ( 64, SESSION ) ;
    //rl->TciDownStack = Stack_New ( 32, SESSION ) ;
    ReadLine_Init ( rl, _CfrTil_GetC, type ) ;
    return rl ;
}

void
_ReadLine_Copy ( ReadLiner * rl, ReadLiner * rl0, int32 type )
{
    memcpy ( rl, rl0, sizeof (ReadLiner ) ) ;
    rl->TabCompletionInfo0 = TabCompletionInfo_New ( type ) ;
    rl->TciNamespaceStack = Stack_New ( 64, SESSION ) ;
    //rl->TciDownStack = Stack_New ( 32, SESSION ) ;
    ReadLine_Init ( rl, rl0->Key, type ) ; //_CfrTil_GetC ) ;
    strcpy ( rl->InputLine, rl0->InputLine ) ;
    rl->InputStringOriginal = rl0->InputStringOriginal ;
    rl->State = rl0->State ;
}

ReadLiner *
ReadLine_Copy ( ReadLiner * rl0, int32 type )
{
    ReadLiner * rl = ( ReadLiner * ) _Mem_Allocate ( sizeof (ReadLiner ), type ) ;
    _ReadLine_Copy ( rl, rl0, type ) ;
    return rl ;
}

#if 0

void
ReadLine_Delete ( ReadLiner * rl )
{
    TabCompletionInfo_Delete ( rl->TabCompletionInfo0 ) ;
    Mem_FreeItem ( _Q_->PermanentMemList, ( byte* ) rl ) ;
}
#endif

void
ReadLine_TabWordCompletion ( ReadLiner * rl )
{
    if ( ! ReadLiner_GetState ( rl, TAB_WORD_COMPLETION ) )
    {
        TabCompletionInfo_Init ( ) ;
    }
    TabCompletion_Run ( ) ; // the main workhorse here
}

void
__ReadLine_AppendCharacterAndCursoRight ( ReadLiner * rl, byte c )
{
    __ReadLine_AppendCharacter ( rl, c ) ;
    _ReadLine_CursorRight ( rl ) ;
    _ReadLine_SetEndPosition ( rl ) ;
}

void
_ReadLine_AppendCharacterAndCursoRight ( ReadLiner * rl )
{
    __ReadLine_AppendCharacterAndCursoRight ( rl, rl->InputKeyedCharacter ) ;
}

void
_ReadLine_AppendAndShowCharacter ( ReadLiner * rl, byte c )
{
    __ReadLine_AppendCharacterAndCursoRight ( rl, c ) ;
    _ReadLine_ShowCharacter ( rl, c ) ;
}

void
ReadLine_AppendAndShowCharacter ( ReadLiner * rl )
{
    _ReadLine_AppendAndShowCharacter ( rl, rl->InputKeyedCharacter ) ;
}

byte *
ReadLine_GetPrompt ( ReadLiner * rl )
{
    if ( rl->i32_CursorPosition < rl->EndPosition ) rl->Prompt = ReadLine_GetAltPrompt ( rl ) ;
    else rl->Prompt = ReadLine_GetNormalPrompt ( rl ) ;
    return rl->Prompt ;
}

void
ReadLine_SetPrompt ( ReadLiner * rl, byte * newPrompt )
{
    rl->NormalPrompt = newPrompt ;
}

byte *
ReadLine_GetAltPrompt ( ReadLiner * rl )
{
    return (Debugger_GetState ( _CfrTil_->Debugger0, DBG_ACTIVE ) ? rl->DebugPrompt : rl->AltPrompt ) ;
}

byte *
ReadLine_GetNormalPrompt ( ReadLiner * rl )
{
    return (Debugger_GetState ( _CfrTil_->Debugger0, DBG_ACTIVE ) ? rl->DebugPrompt : rl->NormalPrompt ) ;
}

void
_ReadLine_Show ( ReadLiner * rl, byte * prompt )
{
    _Printf ( "\r%s%s", prompt, rl->InputLine ) ;
}

void
_ReadLine_ShowPad ( ReadLiner * rl, byte * prompt )
{
    _ReadLine_Show ( rl, prompt ) ;
    _ReadLine_SetEndPosition ( rl ) ;
}

void
ReadLine_ShowPad ( ReadLiner * rl )
{
    _ReadLine_ShowPad ( rl, ReadLine_GetPrompt ( rl ) ) ;
}

void
_ReadLine_ClearAndShowPad ( ReadLiner * rl, byte * prompt )
{
    ReadLine_ClearCurrentTerminalLine ( rl, rl->i32_CursorPosition ) ;
    _ReadLine_ShowPad ( rl, prompt ) ;
}

void
__ReadLine_DoStringInput ( ReadLiner * rl, byte * string, byte * prompt )
{
    ReadLine_ClearCurrentTerminalLine ( rl, rl->i32_CursorPosition ) ;
    ReadLine_InputLine_Clear ( rl ) ;
    strcpy ( ( char* ) rl->InputLine, ( char* ) string ) ;
    _ReadLine_ShowPad ( rl, prompt ) ;
}

void
ReadLine_ClearAndShowPad ( ReadLiner * rl )
{
    ReadLine_ClearCurrentTerminalLine ( rl, rl->i32_CursorPosition ) ;
    ReadLine_ShowPad ( rl ) ;
}

void
_ReadLine_ShowCursor ( ReadLiner * rl, byte * prompt )
{
    _ReadLine_MoveInputStartToLineStart ( rl->EndPosition + PROMPT_LENGTH + 1 ) ;
    byte saveChar = rl->InputLine [ rl->i32_CursorPosition ] ; // set up to show cursor at end of new word
    rl->InputLine [ rl->i32_CursorPosition ] = 0 ; // set up to show cursor at end of new word
    _ReadLine_Show ( rl, prompt ) ;
    rl->InputLine [ rl->i32_CursorPosition ] = saveChar ; // set up to show cursor at end of new word
}

void
ReadLine_ShowCursor ( ReadLiner * rl )
{
    _ReadLine_ShowCursor ( rl, ReadLine_GetPrompt ( rl ) ) ;
}

void
_ReadLine_DoStringInput ( ReadLiner * rl, byte * string, byte * prompt )
{
    __ReadLine_DoStringInput ( rl, string, prompt ) ;
    _ReadLine_ShowCursor ( rl, prompt ) ;
}

void
ReadLine_ShowStringWithCursor ( ReadLiner * rl, byte * string )
{
    _ReadLine_DoStringInput ( rl, string, ReadLine_GetPrompt ( rl ) ) ;
}

void
ReadLine_ClearAndShowPadWithCursor ( ReadLiner * rl )
{
    ReadLine_ClearAndShowPad ( rl ) ;
    ReadLine_ShowCursor ( rl ) ;
}

void
ReadLine_ShowNormalPrompt ( ReadLiner * rl )
{
    //_ReadLine_ShowStringWithCursor ( rl, ( byte* ) "", rl->NormalPrompt ) ;
    _ReadLine_PrintfClearTerminalLine ( ) ;
    _Printf ( "\r%s", rl->NormalPrompt ) ;
    rl->EndPosition = 0 ;
    rl->InputLine [ 0 ] = 0 ;
}

void
ReadLine_InsertCharacter ( ReadLiner * rl )
// insert rl->InputCharacter at cursorPostion
{
    String_InsertCharacter ( ( CString ) rl->InputLine, rl->i32_CursorPosition, rl->InputKeyedCharacter ) ;
    ReadLine_ClearAndShowPadWithCursor ( rl ) ;
}

void
ReadLine_SaveCharacter ( ReadLiner * rl )
{
    if ( rl->i32_CursorPosition < rl->EndPosition )
    {
        ReadLine_InsertCharacter ( rl ) ;
        _ReadLine_CursorRight ( rl ) ;
        ReadLine_ClearAndShowPadWithCursor ( rl ) ;
        return ;
    }
    ReadLine_AppendAndShowCharacter ( rl ) ;
}

void
_ReadLine_InsertStringIntoInputLineSlotAndShow ( ReadLiner * rl, int32 startOfSlot, int32 endOfSlot, byte * data )
{
    String_InsertDataIntoStringSlot ( ( CString ) rl->InputLine, startOfSlot, endOfSlot, ( CString ) data ) ; // size in bytes
    ReadLine_ClearAndShowPadWithCursor ( rl ) ;
}

void
ReadLiner_InsertTextMacro ( ReadLiner * rl, Word * word )
{
    int nlen = ( strlen ( ( char* ) word->Name ) + 1 ) ;
    String_InsertDataIntoStringSlot ( ( CString ) rl->InputLine, rl->ReadIndex - nlen, rl->ReadIndex, ( CString ) word->bp_WD_Object ) ; // size in bytes
    rl->ReadIndex -= nlen ;
    _CfrTil_UnAppendFromSourceCode ( nlen ) ;
}

void
ReadLine_DeleteChar ( ReadLiner * rl )
{
    Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;
    byte * b = Buffer_Data ( buffer ) ;
    if ( -- rl->EndPosition < 0 ) rl->EndPosition = 0 ;
    if ( rl->i32_CursorPosition > rl->EndPosition )// shouldn't ever be greater but this will be more robust
    {
        if ( -- rl->i32_CursorPosition < 0 ) _ReadLine_CursorToStart ( rl ) ;
    }
    rl->InputLine [ rl->i32_CursorPosition ] = 0 ;
    // prevent string overwriting itself while coping ...
    strcpy ( ( char* ) b, ( char* ) & rl->InputLine [ rl->i32_CursorPosition + 1 ] ) ;
    if ( rl->i32_CursorPosition < rl->EndPosition ) strcat ( ( char* ) rl->InputLine, ( char* ) b ) ;
    ReadLine_ClearAndShowPadWithCursor ( rl ) ;
    Buffer_SetAsUnused ( buffer ) ;
}

int32
ReadLine_IsLastCharADot ( ReadLiner * rl, int32 pos )
{
    return String_IsLastCharADot ( rl->InputLine, pos ) ;
}

int32
ReadLine_FirstCharOfToken_FromLastChar ( ReadLiner * rl, int32 pos )
{
    return String_FirstCharOfToken_FromPosOfLastChar ( rl->InputLine, pos ) ;
}

int32
ReadLine_IsThereADotSeparator ( ReadLiner * rl, int32 pos )
{
    String_IsThereADotSeparatorBackFromPosToLastNonDelmiter ( rl->InputLine, pos ) ;
}

int32
ReadLine_LastCharOfLastToken_FromPos ( ReadLiner * rl, int32 pos )
{
    return String_LastCharOfLastToken_FromPos ( rl->InputLine, pos ) ;
}

int32
ReadLine_EndOfLastToken ( ReadLiner * rl )
{
    return ReadLine_LastCharOfLastToken_FromPos ( rl, rl->i32_CursorPosition ) ;
}

int32
ReadLine_BeginningOfLastToken ( ReadLiner * rl )
{
    return ReadLine_FirstCharOfToken_FromLastChar ( rl, ReadLine_EndOfLastToken ( rl ) ) ;
}

Boolean
ReadLine_IsReverseTokenQualifiedID ( ReadLiner * rl )
{
#if 0    
    int32 lastChar = ReadLine_LastCharOfLastToken_FromPos ( rl, rl->ReadIndex ) ;
    int32 firstChar = ReadLine_FirstCharOfToken_FromLastChar ( rl, lastChar ) ;
    return ReadLine_IsThereADotSeparator ( rl, firstChar - 1 ) ;
#else    
    String_IsReverseTokenQualifiedID ( rl->InputLine, rl->ReadIndex ) ; //int32 pos ) ;
#endif    
}

byte
ReadLine_Key ( ReadLiner * rl )
{
    rl->InputKeyedCharacter = rl->Key ( rl ) ;
    rl->InputLineCharacterNumber ++ ;
    rl->FileCharacterNumber ++ ;
    return rl->InputKeyedCharacter ;
}

void
ReadLine_SetRawInputFunction ( ReadLiner * rl, ReadLiner_KeyFunction ripf )
{
    rl->Key = ripf ;
}

void
ReadLine_SetInputString ( ReadLiner * rl, byte * string )
{
    rl->InputStringOriginal = string ;
    rl->InputStringCurrent = rl->InputStringOriginal ;
    rl->InputStringIndex = 0 ;
}

void
_ReadLine_TabCompletion_Check ( ReadLiner * rl )
{
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    if ( rl->InputKeyedCharacter != '\t' )
    {
        if ( ReadLiner_GetState ( rl, TAB_WORD_COMPLETION ) )
        {
            ReadLiner_SetState ( rl, TAB_WORD_COMPLETION, false ) ;
            if ( ( rl->InputKeyedCharacter == ' ' ) && ( tci->TrialWord ) )
            {
                TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
                _TC_StringInsert_AtCursor ( tci, ( CString ) tci->TrialWord->Name ) ;
            }
            else if ( rl->InputKeyedCharacter == '\r' ) rl->InputKeyedCharacter = ' ' ; // leave line as is and append a space instead of '\r'
        }
    }
}

void
ReadLine_GetLine ( ReadLiner * rl )
// we're here until we get a newline char ( '\n' or '\r' ), a eof or a buffer overflow
// note : ReadLinePad [ 0 ] starts after the prompt ( "-: " | "> " ) and doesn't include them
{
    ReadLine_RunInit ( rl ) ;
    rl->LineStartFileIndex = rl->InputStringIndex ;
    while ( ! ReadLiner_IsDone ( rl ) )
    {
        ReadLine_Key ( rl ) ;
        if ( AtCommandLine ( rl ) ) _ReadLine_TabCompletion_Check ( rl ) ;
        _CfrTil_->ReadLine_FunctionTable [ _CfrTil_->ReadLine_CharacterTable [ rl->InputKeyedCharacter ] ] ( rl ) ;
        ReadLiner_SetState ( rl, ANSI_ESCAPE, false ) ;
        SetBuffersUnused ;
    }
}

byte
ReadLine_NextChar ( ReadLiner * rl )
{
    byte nchar = _ReadLine_GetNextChar ( rl ) ;
    if ( nchar ) return nchar ;
    else
    {
        if ( ReadLiner_GetState ( rl, STRING_MODE ) )
        {
            ReadLiner_SetState ( rl, STRING_MODE, false ) ; // only good once
            return nchar ;
        }
        else ReadLine_GetLine ( rl ) ; // get a line of characters
    }
    nchar = _ReadLine_GetNextChar ( rl ) ;
    return nchar ;
}

byte
ReadLine_GetNextCharFromString ( ReadLiner * rl )
{
    rl->InputStringIndex ++ ;
    return *( rl->InputStringCurrent ++ ) ;
}

void
Readline_Setup_OneStringInterpret ( ReadLiner * rl, byte * str )
{
    rl->ReadIndex = 0 ;
    ReadLiner_SetState ( rl, STRING_MODE, true ) ;
    ReadLine_SetInputLine ( rl, str ) ;
}

int32
_Readline_CheckArrayDimensionForVariables ( ReadLiner * rl )
{
    char *p, * ilri = & rl->InputLine [ rl->ReadIndex ], * prb = strchr ( &rl->InputLine [ rl->ReadIndex ], ']' ) ;
    if ( prb )
    {
        for ( p = ilri ; p != prb ; p ++ ) if ( isalpha ( * p ) ) return true ;
    }
    return false ;
}

