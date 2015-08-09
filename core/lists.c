
#include "../includes/cfrtil.h"

int32
List_Length ( DLList * list )
{
    int32 i ;
    DLNode * node, *nextNode ;
    for ( i = 0, node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        i++ ;
    }
    return i ;
}

DLNode *
List_PrintValues ( DLList * list )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        Printf ( " %s,", ((Word*) ((Symbol *) node)->S_Value)->Name )  ;
    }
    return 0 ;
}

DLNode *
List_Search ( DLList * list, int32 value )
{
    DLNode * node, *nextNode ;
    for ( node = DLList_First ( list ) ; node ; node = nextNode )
    {
        nextNode = DLNode_Next ( node ) ;
        if ( ((Symbol *) node)->S_Value == (byte*) value ) return node ;
    }
    return 0 ;
}

DLNode *
List_AddValue ( DLList * list, int32 value )
{
    return _DLList_AddValue ( list, value, SESSION );
}

DLNode *
List_AddNamedValue ( DLList * list, byte * name, int32 value )
{
    return _DLList_AddNamedValue ( list, name, value, SESSION );
}

