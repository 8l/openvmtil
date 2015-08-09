#include "../../includes/cfrtil.h"
// a combinator can be thought of as a finite state machine that
// operates on a stack or more theoretically as a finite state control
// for a pda/turing machine but more simply as a function on a stack to
// a stack like a forth word but the items on the stack can be taken,
// depending on the combinator, as subroutine calls. The idea comes from, for me, 
// Foundations of Mathematical Logic by Haskell Curry and the joy
// and factor programming languages. It works out
// to be an intuitive idea ; you may not need to understand it, but you can
// see how it works. It simplifies syntax beyond Forth because
// it reduces the number of necessary prefix operators to one - tick ("'") = quote.

// nb : can't fully optimize if there is code between blocks
// check if there is any code between blocks if so we can't optimize fully

void
CfrTil_EndCombinator ( int32 quotesUsed, int32 moveFlag )
{
    Compiler * compiler = _Context_->Compiler0 ;
    BlockInfo *bi = ( BlockInfo * ) _Stack_Pick ( compiler->CombinatorBlockInfoStack, quotesUsed - 1 ) ; // -1 : remember - stack is zero based ; stack[0] is top
    _CfrTil_InstallGotoCallPoints_Keyed ( ( BlockInfo* ) bi, GI_BREAK | GI_CONTINUE ) ;
    if ( moveFlag && CfrTil_GetState ( _CfrTil_, INLINE_ON ) )
    {
        byte * qCodeStart ;
        if ( bi->FrameStart ) qCodeStart = bi->bp_First ; // after the stack frame
        else qCodeStart = bi->ActualCodeStart ;
        Block_Copy ( qCodeStart, bi->CombinatorStartsAt, Here - bi->CombinatorStartsAt ) ;
    }
    _Stack_DropN ( compiler->CombinatorBlockInfoStack, quotesUsed ) ;
    if ( Compiler_GetState ( compiler, LISP_COMBINATOR_MODE ) )
    {
        _Stack_Pop ( compiler->CombinatorInfoStack ) ;
        if ( ! Stack_Depth ( compiler->CombinatorInfoStack ) ) Compiler_SetState ( compiler, LISP_COMBINATOR_MODE, false ) ;
    }
}

void
CfrTil_BeginCombinator ( int32 quotesUsed )
{
    Compiler * compiler = _Context_->Compiler0 ;
    BlockInfo *bi = ( BlockInfo * ) _Stack_Pick ( compiler->CombinatorBlockInfoStack, quotesUsed - 1 ) ; // -1 : remember - stack is zero based ; stack[0] is top
    // optimize out jmps such that the jmp from first block is to Here the start of the combinator code
    bi->CombinatorStartsAt = Here ;
    _SetOffsetForCallOrJump ( bi->JumpOffset, bi->CombinatorStartsAt, 1 ) ;
}

// ( q -- )

void
CfrTil_DropBlock ( )
{
    _DataStack_DropN ( 1 ) ;
    if ( CompileMode )
    {
        CfrTil_BeginCombinator ( 1 ) ;
        CfrTil_EndCombinator ( 1, 0 ) ;
    }
}

void
_CfrTil_BlockRun ( Boolean flag )
{
    block doBlock = ( block ) TOS ;
    _DataStack_DropN ( 1 ) ;
    if ( flag & FORCE_RUN )
    {
        _Block_Eval ( doBlock ) ;
    }
    else //if ( flag & FORCE_COMPILE )
    {
        CfrTil_BeginCombinator ( 1 ) ;
        _Compile_Block ( ( byte* ) doBlock, 0, 0 ) ;
        CfrTil_EndCombinator ( 1, 1 ) ;
        //return doBlock ;
    }
}

void
CfrTil_BlockRun ( )
{
    block doBlock = ( block ) TOS ;
    if ( CompileMode )
    {
        _DataStack_DropN ( 1 ) ;
        CfrTil_BeginCombinator ( 1 ) ;
        _Compile_Block ( ( byte* ) doBlock, 0, 0 ) ;
        CfrTil_EndCombinator ( 1, 1 ) ;
    }
    else
    {
        _Block_Eval ( doBlock ) ;
        _DataStack_DropN ( 1 ) ; // needs to be here to correctly run lisp blocks from LO_EndBlock ( ) ?!?
    }
}

// ( q -- )

