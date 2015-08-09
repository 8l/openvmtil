
#include "../includes/cfrtil.h"

/*
struct _Stack
{
        cell_t				*StackPointer ;
        cell_t				*StackMin ;
        cell_t				*StackMax ;
        cell_t				StackData 			[  ] ;
}; 
typedef struct _Stack 			Stack ;

 */

void
_Stack_PrintHeader ( Stack * stack, char * name )
{
    int size = Stack_Depth ( stack ), * sp = stack->StackPointer ; // 0 based stack
    Printf ( ( byte* ) "\n%s%3d : %s = Top = " UINT_FRMT_0x08 ", InitialTos = " UINT_FRMT_0x08 ", Max = " UINT_FRMT_0x08 ", Min = " UINT_FRMT_0x08 ", Size = " UINT_FRMT_0x08,
        name, size, stack == _DataStack_ ? "Dsp (ESI)" : "", ( int32 ) sp, ( int32 ) stack->InitialTosPointer, ( int32 ) stack->StackMax, ( int32 ) stack->StackMin, stack->StackMax - stack->StackMin + 1 ) ;
}

void
_Stack_PrintValues ( byte * name, int * stackPointer, int stackDepth )
{
    int i ; //, stackDepth = _Stack_Depth ( stack ), * stackPointer = stack->StackPointer ; // 0 based stack
    byte * buffer = Buffer_New_pbyte ( BUFFER_SIZE ) ;
    if ( stackDepth >= 0 )
    {
        for ( i = 0 ; stackDepth -- > 0 ; i -- )
        {
            Word * word = Word_GetFromCodeAddress ( ( byte* ) ( stackPointer [ i ] ) ) ;
            if ( word )
            {
                sprintf ( ( char* ) buffer, "< %s.%s >", word->ContainingNamespace->Name, c_dd ( word->Name ) ) ;
            }
            Printf ( ( byte* ) "\n\t\t    %s   [ %3d ] < " UINT_FRMT_0x08 " > = " UINT_FRMT_0x08 "\t\t%s", name, i, ( uint ) & stackPointer [ i ], stackPointer [ i ], word ? ( char* ) buffer : "" ) ;
        }
    }
    else //if ( stackDepth < 0 )
    {
        CfrTil_Exception ( STACK_UNDERFLOW, QUIT ) ;
    }
    Printf ( ( byte* ) "\n" ) ;
}

void
_Stack_Print ( Stack * stack, char * name )
{
    _Stack_PrintHeader ( stack, name ) ;
    _Stack_PrintValues ( name, stack->StackPointer, Stack_Depth ( stack ) ) ;
}

int32
_Stack_Overflow ( Stack * stack )
{
    return ( stack->StackPointer >= stack->StackMax ) ;
}

int32
_Stack_IsEmpty ( Stack * stack )
{
    return ( stack->StackPointer < stack->StackMin ) ;
}

void
_Stack_Push ( Stack * stack, int32 value )
{
    stack->StackPointer ++ ;
    *( stack->StackPointer ) = value ;
}

void
_Stack_Dup ( Stack * stack )
{
    _Stack_Push ( stack, *( stack->StackPointer ) ) ;
}

int32
__Stack_Pop ( Stack * stack )
{
    return *( stack->StackPointer -- ) ;
}

int32
_Stack_Pop_ExceptionFlag ( Stack * stack, int32 exceptionOnEmptyFlag  )
{
    if ( _Stack_IsEmpty ( stack ) )
    {
        if ( exceptionOnEmptyFlag ) CfrTil_Exception ( STACK_UNDERFLOW, QUIT ) ;
        else return 0 ;
    }
    return __Stack_Pop ( stack ) ;
}

int32
_Stack_Pop ( Stack * stack )
{
    if ( _Stack_IsEmpty ( stack ) ) CfrTil_Exception ( STACK_UNDERFLOW, QUIT ) ;
    return __Stack_Pop ( stack ) ;
}

int32
_Stack_PopOrTop ( Stack * stack )
{
    int sd = Stack_Depth ( stack ) ;
    if (sd <= 0) CfrTil_Exception ( STACK_UNDERFLOW, QUIT ) ;
    else if ( sd == 1 ) return _Stack_Top ( stack ) ; 
    else return __Stack_Pop ( stack ) ;
}

int32
_Stack_DropN ( Stack * stack, int32 n )
{
    return * ( stack->StackPointer -= n ) ;
}

int32
_Stack_Top ( Stack * stack )
{
    return *stack->StackPointer ;
}

int32
_Stack_Pick ( Stack * stack, int32 offset )
{
    return * ( stack->StackPointer - offset ) ;
}

int32
_Stack_PickFromBottom ( Stack * stack, int32 offset )
{
    return * ( stack->StackMin + offset ) ;
}

int32
_Stack_Bottom ( Stack * stack )
{
    return * ( stack->StackMin ) ;
}

