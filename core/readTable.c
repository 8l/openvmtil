#include "../includes/cfrtil.h"
#define ESC 27

#define _ReadLine_SetFinalCharacter( rl, chr ) rl->InputBuffer [ rl->EndPosition ] = chr 

void
CfrTil_ReadTables_Setup ( CfrTil * cfrl )
{
    int32 i ;
    for ( i = 0 ; i < 256 ; i ++ )
    {
        cfrl->ReadLine_CharacterTable [ i ] = 0 ;
    }
    cfrl->ReadLine_CharacterTable [ 0 ] = 21 ;
    cfrl->ReadLine_CharacterTable [ 127 ] = 1 ;
    cfrl->ReadLine_CharacterTable [ '\b' ] = 1 ;
    cfrl->ReadLine_CharacterTable [ '\t' ] = 2 ;
    cfrl->ReadLine_CharacterTable [ 3 ] = 3 ; // Ctrl-C
    cfrl->ReadLine_CharacterTable [ 4 ] = 4 ; // Ctrl-D
    cfrl->ReadLine_CharacterTable [ '\r' ] = 20 ; // remember this is raw char input : the lexer also handles some of these
    cfrl->ReadLine_CharacterTable [ '\n' ] = 5 ;
    cfrl->ReadLine_CharacterTable [ eof ] = 6 ;
    cfrl->ReadLine_CharacterTable [ ESC ] = 7 ;
    cfrl->ReadLine_CharacterTable [ '[' ] = 8 ;
    cfrl->ReadLine_CharacterTable [ '1' ] = 9 ;
    cfrl->ReadLine_CharacterTable [ '4' ] = 10 ;
    cfrl->ReadLine_CharacterTable [ 'A' ] = 11 ;
    cfrl->ReadLine_CharacterTable [ 'B' ] = 12 ;
    cfrl->ReadLine_CharacterTable [ 'C' ] = 13 ;
    cfrl->ReadLine_CharacterTable [ 'D' ] = 14 ;
    cfrl->ReadLine_CharacterTable [ '3' ] = 15 ;
    cfrl->ReadLine_CharacterTable [ 'H' ] = 16 ;
    cfrl->ReadLine_CharacterTable [ 'O' ] = 17 ;
    cfrl->ReadLine_CharacterTable [ 'F' ] = 18 ;
    cfrl->ReadLine_CharacterTable [ '~' ] = 19 ;
    cfrl->ReadLine_CharacterTable [ '(' ] = 22 ;
    //cfrl->ReadLine_CharacterTable [ '\\' ] = 20 ; // the lexer handles this

    cfrl->ReadLine_FunctionTable [ 0 ] = ReadTable_Default ;
    cfrl->ReadLine_FunctionTable [ 1 ] = ReadTable_BackSpace ;
    cfrl->ReadLine_FunctionTable [ 2 ] = ReadTable_Tab ;
    cfrl->ReadLine_FunctionTable [ 3 ] = ReadTable_0x03 ; // Ctrl-C
    cfrl->ReadLine_FunctionTable [ 4 ] = ReadTable_0x04 ; // Ctrl-D
    cfrl->ReadLine_FunctionTable [ 5 ] = ReadTable_Newline ;
    cfrl->ReadLine_FunctionTable [ 6 ] = ReadTable_EOF ;
    cfrl->ReadLine_FunctionTable [ 7 ] = ReadTable_ESC ;
    cfrl->ReadLine_FunctionTable [ 8 ] = ReadTable_LeftBracket ;
    cfrl->ReadLine_FunctionTable [ 9 ] = ReadTable_1 ; // ESC[1 Home ;
    cfrl->ReadLine_FunctionTable [ 10 ] = ReadTable_4 ; // ESC[4 End ;
    cfrl->ReadLine_FunctionTable [ 11 ] = ReadTable_A ; // ESC[A Up Arrow ;
    cfrl->ReadLine_FunctionTable [ 12 ] = ReadTable_B ; // ESC[B Down Arrow ;
    cfrl->ReadLine_FunctionTable [ 13 ] = ReadTable_C ; // ESC[C Right Arrow ;
    cfrl->ReadLine_FunctionTable [ 14 ] = ReadTable_D ; // ESC[D Left Arrow ;
    cfrl->ReadLine_FunctionTable [ 15 ] = ReadTable_3 ; // part of Delete escape sequence ;
    cfrl->ReadLine_FunctionTable [ 16 ] = ReadTable_H ; // ;
    cfrl->ReadLine_FunctionTable [ 17 ] = ReadTable_O ; // ESC[O Home ;
    cfrl->ReadLine_FunctionTable [ 18 ] = ReadTable_F ; // End ;
    cfrl->ReadLine_FunctionTable [ 19 ] = ReadTable_Tilde ; // ESC[~ Delete ;
    cfrl->ReadLine_FunctionTable [ 20 ] = ReadTable_CarriageReturn ; // '\r'
    cfrl->ReadLine_FunctionTable [ 21 ] = ReadTable_Zero ;
    cfrl->ReadLine_FunctionTable [ 22 ] = ReadTable_LParen ;
}

