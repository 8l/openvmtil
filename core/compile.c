
#include "../includes/cfrtil.h"

void
_CompileN ( byte * data, int32 size )
{
    ByteArray_AppendCopy ( CompilerMemByteArray, size, data ) ; // size in bytes
}

// nb : can be used by an compiler on
// a code body that has only one RET point

void
_CompileFromUptoRET ( byte * data )
{
    ByteArray_AppendCopyUpToRET ( CompilerMemByteArray, data ) ; // size in bytes
}

// copy block from 'address' to Here

int32
_Compile_Block_WithLogicFlag ( byte * srcAddress, int32 bindex, int32 jccFlag, int n )
{
    Compiler * compiler = _Context_->Compiler0 ;
    int32 jccFlag2 ;
    BlockInfo *bi = ( BlockInfo * ) _Stack_Pick ( compiler->CombinatorBlockInfoStack, bindex ) ; // -1 : remember - stack is zero based ; stack[0] is top
    if ( jccFlag )
    {
        if ( ! ( _LC_ && GetState ( _LC_, ( LISP_COMPILE_MODE ) ) ) )
        {
            if ( bi->LiteralWord )//&& bi->LiteralWord->StackPushRegisterCode ) // leave value in EAX, don't push it
            {
                if ( bi->LiteralWord->Value != 0 )
                {
                    return 1 ; // nothing need to be compiled 
                }
                // else somehow don't use this block at all ie. eliminate the dead code and don't just ...
                return 0 ; // TODO : don't use the block/combinator
            }
        }
        jccFlag2 = Compile_ReConfigureLogicInBlock ( bi, 1 ) ;
    }
    if ( ! CfrTil_GetState ( _CfrTil_, INLINE_ON ) ) _Compile_Call ( srcAddress ) ;
    else
    {
        _Block_Copy ( srcAddress, bi->bp_Last - bi->bp_First ) ;
    }
    if ( jccFlag )
    {
        if ( jccFlag2 )
        {
            _Compile_JCC ( n ? bi->NegFlag : ! bi->NegFlag, bi->Ttt, 0 ) ;
        }
        else
        {
#if 1           
            Compile_GetLogicFromTOS ( bi ) ;
            _Compile_JCC ( n, ZERO, 0 ) ;
#else
            if ( Compile_GetLogicFromTOS ( bi ) ) _Compile_JCC ( n, ZERO, 0 ) ;
            else return ; //_Compile_UninitializedJump ( ) ;
#endif            
        }
        __Stack_PointerToJmpOffset_Set ( Here - CELL ) ;
    }
    return 1 ;
}

int32
_Compile_Block ( byte * srcAddress, int32 bindex, int32 jccFlag )
{
    return _Compile_Block_WithLogicFlag ( srcAddress, bindex, jccFlag, 0 ) ;
}

void
_Compile_WordInline ( Word * word ) // , byte * dstAddress )
{
    _Block_Copy ( ( byte* ) word->Definition, word->S_CodeSize ) ;
}

void
_CompileWord ( Word * word )
{
    if ( word->CType & CATEGORY_RECURSIVE && ( word->State & NOT_COMPILED ) )
    {
        CfrTil_SetupRecursiveCall ( ) ;
    }
    else
    {
        //if ( word->CType & RT_STACK_OP ) _Compile_ESI_To_Dsp ( ) ;
        if ( ( CfrTil_GetState ( _CfrTil_, INLINE_ON ) ) && ( word->CType & INLINE ) && ( word->S_CodeSize ) )
        {
            _Compile_WordInline ( word ) ;
        }
        else
        {
            _Compile_Call ( ( byte* ) word->Definition ) ; // jsr
        }
        //if ( word->CType & RT_STACK_OP ) _Compile_Dsp_To_ESI ( ) ;
    }
}

void
_CompileFromName ( byte * wordName )
{
    Word * word = Finder_Word_FindUsing ( _Context_->Finder0, wordName ) ;
    // ?? Exception : error message here
    if ( ! word ) _Throw ( QUIT ) ;
    _CompileWord ( word ) ;
}

void
_CompileFromName_Inline ( byte * wordName )
{
    Word * word = Finder_Word_FindUsing ( _Context_->Finder0, wordName ) ;
    if ( ! word ) _Throw ( QUIT ) ;
    _Compile_WordInline ( word ) ;
}

void
_MoveGotoPoint ( DLNode * node, int32 srcAddress, int32 key, int32 dstAddress )
{
    GotoInfo * gotoInfo = ( GotoInfo* ) node ;
    byte * address = gotoInfo->pb_JmpOffsetPointer ;
    if ( ( byte* ) srcAddress == address ) gotoInfo->pb_JmpOffsetPointer = ( byte* ) dstAddress ;
}

