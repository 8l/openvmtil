
#include "../../includes/cfrtil.h"

// ?!? this file could be reworked for simplity, elegance and integration ?!?

void
CfrTil_If ( )
{
    if ( CompileMode )
    {
        _Compile_Jcc ( 0, 0, N, ZERO ) ;
        // N, ZERO : use inline|optimize logic which needs to get flags immediately from a 'cmp', jmp if the zero flag is not set
        // for non-inline|optimize ( reverse polarity : cf. _Compile_Jcc comment ) : jmp if cc is not true; cc is set by setcc after 
        // the cmp, or is a value on the stack. 
        // We cmp that value with zero and jmp if this second cmp sets the flag to zero else do the immediately following block code
        // ?? an explanation of the relation of the setcc terms with the flags is not clear to me yet (20110801) from the intel literature ?? 
        // but by trial and error this works; the logic i use is given in _Compile_Jcc.
        // ?? if there are problems check this area ?? cf. http://webster.cs.ucr.edu/AoA/Windows/HTML/IntegerArithmetic.html
        _Stack_PointerToJmpOffset_Set ( ) ;
    }
    else
    {
        Error_Abort ( ( byte* ) "\n\"if\" can only be used in compile mode." ) ;
    }
}

// same as CfrTil_JMP

void
CfrTil_Else ( )
{
    if ( CompileMode )
    {
        _Compile_UninitializedJump ( ) ; // at the end of the 'if block' we need to jmp over the 'else block'
        CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
        _Stack_PointerToJmpOffset_Set ( ) ;
    }
    else
    {
        Error_Abort ( ( byte* ) "\n\"else\" can only be used in compile mode." ) ;
    }
}

void
CfrTil_EndIf ( )
{
    if ( CompileMode )
    {
        CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
    }
    else
    {
        Error_Abort ( ( byte* ) "\n\"endif\" can only be used in compile mode." ) ;
    }
}

// ttt n : notation from intel manual 253667 ( N-Z ) - table B-10 : ttt = condition codes, n is a negation bit
// tttn notation is used with the SET and JCC instructions

// note : intex syntax  : instruction dst, src
//        att   syntax  : instruction src, dst
// cmp : compares by subtracting src from dst, dst - src, and setting eflags like a "sub" insn 
// eflags affected : cf of sf zf af pf : Intel Instrucion Set Reference Manual for "cmp"
// ?? can this be done better with test/jcc ??
// want to use 'test eax, 0' as a 0Branch (cf. jonesforth) basis for all block conditionals like if/else, do/while, for ...

void
Compile_Cmp_Set_tttn_Logic ( Compiler * compiler, int32 ttt, int32 negateFlag )
{
    int32 optFlag = CheckOptimizeOperands ( compiler, 5 ) ;
    if ( optFlag == OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        if ( ( optFlag == 2 ) && ( compiler->Optimizer->Optimize_Rm == DSP ) )
        {
            _Compile_Stack_PopToReg ( DSP, ECX ) ; // assuming optimize always uses EAX first
            compiler->Optimizer->Optimize_Rm = ECX ;
            compiler->Optimizer->Optimize_Mod = REG ;
        }
        // Compile_CMPI( mod, operandReg, offset, immediateData, size
        if ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_IMM ) Compile_CMPI ( compiler->Optimizer->Optimize_Mod,
            compiler->Optimizer->Optimize_Rm, compiler->Optimizer->Optimize_Disp, compiler->Optimizer->Optimize_Imm, CELL ) ;
            // Compile_CMP( toRegOrMem, mod, reg, rm, sib, disp )
        else Compile_CMP ( compiler->Optimizer->Optimize_Dest_RegOrMem, compiler->Optimizer->Optimize_Mod,
            compiler->Optimizer->Optimize_Reg, compiler->Optimizer->Optimize_Rm, 0, compiler->Optimizer->Optimize_Disp, CELL ) ;
    }
    else
    {
        _Compile_Move_StackN_To_Reg ( ECX, DSP, 0 ) ;
        _Compile_Move_StackN_To_Reg ( EAX, DSP, - 1 ) ;
        // ?? must do the DropN before the CMP because CMP sets eflags 
        _Compile_Stack_DropN ( DSP, 2 ) ; // before cmp allows smoother optimizing with C conditionals
        Compile_CMP ( REG, REG, EAX, ECX, 0, 0, CELL ) ;
    }
    _Compile_SET_tttn_REG ( ttt, negateFlag, EAX ) ; // immediately after the 'cmp' insn which changes the flags appropriately
    _Compile_MOVZX_REG ( EAX ) ;
    Word *zero = ( Word* ) Compiler_WordStack ( compiler, 0 ) ;
    _Word_CompileAndRecord_PushEAX ( zero ) ;
}

