#include "../includes/cfrtil.h"

void
Debugger_TableSetup ( Debugger * debugger )
{
    int32 i ;
    for ( i = 0 ; i < 128 ; i ++ ) debugger->CharacterTable [ i ] = 0 ;
    debugger->CharacterTable [ 0 ] = 0 ;
    debugger->CharacterTable [ 's' ] = 1 ;
    debugger->CharacterTable [ 'e' ] = 2 ;
    debugger->CharacterTable [ 'w' ] = 3 ;
    debugger->CharacterTable [ 'd' ] = 4 ;
    debugger->CharacterTable [ 'I' ] = 5 ;
    debugger->CharacterTable [ 'm' ] = 6 ;
    debugger->CharacterTable [ 't' ] = 7 ;
    debugger->CharacterTable [ 'V' ] = 8 ;
    //debugger->CharacterTable [ 'U' ] = 9 ;
    debugger->CharacterTable [ 'r' ] = 10 ;
    debugger->CharacterTable [ 'c' ] = 11 ;
    debugger->CharacterTable [ 'q' ] = 12 ;
    debugger->CharacterTable [ 'o' ] = 1 ;
    debugger->CharacterTable [ 'i' ] = 1 ;
    debugger->CharacterTable [ 'u' ] = 9 ;
    debugger->CharacterTable [ 'f' ] = 14 ;
    debugger->CharacterTable [ '\\'] = 15 ;
    debugger->CharacterTable [ 'G' ] = 16 ;
    debugger->CharacterTable [ 'n' ] = 17 ;
    debugger->CharacterTable [ 'p' ] = 18 ;
    debugger->CharacterTable [ 'h' ] = 19 ;
    debugger->CharacterTable [ 'a' ] = 20 ;
    debugger->CharacterTable [ 'z' ] = 21 ;
    debugger->CharacterTable [ 'w' ] = 22 ;
    debugger->CharacterTable [ 'A' ] = 23 ;
    debugger->CharacterTable [ 'P' ] = 24 ;
    debugger->CharacterTable [ 'S' ] = 27 ;
    debugger->CharacterTable [ 'l' ] = 25 ;
    debugger->CharacterTable [ 'v' ] = 26 ;


    // debugger : system related
    debugger->CharacterFunctionTable [ 1 ] = Debugger_Step ;
    debugger->CharacterFunctionTable [ 2 ] = Debugger_Eval ;
    debugger->CharacterFunctionTable [ 21 ] = Debugger_AutoMode ;
    debugger->CharacterFunctionTable [ 11 ] = Debugger_Continue ;
    debugger->CharacterFunctionTable [ 12 ] = Debugger_Quit ;

    debugger->CharacterFunctionTable [ 13 ] = Debugger_Parse ;
    debugger->CharacterFunctionTable [ 14 ] = Debugger_FindAny ;
    debugger->CharacterFunctionTable [ 15 ] = Debugger_Escape ;
    // debugger internal
    //debugger->CharacterFunctionTable [ 3 ] = Debugger_InterpretTokenWriteCode ;
    debugger->CharacterFunctionTable [ 0 ] = Debugger_Default ;
    debugger->CharacterFunctionTable [ 4 ] = Debugger_Disassemble ;
    debugger->CharacterFunctionTable [ 5 ] = Debugger_Info ;
    debugger->CharacterFunctionTable [ 6 ] = Debugger_DoMenu ;
    debugger->CharacterFunctionTable [ 7 ] = Debugger_Stack ;
    debugger->CharacterFunctionTable [ 8 ] = _Debugger_Verbosity ;
    debugger->CharacterFunctionTable [ 9 ] = Debugger_Source ;
    debugger->CharacterFunctionTable [ 10 ] = Debugger_Registers ;
    debugger->CharacterFunctionTable [ 16 ] = Debugger_OptimizeToggle ;
    debugger->CharacterFunctionTable [ 17 ] = Debugger_CodePointerUpdate ;
    debugger->CharacterFunctionTable [ 18 ] = Debugger_Dump ;
    debugger->CharacterFunctionTable [ 19 ] = Debugger_ConsiderAndShowWord ;
    debugger->CharacterFunctionTable [ 20 ] = Debugger_DisassembleAccumulated ;
    debugger->CharacterFunctionTable [ 22 ] = Debugger_WDis ;
    debugger->CharacterFunctionTable [ 23 ] = Debugger_Abort ;
    debugger->CharacterFunctionTable [ 24 ] = Debugger_Stop ;
    debugger->CharacterFunctionTable [ 25 ] = Debugger_Locals_Show ;
    debugger->CharacterFunctionTable [ 26 ] = Debugger_Variables ;
    debugger->CharacterFunctionTable [ 27 ] = _Debugger_State ;
}