void
CfrTil_LoopCombinator ( )
{
    block loopBlock = ( block ) TOS ;
    _DataStack_DropN ( 1 ) ;
    if ( CompileMode )
    {
        CfrTil_BeginCombinator ( 1 ) ;
        byte * start = Here ;
        _Context_->Compiler0->ContinuePoint = start ;
        _Compile_Block ( ( byte* ) loopBlock, 0, 0 ) ;
        _Compile_JumpToAddress ( start ) ; // runtime
        _Context_->Compiler0->BreakPoint = Here ;
        CfrTil_EndCombinator ( 1, 1 ) ;
    }
    else while ( 1 ) _Block_Eval ( loopBlock ) ;
}

void
CfrTil_NLoopCombinator ( )
{
    int32 count = Dsp [ - 1 ] ;
    block loopBlock = ( block ) TOS ;
    _DataStack_DropN ( 2 ) ;
#if 0    
    if ( CompileMode )
    {
        CfrTil_BeginCombinator ( 1 ) ;
        byte * start = Here ;
        _Context_->Compiler0->ContinuePoint = start ;
        _Compile_Block ( ( byte* ) loopBlock ) ;
        _Compile_JumpToAddress ( start ) ; // runtime
        _Context_->Compiler0->BreakPoint = Here ;
        CfrTil_EndCombinator ( 1 ) ;
    }
#endif    
    while ( count -- )
        _Block_Eval ( loopBlock ) ;
}


// ( b q -- ) 

void
CfrTil_If1Combinator ( )
{
    block doBlock = ( block ) TOS ;
    _DataStack_DropN ( 1 ) ;
    if ( CompileMode )
    {
        CfrTil_BeginCombinator ( 1 ) ;

        Compile_GetLogicFromTOS ( 0 ) ;
        _Compile_UninitializedJumpEqualZero ( ) ;
        _Stack_PointerToJmpOffset_Set ( ) ;

        _Compile_Block ( ( byte* ) doBlock, 0, 0 ) ;
        CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
        CfrTil_EndCombinator ( 1, 1 ) ;
    }
    else
    {
        if ( _DataStack_Pop ( ) ) _Block_Eval ( doBlock ) ;
    }
}

// ( q q -- )

int32
CfrTil_WhileCombinator ( )
{
    block testBlock = ( block ) Dsp [ - 1 ], trueBlock = ( block ) TOS ;
    _DataStack_DropN ( 2 ) ;
    if ( CompileMode )
    {
        CfrTil_BeginCombinator ( 2 ) ;
        byte * start = Here ;
        _Context_->Compiler0->ContinuePoint = Here ;
        if ( ! _Compile_Block ( ( byte* ) testBlock, 1, 1 ) ) 
        {
            SetHere ( start ) ;
            return 0 ;
        }
        _Compile_Block ( ( byte* ) trueBlock, 0, 0 ) ;
        _Compile_JumpToAddress ( start ) ; 
        _Context_->Compiler0->BreakPoint = Here ;
        CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
        CfrTil_EndCombinator ( 2, 1 ) ;
    }
    else
    {
        while ( 1 )
        {
            _Block_Eval ( testBlock ) ;
            if ( ! _DataStack_Pop ( ) ) break ;
            _Block_Eval ( trueBlock ) ;
        }
    }
    return 1 ;
}

int32
CfrTil_DoWhileCombinator ( )
{
    block testBlock = ( block ) TOS, doBlock = ( block ) Dsp [ - 1 ] ;
    _DataStack_DropN ( 2 ) ;
    if ( CompileMode )
    {
        CfrTil_BeginCombinator ( 2 ) ;
        byte * start = Here ;
        _Context_->Compiler0->ContinuePoint = Here ;
        _Compile_Block ( ( byte* ) doBlock, 1, 0 ) ;
        //_Compile_Block ( ( byte* ) testBlock, 0, 1 ) ;
        if ( ! _Compile_Block ( ( byte* ) testBlock, 0, 1 ) ) 
        {
            SetHere ( start ) ;
            return 0 ;
        }
        _Compile_JumpToAddress ( start ) ; 
        _Context_->Compiler0->BreakPoint = Here ;
        CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
        CfrTil_EndCombinator ( 2, 1 ) ;
    }
    else
    {
        do
        {
            _Block_Eval ( doBlock ) ;
            _Block_Eval ( testBlock ) ;
            if ( ! _DataStack_Pop ( ) ) break ;
        }
        while ( 1 ) ;
    }
    return 1 ;
}

// ( q q -- )

