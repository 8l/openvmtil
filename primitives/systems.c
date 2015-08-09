#include "../includes/cfrtil.h"

// void getStdin(void) {Chr = getc(InFile), Env.put(Chr) ; }
// void putStdout(int c) {putc(c, OutFile);}
#if PICOLISP
extern int Chr ;

void
key ( )
{
    Chr = _Context_->Lexer0->NextChar ( ) ;
    //putc ( Chr, stdout ) ;
    //emit ( Chr ) ;
}

void
emit ( int c )
{
    putc ( Chr, stdout ) ;
    //Printf ( (byte*)"%c", (char) c ) ;
}
#endif

void
CfrTil_InitTime ( )
{
    System_InitTime ( _Context_->System0 ) ;
}

void
CfrTil_TimerInit ( )
{
    int32 timer = _DataStack_Pop ( ) ;
    if ( timer < 8 )
    {
        _System_TimerInit ( _Context_->System0, timer ) ;
    }
    else Throw ( ( byte* ) "Error: timer index must be less than 8", QUIT ) ;
}

void
CfrTil_Time ( )
{
    int32 timer = DataStack_Pop ( ) ;
    System_Time ( _Context_->System0, timer, ( char* ) "Timer", 1 ) ;
}

void
_ShellEscape ( char * str )
{
    int returned = system ( str ) ;
    if ( _Q_->Verbosity > 1 ) Printf ( c_dd ( "\nCfrTil : system ( \"%s\" ) returned %d.\n" ), str, returned ) ;
    D0 ( CfrTil_PrintDataStack ( ) ) ;
    Interpreter_SetState ( _Context_->Interpreter0, DONE, true ) ; // 
}

void
CfrTil_Throw ( )
{
    byte * msg = ( byte * ) DataStack_Pop ( ) ;
    Throw ( msg, 0 ) ;
}

void
ShellEscape ( )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    _ShellEscape ( ( CString ) & rl->InputLine [rl->ReadIndex] ) ;
    Lexer_SetState ( _Context_->Lexer0, LEXER_DONE, true ) ;
    Interpreter_SetState ( _Context_->Interpreter0, END_OF_STRING, true ) ;
}

void
CfrTil_Filename ( )
{
    byte * filename = _Context_->ReadLiner0->bp_Filename ;
    if ( ! filename ) filename = ( byte* ) "command line" ;
    _DataStack_Push ( ( int32 ) filename ) ;
}

void
CfrTil_LineNumber ( )
{
    _DataStack_Push ( ( int32 ) _Context_->ReadLiner0->i32_LineNumber ) ;
}

void
CfrTil_LineCharacterNumber ( )
{
    _DataStack_Push ( ( int32 ) _Context_->ReadLiner0->InputLineCharacterNumber ) ;
}

void
_CfrTil_Version ( int flag )
{
    if ( flag || ( ( _Q_->Verbosity ) && ( _Q_->StartedTimes == 1 ) ) )
    {
        AllowNewlines ;
        Printf ( ( byte* ) "\ncfrTil %s", _Q_->VersionString ) ;
    }
}

void
CfrTil_Version ( )
{
    _CfrTil_Version ( 1 ) ;
}

void
CfrTil_SystemState_Print ( )
{
    _CfrTil_SystemState_Print ( 1 ) ;
}

void
_SetEcho ( int32 boolFlag )
{
    SetState ( _Context_->ReadLiner0, CHAR_ECHO, boolFlag ) ;
    SetState ( _CfrTil_, READLINE_ECHO_ON, boolFlag ) ;
}

void
CfrTil_Echo ( )
{
    // toggle flag
    if ( CfrTil_GetState ( _CfrTil_, READLINE_ECHO_ON ) )
    {
        _SetEcho ( false ) ;
    }
    else
    {
        _SetEcho ( true ) ;
    }
}

void
CfrTil_EchoOn ( )
{
    _SetEcho ( true ) ;
}

void
CfrTil_EchoOff ( )
{
    _SetEcho ( false ) ;
}

// ?? optimize state should be in either CfrTil or OpenVmTil not System structure

void
CfrTil_NoOp ( void )
{
    //if ( CompileMode ) _Compile_Return ( ) ;
}

void
CfrTil_Hex ( ) // !
{

    _Context_->System0->NumberBase = 16 ;
}

void
CfrTil_Binary ( ) // !
{

    _Context_->System0->NumberBase = 2 ;
}

void
CfrTil_Decimal ( ) // !
{

    _Context_->System0->NumberBase = 10 ;
}

void
CfrTil_Dump ( )
{

    _CfrTil_Dump ( 8 ) ;
    Printf ( ( byte* ) "\n" ) ;
}

// this and the next function are under construction 

void
_CfrTil_PrintNReturnStack ( int32 size )
{
    // Intel SoftwareDevelopersManual-253665.pdf section 6.2 : a push decrements ESP, a pop increments ESP
    // therefore TOS is in lower mem addresses, bottom of stack is in higher memory addresses
    Buffer * b = Buffer_New ( BUFFER_SIZE ) ;
    int32 * esp, i, saveSize = size ;
    byte * buffer = Buffer_Data ( b ) ;
    _CfrTil_WordName_Run ( ( byte* ) "getESP" ) ;
    esp = ( int32 * ) _DataStack_Pop ( ) ;
    if ( esp )
    {
        Printf ( ( byte* ) "\nReturnStack   :%3i  : Esp (ESP) = " UINT_FRMT_0x08 " : Top = " UINT_FRMT_0x08 "", size, ( uint ) esp, ( uint ) esp ) ;
        // print return stack in reverse of usual order first
        for ( i = 0 ; size -- > 0 ; i ++ )
        {
            Word * word = Word_GetFromCodeAddress ( ( byte* ) ( esp [ size ] ) ) ;
            if ( word ) sprintf ( ( char* ) buffer, "< %s.%s >", word->ContainingNamespace->Name, word->Name ) ;
            Printf ( ( byte* ) "\n\t\t    ReturnStack   [ %3d ] < " UINT_FRMT_0x08 " > = " UINT_FRMT_0x08 "\t\t%s", size, ( uint ) & esp [ size ], esp [ size ], word ? ( char* ) buffer : "" ) ;
        }
        _Stack_PrintValues ( "ReturnStack", esp, saveSize ) ;
    }
    Buffer_SetAsUnused ( b ) ;
}

