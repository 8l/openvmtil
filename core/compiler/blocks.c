#include "../../includes/cfrtil.h"

void
_Block_Copy ( byte * srcAddress, int32 qsize )
{
    byte * saveHere = Here, * saveAddress = srcAddress ;
    ud_t * ud = Debugger_UdisInit ( _CfrTil_->Debugger0 ) ;
    int32 isize, left ;

    for ( left = qsize ; left > 0 ; srcAddress += isize )
    {
        isize = _Udis_GetInstructionSize ( ud, srcAddress ) ;
        left -= isize ;
        if ( * srcAddress == _RET )
        {
            if ( left && ( ( * srcAddress + 1 ) != NOOP ) ) //  noop from our logic overwrite
            {
                // ?? unable at present to compile inline with more than one return in the block
                SetHere ( saveHere ) ;
                _Compile_Call ( saveAddress ) ;
            }
            break ; // don't include RET
        }
        else if ( * srcAddress == CALLI32 )
        {
            int32 offset = * ( int32* ) ( srcAddress + 1 ) ; // 1 : 1 byte opCode
            if ( ! offset )
            {
                _CfrTil_WordName_Run ( ( byte* ) "recurse" ) ;
                continue ;
            }
            else
            {
                byte * jcAddress = JumpCallInstructionAddress ( srcAddress ) ;
                Word * word = Word_GetFromCodeAddress ( jcAddress ) ;
                if ( word )
                {
                    _CompileWord ( word ) ;
                    continue ;
                }
                //else (drop to) _CompileN ( srcAddress, isize )
            }
        }
        else if ( * srcAddress == JMPI32 )
        {
            int32 offset = * ( int32* ) ( srcAddress + 1 ) ; // 1 : 1 byte opCode
            if ( offset == 0 ) // signature of a goto point
            {
                _CfrTil_MoveGotoPoint ( ( int32 ) srcAddress + 1, 0, ( int32 ) Here + 1 ) ;
                _CompileN ( srcAddress, isize ) ; // memcpy ( dstAddress, address, size ) ;
                continue ;
            }
            //else (drop to) _CompileN ( srcAddress, isize )
        }
        _CompileN ( srcAddress, isize ) ; // memcpy ( dstAddress, address, size ) ;
    }
}

// nb : only blocks with one ret insn can be successfully compiled inline

void
Block_Copy ( byte * dst, byte * src, int32 qsize )
{
    if ( dst > src )
    {
        Error_Abort ( ( byte* ) "\nBlock_Copy :: Error : dst > src.\n" ) ;
        //return ; // ?? what is going on here ??
    }
    SetHere ( dst ) ;
    _Block_Copy ( src, qsize ) ;
}

// 'tttn' is a notation from the intel manuals

void
BlockInfo_Set_tttn ( BlockInfo * bi, int32 ttt, int32 n )
{
    bi->LogicCode = Here ; // used by combinators
    bi->LogicCodeWord = _Context_->Interpreter0->w_Word ;
    bi->Ttt = ttt ;
    bi->NegFlag = n ;
}

// a 'block' is merely a notation borrowed from C
// for a pointer to an anonymous subroutine 'call'

void
_Block_Eval ( block block )
{
    block ( ) ;
}

void
CfrTil_TurnOffBlockCompiler ( )
{
    Compiler_SetState ( _Context_->Compiler0, COMPILE_MODE, false ) ;
    _Compiler_FreeAllLocalsNamespaces ( _Context_->Compiler0 ) ;
    _CfrTil_RemoveNamespaceFromUsingListAndClear ( ( byte* ) "__labels__" ) ;
}

void
CfrTil_TurnOnBlockCompiler ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    Compiler_SetState ( compiler, COMPILE_MODE, true ) ;
    Stack_Init ( compiler->WordStack ) ;
}

// blocks are a notation for subroutines or blocks of code compiled in order,
// nested in code, for any purpose, worded or anonymously
// we currently jmp over them to code which pushes
// a pointer to the beginning of the block on the stack
// so the next word will see it on the top of the stack
// some combinators take more than one block on the stack