void
CfrTil_If2Combinator ( )
{
    block testBlock = ( block ) Dsp [ - 1 ], doBlock = ( block ) TOS ;
    _DataStack_DropN ( 2 ) ;
    if ( CompileMode )
    {
        CfrTil_BeginCombinator ( 2 ) ;
        _Compile_Block ( ( byte* ) testBlock, 1, 1 ) ;
        _Compile_Block ( ( byte* ) doBlock, 0, 0 ) ;
        CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
        CfrTil_EndCombinator ( 2, 1 ) ;
    }
    else
    {
        _Block_Eval ( testBlock ) ;
        if ( _DataStack_Pop ( ) ) _Block_Eval ( doBlock ) ;
    }
}
// ( b q q -- )
// takes 2 blocks
// nb. does not drop the boolean so it can be used in a block which takes a boolean - an on the fly combinator

void
CfrTil_TrueFalseCombinator2 ( )
{
    int32 testCondition = Dsp [ - 2 ] ;
    block trueBlock = ( block ) Dsp [ - 1 ], falseBlock = ( block ) TOS ;
    _DataStack_DropN ( 2 ) ;
    if ( CompileMode )
    {
        CfrTil_BeginCombinator ( 2 ) ;

        Compile_GetLogicFromTOS ( 0 ) ;
        _Compile_UninitializedJumpEqualZero ( ) ;
        _Stack_PointerToJmpOffset_Set ( ) ;

        _Compile_Block ( ( byte* ) trueBlock, 1, 0 ) ;
        CfrTil_Else ( ) ;
        _Compile_Block ( ( byte* ) falseBlock, 0, 0 ) ;
        CfrTil_EndIf ( ) ;

        CfrTil_EndCombinator ( 2, 1 ) ;
    }
    else
    {
        if ( testCondition )
        {
            _Block_Eval ( trueBlock ) ;
        }
        else
        {
            _Block_Eval ( falseBlock ) ;
        }
    }
}

// ( q q q -- )
// takes 3 blocks

void
CfrTil_TrueFalseCombinator3 ( )
{
    block testBlock = ( block ) Dsp [ - 2 ], trueBlock = ( block ) Dsp [ - 1 ],
        falseBlock = ( block ) TOS ;
    _DataStack_DropN ( 3 ) ;
    if ( CompileMode )
    {
        CfrTil_BeginCombinator ( 3 ) ;
        _Compile_Block ( ( byte* ) testBlock, 2, 1 ) ;
        _Compile_Block ( ( byte* ) trueBlock, 1, 0 ) ;
        CfrTil_Else ( ) ;
        _Compile_Block ( ( byte* ) falseBlock, 0, 0 ) ;
        CfrTil_EndIf ( ) ;
        CfrTil_EndCombinator ( 3, 1 ) ;
    }
    else
    {
        _Block_Eval ( testBlock ) ;
        if ( _DataStack_Pop ( ) )
        {
            _Block_Eval ( trueBlock ) ;
        }
        else
        {
            _Block_Eval ( falseBlock ) ;
        }
    }
}

//  ( q q q -- )

void
CfrTil_IfElseCombinator ( )
{
    CfrTil_TrueFalseCombinator3 ( ) ;
}

void
CfrTil_DoWhileDoCombinator ( )
{
    block testBlock = ( block ) Dsp [ - 1 ], doBlock2 = ( block ) TOS, doBlock1 =
        ( block ) Dsp [ - 2 ] ;
    byte * start ;
    _DataStack_DropN ( 3 ) ;
    if ( CompileMode )
    {
        CfrTil_BeginCombinator ( 3 ) ;
        _Context_->Compiler0->ContinuePoint = Here ;
        start = Here ;
        _Compile_Block ( ( byte* ) doBlock1, 2, 0 ) ;

        _Compile_Block ( ( byte* ) testBlock, 1, 1 ) ;

        _Compile_Block ( ( byte* ) doBlock2, 0, 0 ) ;
        _Compile_JumpToAddress ( start ) ; // runtime
        _Context_->Compiler0->BreakPoint = Here ;
        CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
        CfrTil_EndCombinator ( 3, 1 ) ;
    }
    else
    {
        do
        {
            _Block_Eval ( doBlock1 ) ;
            _Block_Eval ( testBlock ) ;
            if ( ! _DataStack_Pop ( ) )
                break ;
            _Block_Eval ( doBlock2 ) ;
        }
        while ( 1 ) ;
    }
}

// ( q q q q -- )