void
ReadTable_Default ( ReadLiner * rl )
{
    rl->EscapeModeFlag = 0 ;
    ReadLine_SaveCharacter ( rl ) ;
}

void
ReadTable_LParen ( ReadLiner * rl )
{
#if MARU || MARU_2_4 || MARU_NILE
    if ( ( rl->InputFile != stdin ) && _CfrTil_->InNamespace )
    {
        if ( String_Equal ( ( CString ) _CfrTil_->InNamespace->s_Symbol.S_Name, "Maru" ) )
        {
            ungetc ( rl->InputKeyedCharacter, rl->InputFile ) ;
            Maru_RawReadFlag = 1 ;
            imaru_nile ( ) ;
            Maru_RawReadFlag = 0 ;
            return ;
        }
    }
#endif    
    ReadTable_Default ( rl ) ;
}

void
ReadTable_Tab ( ReadLiner * rl ) // '\t':
{
    if ( AtCommandLine ( rl ) )
    {
        ReadLine_TabWordCompletion ( rl ) ;
    }
    else ReadLine_SaveCharacter ( rl ) ;
}

void
ReadTable_0x03 ( ReadLiner * rl ) //  <CTRL-C>
{
    CfrTil_Quit ( ) ;
}

void
ReadTable_0x04 ( ReadLiner * rl ) // <CTRL-D>
{
    CfrTil_RestartInit ( ) ;
}

void
ReadTable_CarriageReturn ( ReadLiner * rl ) // '\r' 
{
    rl->InputKeyedCharacter = '\n' ; // convert '\r' to '\n'
    ReadTable_Newline ( rl ) ;
}

void
ReadTable_Newline ( ReadLiner * rl ) // \n'
{
    if ( System_GetState ( _Context_->System0, ADD_READLINE_TO_HISTORY ) || ReadLiner_GetState ( rl, ADD_TO_HISTORY ) )
        _OpenVmTil_AddStringToHistoryList ( rl->InputLine ) ;
    rl->i32_LineNumber ++ ;
    _ReadLine_AppendCharacter_Actual ( rl ) ;
    ReadLine_ShowCharacter ( rl ) ;
    ReadLiner_Done ( rl ) ;
}

void
ReadTable_Zero ( ReadLiner * rl ) // eof
{
    _ReadLine_NullDelimitInputBuffer ( rl ) ;
    ReadLiner_Done ( rl ) ;
    ReadLiner_SetState ( rl, END_OF_STRING, true ) ;
}

//ReadTable_0 255:

void
ReadTable_EOF ( ReadLiner * rl ) // eof
{
    _ReadLine_AppendCharacter_Actual ( rl ) ;
    ReadLiner_Done ( rl ) ;
    ReadLiner_SetState ( rl, END_OF_FILE, true ) ;
}

void
ReadTable_ESC ( ReadLiner * rl ) // 27 - ESC '^'
{
    rl->EscapeModeFlag = 1 ;
    ReadLine_ShowCharacter ( rl ) ; // does the escape action
}

void
ReadTable_LeftBracket ( ReadLiner * rl ) // '['  91 - second char of standard escape sequence
{
    if ( rl->EscapeModeFlag == 1 )
    {
        rl->EscapeModeFlag = 2 ;
    }
    else ReadLine_SaveCharacter ( rl ) ;
}

// end of char sequence - 'ESC [ A'

void
ReadTable_A ( ReadLiner * rl ) // 'A' - back in history - UP arrow - ESC[A toward Head of list
{
    if ( rl->EscapeModeFlag == 2 )
    {
        DLNode * node = DLList_Before ( _HistorySpace_.StringList ) ;
        if ( node )
        {
            rl->HistoryNode = ( HistoryStringNode* ) node ;
        }
        else rl->HistoryNode = 0 ;
        ReadLine_ShowHistoryNode ( rl ) ;
    }
    else ReadLine_SaveCharacter ( rl ) ;
}

