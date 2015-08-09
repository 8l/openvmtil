
#include "../includes/cfrtil.h"

void
_Compiler_SetCompilingSpace ( byte * name )
{
    NamedByteArray *nba = _OVT_Find_NBA ( name ) ;
    Set_CompilerSpace ( nba->ba_ByteArray ) ;
}

Word *
Compiler_PreviousNonDebugWord ( )
{
    Word * word ;
    int32 i ;
    for ( i = - 1 ; ( word = ( Word* ) Compiler_WordStack ( _Context_->Compiler0, i ) ) && i > - 3 ; i -- )
    {
        if ( ( Symbol* ) word && ( ! ( word->CType & DEBUG_WORD ) ) ) break ;
    }
    return word ;
}

void
_Compiler_FreeBlockInfoLocalsNamespace ( BlockInfo * bi, Compiler * compiler )
{
    Namespace * ns = bi->LocalsNamespace ;
    if ( ns )
    {
        //ns = (Namespace*) Stack_Pop ( compiler->LocalNamespaces ) ;
        _Namespace_RemoveFromUsingListAndClear ( ns ) ;
    }
}

void
_Compiler_FreeLocalsNamespace ( Compiler * compiler )
{
    Namespace * ns = ( Namespace* ) Stack_Pop ( compiler->LocalNamespaces ) ;
    if ( ns ) _Namespace_RemoveFromUsingListAndClear ( ns ) ;
}

void
_Compiler_WordStack_PushWord ( Compiler * compiler, Word * word )
{
    if ( ! ( word->CType & ( DEBUG_WORD ) ) ) Stack_Push ( compiler->WordStack, ( int32 ) word ) ;
}

void
_Compiler_FreeAllLocalsNamespaces ( Compiler * compiler )
{
#if 0    
    Namespace * ns ;
    while ( ( ns = ( Namespace* ) Stack_Pop_WithZeroOnEmpty ( compiler->LocalNamespaces ) ) )
    {
        _Namespace_RemoveFromUsingListAndClear ( ns ) ;
    }
#else
    while ( Stack_Depth ( compiler->LocalNamespaces ) )
    {
        _Compiler_FreeLocalsNamespace ( compiler ) ;
    }
#endif    
}

void
CompileOptimizer_Init ( CompileOptimizer * optimizer )
{
    memset ( optimizer, 0, sizeof (CompileOptimizer ) ) ;
}

void
CompileOptimizer_NewInit ( CompileOptimizer * optimizer )
{
    CompileOptimizer_Init ( optimizer ) ;
}

CompileOptimizer *
CompileOptimizer_New ( int32 type )
{
    CompileOptimizer * optimizer = ( CompileOptimizer * ) _Mem_Allocate ( sizeof (CompileOptimizer ), type ) ;
    CompileOptimizer_NewInit ( optimizer ) ;
    return optimizer ;
}

void
CompileOptimizer_Delete ( CompileOptimizer * optimizer )
{
    Mem_FreeItem ( _Q_->PermanentMemList, ( byte* ) optimizer ) ;
}

void
CfrTil_InitBlockSystem ( Compiler * compiler )
{
    Stack_Init ( compiler->BlockStack ) ;
    Stack_Init ( compiler->CombinatorBlockInfoStack ) ;
}

void
Compiler_Init ( Compiler * compiler, int32 state )
{
    _DLList_Init ( compiler->GotoList ) ;
    Stack_Init ( compiler->WordStack ) ;
    Stack_Init ( compiler->ObjectStack ) ;
    CfrTil_InitBlockSystem ( compiler ) ;
    compiler->ContinuePoint = 0 ;
    compiler->BreakPoint = 0 ;
    compiler->InitHere = Here ;
    compiler->State = state ;
    compiler->LispParenLevel = 0 ;
    compiler->ParenLevel = 0 ;
    compiler->BlockLevel = 0 ;
    compiler->ArrayEnds = 0 ;
    CompileOptimizer_NewInit ( compiler->Optimizer ) ;
    compiler->NumberOfLocals = 0 ;
    compiler->NumberOfStackVariables = 0 ;
    compiler->NumberOfRegisterVariables = 0 ;
    compiler->LocalsFrameSize = 0 ;
    compiler->FunctionTypesArray = 0 ;
    compiler->AccumulatedOffsetPointer = 0 ;
    compiler->RecursiveWord = 0 ;
    Stack_Init ( compiler->PointerToOffset ) ;
    Stack_Init ( compiler->CombinatorInfoStack ) ;
    _Compiler_FreeAllLocalsNamespaces ( compiler ) ;
    Stack_Init ( compiler->LocalNamespaces ) ;
    Stack_Init ( compiler->InfixOperatorStack ) ;
    _Compiler_SetCompilingSpace ( ( byte* ) "CodeSpace" ) ;
    _Compiler_ = compiler ;
    OVT_MemListFree_TempObjects ( ) ;
    _DLList_Init ( _CfrTil_->PeekTokenList ) ;
    _DLList_Init ( _CfrTil_->TokenList ) ;
}

Compiler *
Compiler_New ( int32 type )
{
    Compiler * compiler = ( Compiler * ) _Mem_Allocate ( sizeof (Compiler ), type ) ;
    compiler->BlockStack = Stack_New ( 64, type ) ;
    compiler->WordStack = Stack_New ( 10 * K, type ) ;
    compiler->ObjectStack = Stack_New ( 64, type ) ;
    compiler->CombinatorBlockInfoStack = Stack_New ( 64, type ) ;
    compiler->GotoList = _DLList_New ( type ) ;
    compiler->LocalNamespaces = Stack_New ( 32, type ) ;
    compiler->NamespacesStack = Stack_New ( 32, type ) ;
    compiler->PointerToOffset = Stack_New ( 32, type ) ;
    compiler->CombinatorInfoStack = Stack_New ( 64, type ) ;
    compiler->InfixOperatorStack = Stack_New ( 32, type ) ;
    compiler->Optimizer = CompileOptimizer_New ( type ) ;
    Compiler_Init ( compiler, 0 ) ;
    return compiler ;
}

#if 0

void
Compiler_Delete ( Compiler * compiler )
{
    _Compiler_FreeAllLocalsNamespaces ( compiler ) ;
    Mem_FreeItem ( _Q_->PermanentMemList, ( byte* ) compiler ) ;
}
#endif

void
CfrTil_CalculateAndSetPreviousJmpOffset ( byte * jmpToAddress )
{
    // we can now not compile blocks (cf. _Compile_Block_WithLogicFlag ) if their logic is not called so depth check is necessary
    if ( _Stack_Depth ( _Context_->Compiler0->PointerToOffset ) ) _SetOffsetForCallOrJump ( ( byte* ) Stack_Pop ( _Context_->Compiler0->PointerToOffset ), jmpToAddress, 0 ) ;
}

void
CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( )
{
    CfrTil_CalculateAndSetPreviousJmpOffset ( Here ) ;
}

void
__Stack_PointerToJmpOffset_Set ( byte * address )
{
    Stack_Push ( _Context_->Compiler0->PointerToOffset, ( int32 ) address ) ;
}

void
_Stack_PointerToJmpOffset_Set ( )
{
    __Stack_PointerToJmpOffset_Set ( Here - CELL_SIZE ) ;
}