void
CfrTil_ForCombinator ( )
{
    block doBlock = ( block ) TOS, doPostBlock = ( block ) Dsp [ - 1 ], testBlock =
        ( block ) Dsp [ - 2 ], doPreBlock = ( block ) Dsp [ - 3 ] ;
    _DataStack_DropN ( 4 ) ;
    if ( CompileMode )
    {
        CfrTil_BeginCombinator ( 4 ) ;
        _Compile_Block ( ( byte* ) doPreBlock, 3, 0 ) ;

        byte * start = Here ;

        _Compile_Block ( ( byte* ) testBlock, 2, 1 ) ;

        _Context_->Compiler0->ContinuePoint = Here ;

        _Compile_Block ( ( byte* ) doBlock, 0, 0 ) ;

        _Compile_Block ( ( byte* ) doPostBlock, 1, 0 ) ;
        _Compile_JumpToAddress ( start ) ; // runtime

        _Context_->Compiler0->BreakPoint = Here ;
        CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;

        CfrTil_EndCombinator ( 4, 1 ) ;
    }
    else
    {
        _Block_Eval ( doPreBlock ) ;
        do
        {
            _Block_Eval ( testBlock ) ;
            if ( ! _DataStack_Pop ( ) )
                break ;
            _Context_->Compiler0->ContinuePoint = Here ;
            _Block_Eval ( doBlock ) ;
            _Block_Eval ( doPostBlock ) ;
            _Context_->Compiler0->BreakPoint = Here ;
        }
        while ( 1 ) ;
    }
}


#if 0 // based on ideas from the joy programing language
block ifBlock, thenBlock, else1Block, else2Block ;

void
linrec ( )
{
    CfrTil_BeginCombinator ( 4 ) ;
    byte * start = Here ;
    _Compile_Block ( ( byte* ) ifBlock, 3, 1 ) ;
    _Compile_Block ( ( byte* ) thenBlock, 2, 0 ) ;
    CfrTil_Else ( ) ;
    _Compile_Block ( ( byte* ) else1Block, 1, 0 ) ;
    _Compile_Call ( ( byte* ) start ) ;
    _Compile_Block ( ( byte* ) else2Block, 0, 0 ) ;
    CfrTil_EndIf ( ) ;
    CfrTil_EndCombinator ( 4, 1 ) ;
}

void
ilinrec ( )
{
    _Block_Eval ( ifBlock ) ;
    if ( _DataStack_Pop ( ) )
    {
        _Block_Eval ( thenBlock ) ;
    }
    else
    {
        _Block_Eval ( else1Block ) ;
        D0 ( CfrTil_PrintDataStack ( ) ) ;
        ilinrec ( ) ;
        _Block_Eval ( else2Block ) ;
    }
}

void
CfrTil_Combinator_LinRec ( )
{
    else2Block = ( block ) TOS, else1Block = ( block ) Dsp [ - 1 ],
        thenBlock = ( block ) Dsp [ - 2 ], ifBlock = ( block ) Dsp [ - 3 ] ;
    _DataStack_DropN ( 4 ) ;
    if ( CompileMode )
    {
        linrec ( ) ;
    }
    else ilinrec ( ) ;
}
//#else

void
ilinrec ( block ifBlock, block thenBlock, block else1Block, block else2Block )
{
    _Block_Eval ( ifBlock ) ;
    if ( _DataStack_Pop ( ) )
    {
        _Block_Eval ( thenBlock ) ;
    }
    else
    {
        _Block_Eval ( else1Block ) ;
        D0 ( CfrTil_PrintDataStack ( ) ) ;
        ilinrec ( ifBlock, thenBlock, else1Block, else2Block ) ;
        _Block_Eval ( else2Block ) ;
    }
}

void
CfrTil_Combinator_LinRec ( )
{
    block else2Block = ( block ) TOS, else1Block = ( block ) Dsp [ - 1 ],
        thenBlock = ( block ) Dsp [ - 2 ], ifBlock = ( block ) Dsp [ - 3 ] ;
    _DataStack_DropN ( 4 ) ;
    if ( CompileMode )
    {
        CfrTil_BeginCombinator ( 4 ) ;
        byte * start = Here ;
        _Compile_Block ( ( byte* ) ifBlock, 3, 1 ) ;
        _Compile_Block ( ( byte* ) thenBlock, 2, 0 ) ;
        CfrTil_Else ( ) ;
        _Compile_Block ( ( byte* ) else1Block, 1, 0 ) ;
        _Compile_Call ( ( byte* ) start ) ;
        _Compile_Block ( ( byte* ) else2Block, 0, 0 ) ;
        CfrTil_EndIf ( ) ;
        CfrTil_EndCombinator ( 4, 1 ) ;
        RET ( ) ;
    }
    else ilinrec ( ifBlock, thenBlock, else1Block, else2Block ) ;
}
#endif

