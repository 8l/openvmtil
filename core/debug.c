
#include "../includes/cfrtil.h"

byte *
Debugger_GetStateString ( Debugger * debugger )
{
    //byte * buffer = Buffer_New_pbyte ( BUFFER_SIZE ) ;
    char * buffer = ( char* ) Buffer_Data ( _CfrTil_->DebugB ) ;
    sprintf ( ( char* ) buffer, "%s : %s : %s",
        GetState ( debugger, DBG_STEPPING ) ? "Stepping" : ( CompileMode ? ( char* ) "Compiling" : ( char* ) "Interpreting" ),
        ( CfrTil_GetState ( _CfrTil_, INLINE_ON ) ? ( char* ) "InlineOn" : ( char* ) "InlineOff" ),
        ( GetState ( _CfrTil_, OPTIMIZE_ON ) ? ( char* ) "OptimizeOn" : ( char* ) "OptimizeOff" )
        ) ;
    buffer = String_New ( ( byte* ) buffer, TEMPORARY ) ;
    return buffer ;
}

void
Debugger_CanWeStep ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    Debugger_SetState ( debugger, DBG_CAN_STEP, false ) ; // debugger->State flag = false ;
    if ( word ) // then it wasn't a literal
    {
        if ( ! ( word->CType & ( CPRIMITIVE | DLSYM_WORD ) ) ) Debugger_SetState ( debugger, DBG_CAN_STEP, true ) ;
    }
}

void
Debugger_NextToken ( Debugger * debugger )
{
    if ( ReadLine_IsThereNextChar ( _Context_->ReadLiner0 ) ) debugger->Token = Lexer_ReadToken ( _Context_->Lexer0 ) ;
    else debugger->Token = 0 ;
}

void
Debugger_CurrentToken ( Debugger * debugger )
{
    debugger->Token = _Context_->Lexer0->OriginalToken ;
}

void
Debugger_Parse ( Debugger * debugger )
{
    Lexer_ParseObject ( _Context_->Lexer0, _Context_->Lexer0->OriginalToken ) ;
}

void
_Debugger_FindAny ( Debugger * debugger )
{
    if ( debugger->Token ) debugger->w_Word = _CfrTil_FindInAnyNamespace ( debugger->Token ) ;
}

void
Debugger_FindAny ( Debugger * debugger )
{
    _Debugger_FindAny ( debugger ) ;
    if ( debugger->w_Word ) Printf ( ( byte* ) ( byte* ) "\nFound Word :: %s.%s\n", _Context_->Finder0->FoundWordNamespace->Name, debugger->w_Word->Name ) ;
    else Printf ( ( byte* ) ( byte* ) "\nToken not found : %s\n", debugger->Token ) ;
}

void
Debugger_FindUsing ( Debugger * debugger )
{
    if ( debugger->Token )
    {
        debugger->w_Word = Finder_Word_FindUsing ( _Context_->Finder0, debugger->Token ) ;
        debugger->WordDsp = Dsp ;
    }

}

void
Debugger_Variables ( Debugger * debugger )
{
    CfrTil_Variables ( ) ;
}

void
Debugger_InterpretTokenWriteCode ( Debugger * debugger )
{
    Interpreter * interp = _Context_->Interpreter0 ;
    debugger->SaveDsp = Dsp ;
    debugger->PreHere = Here ;
    debugger->WordDsp = Dsp ;
    debugger->SaveTOS = TOS ;
    debugger->SaveStackDepth = DataStack_Depth ( ) ;
    DefaultColors ;
    if ( ( debugger->w_Word ) && ( ! ( debugger->w_Word->CType & LITERAL ) ) ) _Interpreter_Do_MorphismWord ( interp, debugger->w_Word ) ;
    else interp->w_Word = Lexer_ObjectToken_New ( _Context_->Lexer0, debugger->Token, 1 ) ; //_Interpreter_InterpretAToken ( interp, debugger->Token ) ;
    DebugColors ;
    if ( ! debugger->w_Word ) debugger->w_Word = interp->w_Word ;
    Debugger_ShowWrittenCode ( debugger, 0 ) ;
    debugger->Token = 0 ;
    debugger->DebugAddress = 0 ;
}

