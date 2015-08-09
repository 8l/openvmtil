
#include "../includes/cfrtil.h"

void
Debugger_Menu ( )
{
    Printf ( ( byte* ) 
        "\n(m)enu, so(u)rce, dum(p), (e)val, (d)is, dis(a)ccum (r)egisters, (l)ocals, (v)ariables, (I)nfo, (w)dis, s(h)ow"
        "\nsto(P), (S)tate, (c)ontinue, (s)tep, (o)ver, (i)nto, s(t)ack, (z)auto, (V)erbosity, (q)uit, (A)bort, '\\\' - escape" ) ;
}

void
Debugger_Disassemble ( Debugger * debugger )
{
    Printf ( ( byte* ) "\n" ) ;
    Debugger_GetWordFromAddress ( debugger ) ;
    Word * word = debugger->w_Word ;
    if ( word )
    {
        Printf ( ( byte* ) "\rDisassembly of : %s.%s\n", word->ContainingNamespace->Name, c_dd ( word->Name ) ) ;
        int32 codeSize = word->S_CodeSize ;
        _Debugger_Disassemble ( debugger, ( byte* ) word->CodeStart, codeSize ? codeSize : 64, word->CType & ( CPRIMITIVE | DLSYM_WORD ) ? 1 : 0 ) ;
        if ( debugger->DebugAddress )
        {
            Printf ( ( byte* ) "\nNext instruction ..." ) ;
            Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\n", ( byte* ) "" ) ; // the next instruction
        }
    }
    Printf ( ( byte* ) "\n" ) ;
}

void
_Debugger_DisassembleWrittenCode ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    uint32 codeSize ; 
    byte * optimizedCode ; 
    if ( debugger->LastShowWord == word ) return ;
    optimizedCode = debugger->OptimizedCodeAffected ;
    if ( optimizedCode )
    {
        if ( optimizedCode < debugger->PreHere ) debugger->PreHere = optimizedCode ;
    }
    codeSize = Here - debugger->PreHere ;
    if ( word && codeSize )
    {
        ConserveNewlines ;
        Printf ( ( byte* ) "\nCode compiled for word :> %s <: ...\n", cc ( word->Name, &_Q_->Default ) ) ;
        _Debugger_Disassemble ( debugger, debugger->PreHere, codeSize, word->CType & ( CPRIMITIVE | DLSYM_WORD ) ? 1 : 0 ) ;
    }
}