/*
 *  Logical operators
 */

void
Compile_Equals ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, EQUAL, 0 ) ;
}

void
Compile_DoesNotEqual ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, EQUAL, N ) ;
}

void
Compile_LessThan ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, LESS, 0 ) ;
}

void
Compile_GreaterThan ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, LE, N ) ;
}

void
Compile_LessThanOrEqual ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, LE, 0 ) ;
}

void
Compile_GreaterThanOrEqual ( Compiler * compiler )
{
    Compile_Cmp_Set_tttn_Logic ( compiler, LESS, N ) ;
}

void
Compile_LogicalNot ( Compiler * compiler )
{
    Word *one = Compiler_WordStack ( compiler, - 1 ) ;
    int optFlag = CheckOptimizeOperands ( compiler, 2 ) ;
    if ( optFlag == OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        if ( compiler->Optimizer->OptimizeFlag & OPTIMIZE_IMM )
        {
            _Compile_MoveImm_To_Reg ( EAX, compiler->Optimizer->Optimize_Imm, CELL ) ;
        }
        else if ( compiler->Optimizer->Optimize_Rm == DSP )
        {
            _Compile_Move_StackN_To_Reg ( EAX, DSP, 0 ) ;
        }
        else if ( compiler->Optimizer->Optimize_Rm != EAX )
        {
            _Compile_VarConstOrLit_RValue_To_Reg ( one, EAX ) ;
        }
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        _Compile_SET_tttn_REG ( ZERO, 0, EAX ) ; // SET : this only sets one byte of reg
        _Compile_MOVZX_REG ( EAX ) ; // so Zero eXtend reg
        if ( compiler->Optimizer->Optimize_Rm != DSP ) // optimize sets this
        {
            Word *zero = Compiler_WordStack ( compiler, 0 ) ;
            _Word_CompileAndRecord_PushEAX ( zero ) ;
        }
        else Compile_Move_EAX_To_TOS ( DSP ) ;
    }
    else
    {
        if ( one->StackPushRegisterCode ) SetHere ( one->StackPushRegisterCode ) ;
        else Compile_Move_TOS_To_EAX ( DSP ) ; //_Compile_Move_StackN_To_Reg ( EAX, DSP, 0 ) ;
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ; //logical and eax eax => if ( eax > 0 ) 1 else 0
        _Compile_SET_tttn_REG ( ZERO, 0, EAX ) ; // if (eax == zero) => zero flag is 1 if zero flag is true (1) set EAX to 1 else set eax to 0 :: SET : this only sets one byte of reg
        _Compile_MOVZX_REG ( EAX ) ; // so Zero eXtend reg
        Word *zero = Compiler_WordStack ( compiler, 0 ) ;
        zero->StackPushRegisterCode = Here ;
        Compile_Move_EAX_To_TOS ( DSP ) ;
        //int a, b, c= 0, d ; a = 1; b = !a, d= !c ; Printf ( "a = %d b = %d c =%d ~d = %d", a, b, c, d ) ;
    }
}

// for : AND OR XOR : Group1 logic ops

void
Compile_Logical_X ( Compiler * compiler, int32 op )
{
    int optFlag = CheckOptimizeOperands ( compiler, 5 ) ;
    if ( optFlag == OPTIMIZE_DONE ) return ;
    else if ( optFlag )
    {
        // TODO : this optimization somehow is *very* little used, why is that ?!? 
        // assumes we have unordered operands in eax, ecx
        _Compile_Group1 ( op, REG, REG, EAX, ECX, 0, 0, CELL ) ;
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        _Compiler_Setup_BI_tttn ( _Context_->Compiler0, ZERO, N ) ; // not less than 0 == greater than 0
        Word *zero = Compiler_WordStack ( compiler, 0 ) ;
        _Word_CompileAndRecord_PushEAX ( zero ) ;
    }
    else
    {
        // operands are still on the stack
        _Compile_Move_StackN_To_Reg ( EAX, DSP, 0 ) ;
        //_Compile_Group1 ( int32 code, int32 toRegOrMem, int32 mod, int32 reg, int32 rm, int32 sib, int32 disp, int32 osize )
        _Compile_Group1 ( op, REG, MEM, EAX, DSP, 0, - 4, CELL ) ;
        _Compile_Stack_DropN ( DSP, 2 ) ;
        _Compile_TEST_Reg_To_Reg ( EAX, EAX ) ;
        _Compiler_Setup_BI_tttn ( _Context_->Compiler0, ZERO, N ) ; // not less than 0 == greater than 0
        Word *zero = Compiler_WordStack ( compiler, 0 ) ;
        _Word_CompileAndRecord_PushEAX ( zero ) ;
    }
}