void
Debugger_InterpretWord ( Debugger * debugger )
{
    if ( debugger->w_Word )
    {
        debugger->Token = debugger->w_Word->Name ;
        Debugger_InterpretTokenWriteCode ( debugger ) ;
    }
}

// by 'eval' we stop debugger->Stepping and //continue thru this word as if we hadn't stepped

void
Debugger_Eval ( Debugger * debugger )
{
    debugger->SaveStackDepth = DataStack_Depth ( ) ;
    if ( Debugger_IsStepping ( debugger ) )
    {
        Debugger_Stepping_Off ( debugger ) ;
        Debugger_Info ( debugger ) ;
        //_Debugger_DoState ( debugger ) ;
    }
    else
    {
        //Debugger_ConsiderAndShowWord ( debugger ) ;
        //if ( debugger->Token || debugger->w_Word ) Debugger_InterpretTokenWriteCode ( debugger ) ;
        //else Debugger_Quit ( debugger ) ; //Debugger_InterpretWord ( debugger ) ;
    }
    Debugger_SetState ( debugger, DBG_PRE_DONE, true ) ;
}

void
Debugger_SetupNextToken ( Debugger * debugger )
{
    Debugger_NextToken ( debugger ) ;
    Debugger_FindUsing ( debugger ) ;
}

void
Debugger_WDis ( Debugger * debugger )
{
    Printf ( ( byte* ) "\n" ) ;
    Word * word = debugger->w_Word ;
    if ( ! word ) word = _Interpreter_->w_Word ;
    if ( word )
    {
        Printf ( ( byte* ) "\nWord : %s : disassembly :> \n", word->Name ) ;
        _CfrTil_Word_Disassemble ( word ) ;
    }
    Printf ( ( byte* ) "\n" ) ;
}

int32
Debugger_Udis_GetInstructionSize ( Debugger * debugger )
{
    return _Udis_GetInstructionSize ( debugger->Udis, debugger->DebugAddress ) ;
}

int32
Debugger_UdisOneInstruction ( Debugger * debugger, byte * address, byte * prefix, byte * postfix )
{
    return _Udis_OneInstruction ( debugger->Udis, address, prefix, postfix ) ;
}

ud_t *
Debugger_UdisInit ( Debugger * debugger )
{
    if ( ! debugger->Udis ) debugger->Udis = ( ud_t* ) _Mem_Allocate ( sizeof (ud_t ), CFRTIL ) ;
    return _Udis_Init ( debugger->Udis ) ;
}

void
_Debugger_Disassemble ( Debugger * debugger, byte* address, int32 number, int32 cflag )
{
    _Udis_Disassemble ( Debugger_UdisInit ( debugger ), address, number, cflag, debugger->DebugAddress ) ;
}

void
Debugger_DisassembleAccumulated ( Debugger * debugger )
{
    Printf ( ( byte* ) "\nDisassembling the accumulated code ...\n" ) ;
    _Debugger_Disassemble ( debugger, _Context_->Compiler0->InitHere, Here - _Context_->Compiler0->InitHere, 0 ) ;
    Printf ( ( byte* ) "\n" ) ;
}

void
Debugger_Info ( Debugger * debugger )
{
    Debugger_SetState ( debugger, DBG_INFO, true ) ;
}

void
Debugger_DoMenu ( Debugger * debugger )
{
    Debugger_SetState ( debugger, DBG_MENU | DBG_PROMPT | DBG_NEWLINE, true ) ;
}

void
Debugger_Stack ( Debugger * debugger )
{
    CfrTil_PrintDataStack ( ) ;
}