void
Debugger_Locals_Show ( Debugger * debugger )
{
    byte *address, * buffer = Buffer_New_pbyte ( BUFFER_SIZE ) ;
    Word * word, * word2 ;
    if ( ! GetState ( debugger, DBG_STEPPING ) ) //CompileMode )
    {
        Printf ( ( byte* ) cc ( "\nLocal variables can be shown only at run time not at Compile time!", &_Q_->Alert ) ) ;
        return ;
    }
    ReadLiner * rl = _Context_->ReadLiner0 ;
    if ( word = debugger->w_Word )
    {
        if ( debugger->Locals ) _Namespace_Clear ( debugger->Locals ) ;
        Compiler_Init ( _Context_->Compiler0, 0 ) ;
        //debugger->Locals = _Namespace_New ( ( byte* ) "DebugLocals", _CfrTil_->Namespaces ) ;
        debugger->Locals = _DataObject_New ( NAMESPACE, ( byte* ) "DebugLocals", 0, 0, 0, (int32) _CfrTil_->Namespaces ) ;
        int32 s, e ;
        byte buffer [ 256 ], * start, * sc = debugger->w_Word->SourceCode ;
        if ( sc )
        {
            for ( s = 0 ; sc [ s ] && sc [ s ] != '(' ; s ++ ) ;
            if ( sc [ s ] )
            {
                start = & sc [ s + 1 ] ; // right after '(' is how _CfrTil_Parse_LocalsAndStackVariables is set up
                for ( e = s ; sc [ e ] && sc [ e ] != ')' ; e ++ ) ; // end = & sc [ e ] ;
                if ( sc [ e ] )
                {
                    strncpy ( ( char* ) buffer, ( char* ) start, e - s + 1 ) ;
                    buffer [ e - s + 1 ] = 0 ;
                    String_InsertDataIntoStringSlot ( ( char* ) rl->InputLine, rl->ReadIndex, rl->ReadIndex, ( char* ) buffer ) ;
                    _CfrTil_Parse_LocalsAndStackVariables ( 1, 1, 0, 0 ) ; // stack variables & debug flags
                }
            }
        }
        DLNode * node ;
        // show value of each local var on Locals list
        char * registerNames [ 8 ] = { ( char* ) "EAX", ( char* ) "ECX", ( char* ) "EDX", ( char* ) "EBX", ( char* ) "ESP", ( char* ) "EBP", ( char* ) "ESI", ( char* ) "EDI" } ;
        if ( ! Debugger_IsStepping ( debugger ) )
        {
            debugger->SaveCpuState ( ) ;
        }
        int32 * fp = ( int32* ) debugger->cs_CpuState->Edi, * dsp = ( int32* ) debugger->cs_CpuState->Esi ;
        if ( ( uint32 ) fp > 0xf0000000 )
        {
            Printf ( ( byte* ) "\nLocal Variables for %s.%s : Frame Pointer = EDI = <0x%08x> = 0x%08x : Stack Pointer = ESI <0x%08x> = 0x%08x",
                c_dd ( word->ContainingNamespace->Name ), c_dd ( word->Name ), ( uint ) fp, fp ? *fp : 0, ( uint ) dsp, dsp ? *dsp : 0 ) ;
            for ( node = DLList_Last ( debugger->Locals->Lo_List ) ; node ; node = DLNode_Previous ( node ) )
            {
                word = ( Word * ) node ;
                int32 wi = word->Index ;
                if ( word->CType & REGISTER_VARIABLE ) Printf ( "\nReg   Variable : %-12s : %s : 0x%x", word->Name, registerNames [ word->RegToUse ], _CfrTil_->cs_CpuState->Registers [ word->RegToUse ] ) ;
                else if ( word->CType & LOCAL_VARIABLE )
                {
                    address = ( byte* ) fp [ wi + 1 ] ;
                    word2 = Word_GetFromCodeAddress ( ( byte* ) ( address ) ) ;
                    if ( word2 ) sprintf ( ( char* ) buffer, "< %s.%s >", word2->ContainingNamespace->Name, word2->Name ) ;
                    Printf ( ( byte* ) "\nLocal Variable : %s :  index = +%-2d : <0x%08x> = 0x%08x\t\t%s", word->Name, wi + 1, fp + wi + 1, fp [ wi + 1 ], word2 ? ( char* ) buffer : "" ) ;
                }
                else if ( word->CType & STACK_VARIABLE )
                {
                    address = ( byte* ) fp [ - ( wi + 1 ) ] ;
                    word2 = Word_GetFromCodeAddress ( ( byte* ) ( address ) ) ;
                    if ( word2 ) sprintf ( ( char* ) buffer, "< %s.%s >", word2->ContainingNamespace->Name, word2->Name ) ;
                    Printf ( ( byte* ) "\nStack Variable : %s :  index = %-2d  : <0x%08x> = 0x%08x\t\t%s", word->Name, - ( wi + 1 ), dsp + wi + 1, fp [ - ( wi + 1 ) ], word2 ? ( char* ) buffer : "" ) ;
                }
            }
            Printf ( ( byte * ) "\n" ) ;
        }
        else Printf ( ( byte* ) "\nNo locals available - (yet).\n" ) ;
    }
}

