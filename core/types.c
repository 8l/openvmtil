
#include "../includes/cfrtil.h"



#if 0

void
_Compiler_TypeCheck1 ( Compiler * compiler, int32 type32 )
{
    //return ;
    Word * word = ( Word* ) _Stack_Pop ( compiler->TypeStack ) ;
    if ( ! ( word->S_Category & type32 ) ) Error_Abort ( ( byte* ) "CType Error", 1 ) ;
}

void
_Compiler_TypeCheck2 ( Compiler * compiler, uint64 type0, uint64 type1 )
{
    //return ;
    Word * word = ( Word* ) _Stack_Pop ( compiler->TypeStack ) ;
    if ( ! ( word->S_Category & type0 ) ) Error_Abort ( ( byte* ) "CType Error", 1 ) ;
    if ( ! ( word->S_Category & type1 ) ) Error_Abort ( ( byte* ) "CType Error", 1 ) ;
}
int32
_TypeCheck1 ( uint64 zero )
{
    //if ( Compiler_GetState( _Context_->Compiler0, BLOCK_MODE ) )
    {
        if ( ( WordStack ( 0 )->S_Category & zero ) ) return true ;
        else Error_Abort ( "", ABORT ) ;
        return false ;
    }
    //else return true ;
}

int32
_TypeCheck2 ( uint64 zero, uint64 one )
{
    //if ( Compiler_GetState( _Context_->Compiler0, BLOCK_MODE ) )
    {
        if ( ( WordStack ( 0 )->S_Category & zero ) && ( WordStack ( 1 )->S_Category & one ) ) return true ;
        else Error_Abort ( "\nTypeError", ABORT ) ;
        return false ;
    }
    //else return true ;
}
#endif

