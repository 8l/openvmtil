#include "../includes/cfrtil.h"

#if 1

void
_DLNode_Init ( DLNode * node )
{
    node->After = 0 ;
    node->Before = 0 ;
}
#endif

DLNode *
_DLNode_New ( uint64 allocType )
{
    DLNode * node = ( DLNode* ) _Mem_Allocate ( sizeof (DLNode ), allocType ) ;
    return node ;
}

// toward the TailNode
#define _DLNode_Next( node ) node->After
// toward the HeadNode
#define _DLNode_Previous( node ) node->Before

// toward the TailNode

DLNode *
DLNode_Next ( DLNode * node )
{
    // don't return TailNode return 0
    if ( node && node->After && node->After->After )
    {
        return _DLNode_Next ( node ) ;
    }
    return 0 ;
}

// toward the HeadNode

DLNode *
DLNode_Previous ( DLNode * node )
{
    // don't return HeadNode return 0
    if ( node && node->Before && node->Before->Before )
    {
        return _DLNode_Previous ( node ) ;
    }
    return 0 ;
}

void
DLNode_InsertThisAfterANode ( DLNode * node, DLNode * anode ) // Insert this After node : toward the tail of the list - "after" the Head
{
    if ( node && anode )
    {
        D0 ( if ( anode->N_CType & T_TAIL ) Error ( "\nCan't Insert a node after the TailNode!\n", QUIT ) ; ) ;
        if ( anode->After ) anode->After->Before = node ; // don't overwrite a Head or Tail node 
        node->After = anode->After ;
        anode->After = node ; // after the above statement ! obviously
        node->Before = anode ;
    }
}

void
DLNode_InsertThisBeforeANode ( DLNode * node, DLNode * anode ) // Insert this Before node : toward the head of the list - "before" the Tail
{
    if ( node && anode )
    {
        D0 ( if ( anode->N_CType & T_HEAD ) Error ( "\nCan't Insert a node before the HeadNode!\n", QUIT ) ; ) ;
        if ( anode->Before ) anode->Before->After = node ; // don't overwrite a Head or Tail node
        node->Before = anode->Before ;
        anode->Before = node ; // after the above statement ! obviously
        node->After = anode ;
    }
}

void
DLNode_ReplaceNodeWithANode ( DLNode * node, DLNode * anode )
{
    if ( node && anode )
    {
        // not checking for head/tail node
        DLNode * after = node->N_pdln_After ;
        DLNode_Remove ( node ) ;
        DLNode_InsertThisBeforeANode ( anode, after ) ;
    }
}

DLNode *
DLNode_Remove ( DLNode * node )
{
    if ( node )
    {
        D0 ( 
        if ( node->N_CType & ( T_HEAD | T_TAIL ) )
        {
            //return node ;
            Error ( "\nCan't remove the Head or Tail node!\n", QUIT ) ;
        }
        ) ;
        if ( node->Before ) node->Before->After = node->After ;
        if ( node->After ) node->After->Before = node->Before ;
    }
    return node ;
}

void
_DLList_Init ( DLList * list )
{
    list->Head->After = ( DLNode * ) list->Tail ;
    list->Head->Before = ( DLNode * ) 0 ;
    list->Tail->After = ( DLNode* ) 0 ;
    list->Tail->Before = ( DLNode * ) list->Head ;
    list->Head->N_CType = T_HEAD ;
    list->Tail->N_CType = T_TAIL ;
    list->S_CurrentNode = 0 ;
}

void
DLList_Init ( DLList * list, DLNode * head, DLNode *tail )
{
    list->Head = head ;
    list->Tail = tail ;
    _DLList_Init ( list ) ;
}

DLList *
_DLList_New ( uint64 allocType )
{
    DLList * list = ( DLList* ) _Mem_Allocate ( sizeof ( DLList ), allocType ) ;
    list->Head = _DLNode_New ( allocType ) ;
    list->Tail = _DLNode_New ( allocType ) ;
    _DLList_Init ( list ) ;
    return list ;
}