void
CfrTil_PrintNReturnStack ( )
{
    // Intel SoftwareDevelopersManual-253665.pdf section 6.2 : a push decrements ESP, a pop increments ESP
    // therefore TOS is in lower mem addresses, bottom of stack is in higher memory addresses
    int32 size = _DataStack_Pop ( ) ;
    _CfrTil_PrintNReturnStack ( size ) ;
}

void
CfrTil_PrintReturnStack ( )
{
    // Intel SoftwareDevelopersManual-253665.pdf section 6.2 : a push decrements ESP, a pop increments ESP
    // therefore TOS is in lower mem addresses, bottom of stack is in higher memory addresses
    _CfrTil_PrintNReturnStack ( 8 ) ;
}

void
CfrTil_PrintDataStack ( )
{
    CfrTil_SyncStackPointerFromDsp ( _CfrTil_ ) ;
    _Stack_Print ( _DataStack_, "DataStack" ) ;
    Printf ( ( byte* ) "\n" ) ;
}

void
CfrTil_CheckInitDataStack ( )
{
    CfrTil_SyncStackPointerFromDsp ( _CfrTil_ ) ;
    if ( Stack_Depth ( _DataStack_ ) < 0 )
    {
        _Stack_PrintHeader ( _DataStack_, "DataStack" ) ;
        Printf ( ( byte* ) c_dd( "\nReseting DataStack.\n") ) ;
        _CfrTil_DataStack_Init ( _CfrTil_ ) ;
        _Stack_PrintHeader ( _DataStack_, "DataStack" ) ;
    }
    Printf ( ( byte* ) "\n" ) ;
}

void
CfrTil_DataStack_Size ( )
{
    _DataStack_Push ( DataStack_Depth ( ) ) ;
}

void
CfrTil_Source_AddToHistory ( )
{
    Word *word = ( Word* ) _DataStack_Pop ( ) ;
    if ( word )
    {
        _CfrTil_Source ( word, 1 ) ;
    }
    else CfrTil_Exception ( NOT_A_KNOWN_OBJECT, QUIT ) ;
}

void
CfrTil_Source_DontAddToHistory ( )
{
    Word *word = ( Word* ) _DataStack_Pop ( ) ;
    if ( word )
    {
        _CfrTil_Source ( word, 0 ) ;
    }

    else CfrTil_Exception ( NOT_A_KNOWN_OBJECT, QUIT ) ;
}

void
CfrTil_AllocateNew ( )
{
    _DataStack_Push ( ( int32 ) _CfrTil_AllocateNew ( _DataStack_Pop ( ), OBJECT_MEMORY ) ) ;
}

void
CfrTil_SystemBreak ( )
{
    _OpenVmTil_SigLongJmp_WithMsg ( BREAK, ( byte* ) "System.interpreterBreak : returning to main interpreter loop." ) ;
}

void
CfrTil_Quit ( )
{
    _OpenVmTil_SigLongJmp_WithMsg ( QUIT, ( byte* ) "Quit : Session Memory, temporaries, are reset." ) ;
}

void
CfrTil_Abort ( )
{
    _OpenVmTil_SigLongJmp_WithMsg ( ABORT, ( byte* ) "Abort : Session Memory and the DataStack are reset (as in a cold restart)." ) ;
}

void
CfrTil_DebugStop ( )
{
    _OpenVmTil_SigLongJmp_WithMsg ( STOP, ( byte* ) "Stop : Debug Stop. " ) ;
}

void
CfrTil_ResetAll ( )
{
    _OpenVmTil_SigLongJmp_WithMsg ( RESET_ALL, ( byte* ) "ResetAll. " ) ;
}

void
CfrTil_Restart ( )
{
    _OpenVmTil_SigLongJmp_WithMsg ( RESTART, ( byte* ) "Restart. " ) ;
}

// cold init

void
CfrTil_RestartInit ( )
{
    _OpenVmTil_SigLongJmp_WithMsg ( FULL_RESTART, ( byte* ) "Restart Init... " ) ;
}

void
CfrTil_FullRestart ( )
{
    jmp_buf * sjb = & _Q_->JmpBuf0 ;
    _Q_->Signal = 0 ;
    _OpenVmTil_Throw ( sjb, ( byte* ) "Full Restart. ", INITIAL_START ) ;
}

void
CfrTil_WarmInit ( )
{
    _CfrTil_Init_SessionCore ( _CfrTil_, 1, 1 ) ;
}

void
CfrTil_Exit ( )
{

    if ( _Q_->Verbosity > 3 ) Printf ( ( byte* ) "\nbye\n" ) ;
    if ( _Q_->SignalExceptionsHandled ) _OpenVmTil_ShowExceptionInfo ( ) ;
    _ShowColors ( Color_Default, Color_Default ) ;
    exit ( 0 ) ;
}

void
CfrTil_ReturnFromFile ( )
{
    _EOF ( _Context_->Lexer0 ) ;
}

void
CfrTil_ShellEscape ( )
{
    _ShellEscape ( ( char* ) _DataStack_Pop ( ) ) ;
    NewLine ( _Context_->Lexer0 ) ;
}

