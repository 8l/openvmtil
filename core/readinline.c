#include "../includes/cfrtil.h"

byte *
_ReadLine_pb_NextChar ( ReadLiner * rl )
{
    if ( rl->ReadIndex < BUFFER_SIZE ) return &rl->InputLine [ rl->ReadIndex ] ;
    else return 0 ;
}

byte
_ReadLine_NextChar ( ReadLiner * rl )
{
    if ( rl->ReadIndex < BUFFER_SIZE ) return rl->InputLine [ rl->ReadIndex ] ;
    else return 0 ;
}

byte
ReadLine_PeekNextChar ( ReadLiner * rl )
{
    return _ReadLine_NextChar ( rl );
}

byte
_ReadLine_GetNextChar ( ReadLiner * rl )
{
    byte c = _ReadLine_NextChar ( rl ) ;
    if ( c ) rl->ReadIndex++ ;
    return c ;
}

void
_ReadLine_EndThisLine ( ReadLiner * rl )
{
    rl->ReadIndex = -1 ;
}

byte
ReadLine_CurrentReadChar ( ReadLiner * rl )
{
    return rl->InputLine [ rl->ReadIndex ] ;
}

byte *
ReadLine_BytePointerToCurrentReadChar ( ReadLiner * rl )
{
    return &rl->InputLine [ rl->ReadIndex ] ;
}

byte
ReadLine_LastChar ( ReadLiner * rl )
{
    return rl->InputLine [ rl->ReadIndex - 1 ] ;
}

byte
ReadLine_PeekNextNonWhitespaceChar ( ReadLiner * rl )
{
    int32 index = rl->ReadIndex ;
    byte atIndex = 0 ;
    do 
    {
        if ( index >= BUFFER_SIZE ) break ;
        atIndex = rl->InputLine [ index++ ] ;
    }
    while ( atIndex <= ' ' ) ;
    return atIndex ;
}

int
ReadLine_IsThereNextChar( ReadLiner * rl ) 
{
    if ( ! rl->InputLine ) return false ;// in case we are at in a _OpenVmTil_Pause
    char c = ReadLine_PeekNextChar ( rl ) ;
    return c || ( rl->InputFile && (rl->InputKeyedCharacter != eof) ) ; // || (c != '\n') ) ;
}

void
ReadLine_UnGetChar ( ReadLiner * rl )
{
    if ( rl->ReadIndex ) rl->ReadIndex -- ;
}

void
_ReadLine_ShowCharacter ( ReadLiner * rl, byte chr )
{
    if ( ReadLiner_GetState ( rl, CHAR_ECHO ) ) 
        putc ( ( char ) chr, rl->OutputFile ) ;
}

void
ReadLine_ShowCharacter ( ReadLiner * rl )
{
    _ReadLine_ShowCharacter ( rl, rl->InputKeyedCharacter ) ;
}

void
_ReadLine_SetMaxEndPosition ( ReadLiner * rl )
{
    //if ( overrideFlag ) rl->MaxEndPosition = rl->EndPosition ;
    if ( rl->EndPosition > rl->MaxEndPosition )
    {
        rl->MaxEndPosition = rl->EndPosition ;
    }
    //if ( rl->MaxEndPosition > 80 ) rl->MaxEndPosition = 80 ; // 80 : should be console char width
}

void
_ReadLine_SetEndPosition ( ReadLiner * rl )
{
    rl->EndPosition = strlen ( ( char* ) rl->InputLine ) ;
    _ReadLine_SetOutputLineCharacterNumber ( rl ) ;
    _ReadLine_SetMaxEndPosition ( rl ) ; // especially for the case of show history node
}

byte
_ReadLine_CharAtCursor ( ReadLiner * rl )
{
    return rl->InputLine [ rl->i32_CursorPosition ] ;
}

_ReadLine_CharAtACursorPos ( ReadLiner * rl, int32 pos  )
{
    return rl->InputLine [ pos ] ;
}

void
_ReadLine_CursorToEnd ( ReadLiner * rl )
{
    ReadLine_SetCursorPosition ( rl, rl->EndPosition ) ;
    rl->InputLine [ rl->i32_CursorPosition ] = 0 ;
}

void
_ReadLine_CursorToStart ( ReadLiner * rl )
{
    ReadLine_SetCursorPosition ( rl, 0 ) ;
}

void
_ReadLine_CursorRight ( ReadLiner * rl )
{
    ReadLine_SetCursorPosition ( rl, rl->i32_CursorPosition + 1 ) ;
}

void
_ReadLine_CursorLeft ( ReadLiner * rl )
{
    ReadLine_SetCursorPosition ( rl, rl->i32_CursorPosition - 1 ) ;
}