void
_Debugger_Verbosity ( Debugger * debugger )
{
#if 0    
    if ( System_GetState ( _Context_->System0, VERBOSE ) ) System_SetState ( _Context_->System0, VERBOSE, false ) ;
    else System_SetState ( _Context_->System0, VERBOSE, true ) ;
    _CfrTil_SystemState_Print ( 0 ) ;
#endif
    //int32 verbosity = _CfrTil_VariableValueGet ( "Debug", ( byte* ) "verbosity" ) ;
    Printf ( "\nDebuggerVerbosity = %d", _CfrTil_->DebuggerVerbosity ) ;
}

void
Debugger_Source ( Debugger * debugger )
{
    _CfrTil_Source ( debugger->w_Word, 0 ) ;
}

void
Debugger_Registers ( Debugger * debugger )
{
    if ( ! Debugger_IsStepping ( debugger ) )
    {
        debugger->SaveCpuState ( ) ;
    }
    _CpuState_Show ( debugger->cs_CpuState ) ;
    Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\r", ( byte* ) "\r" ) ; // current insn
}

void
Debugger_Continue ( Debugger * debugger )
{
    //SetState ( _CfrTil_->Debugger0, DBG_CONTINUE, true ) ;
    SetState ( _CfrTil_, DEBUG_MODE, false ) ;
    SetState ( debugger, DBG_PRE_DONE, true ) ;
    SetState ( debugger, DBG_STEPPING, false ) ;
    debugger->w_Word = 0 ;
    //_Debugger_Eval ( debugger ) ;
    //debugger->SaveKey = 'e' ;
    //Debugger_AutoMode ( debugger ) ;
}

void
Debugger_Quit ( Debugger * debugger )
{
    Debugger_Stepping_Off ( debugger ) ;
    Debugger_SetState_TrueFalse ( _CfrTil_->Debugger0, DBG_DONE, DBG_CONTINUE | DBG_ACTIVE ) ;
    SetState ( _CfrTil_, DEBUG_MODE, false ) ;
    SetState ( debugger, DBG_PRE_DONE, true ) ;
    _Throw ( QUIT ) ;
}

void
Debugger_Abort ( Debugger * debugger )
{
    Debugger_Stepping_Off ( debugger ) ;
    Debugger_SetState_TrueFalse ( _CfrTil_->Debugger0, DBG_DONE, DBG_CONTINUE | DBG_ACTIVE ) ;
    _Throw ( ABORT ) ;
}

void
Debugger_Stop ( Debugger * debugger )
{
    Printf ( "\nStop!\n" ) ;
    Debugger_Stepping_Off ( debugger ) ;
    Debugger_SetState_TrueFalse ( _CfrTil_->Debugger0, DBG_DONE, DBG_CONTINUE | DBG_ACTIVE ) ;
    _CfrTil_->SaveDsp = Dsp ;
    _Throw ( STOP ) ;
}

void
Debugger_InterpretLine ( )
{
    _CfrTil_Contex_NewRun_1 ( _CfrTil_, ( ContextFunction_1 ) CfrTil_InterpretPromptedLine, 0, 0 ) ; // can't clone cause we may be in a file and we want input from stdin
}

void
Debugger_Escape ( Debugger * debugger )
{
    Boolean saveStateBoolean = System_GetState ( _Context_->System0, ADD_READLINE_TO_HISTORY ) ;
    System_SetState ( _Context_->System0, ADD_READLINE_TO_HISTORY, true ) ;
    Debugger_SetState_TrueFalse ( debugger, DBG_COMMAND_LINE | DBG_ESCAPED, DBG_ACTIVE ) ;
    _CfrTil_->Debugger0 = Debugger_Copy ( debugger, TEMPORARY ) ;
    DefaultColors ;
    SetState ( _CfrTil_, DEBUG_MODE, false ) ;
    Debugger_InterpretLine ( ) ;
    SetState ( _CfrTil_, DEBUG_MODE, true ) ;
    DebugColors ;
    _CfrTil_->Debugger0 = debugger ;
    System_SetState ( _Context_->System0, ADD_READLINE_TO_HISTORY, saveStateBoolean ) ; // reset state 
    Debugger_SetState_TrueFalse ( debugger, DBG_ACTIVE | DBG_INFO | DBG_NEWLINE, DBG_COMMAND_LINE | DBG_ESCAPED ) ;
}

