#include "../../includes/cfrtil.h"

CaseNode *
_CaseNode_New ( int32 type, block block, int32 value )
{
    CaseNode * cnode = ( CaseNode* ) _Mem_Allocate ( sizeof ( CaseNode ), type ) ;
    cnode->CaseBlock = block ;
    cnode->CN_CaseValue = (byte*) value ;
    return cnode ;
}

// ( q n -- )

void
_CfrTil_Case ( int32 type )
{
    block caseBlock ;
    int32 caseValue ;
    if ( CompileMode )
    {
        caseBlock = ( block ) TOS ;
        Word * literalWord = WordsBack ( 1 ) ;
        if ( ! ( literalWord->CType & LITERAL ) ) CfrTil_Exception ( CASE_NOT_LITERAL_ERROR, 1 ) ;
        caseValue = ( int32 ) literalWord->bp_WD_Object ;
        SetHere ( literalWord->Coding ) ;
        _DropN ( 1 ) ;
        //Dsp -- ;
    }
    else
    {
        caseBlock = ( block ) Dsp [ - 1 ] ;
        caseValue = TOS ;
        _DropN ( 2 ) ;
        //Dsp -= 2 ;
    }
    if ( ! _Context_->Compiler0->CurrentSwitchList )
    {
        _Context_->Compiler0->CurrentSwitchList = _DLList_New ( type ) ;
    }
    CaseNode * cnode = _CaseNode_New ( type, caseBlock, caseValue ) ;
    DLList_AddNodeToTail ( _Context_->Compiler0->CurrentSwitchList, (DLNode*) cnode ) ;
}

void
CfrTil_Case ( )
{
    _CfrTil_Case ( DICTIONARY ) ;
}

void
Switch_MapFunction ( DLNode * node, uint32 switchValue )
{
    CaseNode * cnode = ( CaseNode* ) node ;
    if ( cnode->CN_CaseValue == (byte*) switchValue ) cnode->CaseBlock ( ) ;
}

void
SwitchAccessFunction ( )
{
    //DLList * list = ( DLList * ) _Pop ( ) ;
    //cell switchValue = _Pop ( ) ;
    //DLList_Map1 ( list, Switch_MapFunction, switchValue ) ;
    DLList_Map1 ( ( DLList* ) TOS, (MapFunction1) Switch_MapFunction, Dsp [ - 1 ] ) ;
    _DropN ( 2 ) ;
}

void
CfrTil_Switch ( )
{
    if ( CompileMode )
    {
        // try to build table
        // setup SwitchAccessFunction 
        // call SwitchAccessFunction 
        //_Compile_PushLiteral ( ( int32 ) _Context_->Compiler0->CurrentSwitchList ) ;
        _Do_Literal ( ( int32 ) _Context_->Compiler0->CurrentSwitchList ) ;
        _Compile_Call ( ( byte* ) SwitchAccessFunction ) ;
    }
    else
    {
        //cell switchValue = TOS ;
        //_CfrTil_DropN ( 1 ) ;
        //DLList_Map1 ( _Context->Compiler0->CurrentSwitchList, Switch_MapFunction, switchValue ) ;
        DLList_Map1 ( _Context_->Compiler0->CurrentSwitchList, (MapFunction1) Switch_MapFunction, TOS ) ;
        _DropN ( 1 ) ;
    }
    _Context_->Compiler0->CurrentSwitchList = 0 ; // this allows no further "case"s to be added to this "switch" list a new list will be started with the next "case"
}