DLList *
DLList_New ( )
{
    return _DLList_New ( DICTIONARY ) ;
}

void
DLList_ReInit ( DLList * list )
{
    DLNode * node, * nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        DLNode_Remove ( node ) ;
    }
    _DLList_Init ( list ) ;
}

void
_DLList_AddNodeToHead ( DLList *list, DLNode * node )
{
    if ( node )
    {
        DLNode_InsertThisAfterANode ( node, list->Head ) ; // after Head toward Tail
    }
}

void
DLList_AddNodeToHead ( DLList *list, DLNode * node )
{
    if ( node )
    {
        // prevent trying to add nodes already on the list; this will move it to the beginning
        DLNode_Remove ( node ) ; // if the node is already on a list it will be first removed
        _DLList_AddNodeToHead ( list, node ) ;
        list->S_CurrentNode = 0 ;
    }
}

void
DLList_AddNodeToTail ( DLList *list, DLNode * node )
{
    if ( node )
    {
        // prevent trying to add nodes already on the list; this will move it to the beginning
        DLNode_Remove ( node ) ; // prevent trying to add nodes already on the list
        DLNode_InsertThisBeforeANode ( node, list->Tail ) ; // before Tail toward Head
        list->S_CurrentNode = node ;
    }
}

DLNode *
DLList_Head ( DLList * list )
{
    if ( ! list ) return 0 ;
    return list->Head ;
}

DLNode *
DLList_Tail ( DLList * list )
{
    if ( ! list ) return 0 ;
    return list->Tail ;
}

DLNode *
_DLList_First ( DLList * list )
{
    return DLNode_Next ( list->Head ) ;
}

DLNode *
DLList_First ( DLList * list )
{
    if ( ! list ) return 0 ;
    return DLNode_Next ( list->Head ) ;
}

DLNode *
_DLList_Last ( DLList * list )
{
    return DLNode_Previous ( list->Tail ) ;
}

DLNode *
DLList_Last ( DLList * list )
{
    if ( ! list ) return 0 ;
    return DLNode_Previous ( list->Tail ) ;
}

DLNode *
DLList_NodePrevious ( DLList * list, DLNode * node )
{
    if ( node )
    {
        node = _DLNode_Previous ( node ) ;
    }
    if ( ! node ) node = DLList_Head ( list ) ;
    return node ;
}

DLNode *
DLList_NodeNext ( DLList * list, DLNode * node )
{
    if ( node )
    {
        node = _DLNode_Next ( node ) ;
    }
    if ( ! node ) node = DLList_Tail ( list ) ;
    return node ;
}

DLNode *
_DLList_Before ( DLList * list )
{
    return DLNode_Previous ( list->S_CurrentNode ) ;
}

DLNode *
DLList_Before ( DLList * list )
{
    list->S_CurrentNode = _DLList_Before ( list ) ;
    if ( list->S_CurrentNode == 0 )
    {
        list->S_CurrentNode = DLList_Head ( list ) ;
        //list->CurrentNode = DLList_First ( list ) ;
        return 0 ;
    }
    return list->S_CurrentNode ;
}
// toward the TailNode

DLNode *
_DLList_After ( DLList * list )
{
    return DLNode_Next ( list->S_CurrentNode ) ;
}
// toward the TailNode

DLNode *
DLList_After ( DLList * list )
{
    list->S_CurrentNode = _DLList_After ( list ) ;
    if ( list->S_CurrentNode == 0 )
    {
        list->S_CurrentNode = DLList_Tail ( list ) ;
        //list->CurrentNode = DLList_Last ( list ) ;
        return 0 ;
    }
    return ( DLNode* ) list->S_CurrentNode ;
}