void
Debugger_ShowWrittenCode ( Debugger * debugger, int32 stepFlag )
{
    char * b = ( char* ) Buffer_Data ( _CfrTil_->DebugB ) ;
    char * c = ( char* ) Buffer_Data ( _CfrTil_->DebugB2 ) ;
    const char * insert ;
    byte * name ;
    int32 change, depthChange ;
    NoticeColors ;
    _Debugger_DisassembleWrittenCode ( debugger ) ;
    if ( Debugger_IsStepping ( debugger ) ) change = Dsp - debugger->SaveDsp ;
    else change = Dsp - debugger->WordDsp ;
    depthChange = DataStack_Depth ( ) - debugger->SaveStackDepth ;
    if ( GetState ( debugger, DBG_STACK_CHANGE ) || ( change ) || ( debugger->SaveTOS != TOS ) || ( depthChange ) )
    {
        byte pb_change [ 256 ] ;
        pb_change [ 0 ] = 0 ;

        if ( GetState ( debugger, DBG_STACK_CHANGE ) ) SetState ( debugger, DBG_STACK_CHANGE, false ) ;
        if ( depthChange > 0 ) sprintf ( ( char* ) pb_change, "%d %s%s", depthChange, ( depthChange > 1 ) ? "cells" : "cell", " pushed onto to the stack. " ) ;
        else if ( depthChange ) sprintf ( ( char* ) pb_change, "%d %s%s", - depthChange, ( depthChange > 1 ) ? "cells" : "cell", " popped off the stack. " ) ;
        if ( debugger->SaveTOS != TOS )
        {
            sprintf ( ( char* ) c, ( char* ) "0x%x", TOS ) ;
            sprintf ( ( char* ) b, ( char* ) "TOS changed to %s.", cc ( c, &_Q_->Default ) ) ;
            strcat ( ( char* ) pb_change, ( char* ) b ) ; // strcat ( (char*) _change, cc ( ( char* ) c, &_Q_->Default ) ) ;
        }
        if ( debugger->w_Word )
        {
            name = debugger->w_Word->Name ;
        }
        else
        {
            name = _Context_->Lexer0->OriginalToken ;
        }
        char * achange = ( char* ) pb_change ;
        if ( stepFlag )
        {
            Word * word = Word_GetFromCodeAddress ( debugger->DebugAddress ) ;
            if ( ( word ) && ( ( byte* ) word->Definition == debugger->DebugAddress ) )
            {
                insert = "function call" ;
                if ( achange [0] )
                {
                    if ( GetState ( debugger, DBG_STEPPING ) ) Printf ( ( byte* ) "Stack changed by %s :> %s <: %s ...", insert, word->Name, achange ) ;
                    else Printf ( ( byte* ) "\nStack changed by %s :> %s <: %s ...", insert, word->Name, achange ) ;
                }
            }
            else
            {
                if ( ( ( * debugger->DebugAddress ) != 0x83 ) && ( ( * debugger->DebugAddress ) != 0x81 ) )// add esi
                {
                    insert = "instruction" ;
                    if ( achange [0] )
                    {
                        if ( GetState ( debugger, DBG_STEPPING ) ) Printf ( ( byte* ) "Stack changed by %s :> 0x%x <: %s ...", insert, ( uint ) debugger->DebugAddress, achange ) ;
                        else Printf ( ( byte* ) "\nStack changed by %s :> 0x%x <: %s ...", insert, ( uint ) debugger->DebugAddress, achange ) ;
                    }
                }
                else SetState ( debugger, DBG_STACK_CHANGE, true ) ;
            }
        }
        else
        {
            if ( debugger->w_Word )
            {
                insert = "word" ;
            }
            else
            {
                insert = "token" ;
            }
            if ( achange [0] )
            {
                if ( GetState ( debugger, DBG_STEPPING ) ) Printf ( ( byte* ) "Stack changed by %s :> %s <: %s ...", insert, cc ( name, &_Q_->Default ), achange ) ;
                else Printf ( ( byte* ) "\nStack changed by %s :> %s <: %s ...", insert, cc ( name, &_Q_->Default ), achange ) ;
            }
        }
        if ( Lexer_GetState ( _Context_->Lexer0, KNOWN_OBJECT ) )
        {
            if ( Dsp > debugger->SaveDsp )
            {
                Printf ( ( byte* ) "\nLiteral :> 0x%x <: was pushed onto the stack ...", TOS ) ;
            }
            else if ( Dsp < debugger->SaveDsp )
            {
                Printf ( ( byte* ) "\n%s popped %d value off the stack.", insert, ( debugger->SaveDsp - Dsp ) ) ;
            }
            // else if (TOS != old TOS ) Printf ...
        }
        if ( change && ( _CfrTil_->DebuggerVerbosity > 1 ) ) CfrTil_PrintDataStack ( ) ; //!! nb. commented out for DEBUG ONLY - normally uncomment !!
    }
    debugger->LastShowWord = debugger->w_Word ;
    DebugColors ;
}

