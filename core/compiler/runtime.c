
#include "../../includes/cfrtil.h"

#if 0
void
_CompileStackNToReg ( cell n, cell reg ) 
{
#if 0 // under construction
    //if ( _CfrTil_GetCompileMode () )
    {
        // mov n to edx - index register
        // sib = sizeof (cell_t), n, edi 
        //_CompileMoveImm ( cell_t direction, cell_t rmReg, cell_t disp, cell_t immediateData, cell_t isize )
        _CompileMoveImm ( REG, EDX, 0, n, 0 ) ;
        // _CompileMove ( cell_t direction, cell_t reg, cell_t rmReg, cell_t sib, cell_t disp )
        _Compile_NOT ( REG, EDX, 0, 0, 0, 0 ) ;
        _CompileMove ( REG, EDX, ESI, _CfrTil_Sib ( sizeof (cell), NO_INDEX, EDX ), 0 ) ; // n
    }
#endif
}

void
CompileStackNToReg ( ) 
{
    cell reg, n ;
    reg = _DataStack_Pop ( ) ;
    n = _DataStack_Pop ( ) ;
    CompileStackNToReg ( n, reg ) ;
}

void
_CompileRegToStackN ( cell n, cell reg ) 
{
    //if ( _CfrTil_GetCompileMode () )
    {

        _CompileCompileMoveRegToMem ( ESI, reg, - ( n * CELL_SIZE ) ;
    }
}

void
CompileRegToStackN ( ) 
{

    cell reg, n ;
        n = _DataStack_Pop ( ) ;
        reg = _DataStack_Pop ( ) ;
        CompileRegToStackN ( n, reg ) ;
}

void
_CompileLocalsNToReg ( cell n, cell reg ) 
{
    //if ( _CfrTil_GetCompileMode () )
    {

        _CompileCompileMoveMemToReg ( reg, EBP, - ( n * CELL_SIZE ) ) ; // n
    }
}

void
CompileLocalsNToReg ( ) 
{

    cell reg, n ;
        reg = _DataStack_Pop ( ) ;
        n = _DataStack_Pop ( ) ;
        CompileLocalsNToReg ( n, reg ) ;
}

void
_CompileRegToLocalsN ( cell n, cell reg ) 
{
    //if ( _CfrTil_GetCompileMode () )
    {

        _CompileCompileMoveRegToMem ( EBP, reg, - ( n * CELL_SIZE ) ) ;
    }
}

void
CompileRegToLocalsN ( ) 
{

    cell reg, n ;
    
    n = _DataStack_Pop ( ) ;
    reg = _DataStack_Pop ( ) ;
    CompileRegToLocalsN ( n, reg ) ;
}

#if 0
// intel syntax : opcode dst, src
// mov reg to mem or mem to reg
// note the order of the operands match intel syntax with dst always before src
void
_CompileCompileMoveRegToReg ( cell dstReg, cell srcReg, cell offset )
{

    cell modRm = _CompileModRm ( 3, dstReg, srcReg ) ;
        _CompileCompileInstructionX86 ( 0x8b, modRm, 1, offset, 0, 0 ) ;
}

void
_CompileCompileOpRegToReg ( cell opCode, cell dstReg, cell srcReg )
{

    cell modRm = _CompileModRm ( 3, dstReg, srcReg ) ;
        _CompileCompileInstructionX86 ( opCode, modRm, 1, 0, 0, 0 ) ;
}
#endif

void
_CompileMoveRegToReg ( cell dst, cell src ) 
{
    //if ( _CfrTil_GetCompileMode () )
    {
        //_CompileCompileMoveRegToReg ( cell_t dstReg, cell_t srcReg, cell_t offset )

        _CompileCompileMoveRegToReg ( dstReg, srcReg, 0 ) ;
    }
}

void
CompileMoveRegToReg ( ) 
{

    cell dst, src ;

    dst = _DataStack_Pop ( ) ;
    src = _DataStack_Pop ( ) ;
    _CompileMovRegToReg ( dst, src ) ;
}

void
_CompileOpRegToReg ( cell opCode, cell dst, cell src ) 
{
    //if ( _CfrTil_GetCompileMode () )
    {
        //_CompileCompileOpRegToReg ( cell_t opCode, cell_t dstReg, cell_t srcReg )

        _CompileCompileOpRegToReg ( opCode, dst, src ) ;
    }
}

void
CompileOpRegToReg ( ) 
{
    cell dst, src, opCode ;
    opCode = _DataStack_Pop ( ) ;
    dst = _DataStack_Pop ( ) ;
    src = _DataStack_Pop ( ) ;
    _CompileOpRegToReg ( opCode, dst, src ) ;
}

void
_CompileOpReg ( cell opCode, cell dst ) 
{
    //if ( _CfrTil_GetCompileMode () )
    {
        //_CompileCompileOpRegToReg ( cell_t opCode, cell_t dstReg, cell_t srcReg )

        //_CompileCompileOpRegToReg ( opCode, dst, src ) ;
    }
}

void
CompileOpReg ( ) 
{
    cell dst, opCode ;
    opCode = _DataStack_Pop ( ) ;
    dst = _DataStack_Pop ( ) ;
    _CompileOpReg ( opCode, dst, src ) ;
}

// CompileRegToMem
// CompileMemToReg
// conditional branch, jump
#endif