void
Debugger_AutoMode ( Debugger * debugger )
{
    if ( ( debugger->SaveKey == 's' ) || ( debugger->SaveKey == 'e' ) || ( debugger->SaveKey == 'c' ) ) // not everything makes sense here
    {
        AlertColors ;
        if ( debugger->SaveKey == 'c' )
        {
            Printf ( ( byte* ) "\nContinuing : automatically repeating key \'e\' ..." ) ;
            debugger->SaveKey = 'e' ;
        }
        else Printf ( ( byte* ) "\nDebugger :: Starting AutoMode : automatically repeating key :: \'%c\' ...", debugger->SaveKey ) ;
        DefaultColors ;
        debugger->Key = debugger->SaveKey ;
        Debugger_SetState ( debugger, DBG_AUTO_MODE, true ) ;
    }
}

void
Debugger_OptimizeToggle ( Debugger * debugger )
{
    if ( GetState ( _CfrTil_, OPTIMIZE_ON ) ) SetState ( _CfrTil_, OPTIMIZE_ON, false ) ;
    else SetState ( _CfrTil_, OPTIMIZE_ON, true ) ;
    _CfrTil_SystemState_Print ( 0 ) ;
}

void
Debugger_CodePointerUpdate ( Debugger * debugger )
{
    if ( debugger->w_Word && ( ! debugger->DebugAddress ) )
    {
        debugger->DebugAddress = ( byte* ) debugger->w_Word->Definition ;
        Printf ( ( byte* ) "\ncodePointer = 0x%08x", ( int32 ) debugger->DebugAddress ) ;
    }
}

void
Debugger_Dump ( Debugger * debugger )
{
    if ( ! debugger->w_Word )
    {
        if ( debugger->DebugAddress ) __CfrTil_Dump ( ( int32 ) debugger->DebugAddress, ( int32 ) ( Here - ( int32 ) debugger->DebugAddress ), 8 ) ;
    }
    else __CfrTil_Dump ( ( int32 ) debugger->w_Word->CodeStart, ( int32 ) debugger->w_Word->S_CodeSize, 8 ) ;
}

void
Debugger_Default ( Debugger * debugger )
{
    if ( isgraph ( debugger->Key ) )
    {
        Printf ( ( byte* ) "\rdbg :> %c <: is not an assigned key code", debugger->Key ) ;
    }
    else
    {
        Printf ( ( byte* ) "\rdbg :> <%d> <: is not an assigned key code", debugger->Key ) ;
    }
    Debugger_SetState ( debugger, DBG_MENU | DBG_PROMPT | DBG_NEWLINE, true ) ;
}

void
Debugger_Stepping_Off ( Debugger * debugger )
{
    if ( Debugger_IsStepping ( debugger ) )
    {
        //if ( debugger->WordDsp ) Dsp = debugger->WordDsp ; // by 'eval' we stop debugger stepping and //continue thru this word as if we hadn't stepped
        //if ( debugger->SaveDsp ) Dsp = debugger->SaveDsp ; // by 'eval' stops debugger->Stepping and //continues thru this word as if we hadn't stepped
        Debugger_SetStepping ( debugger, false ) ;
        debugger->DebugAddress = 0 ;
    }
}