void
_Stack_SetBottom ( Stack * stack, int32 value )
{
    *stack->StackMin = value ;
}

void
_Stack_SetTop ( Stack * stack, int32 value )
{
    *stack->StackPointer = value ;
}

int32
_Stack_NOS ( Stack * stack )
{
    return *( stack->StackPointer - 1 ) ;
}

void
_Stack_Drop ( Stack * stack )
{
    stack->StackPointer -- ;
}

void
Stack_Push ( Stack * stack, int32 value )
{
#if STACK_CHECK_ERROR
    if ( _Stack_Overflow ( stack ) )
    {
        AlertColors ;
        Printf ( ( byte* ) "\nStackDepth = %d\n", Stack_Depth ( stack ) ) ;
        CfrTil_Exception ( STACK_OVERFLOW, QUIT ) ;
    }
    _Stack_Push ( stack, value ) ;
#else
    _Stack_Push ( stack, value ) ;
#endif
}

int32
Stack_Pop_WithExceptionOnEmpty ( Stack * stack )
{
#if STACK_CHECK_ERROR
    if ( _Stack_IsEmpty ( stack ) ) CfrTil_Exception ( STACK_UNDERFLOW, QUIT ) ;
    return _Stack_Pop ( stack ) ;
#else
    _Stack_Pop ( stack ) ;
#endif
}

int32
Stack_Pop_WithZeroOnEmpty ( Stack * stack )
{
    if ( _Stack_IsEmpty ( stack ) ) return 0 ;
    return _Stack_Pop ( stack ) ;
}

void
Stack_Dup ( Stack * stack )
{
#if STACK_CHECK_ERROR
    if ( _Stack_Overflow ( stack ) ) CfrTil_Exception ( STACK_OVERFLOW, QUIT ) ;
    _Stack_Dup ( stack ) ;
#else
    _Stack_Dup ( stack ) ;
#endif
}

int32
_Stack_IntegrityCheck ( Stack * stack )
{
    // first a simple integrity check of the stack info struct
    if ( ( stack->StackMin == & stack->StackData [ 0 ] ) &&
        ( stack->StackMax == & stack->StackData [ stack->StackSize - 1 ] ) &&
        ( stack->InitialTosPointer == & stack->StackData [ - 1 ] ) )
    {
        return true ;
    }
    return false ;
}

int32
_Stack_Depth ( Stack * stack )
{
    int32 depth = stack->StackPointer - stack->InitialTosPointer ;
    if ( depth <= stack->StackSize ) return depth ;
    return ( 0 ) ;
}

int32
Stack_Depth ( Stack * stack )
{
    // first a simple integrity check of the stack info struct
    if ( _Stack_IntegrityCheck ( stack ) )
    {
        return _Stack_Depth ( stack ) ;
    }
    return ( - 1 ) ;
}

void
Stack_SetStackMax ( Stack * stack, int32 value )
{
    stack->StackData [ stack->StackSize - 1 ] = value ;
}

// Stack_Clear => Stack_Init

void
CfrTil_memset ( register byte * dst, int32 value, register int32 n )
{
    while ( -- n >= 0 ) dst [n] = ( byte ) value ;
}

void
_Stack_Init ( Stack * stack, int32 slots )
{
    memset ( stack, 0, sizeof ( Stack ) + ( slots * sizeof (int32 ) ) ) ;
    stack->StackSize = slots ; // re-init size after memset cleared it
    stack->StackMin = & stack->StackData [ 0 ] ;
    stack->StackMax = & stack->StackData [ stack->StackSize - 1 ] ;

    stack->InitialTosPointer = & stack->StackData [ - 1 ] ;
    stack->StackPointer = stack->InitialTosPointer ;
}

void
Stack_Delete ( Stack * stack )
{
    Mem_FreeItem ( _Q_->PermanentMemList, ( byte* ) stack ) ;
}

void
Stack_Init ( Stack * stack )
{
    _Stack_Init ( stack, stack->StackSize ) ;
}

Stack *
Stack_New ( int32 slots, int32 type )
{
    Stack * stack = ( Stack* ) _Mem_Allocate ( sizeof ( Stack ) + ( slots * sizeof (int32 ) ), type ) ;
    _Stack_Init ( stack, slots ) ;
    return stack ;
}

Stack *
Stack_Copy ( Stack * stack, int32 type )
{
    Stack * nstack = Stack_New ( stack->StackSize, type ) ;
    memcpy ( nstack->StackData, stack->StackData, stack->StackSize * sizeof (int32 ) ) ;

    // ?? -> preserve relative stack pointer
    int32 depth = Stack_Depth ( stack ) ;
    //depth = stack->StackPointer - stack->InitialTosPointer ;
    nstack->StackPointer = nstack->InitialTosPointer + depth ;

    return nstack ;
}