void
_GotoInfo_SetAndDelete ( GotoInfo * gotoInfo, byte * address )
{
    _SetOffsetForCallOrJump ( gotoInfo->pb_JmpOffsetPointer, address, 0 ) ;
    GotoInfo_Delete ( ( DLNode* ) gotoInfo ) ;
}

void
_InstallGotoPoint_Key ( DLNode * node, int32 bi, int32 key )
{
    Word * word ;
    GotoInfo * gotoInfo = ( GotoInfo* ) node ;
    byte * address = gotoInfo->pb_JmpOffsetPointer ;
    if ( *( int32* ) address == 0 ) // if we move a block its recurse offset remains, check if this looks like at real offset pointer
    {
        if ( ( gotoInfo->GI_CType & ( GI_GOTO | GI_CALL_LABEL ) ) && ( key & ( GI_GOTO | GI_CALL_LABEL ) ) )
        {
            Namespace * ns = Namespace_FindOrNew_SetUsing ( ( byte* ) "__labels__", _CfrTil_->Namespaces, 1 ) ;
            if ( ( word = Word_FindInOneNamespace ( ns, gotoInfo->pb_LabelName ) ) )
            {
                _GotoInfo_SetAndDelete ( gotoInfo, word->bp_WD_Object ) ;
            }
        }
        else if ( ( gotoInfo->GI_CType & GI_RETURN ) && ( key & GI_RETURN ) )
        {
            _GotoInfo_SetAndDelete ( gotoInfo, Here ) ;
        }
        else if ( ( gotoInfo->GI_CType & GI_BREAK ) && ( key & GI_BREAK ) )
        {
            if ( _Context_->Compiler0->BreakPoint )
            {
                _GotoInfo_SetAndDelete ( gotoInfo, _Context_->Compiler0->BreakPoint ) ;
            }
        }
        else if ( ( gotoInfo->GI_CType & GI_CONTINUE ) && ( key & GI_CONTINUE ) )
        {
            if ( _Context_->Compiler0->ContinuePoint )
            {
                _GotoInfo_SetAndDelete ( gotoInfo, _Context_->Compiler0->ContinuePoint ) ;
            }
        }
        else if ( ( gotoInfo->GI_CType & GI_RECURSE ) && ( key & GI_RECURSE ) )
        {
            _GotoInfo_SetAndDelete ( gotoInfo, ( byte* ) ( ( BlockInfo * ) bi )->bp_First ) ;
        }
        else if ( ( gotoInfo->GI_CType & GI_TAIL_CALL ) && ( key & GI_TAIL_CALL ) )
        {
            _GotoInfo_SetAndDelete ( gotoInfo, ( ( BlockInfo * ) bi )->Start ) ; // ( byte* ) _DataStack_GetTop ( ) ) ; // , ( byte* ) bi->bi_FrameStart ) ;
        }
    }
}

void
_CheckForGotoPoint ( DLNode * node, int32 key, int32 * status )
{
    GotoInfo * gotoInfo = ( GotoInfo* ) node ;
    if ( gotoInfo->GI_CType & key )
    {
        *status = DONE ;
    }
}

void
_RemoveGotoPoint ( DLNode * node, int32 key, int32 * status )
{
    GotoInfo * gotoInfo = ( GotoInfo* ) node ;
    if ( gotoInfo->GI_CType & key )
    {
        DLNode_Remove ( ( DLNode* ) gotoInfo ) ;
        *status = DONE ;
    }
}

void
_CfrTil_InstallGotoCallPoints_Keyed ( BlockInfo * bi, int32 key )
{
    DLList_Map2 ( _Context_->Compiler0->GotoList, ( MapFunction2 ) _InstallGotoPoint_Key, ( int32 ) bi, key ) ;
}

void
_CfrTil_MoveGotoPoint ( int32 srcAddress, int32 key, int32 dstAddress )
{
    DLList_Map3 ( _Context_->Compiler0->GotoList, ( MapFunction3 ) _MoveGotoPoint, srcAddress, key, dstAddress ) ;
}

int32
CfrTil_CheckForGotoPoints ( int32 key ) // compile time
{
    int32 status = 0 ;
    DLList_Map_OnePlusStatus ( _Context_->Compiler0->GotoList, ( MapFunction2 ) _CheckForGotoPoint, key, &status ) ;
    return status ;
}

int32
CfrTil_RemoveGotoPoints ( int32 key ) // compile time
{
    int32 status = 0 ;
    DLList_Map_OnePlusStatus ( _Context_->Compiler0->GotoList, ( MapFunction2 ) _RemoveGotoPoint, key, &status ) ;
    return status ;
}

