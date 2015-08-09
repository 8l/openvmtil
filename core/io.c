
#include "../includes/cfrtil.h"

int32
GetTerminalWidth ( )
{
#ifdef TIOCGSIZE
    struct ttysize ts ;
    ioctl ( STDIN_FILENO, TIOCGSIZE, &ts ) ;
    //cols = ts.ts_cols ;
    //lines = ts.ts_lines;
    return ts.ts_cols ;
#elif defined(TIOCGWINSZ)
    struct winsize ts ;
    ioctl ( STDIN_FILENO, TIOCGWINSZ, &ts ) ;
    //ioctl ( STDOUT_FILENO, TIOCGWINSZ, &ts ) ;
    //cols = ts.ws_col ;
    //lines = ts.ws_row;
    return ts.ws_col ;
#endif /* TIOCGSIZE */
}

char
kbhit ( void )
{
    int oldf ;

    oldf = fcntl ( STDIN_FILENO, F_GETFL, 0 ) ;
    fcntl ( STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK ) ;
    char ch = getchar ( ) ;
    if ( ch < 0 ) ch = 0 ;
    fcntl ( STDIN_FILENO, F_SETFL, oldf ) ;
    return ch ;
}

int
_Key ( FILE * f )
{
    int key = getc ( f ) ;
    if ( kbhit ( ) == ESC ) OpenVmTil_Pause ( ) ; //CfrTil_Quit ( ) ;
    return key ;
}

int
Key ( )
{
    return _Key ( stdin ) ;
}

void
_DoPrompt ( )
{
    Printf ( ( byte* ) "\n" ) ;
    _ReadLine_PrintfClearTerminalLine ( ) ;
    Printf ( ( byte* ) "%s", ( char* ) _Context_->ReadLiner0->NormalPrompt ) ; // for when including files
}

void
_CfrTil_Prompt ( int32 control )
{
    if ( ( control && ( ! _Context_->System0->IncludeFileStackNumber ) &&
        ( _Context_->ReadLiner0->OutputLineCharacterNumber != ( int32 ) strlen ( ( char* ) _Context_->ReadLiner0->Prompt ) ) ) ||
        ( Debugger_GetState ( _CfrTil_->Debugger0, DBG_ACTIVE ) ) )
    {
        _DoPrompt ( ) ;
    }
}

byte
_CfrTil_GetC ( ReadLiner * rl )
{
    return getc ( rl->InputFile ) ;
}

// all output comes thru here

void
_Printf ( byte *format, ... )
{
    va_list args ;
    va_start ( args, ( char* ) format ) ;
    vprintf ( ( char* ) format, args ) ;
    if ( _CfrTil_ && _CfrTil_->LogFlag ) vfprintf ( _CfrTil_->LogFILE, ( char* ) format, args ) ;
    va_end ( args ) ;
    fflush ( stdout ) ;
}

// try not to (don't) print extra newlines
// this is called on exceptions so alot of checking 

void
Printf ( byte *format, ... )
{
    if ( kbhit ( ) == ESC ) OpenVmTil_Pause ( ) ; //CfrTil_Quit ( ) ;
    if ( _Q_ && _Q_->Verbosity )
    {
        va_list args ;
        va_start ( args, ( char* ) format ) ;
        char * out = ( char* ) Buffer_Data ( _CfrTil_->PrintfB ) ;
        vsprintf ( ( char* ) out, ( char* ) format, args ) ;
        va_end ( args ) ;
        int32 len = strlen ( ( char* ) out ) ;
        byte final = out [ len - 1 ] ;
        if ( _Q_->psi_PrintStateInfo )
        {
            if ( out [0] == '\n' )
            {
                if ( ( ! _Q_->psi_PrintStateInfo->OutputLineCharacterNumber ) && ( PrintStateInfo_GetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE ) ) )
                    out = & out [1] ;
                else if ( _Q_->psi_PrintStateInfo && PrintStateInfo_GetState ( _Q_->psi_PrintStateInfo, PSI_PROMPT ) )
                {
                    out [0] = '\r' ;
                    PrintStateInfo_SetState ( _Q_->psi_PrintStateInfo, PSI_PROMPT, false ) ;
                }
            }
        }
        printf ( "%s", out ) ;
        if ( _CfrTil_ && _CfrTil_->LogFlag ) fprintf ( _CfrTil_->LogFILE, "%s", out ) ;
        if ( _Q_->psi_PrintStateInfo )
        {
            if ( ( final == '\n' ) || ( final == '\r' ) )
            {
                _Q_->psi_PrintStateInfo->OutputLineCharacterNumber = 0 ;
                ConserveNewlines ;
            }
            else
            {
                _Q_->psi_PrintStateInfo->OutputLineCharacterNumber += len ;
                AllowNewlines ;
            }
        }
        fflush ( stdout ) ;
    }
}

PrintStateInfo *
PrintStateInfo_New ( )
{
    PrintStateInfo * psi = ( PrintStateInfo * ) _Mem_Allocate ( sizeof ( PrintStateInfo ), OPENVMTIL ) ;
    //PrintStateInfo * psi = ( PrintStateInfo * ) MemList_AllocateChunk ( &_MemList_, sizeof ( PrintStateInfo ), OPENVMTIL ) ; ;
    PrintStateInfo_SetState ( psi, PSI_PROMPT, false ) ;
    PrintStateInfo_SetState ( psi, PSI_NEWLINE, true ) ;
    return psi ;
}

#if LISP_IO

byte *
_vprintf ( FILE * f, char *format, ... )
{
    va_list args ;
    va_start ( args, ( char* ) format ) ;
    __Printf ( ( byte* ) format, args ) ;
}

unsigned int
Getc ( FILE * f )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    if ( f != stdin ) return fgetc ( f ) ;
    if ( Maru_RawReadFlag ) return ReadLine_Key ( rl ) ;
    else return ( int ) ReadLine_NextChar ( rl ) ;
}

unsigned int
Getwc ( FILE * f )
{
    return btowc ( Getc ( f ) ) ;
}

void
UnGetc ( int c, FILE * f )
{
    if ( f == stdin )
        ReadLine_UnGetChar ( _Context_->ReadLiner0 ) ;
    else ungetc ( c, f ) ;
}

void
UnGetwc ( int c, FILE * f )
{
    return UnGetc ( wctob ( c ), f ) ;
}
#endif

#if 0

void
__CfrTil_Emit ( byte c )
{
    if ( ( c == '\n' ) || ( c == '\r' ) )
    {
        if ( _Context_->ReadLiner0->OutputLineCharacterNumber == 0 ) return ;
        else
        {
            //if ( ! overWrite ) 
            c = '\n' ; // don't overwrite the existing line
            _Context_->ReadLiner0->OutputLineCharacterNumber = 0 ;
        }
    }
    else _Context_->ReadLiner0->OutputLineCharacterNumber ++ ;
    if ( _Q_->Verbosity ) putc ( c, _Context_->ReadLiner0->OutputFile ) ;
}

void
_CfrTil_EmitString ( byte * string )
{
#if 1
    int32 i ;
    //if ( _Context->ReadLiner0->Flags & CHAR_ECHO )
    {
        for ( i = 0 ; string [ i ] ; i ++ )
        {
            if ( kbhit ( ) == ESC ) CfrTil_Quit ( ) ;
            __CfrTil_Emit ( string [ i ] ) ;
        }
    }
#else
    if ( kbhit ( ) == ESC ) CfrTil_Quit ( ) ;
    puts ( ( char* ) string ) ;
#endif
}
#endif