BlockInfo *
_CfrTil_BeginBlock ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    BlockInfo *bi = ( BlockInfo * ) _Mem_Allocate ( sizeof (BlockInfo ), SESSION ) ;
    compiler->BlockLevel ++ ;
    if ( ! CompileMode ) // first block
    {
        CfrTil_TurnOnBlockCompiler ( ) ;
    }
    bi->ActualCodeStart = Here ;
    _Compile_UninitializedJump ( ) ;
    bi->JumpOffset = Here - CELL_SIZE ; // before CfrTil_CheckCompileLocalFrame after CompileUninitializedJump
    bi->bp_First = Here ; // after the jump for inlining
    if ( _Stack_IsEmpty ( compiler->BlockStack ) )
    {
        // remember : we always jmp around the blocks to the combinator ; the combinator sees the blocks on the stack and uses them otherwise they are lost
        // the jmps are optimized out so the word->Definition is a call to the first combinator
        //CheckAddLocalFrame ( _Context->Compiler0 ) ; // // since we are supporting nested locals a lookahead here would have to check to the end of the word, so ...
        // we always add a frame and if not needed move the blocks to overwrite it
        bi->FrameStart = Here ; // before _Compile_AddLocalFrame
        _Compile_ESP_Save ( ) ;
        bi->AfterEspSave = Here ; // before _Compile_AddLocalFrame
        _Compiler_AddLocalFrame ( compiler ) ; // cf EndBlock : if frame is not needed we use BI_Start else BI_FrameStart -- ?? could waste some code space ??
        Compile_InitRegisterVariables ( compiler ) ; // this function is called twice to deal with words that have locals before the first block and regular colon words
    }
    bi->Start = Here ; // after CheckAddLocalFrame
    return bi ;
}

BlockInfo *
CfrTil_BeginBlock ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    BlockInfo *bi = _CfrTil_BeginBlock ( ) ;
    _Stack_Push ( compiler->BlockStack, ( int32 ) bi ) ; // _Context->CompileSpace->IndexStart before set frame size after turn on
    _Stack_Push ( compiler->CombinatorBlockInfoStack, ( int32 ) bi ) ; // _Context->CompileSpace->IndexStart before set frame size after turn on
    SetState ( _Context_, C_LHS, true ) ;
    _Context_->Compiler0->LHS_Word = 0 ;
    return bi ;
}

BlockInfo *
_CfrTil_EndBlock0 ( )
{
    BlockInfo * bi = ( BlockInfo * ) Stack_Pop_WithExceptionOnEmpty ( _Context_->Compiler0->BlockStack ) ;
    return bi ;
}

Boolean
_Compiler_IsFrameNecessary ( Compiler * compiler )
{
    return ( compiler->NumberOfLocals || compiler->NumberOfStackVariables || Compiler_GetState ( compiler, SAVE_ESP ) ) ;
}

void
_CfrTil_EndBlock1 ( BlockInfo * bi )
{
    Compiler * compiler = _Context_->Compiler0 ;
    if ( _Stack_IsEmpty ( compiler->BlockStack ) )
    {
        _CfrTil_InstallGotoCallPoints_Keyed ( bi, GI_RETURN ) ;
        if ( compiler->NumberOfRegisterVariables && ( compiler->NumberOfStackVariables == 1 ) &&
            Compiler_GetState ( compiler, ( RETURN_TOS | RETURN_EAX ) ) )
        {
            bi->bp_First = bi->Start ;
            if ( Compiler_GetState ( compiler, RETURN_EAX ) )
            {
                Compile_Move_EAX_To_TOS ( DSP ) ;
            }
        }
        else if ( _Compiler_IsFrameNecessary ( compiler ) && ( ! GetState ( compiler, DONT_REMOVE_STACK_VARIABLES ) ) )
        {
            if ( Compiler_GetState ( compiler, SAVE_ESP ) ) // SAVE_ESP is set by 'return'
            {
                _ESP_Setup ( ) ;
                bi->bp_First = bi->FrameStart ;
            }
            else bi->bp_First = bi->AfterEspSave ; // 3 : after ESP_Save code in frame setup code
            _Compiler_RemoveLocalFrame ( compiler ) ;
        }
        else
        {
            bi->bp_First = bi->Start ;
        }
    }
    _Compile_Return ( ) ;
    _DataStack_Push ( ( int32 ) bi->bp_First ) ;
    bi->bp_Last = Here ;
    _SetOffsetForCallOrJump ( bi->JumpOffset, Here, 0 ) ;
}

void
_CfrTil_EndBlock2 ( BlockInfo * bi )
{
    Compiler * compiler = _Context_->Compiler0 ;
    if ( _Stack_IsEmpty ( compiler->BlockStack ) )
    {
        _CfrTil_InstallGotoCallPoints_Keyed ( bi, GI_GOTO | GI_RECURSE | GI_CALL_LABEL ) ;
        CfrTil_TurnOffBlockCompiler ( ) ;
    }
    else _Compiler_FreeBlockInfoLocalsNamespace ( bi, _Context_->Compiler0 ) ;
    compiler->BlockLevel -- ;
}

void
CfrTil_EndBlock ( )
{
    BlockInfo * bi = _CfrTil_EndBlock0 ( ) ;
    _CfrTil_EndBlock1 ( bi ) ;
    _CfrTil_EndBlock2 ( bi ) ;
    SetState ( _Context_, C_RHS, false ) ; // TODO : this logically seems unnecessary but the logic hasn't been tightened up regarding it yet 
    SetState ( _Context_, C_LHS, true ) ;
}