void
ReadTable_B ( ReadLiner * rl )// 'B' - forward, toward end of history - DOWN arrow - ESC[B - ^[[B - towards Tail of List
{
    if ( rl->EscapeModeFlag == 2 )
    {
        DLNode * node = DLList_After ( _HistorySpace_.StringList ) ;
        if ( node )
        {
            rl->HistoryNode = ( HistoryStringNode* ) node ;
        }
        else rl->HistoryNode = 0 ;
        ReadLine_ShowHistoryNode ( rl ) ;
    }
    else ReadLine_SaveCharacter ( rl ) ;
}

void
ReadTable_C ( ReadLiner * rl ) // 'C' - ^[C = right arrow
{
    if ( rl->EscapeModeFlag == 2 )
    {
        rl->EscapeModeFlag = 0 ;
        if ( rl->i32_CursorPosition >= rl->EndPosition )
        {
            __ReadLine_AppendCharacter ( rl, ( byte ) ' ' ) ;
        }
        ReadLine_DoCursorMoveInput ( rl, rl->i32_CursorPosition + 1 ) ;
    }
    else ReadLine_SaveCharacter ( rl ) ;
}

void
ReadTable_D ( ReadLiner * rl ) // 'D' - ^[D = left arrow
{
    if ( rl->EscapeModeFlag == 2 )
    {
        rl->EscapeModeFlag = 0 ;
        if ( rl->i32_CursorPosition > 0 )
        {
            ReadLine_DoCursorMoveInput ( rl, rl->i32_CursorPosition - 1 ) ;
        }
        else return ;
    }
    else ReadLine_SaveCharacter ( rl ) ;
}

void
ReadTable_F ( ReadLiner * rl ) // 'F' - End key
{
    if ( rl->EscapeModeFlag == 2 )
    {
        rl->EscapeModeFlag = 0 ;
        ReadLine_DoCursorMoveInput ( rl, rl->EndPosition ) ;
    }
    else if ( rl->EscapeModeFlag == 1 )
    {
        rl->EscapeModeFlag = 2 ;
    }
    else ReadLine_SaveCharacter ( rl ) ;
}

void
ReadTable_H ( ReadLiner * rl ) // 'H' - Home Delete almost - see '~'
{
    if ( rl->EscapeModeFlag == 2 )
    {
        rl->EscapeModeFlag = 0 ;
        ReadLine_DoCursorMoveInput ( rl, 0 ) ;
    }
    else ReadLine_SaveCharacter ( rl ) ;
}

void
ReadTable_4 ( ReadLiner * rl ) // ESC[4~ End
{
    if ( rl->EscapeModeFlag == 2 )
    {
        rl->EscapeModeFlag = 7 ;
    }
    else ReadLine_SaveCharacter ( rl ) ;
}

void
ReadTable_3 ( ReadLiner * rl ) // ESC[3~ - Delete almost - see '~'
{
    if ( rl->EscapeModeFlag == 2 )
    {
        rl->EscapeModeFlag = 6 ;
    }
    else ReadLine_SaveCharacter ( rl ) ;
}

void
ReadTable_1 ( ReadLiner * rl ) // ESC[1~ Home
{
    if ( rl->EscapeModeFlag == 2 )
    {
        rl->EscapeModeFlag = 4 ;
    }
    else ReadLine_SaveCharacter ( rl ) ;
}

void
ReadTable_O ( ReadLiner * rl ) // 'O' - End key
{
    if ( rl->EscapeModeFlag == 2 )
    {
        rl->EscapeModeFlag = 3 ;
    }
    else ReadLine_SaveCharacter ( rl ) ;
}

void
ReadTable_Tilde ( ReadLiner * rl ) // '~' - Delete
{
    if ( rl->EscapeModeFlag == 3 ) // end ??
    {
        rl->EscapeModeFlag = 0 ;
        ReadLine_DoCursorMoveInput ( rl, rl->EndPosition ) ;
    }
    else if ( rl->EscapeModeFlag == 4 ) // home
    {
        rl->EscapeModeFlag = 0 ;
        ReadLine_DoCursorMoveInput ( rl, 0 ) ;
    }
    else if ( rl->EscapeModeFlag == 6 ) // delete
    {
        rl->EscapeModeFlag = 0 ;
        ReadLine_DeleteChar ( rl ) ;
    }
    else if ( rl->EscapeModeFlag == 7 ) // end
    {
        rl->EscapeModeFlag = 0 ;
        ReadLine_DoCursorMoveInput ( rl, rl->EndPosition ) ;
    }
    else ReadLine_SaveCharacter ( rl ) ;
}

void
ReadTable_BackSpace ( ReadLiner * rl ) // '\b' 127
{
    ReadLine_SetCursorPosition ( rl, rl->i32_CursorPosition - 1 ) ;
    ReadLine_DeleteChar ( rl ) ;
}