void
_Debugger_State ( Debugger * debugger )
{
    Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;
    char * buf = ( char* ) Buffer_Data ( buffer ) ;
    _CfrTil_GetSystemState_String0 ( buf ) ;
    Printf ( ( byte* ) buf ) ;
    Buffer_SetAsUnused ( buffer ) ;
}

void
_Debugger_Copy ( Debugger * debugger, Debugger * debugger0 )
{
    memcpy ( debugger, debugger0, sizeof (Debugger ) ) ;
}

Debugger *
Debugger_Copy ( Debugger * debugger0, int32 type )
{
    Debugger * debugger = ( Debugger * ) _Mem_Allocate ( sizeof (Debugger ), type ) ;
    _Debugger_Copy ( debugger, debugger0 ) ;
    return debugger ;
}

void
Debugger_Delete ( Debugger * debugger )
{
    Mem_FreeItem ( _Q_->PermanentMemList, ( byte* ) debugger ) ;
}

void
Debugger_C_StackPrint ( Debugger * debugger, int i )
{
    debugger->GetEIP ( ) ;
    for ( i = 0 ; i < 100 ; i ++ )
    {
        Printf ( ( byte* ) "\n%02d : 0x%08x", i, ( ( int32* ) debugger->DebugAddress ) [i] ) ;
    }
}

// remember : this stuff is used a little differently since 0.754.10x

void
_Debugger_Init ( Debugger * debugger, Word * word, byte * address )
{
    DebugColors ;
    Debugger_UdisInit ( debugger ) ;
    debugger->SaveDsp = Dsp ;
    debugger->SaveTOS = TOS ;
    debugger->Key = 0 ;
    debugger->State = DBG_MENU | DBG_INFO | DBG_PROMPT ;
    debugger->w_Word = word ;
    //_ByteArray_ReInit ( debugger->StepInstructionBA ) ; // debugger->StepInstructionBA = _ByteArray_AllocateNew ( 64, SESSION ) ;
    Stack_Init ( debugger->DebugStack ) ;

    SetState ( _CfrTil_, DEBUG_MODE, true ) ;
    if ( address )
    {
        debugger->DebugAddress = address ;
    }
    else
    {
        // remember : _CfrTil_->Debugger0->GetESP is called already thru _Compile_Debug
        if ( debugger->DebugESP )
        {
            debugger->DebugAddress = ( byte* ) debugger->DebugESP [0] ; // EIP
        }
        if ( debugger->DebugAddress )
        {
            debugger->w_Word = word = Word_GetFromCodeAddress ( debugger->DebugAddress ) ; //+ 1 + CELL + * ( int32* ) ( debugger->DebugAddress + 1 ) ) ;
            if ( ! word ) debugger->w_Word = word = Word_GetFromCodeAddress ( debugger->DebugAddress + 1 + CELL + * ( int32* ) ( debugger->DebugAddress + 1 ) ) ;
            if ( ! word )
            {
                Printf ( ( byte* ) "\n\nCan't find the Word, but here is some disassembly at the considered \"EIP address\" : \n" ) ;
                _Debugger_Disassemble ( debugger, debugger->DebugAddress, 16, 0 ) ;
                Debugger_NextToken ( debugger ) ;
                Debugger_FindUsing ( debugger ) ;
                debugger->DebugAddress = 0 ;
            }
        }
    }
    if ( ( ! debugger->DebugAddress ) && ( ! debugger->w_Word ) )
    {
        Debugger_NextToken ( debugger ) ;
        Debugger_FindUsing ( debugger ) ;
    }
    if ( debugger->w_Word ) debugger->Token = debugger->w_Word->Name ;
    else
    {
        debugger->w_Word = _Context_->Interpreter0->w_Word ;
        debugger->Token = _Context_->Interpreter0->w_Word->Name ;
    }
}

