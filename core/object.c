
#include "../includes/cfrtil.h"

void
CfrTil_Dot ( ) // .
{
    if ( ! _Context_->Interpreter0->BaseObject )
    {
        SetState ( _Context_, CONTEXT_PARSING_QUALIFIED_ID, true ) ;
        Word * word = Compiler_PreviousNonDebugWord ( ) ;
        if ( word->CType & ( NAMESPACE | CLASS | DOBJECT ) )
        {
            Finder_SetQualifyingNamespace ( _Context_->Finder0, word ) ;
        }
        else _CfrTil_Do_Object ( word ) ;
    }
    // for the optimizer ... this can't be optimized
    //_Stack_DropN ( _Context_->Compiler0->WordStack, 2 ) ; // pop '.' and previous word 
}

void
Class_Object_Init ( byte * object, Word * word, Namespace * ns )
{
    if ( object )// size : not for "this" or anything else without a size
    {
        Stack * stack = _Context_->Compiler0->NamespacesStack ;
        Stack_Init ( stack ) ; // !! ?? put this in Compiler ?? !!
        // init needs to be done by the most super class first successively down to the current class 
        do
        {
            Word * initWord ;
            if ( ( initWord = Word_FindInOneNamespace ( ns, ( byte* ) "init" ) ) )
            {
                _Stack_Push ( stack, ( int32 ) initWord ) ;
            }
            ns = ns->ContainingNamespace ;
        }
        while ( ns ) ;
        int32 i ;
        for ( i = Stack_Depth ( stack ) ; i > 0 ; i -- )
        {
            _Push ( ( int32 ) word->bp_WD_Object ) ;
            Word * initWord = ( Word* ) _Stack_Pop ( stack ) ;
            _Word_Eval ( initWord ) ; //_Word_Run ( initWord ) ;
        }
        while ( ns ) ;
    }
}