void
Debugger_SetupStepping ( Debugger * debugger, int32 sflag, int32 iflag )
{
    Word * word ;
    Stack_Init ( debugger->DebugStack ) ;
    Stack_Init ( debugger->AddressAfterJmpCallStack ) ;
    Printf ( ( byte* ) "\nSetting up stepping ..." ) ;
    if ( ! debugger->DebugAddress ) debugger->DebugAddress = ( byte* ) debugger->w_Word->Definition ;
    else
    {
        if ( ! debugger->w_Word )
        {
            word = Word_GetFromCodeAddress ( debugger->DebugAddress ) ;
            if ( word )
            {
                if ( sflag ) _Word_ShowSourceCode ( word ) ;
                if ( iflag ) Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\nNext stepping instruction\n", ( byte* ) "\r" ) ;
                _Interpreter_SetupFor_MorphismWord ( _Context_->Interpreter0, debugger->w_Word ) ; //since we're not calling the interpret for eval, setup the word 
            }
        }
    }
    SetState_TrueFalse ( debugger, DBG_STEPPING, DBG_NEWLINE | DBG_PROMPT | DBG_INFO | DBG_MENU ) ;
    debugger->SaveDsp = Dsp ; // saved before we start stepping
}
// simply : copy the current insn to a ByteArray buffer along with
// prefix and postfix instructions that restore and
// save the cpu state; then run that ByteArray code buffer

void
Debugger_Step ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    if ( ! Debugger_IsStepping ( debugger ) )
    {
        if ( word )
        {
            if ( ! ( word->CType & CFRTIL_WORD ) && ! ( word->LType & T_LISP_DEFINE ) )
            {
                Debugger_Eval ( debugger ) ;
                return ;
            }
            debugger->WordDsp = Dsp ; // by 'eval' we stop debugger->Stepping and //continue thru this word as if we hadn't stepped
            debugger->PreHere = Here ;
            Debugger_CanWeStep ( debugger ) ;
            if ( ! Debugger_GetState ( debugger, DBG_CAN_STEP ) )
            {
                Debugger_Eval ( debugger ) ;
            }
            else
            {
                Debugger_SetupStepping ( debugger, 1, 0 ) ;
                Printf ( ( byte* ) "\nNext stepping instruction ...\n" ) ;
                Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "", ( byte* ) "" ) ;
                SetState ( debugger, DBG_NEWLINE | DBG_PROMPT | DBG_INFO, false ) ;
            }
        }
        else Debugger_SetState_TrueFalse ( debugger, DBG_NEWLINE, DBG_STEPPING | DBG_RESTORE_REGS ) ;
        return ;
    }
    else
    {
        Debugger_StepOneInstruction ( debugger ) ;
        if ( ( int32 ) debugger->DebugAddress ) // set by StepOneInstruction
        {
            Debugger_GetWordFromAddress ( debugger ) ;
            SetState_TrueFalse ( debugger, DBG_STEPPING | DBG_RESTORE_REGS, ( DBG_INFO | DBG_MENU | DBG_PROMPT ) ) ;
            debugger->SteppedWord = word ;
        }
        else
        {
            if ( debugger->w_Word ) _Debugger_DisassembleWrittenCode ( debugger ) ;
#if 0            
            if ( GetState ( debugger, DBG_AUTO_MODE ) )
            {
                Debugger_SetupNextToken ( debugger ) ;
            }
            else if ( GetState ( debugger, DBG_STEPPING ) )
            {
                SetState ( debugger, DBG_DONE, true ) ;
                debugger->Token = 0 ;
            }
            Debugger_SetState_TrueFalse ( debugger, DBG_NEWLINE | DBG_PROMPT | DBG_INFO, DBG_STEPPING | DBG_RESTORE_REGS ) ;
#endif    
            Debugger_SetState_TrueFalse ( debugger, DBG_PRE_DONE | DBG_STEPPED | DBG_NEWLINE | DBG_PROMPT | DBG_INFO, DBG_AUTO_MODE | DBG_STEPPING | DBG_RESTORE_REGS ) ;
            SetState ( _CfrTil_, DEBUG_MODE, false ) ;
            SetState ( debugger, DBG_PRE_DONE, true ) ;
            return ;
        }
    }
}

void
_Debugger_DoNewline ( Debugger * debugger )
{
    Printf ( ( byte* ) "\n%s", _Context_->ReadLiner0->DebugPrompt ) ; //, (char*) ReadLine_GetPrompt ( _Context_->ReadLiner0 ) ) ;
    Debugger_SetNewLine ( debugger, false ) ;
}

