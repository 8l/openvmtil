#include "../includes/cfrtil.h"

void
CfrTil_Here ( )
{
    _DataStack_Push ( ( int32 ) Here ) ;
}

void
CompileCall ( )
{
    _Compile_Call ( ( byte* ) _DataStack_Pop ( ) ) ;
}

void
CompileACfrTilWord ( )
{
    _CompileWord ( ( Word* ) _DataStack_Pop ( ) ) ;
}

void
CompileInt64 ( )
{
    union
    {
        int q0 [2 ] ; 
        long int q ;
    } li ;
    li.q0[1] = _DataStack_Pop ( ) ;
    li.q0[0] = _DataStack_Pop ( ) ; // little endian - low order bits should be pushed first
    _Compile_Int64 ( li.q ) ;

}

void
CompileInt32 ( )
{
    int l = _DataStack_Pop ( ) ;
    _Compile_Int32 ( l ) ;

}

void
CompileInt16 ( )
{
    int32 w = _DataStack_Pop ( ) ;
    _Compile_Int16 ( ( short ) w ) ;

}

void
CompileByte ( )
{
    int32 b = _DataStack_Pop ( ) ;
    _Compile_Int8 ( b ) ;
}

void
CompileN ( )
{
    int32 size = _DataStack_Pop ( ) ;
    byte * data = ( byte* ) _DataStack_Pop ( ) ;
    _CompileN ( data, size ) ;
}

GotoInfo *
_GotoInfo_Allocate ( )
{
    GotoInfo * gi = ( GotoInfo * ) _Mem_Allocate ( sizeof ( GotoInfo ), SESSION ) ;
    return gi ;
}

void
GotoInfo_Delete ( DLNode * node )
{
    GotoInfo * gi = ( GotoInfo* ) node ;
    DLNode_Remove ( ( DLNode * ) gi ) ;
}

GotoInfo *
_CfrTil_CompileCallGotoPoint ( uint64 type )
{
    GotoInfo * gotoInfo = ( GotoInfo * ) _GotoInfo_Allocate ( ) ;
    if ( type == GI_RECURSE ) _Compile_UninitializedCall ( ) ;
    else _Compile_UninitializedJump ( ) ;
    gotoInfo->pb_JmpOffsetPointer = Here - 4 ; // after the call opcode
    gotoInfo->GI_CType = type ;
    DLList_AddNodeToTail ( _Context_->Compiler0->GotoList, ( DLNode* ) gotoInfo ) ;
    return gotoInfo ;
}

GotoInfo *
GotoInfo_New ( byte * lname )
{
    GotoInfo * gotoInfo = _CfrTil_CompileCallGotoPoint ( GI_GOTO ) ;
    gotoInfo->pb_LabelName = lname ;
    return gotoInfo ;
}

void
_CfrTil_Goto ( byte * lname )
{
    GotoInfo_New ( lname ) ;
}

void
CfrTil_Goto ( ) // runtime
{
    _CfrTil_Goto ( ( byte * ) _DataStack_Pop ( ) ) ; // runtime
}

void
CfrTil_Label ( )
{
    _CfrTil_Label ( ( byte* ) _DataStack_Pop ( ) ) ;
}

void
CfrTil_Return ( ) // runtime
{
    //_Compile_ESP_Restore ( ) ;
    //Compiler_SetState ( _Context_->Compiler0, SAVE_ESP, true ) ;
    _CfrTil_CompileCallGotoPoint ( GI_RETURN ) ;
}

void
CfrTil_Continue ( )
{
    _CfrTil_CompileCallGotoPoint ( GI_CONTINUE ) ;
}

void
CfrTil_Break ( )
{
    _CfrTil_CompileCallGotoPoint ( GI_BREAK ) ;
}

void
CfrTil_SetupRecursiveCall ( )
{
    _CfrTil_CompileCallGotoPoint ( GI_RECURSE ) ;
}

void
CfrTil_Tail ( )
{
    Printf ( ( byte* ) "\nTailCall not implemented yet. Fix me!\n" ) ;
    return ;
    _CfrTil_CompileCallGotoPoint ( GI_TAIL_CALL ) ;
}

void
CfrTil_Literal ( )
{
    if ( Compiling )
    {
        uint32 uliteral = ( uint32 ) _DataStack_Pop ( ) ;
        ConstantOrLiteral_New ( _Context_->Interpreter0, uliteral ) ;
    }
}

void
CfrTil_Variable ( )
{
    _CfrTil_Variable ( ( byte* ) _DataStack_Pop ( ), 0 ) ;
}

// "{|" - exit the Compiler start interpreting
// named after the forth word '[' 
// meaning is reversed from forth which doesn't have blocks

void
CfrTil_LeftBracket ( )
{
    Compiler_SetState ( _Context_->Compiler0, COMPILE_MODE, false ) ;
}

#if 0
void
CfrTil_CompileModeOn ( )
{
    Compiler_SetState ( _Context_->Compiler0, COMPILE_MODE, true ) ;
}
#endif

// "|}" - enter the Compiler
// named after the forth word ']'

void
CfrTil_RightBracket ( )
{
    //CfrTil_CompileModeOn ( ) ;
    Compiler_SetState ( _Context_->Compiler0, COMPILE_MODE, true ) ;
}

void
CfrTil_CompileMode ( )
{
    _DataStack_Push ( GetState ( _Context_->Compiler0, COMPILE_MODE ) ) ;
}

