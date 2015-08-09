
#include "../../includes/cfrtil.h"

// we have the address of a jcc insn 
// get the address it jccs to

byte *
JccInstructionAddress ( byte * address )
{
    int32 offset = * ( int32* ) ( address + 2 ) ; // 2 : 2 byte opCode
    byte * jcAddress = address + offset + 6 ; // 6 : sizeof 0f jcc insn - 0f8x - includes 2 byte opCode
    return jcAddress ;
}

// we have the address of a jmp/call insn 
// get the address it jmp/calls to

byte *
JumpCallInstructionAddress ( byte * address )
{
    // calculate jmp or call address
    int offset = * ( int32* ) ( address + 1 ) ; // 1 : 1 byte opCode
    byte * jcAddress = address + offset + 5 ; // 5 : sizeof jmp insn - includes 1 byte opcode
    return jcAddress ;
}

byte *
Debugger_DoJcc ( Debugger * debugger )
{
    byte * jcAddress = JccInstructionAddress ( debugger->DebugAddress ) ;
    int tttn, ttt, n ;
    tttn = * ( debugger->DebugAddress + 1 ) & 0xf ;
    ttt = ( tttn & 0xe ) >> 1 ;
    n = tttn & 1 ;

    // cf. Intel Software Developers Manual v1. (253665), Appendix B
    // ?? nb. some of this may not be (thoroughly) tested as of 11-14-2011 -- but no known problems after months of testing ??
    // setting jcAddress to 0 means we don't branch and just continue with the next instruction
    if ( ttt == BELOW ) // ttt 001
    {
        if ( ( n == 0 ) && ! ( debugger->cs_CpuState->EFlags & CARRY_FLAG ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( debugger->cs_CpuState->EFlags & CARRY_FLAG ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == ZERO ) // ttt 010
    {
        if ( ( n == 0 ) && ! ( debugger->cs_CpuState->EFlags & ZERO_FLAG ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( debugger->cs_CpuState->EFlags & ZERO_FLAG ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == BE ) // ttt 011 :  below or equal
    {
        if ( ( n == 0 ) && ! ( ( debugger->cs_CpuState->EFlags & CARRY_FLAG ) | ( debugger->cs_CpuState->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( debugger->cs_CpuState->EFlags & CARRY_FLAG ) | ( debugger->cs_CpuState->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == LESS ) // ttt 110
    {
        if ( ( n == 0 ) && ! ( ( debugger->cs_CpuState->EFlags & SIGN_FLAG ) ^ ( debugger->cs_CpuState->EFlags & OVERFLOW_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        else if ( ( n == 1 ) && ( ( debugger->cs_CpuState->EFlags & SIGN_FLAG ) ^ ( debugger->cs_CpuState->EFlags & OVERFLOW_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    else if ( ttt == LE ) // ttt 111
    {
        if ( ( n == 0 ) && !
            ( ( ( debugger->cs_CpuState->EFlags & SIGN_FLAG ) ^ ( debugger->cs_CpuState->EFlags & OVERFLOW_FLAG ) ) | ( debugger->cs_CpuState->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
        if ( ( n == 1 ) &&
            ( ( ( debugger->cs_CpuState->EFlags & SIGN_FLAG ) ^ ( debugger->cs_CpuState->EFlags & OVERFLOW_FLAG ) ) | ( debugger->cs_CpuState->EFlags & ZERO_FLAG ) ) )
        {
            jcAddress = 0 ;
        }
    }
    return jcAddress ;
}

void
Debugger_CompileAndDoInstruction ( Debugger * debugger, byte * jcAddress, int32 size )
{
    ByteArray * svcs = CompilerMemByteArray ;
    _ByteArray_ReInit ( debugger->StepInstructionBA ) ; // we are only compiling one insn here so clear our BA before each use
    Set_CompilerSpace ( debugger->StepInstructionBA ) ;
    byte * newDebugAddress ;

    if ( GetState ( debugger, DBG_RESTORE_REGS ) ) _Compile_Call ( ( byte* ) debugger->RestoreCpuState ) ;
    if ( jcAddress ) // jump or call address
    {
        Word * word = Word_GetFromCodeAddress_NoAlias ( jcAddress ) ;
        if ( word && ( word != debugger->w_Word ) )
        {
            debugger->w_Word = word ;
            debugger->Token = word->Name ;
            _Word_ShowSourceCode ( word ) ;
        }
        if ( ( word && ( word->CType & CPRIMITIVE ) ) && ( ( jcAddress < ( byte* ) svcs->BA_Data ) || ( jcAddress > ( byte* ) svcs->bp_Last ) ) )
        {
            if ( * debugger->DebugAddress == JMPI32 )
            {
                CfrTil_Exception ( MACHINE_CODE_ERROR, 1 ) ; // can't in the current debugger jmp into C compiler code
            }
            else
            {
                newDebugAddress = debugger->DebugAddress + size ;
                Printf ( ( byte* ) "\ncalling thru a C subroutine : %s : .... :> \n", word ? ( char* ) word->Name : "" ) ;
                _Compile_Call ( jcAddress ) ; // 5 : sizeof call insn with offset
            }
        }
        else
        {
            //if ( word ) debugger->w_Word = word ;
            if ( debugger->Key == 'o' ) // step thru ("over") the native code like a non-native subroutine
            {
                Printf ( ( byte* ) "\ncalling thru (over) a native subroutine : %s : .... :> \n", word ? ( char* ) word->Name : "" ) ;
                _Compile_Call ( jcAddress ) ; // 5 : sizeof call insn with offset
                // !?!? this may need work right in here ... !?!?
                newDebugAddress = debugger->DebugAddress + size ;
            }
            else if ( debugger->Key == 'u' ) // step o(u)t of the native code like a non-native subroutine
            {
                Printf ( ( byte* ) "\nstepping out of the subroutine" ) ;
                _Compile_Call ( debugger->DebugAddress ) ; // 5 : sizeof call insn with offset
                // !?!? this may need work !?!?
                if ( Stack_Depth ( debugger->DebugStack ) ) newDebugAddress = ( byte* ) _Stack_Pop ( debugger->DebugStack ) ;
                else
                {
                    newDebugAddress = 0 ;
                }
            }
            else
            {
                if ( * debugger->DebugAddress == CALLI32 )
                {
                    _Stack_Push ( debugger->DebugStack, ( int32 ) ( debugger->DebugAddress + size ) ) ;
                }
                // emulate a call -- all we really needed was its address and to push (above) the return address if necessary - if it was a 'call' instruction
                //_Compile_Call ( _ByteArray_Here ( debugger->StepInstructionBA ) + 5 ) ; // 5 : sizeof call insn with offset - call to immediately after this very instruction
                //_Compile_JumpToAddress ( _ByteArray_Here ( debugger->StepInstructionBA ) + 5 ) ;
                _Compile_Call ( _ByteArray_Here ( debugger->StepInstructionBA ) + 5 ) ; // 5 : sizeof call insn with offset - call to immediately after this very instruction
                newDebugAddress = jcAddress ;
            }
        }
    }
    else
    {
        if ( debugger->Key == 'u' ) // step o(u)t of the native code like a non-native subroutine
        {
            Printf ( ( byte* ) "\nstepping thru and out of the subroutine" ) ;
            _Compile_Call ( debugger->DebugAddress ) ; // 5 : sizeof call insn with offset
            // !?!? this may need work !?!?
            if ( Stack_Depth ( debugger->DebugStack ) ) newDebugAddress = ( byte* ) _Stack_Pop ( debugger->DebugStack ) ;
            else
            {
                newDebugAddress = 0 ;
            }
        }
        else
        {
            ByteArray_AppendCopy ( debugger->StepInstructionBA, size, debugger->DebugAddress ) ;
            newDebugAddress = debugger->DebugAddress + size ;
        }
    }
    _Compile_Call ( ( byte* ) debugger->SaveCpuState ) ;
    _Compile_Return ( ) ;
    Set_CompilerSpace ( svcs ) ; // before "do it" in case "do it" calls the compiler
    if ( ! GetState ( debugger, DBG_STACK_CHANGE ) )
    {
        debugger->SaveDsp = Dsp ;
        debugger->PreHere = Here ;
        debugger->SaveTOS = TOS ;
        debugger->SaveStackDepth = DataStack_Depth ( ) ;
    }
    DefaultColors ;
    //NGR_Dsp_To_ESI ;
    // do it : step the instruction ...
    ( ( VoidFunction ) debugger->StepInstructionBA->BA_Data ) ( ) ;
    DebugColors ;
    Debugger_ShowWrittenCode ( debugger, 1 ) ;
    debugger->DebugAddress = newDebugAddress ;
}

void
Debugger_GetWordFromAddress ( Debugger * debugger )
{
    if ( debugger->DebugAddress )
    {
        Word * word = Word_GetFromCodeAddress_NoAlias ( debugger->DebugAddress ) ;
        //if ( word ) 
        debugger->w_Word = word ;
    }
}

void
Debugger_StepOneInstruction ( Debugger * debugger )
{
    byte *jcAddress = 0 ;
    int32 size ;
    size = Debugger_Udis_GetInstructionSize ( debugger ) ;
    // special cases
    if ( * debugger->DebugAddress == _RET )
    {
        if ( Stack_Depth ( debugger->DebugStack ) )
        {
            debugger->DebugAddress = ( byte* ) _Stack_Pop ( debugger->DebugStack ) ;
            Debugger_GetWordFromAddress ( debugger ) ;
            goto end ;
        }
        else
        {
            debugger->DebugAddress = 0 ;
            Debugger_SetState_TrueFalse ( _CfrTil_->Debugger0, DBG_DONE, DBG_STEPPING ) ;
            return ;
        }
    }
    else if ( ( * debugger->DebugAddress == JMPI32 ) || ( * debugger->DebugAddress == CALLI32 ) )
    {
        jcAddress = JumpCallInstructionAddress ( debugger->DebugAddress ) ;
    }
    else if ( ( * debugger->DebugAddress == CALL_JMP_MOD_RM ) && ( RM ( debugger->DebugAddress ) == 16 ) ) // inc/dec are also opcode == 0xff
    {
        //jcAddress = JumpCallInsnAddress_ModRm ( debugger->DebugAddress ) ;
        //-----------------------------------------------------------------------
        //   modRm byte ( bits )  mod 0 : no disp ; mod 1 : 1 byte disp : mod 2 : 4 byte disp ; mod 3 : just reg value
        //    mod     reg      rm
        //   7 - 6   5 - 3   2 - 0
        //-----------------------------------------------------------------------
        byte * address = debugger->DebugAddress ;
        byte modRm = * ( byte* ) ( address + 1 ) ; // 1 : 1 byte opCode
        if ( modRm & 32 ) SyntaxError ( 1 ) ; // we only currently compile call reg code 2/3, << 3 ; not jmp; jmp reg code == 4/5 : reg code 100/101 ; inc/dec 0/1 : << 3
        int mod = modRm & 192 ;
        if ( mod == 192 ) jcAddress = ( byte* ) _CfrTil_->Debugger0->cs_CpuState->Eax ;
        // else it could be inc/dec
    }
    else if ( ( * debugger->DebugAddress == 0x0f ) && ( ( * ( debugger->DebugAddress + 1 ) >> 4 ) == 0x8 ) ) jcAddress = Debugger_DoJcc ( debugger ) ;
    Debugger_CompileAndDoInstruction ( debugger, jcAddress, size ) ;
end:
    if ( debugger->DebugAddress )
    {
        Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "", ( byte* ) "" ) ; // the next instruction
        // keep eip - instruction pointer - up to date ..
        debugger->cs_CpuState->Eip = ( int32 ) debugger->DebugAddress ;
    }
}

void
_CfrTil_ACharacterDump ( char aChar )
{
    if ( isprint ( aChar ) )
    {
        Printf ( ( byte* ) "%c", aChar ) ;
    }

    else Printf ( ( byte* ) "." ) ;
}

void
CfrTil_CharacterDump ( byte * address, int32 number )
{
    int32 i ;
    for ( i = 0 ; i < number ; i ++ )
    {

        _CfrTil_ACharacterDump ( address [ i ] ) ;
    }
    Printf ( ( byte* ) " " ) ;
}

void
_CfrTil_AByteDump ( byte aByte )
{

    Printf ( ( byte* ) "%02x ", aByte ) ;
}

void
CfrTil_NByteDump ( byte * address, int32 number )
{
    int32 i ;
    for ( i = 0 ; i < number ; i ++ )
    {

        _CfrTil_AByteDump ( address [ i ] ) ;
    }
    Printf ( ( byte* ) " " ) ;
}

byte *
GetPostfix ( byte * address, char* udInsnAsm, byte* postfix, byte * buffer )
{
    byte * iaddress ;
    char * prePostfix = ( char* ) "\t" ;
    if ( ( * address == JMPI32 ) || ( * address == CALLI32 ) )
    {
        int32 offset = * ( ( int32 * ) ( address + 1 ) ) ;
        iaddress = address + offset + 1 + CELL ;
    }
    else if ( ( ( * address == 0x0f ) && ( ( * ( address + 1 ) >> 4 ) == 0x8 ) ) )
    {
        int32 offset = * ( ( int32 * ) ( address + 2 ) ) ;
        iaddress = address + offset + 2 + CELL ;
    }
    else return postfix ;
    {
        Word * word = Word_GetFromCodeAddress ( iaddress ) ;
        if ( word )
        {
            byte * name = ( byte* ) c_dd ( word->Name ) ; //, &_Q_->Default ) ;
            if ( ( byte* ) word->CodeStart == iaddress )
            {
                sprintf ( ( char* ) buffer, "%s< %s.%s >%s", prePostfix, word->ContainingNamespace->Name, name, postfix ) ;
            }
            else
            {
                sprintf ( ( char* ) buffer, "%s< %s.%s+%d >%s", prePostfix,
                    word->ContainingNamespace->Name, name, iaddress - ( byte* ) word->CodeStart, postfix ) ;
            }
        }
        else sprintf ( ( char* ) buffer, "%s< %s >", prePostfix, ( char * ) "C compiler code" ) ;
        postfix = buffer ;
    }
    return postfix ;
}

void
_Compile_Debug_GetESP ( byte * where ) // where we want the acquired pointer
{
    // ! nb : x86 cant do rm offset with ESP reg directly so use EAX
    _Compile_Move_Reg_To_Reg ( EAX, ESP ) ;
    _Compile_MoveImm_To_Reg ( ECX, ( int32 ) where, CELL ) ;
    _Compile_Move_Reg_To_Rm ( ECX, 0, EAX ) ;
}

void
Compile_Debug_GetESP () // where we want the acquired pointer
{
    _Compile_Debug_GetESP ( ( byte* ) & _CfrTil_->Debugger0->DebugESP ) ;
}

void
_Compile_Debug1 ( ) // where we want the acquired pointer
{
    _Compile_Debug_GetESP ( ( byte* ) & _CfrTil_->Debugger0->DebugESP ) ;
    _Compile_Call ( ( byte* ) CfrTil_DebugRuntimeBreakpoint ) ;
}