void
_CfrTil_ShowInfo ( byte * prompt, int32 signal )
{
    byte *location ;
    byte signalAscii [ 128 ] ;
    ReadLiner * rl = _Context_->ReadLiner0 ;

    //NoticeColors ;
    ConserveNewlines ;
    if ( ! ( _Context_ && _Context_->Lexer0 ) )
    {
        Throw ( ( byte* ) "\nNo token at _CfrTil_ShowInfo\n", QUIT ) ;
    }
    if ( rl->bp_Filename ) location = rl->bp_Filename ;
    else location = ( byte* ) "<command line>" ;
    if ( ( signal == 11 ) || _Q_->SigAddress ) sprintf ( ( char* ) signalAscii, "\nError : signal " INT_FRMT ":: attempting address : " UINT_FRMT_0x08, signal, ( uint ) _Q_->SigAddress ) ;
    else if ( signal ) sprintf ( ( char* ) signalAscii, "\nError : signal " INT_FRMT " ", signal ) ;

    byte * token = _Context_->Lexer0->OriginalToken ;
    Word * word = _Context_->Interpreter0->w_Word ;
    if ( token && ( ( ! word ) || ( ! word->Lo_Name ) || strcmp ( ( char* ) word->Lo_Name, ( char* ) token ) ) )
    {
        word = Finder_Word_FindUsing ( _Context_->Finder0, token ) ;
    }
    else if ( word && ( ! token ) )
    {
        token = word->Name ;
    }

    //Buffer * bb = Buffer_New ( BUFFER_SIZE ) ;
    //byte * b = Buffer_Data ( bb ) ;
    char * b = ( char* ) Buffer_Data ( _CfrTil_->DebugB ) ;
    strcpy ( ( char* ) b, ( char* ) rl->InputLine ) ;
    char * line = ( char* ) cc ( ( char* ) String_RemoveFinalNewline ( b ), &_Q_->Default ) ;
    if ( token )
    {
        char * cc_Token = ( char* ) c_dd ( token ) ;
        char * cc_location = ( char* ) c_dd ( location ) ;

        prompt = ( char* ) prompt ? ( char* ) prompt : "" ;
        if ( word )
        {
            if ( word->CType & CPRIMITIVE )
            {
                Printf ( ( byte* ) "\n%s%s:: %s : %03d.%03d : %s :> %s <: cprimitive :> %s <:: " INT_FRMT "." INT_FRMT,
                    prompt, signal ? signalAscii : ( byte* ) " ", cc_location,
                    rl->i32_LineNumber, rl->ReadIndex,
                    word->ContainingNamespace ? ( char* ) word->ContainingNamespace->Name : "no namespace",
                    cc_Token, line, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
            }
            else
            {
                Printf ( ( byte* ) "\n%s%s:: %s : %03d.%03d : %s :> %s <: 0x%08x :> %s <:: " INT_FRMT "." INT_FRMT,
                    prompt, signal ? signalAscii : ( byte* ) " ", cc_location,
                    rl->i32_LineNumber, rl->ReadIndex,
                    word->ContainingNamespace ? ( char* ) word->ContainingNamespace->Name : ( char* ) "no namespace",
                    cc_Token, ( uint ) word->Definition, line, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
            }
        }
        else
        {
            Printf ( ( byte* ) "\n%s%s:: %s : %03d.%03d : %s :> %s <::> %s <:: " INT_FRMT "." INT_FRMT,
                prompt, signal ? signalAscii : ( byte* ) " ", cc_location,
                rl->i32_LineNumber, rl->ReadIndex,
                "<literal>", cc_Token, line, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
        }
    }
    else
    {

        Printf ( ( byte* ) "\n%s %s:: %s : %03d.%03d :> %s <:: " INT_FRMT "." INT_FRMT,
            prompt, signal ? signalAscii : ( byte* ) "", location,
            rl->i32_LineNumber, rl->ReadIndex,
            line, _Q_->StartedTimes, _Q_->SignalExceptionsHandled ) ;
    }
    //Buffer_SetAsUnused ( bb ) ;
}

void
Debugger_ShowInfo ( Debugger * debugger, byte * prompt, int32 signal )
{
    ConserveNewlines ;
    if ( ! ( _Context_ && _Context_->Lexer0 ) )
    {
        Printf ( ( byte* ) "\nSignal Error : signal = %d\n", signal ) ;
        return ;
    }
    if ( ! Debugger_GetState ( _CfrTil_->Debugger0, DBG_ACTIVE ) )
    {
        debugger->Token = _Context_->Lexer0->OriginalToken ;
        if ( signal > SIGSEGV ) Debugger_FindUsing ( debugger ) ;
    }
    if ( debugger->w_Word ) debugger->Token = debugger->w_Word->Name ;
    if ( Debugger_IsStepping ( debugger ) && debugger->DebugAddress )
    {
        Printf ( ( byte* ) "\nDebug Stepping Address : 0x%08x\n", ( uint ) debugger->DebugAddress ) ;
        Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\n", ( byte* ) "\r" ) ; // the next instruction
    }
    _CfrTil_ShowInfo ( prompt, signal ) ;
}

void
Debugger_ShowState ( Debugger * debugger, byte * prompt )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    Word * word = debugger->w_Word ;
    int cflag = 0 ;
    if ( word )
    {
        if ( word->CType & CONSTANT ) cflag = 1 ;
    }
    DebugColors ;
    ConserveNewlines ;
    if ( word )
    {
        Printf ( ( byte* ) ( cflag ? "\n%s :: %03d.%03d : %s : <constant> : %s%s%s " : word->ContainingNamespace ? "\n%s :: %03d.%03d : %s : <word> : %s%s%s " : "\n%s :: %03d.%03d : %s : <word?> : %s%s%s " ),
            prompt, rl->i32_LineNumber, rl->ReadIndex, Debugger_GetStateString ( debugger ),
            // _CfrTil_->Namespaces doesn't have a ContainingNamespace
            word->ContainingNamespace ? word->ContainingNamespace->Name : ( byte* ) "",
            word->ContainingNamespace ? ( byte* ) "." : ( byte* ) "", // the dot between
            cc ( word->Name, &_Q_->Default ) ) ;
    }
    else if ( debugger->Token )
    {
        Printf ( ( byte* ) ( cflag ? "\n%s :: %03d.%03d : %s : <constant> :> %s " : "\n%s :: %03d.%03d : %s : <literal> :> %s " ),
            prompt, rl->i32_LineNumber, rl->ReadIndex, Debugger_GetStateString ( debugger ), cc ( debugger->Token, &_Q_->Default ) ) ;
    }
    else Printf ( ( byte* ) "\n%s :: %03d.%03d : %s : ", prompt, rl->i32_LineNumber, rl->ReadIndex, Debugger_GetStateString ( debugger ) ) ;
    if ( ! debugger->Key )
    {
        if ( word ) _CfrTil_Source ( word, 0 ) ;
        if ( Debugger_GetState ( debugger, DBG_STEPPING ) )
            Debugger_UdisOneInstruction ( debugger, debugger->DebugAddress, ( byte* ) "\r", ( byte* ) "\r" ) ; // current insn
    }
}

void
Debugger_ConsiderAndShowWord ( Debugger * debugger )
{
    Word * word = debugger->w_Word ;
    Debugger_SetState ( debugger, DBG_CAN_STEP, false ) ; // debugger->State flag = false ;
    AllowNewlines ;
    if ( word ) // then it wasn't a literal
    {
        byte * name = ( byte* ) cc ( word->Name, &_Q_->Default ) ;
        if ( ( word->CType & ( CPRIMITIVE | DLSYM_WORD ) ) && ( ! ( CompileMode ) ) )
        {
            if ( word->ContainingNamespace ) Printf ( ( byte* ) "\ncprimitive :> %s.%s <:> 0x%08x <: => evaluating ...", word->ContainingNamespace->Name, name, ( uint ) word->Definition ) ;
        }
        else
        {
            Debugger_SetState ( debugger, DBG_CAN_STEP, true ) ;
            if ( Debugger_GetState ( debugger, DBG_INTERNAL ) )
            {
                Debugger_Info ( debugger ) ;
                //Printf ( ( byte* ) "\nInternal DebugAddress = :> 0x%08x <: ", ( unsigned int ) debugger->DebugAddress ) ;
            }
            else if ( word->CType & VARIABLE )
            {
                Printf ( ( byte* ) "\nVariable :> %s.%s <: => evaluating ... :> ", word->ContainingNamespace->Name, name ) ;
                Debugger_SetState ( debugger, DBG_CAN_STEP, false ) ;
            }
            else if ( word->CType & TEXT_MACRO )
            {
                Printf ( ( byte* ) "\nMacro :> %s.%s <: => evaluating ... :> ", word->ContainingNamespace->Name, name ) ;
                Debugger_SetState ( debugger, DBG_CAN_STEP, false ) ;
            }
            else if ( word->CType & IMMEDIATE )
            {
                Printf ( ( byte* ) "\nImmediate word :> %s.%s <:> 0x%08x <: => evaluating ...",
                    word->ContainingNamespace->Name, name, ( uint ) word->Definition ) ;
            }
            else if ( CompileMode )
            {
                Printf ( ( byte* ) "\nCompileMode :> %s.%s : %s <: not a macro => compiling ...",
                    word->ContainingNamespace->Name, name, Debugger_GetStateString ( debugger ) ) ;
            }
            else
            {
                Printf ( ( byte* ) "\nCompiled word :> %s.%s <:> 0x%08x <: => evaluating ...",
                    word->ContainingNamespace->Name, name, ( uint ) word->Definition ) ;
            }
            _Word_ShowSourceCode ( word ) ;
            DefaultColors ;
        }
    }
    else
    {
        if ( debugger->Token )
        {
            Lexer_ParseObject ( _Context_->Lexer0, debugger->Token ) ;
            if ( ( Lexer_GetState ( _Context_->Lexer0, KNOWN_OBJECT ) ) )
            {
                if ( CompileMode )
                {
                    Printf ( ( byte* ) "\nCompileMode :> %s <: literal stack push will be compiled ...", _Context_->Lexer0->OriginalToken ) ;
                }
                else Printf ( ( byte* ) "\nLiteral :> %s <: will be pushed onto the stack ...", _Context_->Lexer0->OriginalToken ) ;
            }
        }
    }
}