void
_Debugger_DoState ( Debugger * debugger )
{
    if ( ! GetState ( debugger, DBG_STEPPING ) ) Printf ( "\n" ) ;
    if ( GetState ( debugger, DBG_RETURN ) ) Printf ( "\r" ) ;
    if ( GetState ( debugger, DBG_MENU ) ) Debugger_Menu ( ) ;
    Debugger_SetMenu ( debugger, false ) ;
    if ( GetState ( debugger, DBG_INFO ) ) Debugger_ShowInfo ( debugger, ( byte* ) "dbg", 0 ) ;
    else if ( GetState ( debugger, DBG_PROMPT ) ) Debugger_ShowState ( debugger, ( byte* ) "dbg" ) ;
    if ( GetState ( debugger, DBG_NEWLINE ) ) _Debugger_DoNewline ( debugger ) ;
    if ( GetState ( debugger, DBG_BRK_INIT ) )
    {
        //Printf ( "\n<dbrk> :: " UINT_FRMT_0x08 , debugger->DebugAddress ) ;
        Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\n", ( byte* ) "" ) ;
        SetState ( debugger, DBG_BRK_INIT, false ) ;
    }
}

void
_Debugger_InterpreterLoop ( Debugger * debugger )
{
    do
    {
        _Debugger_DoState ( debugger ) ;
        if ( ! Debugger_GetState ( _CfrTil_->Debugger0, DBG_AUTO_MODE ) )
        {
            debugger->Key = Key ( ) ;
            if ( debugger->Key != 'z' ) debugger->SaveKey = debugger->Key ;
        }
        debugger->CharacterFunctionTable [ debugger->CharacterTable [ debugger->Key ] ] ( debugger ) ;
    }
    while ( GetState ( debugger, DBG_STEPPING ) || ( ! GetState ( debugger, DBG_PRE_DONE ) ) ) ;
}

void
_Debugger_PreSetup ( Debugger * debugger, byte * token, Word * word )
{
    if ( ( word == debugger->LastPreWord ) && ( ! GetState ( debugger, DBG_RUNTIME ) ) ) return ; // allow to not reshow result
    SetState ( debugger, DBG_PRE_DONE, false ) ;
    if ( GetState ( debugger, DBG_STEPPED ) && ( word && ( word == debugger->SteppedWord ) ) ) return ; // is this needed anymore ?!?
    DebugColors ;
    if ( ! GetState ( debugger, DBG_ACTIVE ) )
    {
        Debugger_SetState_TrueFalse ( debugger, DBG_ACTIVE | DBG_INFO | DBG_NEWLINE | DBG_MENU | DBG_PROMPT, DBG_CONTINUE | DBG_STEPPING ) ;
        //Debugger_SetState_TrueFalse ( debugger, DBG_ACTIVE | DBG_INFO | DBG_NEWLINE | DBG_MENU, DBG_CONTINUE | DBG_STEPPING ) ;
    }
    debugger->w_Word = word ;
    debugger->SaveDsp = Dsp ;
    debugger->PreHere = Here ;
    debugger->WordDsp = Dsp ;
    debugger->SaveTOS = TOS ;
    if ( word ) debugger->Token = word->Name ;
    else debugger->Token = token ;

    _Debugger_InterpreterLoop ( debugger ) ;

    debugger->SteppedWord = 0 ;
    debugger->LastPreWord = word ;
    debugger->LastShowWord = 0 ;
    debugger->OptimizedCodeAffected = 0 ;
    DefaultColors ;
}

void
_Debugger_PostShow ( Debugger * debugger, byte * token, Word * word )
{
    if ( debugger->LastShowWord == word ) return ; // allow to not reshow result
    else
    {
        debugger->Token = token ;
        Debugger_ShowWrittenCode ( debugger, 0 ) ;
        debugger->LastPreWord = 0 ;
        debugger->LastShowWord = word ;
        debugger->w_Word = 0 ; // word ;
    }
    DefaultColors ;
}