Debugger *
_Debugger_New ( int32 type )
{
    Debugger * debugger = ( Debugger * ) _Mem_Allocate ( sizeof (Debugger ), type ) ;
    debugger->cs_CpuState = CpuState_New ( type ) ;
    debugger->StepInstructionBA = _ByteArray_AllocateNew_ ( _Q_->PermanentMemList, 64, type ) ;
    debugger->DebugStack = Stack_New ( 256, type ) ;
    debugger->AddressAfterJmpCallStack = Stack_New ( 256, type ) ;
    Debugger_TableSetup ( debugger ) ;
    Debugger_SetState ( debugger, DBG_ACTIVE, true ) ;
    Debugger_UdisInit ( debugger ) ;
    return debugger ;
}

void
_CfrTil_DebugInfo ( )
{
    Debugger_ShowInfo ( _CfrTil_->Debugger0, ( byte* ) "\ninfo", 0 ) ;
}

void
CfrTil_DebugInfo ( )
{
    if ( _Q_->Verbosity )
    {
        _CfrTil_DebugInfo ( ) ;
        Debugger_Source ( _CfrTil_->Debugger0 ) ;
    }
}

void
_CfrTil_Debug_AtAddress ( byte * address )
{
    if ( ! Debugger_GetState ( _CfrTil_->Debugger0, DBG_ACTIVE ) )
    {
        _Debugger_Init ( _CfrTil_->Debugger0, 0, address ) ;
    }
    else
    {
        _CfrTil_DebugContinue ( 1 ) ;
    }
}

void
CfrTil_Debug_AtAddress ( )
{
    byte * address ;
    address = ( byte* ) _DataStack_Pop ( ) ;
    _CfrTil_Debug_AtAddress ( address ) ;
}

void
_CfrTil_DebugContinue ( int autoFlagOff )
{
    if ( Debugger_GetState ( _CfrTil_->Debugger0, DBG_AUTO_MODE ) )
    {
        if ( autoFlagOff ) Debugger_SetState ( _CfrTil_->Debugger0, DBG_AUTO_MODE, false ) ;
        //_Debugger_Interpret ( _CfrTil_->Debugger0, 1 ) ;
    }
}

// "d:"

void
CfrTil_DebugModeOn ( )
{
    SetState ( _CfrTil_, DEBUG_MODE, true ) ;
}

void
CfrTil_DebugModeOff ( )
{
    SetState ( _CfrTil_, DEBUG_MODE, false ) ;
}

void
CfrTil_LocalsShow ( )
{
    Debugger_Locals_Show ( _CfrTil_->Debugger0 ) ;
}

void
CfrTil_Debugger_Verbosity ( )
{
    _DataStack_Push ( ( int32 ) & _CfrTil_->DebuggerVerbosity ) ;
}

// put this '<dbg>' into cfrtil code for a runtime break into the debugger

void
CfrTil_DebugRuntimeBreakpoint ( )
{
    if ( ! CompileMode ) //Debugger_GetState ( debugger, DBG_ACTIVE ) )
    {
        Debugger * debugger = _CfrTil_->Debugger0 ;
        // GetESP has been called thru _Compile_Debug1 by
        debugger->SaveCpuState ( ) ;
        _Debugger_Init ( debugger, 0, 0 ) ;
        SetState ( debugger, DBG_RUNTIME|DBG_BRK_INIT|DBG_RESTORE_REGS, true ) ;
        _Debugger_PreSetup ( debugger, debugger->Token, debugger->w_Word ) ;
    }
}

void
CfrTil_Debug ( )
{
    Debugger * debugger = _CfrTil_->Debugger0 ;
    if ( GetState ( _CfrTil_, DEBUG_MODE ) )
    {
        _Debugger_Init ( debugger, 0, 0 ) ;
    }
    else if ( ! Debugger_GetState ( debugger, DBG_ACTIVE ) )
    {
        debugger->SaveCpuState ( ) ;
        _Debugger_Init ( debugger, 0, 0 ) ;
        if ( ! Debugger_GetState ( debugger, DBG_DONE | DBG_AUTO_MODE ) )
        {
            ThrowIt ( "CfrTil_Debug" ) ; // back to the _Word_Run try-catchAll
        }
    }
}