void
DLList_Map ( DLList * list, MapFunction0 mf )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        // get nextNode before map function (mf) in case mf changes list by a Remove of current node
        // problem could arise if mf removes Next node
        nextNode = DLNode_Next ( node ) ;
        mf ( node ) ;
    }
}

void
DLList_Map1 ( DLList * list, MapFunction1 mf, int32 one )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        mf ( node, one ) ;
    }
}

void
DLList_Map2 ( DLList * list, MapFunction2 mf, int32 one, int32 two )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        mf ( node, one, two ) ;
    }
}

void
DLList_Map2_64 ( DLList * list, MapFunction2_64 mf, uint64 one, int32 two )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        if ( two ) // EXACT
        {
            if ( node->N_AType != one ) continue ;
        }
        else if ( ! ( node->N_AType & one ) ) continue ;
        mf ( node, one, two ) ;
    }
}

void
DLList_Map3 ( DLList * list, MapFunction3 mf, int32 one, int32 two, int32 three )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        mf ( node, one, two, three ) ;
    }
}

void
DLList_Map_OnePlusStatus ( DLList * list, MapFunction2 mf, int32 one, int32 * status )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node && ( *status != DONE ) ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        mf ( node, one, ( int32 ) status ) ;
    }
}

DLNode *
_DLList_AddNamedValue ( DLList * list, byte * name, int32 value, int32 allocType )
{
    Symbol * sym = _Symbol_New ( name, allocType ) ;
    sym->S_Value = (byte*) value ;
    _DLList_AddNodeToHead ( list, (DLNode*) sym ) ;
}

DLNode *
_DLList_AddValue ( DLList * list, int32 value, int32 allocType )
{
    Symbol * sym = Symbol_NewValue ( value, allocType ) ;
    _DLList_AddNodeToHead ( list, (DLNode*) sym ) ;
}

#if 0

void
DLList_Map4 ( DLList * list, MapFunction4 mf, int32 one, int32 two, int32 three, int32 four )
{
    DLNode * tnode, *nextNode ;
    for ( tnode = DLList_First ( list ) ; tnode ; tnode = nextNode )
    {
        nextNode = DLNode_Next ( tnode ) ;
        mf ( tnode, one, two, three, four ) ;
    }
}

void
DLList_Map5 ( DLList * list, MapFunction5 mf, int32 one, int32 two, int32 three, int32 four, int32 five )
{
    DLNode * tnode, *nextNode ;
    for ( tnode = DLList_First ( list ) ; tnode ; tnode = nextNode )
    {
        nextNode = DLNode_Next ( tnode ) ;
        mf ( tnode, one, two, three, four, five ) ;
    }
}
// suports recursive calls

void
DLList_Map_ThreePlusStatus ( DLList * list, MapFunction4 mf, int32 one, int32 two, int32 three, int32 * status )
{
    DLNode * tnode, *nextNode ;
    for
        ( tnode = DLList_First ( list ) ; tnode && ( *status != DONE ) ; tnode = nextNode )
    {
        nextNode = DLNode_Next ( tnode ) ;
        mf ( tnode, one, two, three, ( int32 ) status ) ;
    }
}

void
DLList_Map_TwoPlusStatus ( DLList * list, MapFunction3 mf, int32 one, int32 two, int32 * status )
{
    DLNode * tnode, *nextNode ;
    for ( tnode = DLList_First ( list ) ; tnode && ( *status != DONE ) ; tnode = nextNode )
    {
        nextNode = DLNode_Next ( tnode ) ;
        mf ( tnode, one, two, ( int32 ) status ) ;
    }
}
#endif


/*
TreeNode *
TreeNode_New ( cell allocType, cell allocType, byte * object )
{
    TreeNode * tn = (TreeNode*) Mem_Allocate ( sizeof (TreeNode) ) ;
    if ( allocType == OBJECT ) tn->Element.Object = object ;
    else tn->Element.List = _DLList_New ( Mem_Allocate ) ;
    return tn ;
}
 */
