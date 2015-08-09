
#include "../includes/cfrtil.h"

#if 0 // not used yet -- maybe replaced by better DObject functionality
Property *
_Property_Create ( )
{
    Property * property ;
#if 0    
    if ( category & ( LOCAL_VARIABLE | STACK_VARIABLE ) )
    {
        property = ( Property* ) _Mem_Allocate ( sizeof (Property ), SESSION, 0 ) ;
    }
    else
#endif        
    {
        property = ( Property * ) _Mem_Allocate ( sizeof (Property ), DICTIONARY ) ;
    }
    return property ;
}

Property *
_Property_Init ( Property * property, byte * name, int32 value, uint64 category )
{
    //property->Symbol = ( Symbol * ) Mem_Allocate ( sizeof ( Symbol ), DICTIONARY ) ;
    _Symbol_Init_AllocName ( (Symbol*) property, name, DICTIONARY ) ;
    property->S_Value = value ;
    return property ;
}

Property *
_Property_New ( byte * name, int32 value )
{
    Property * property = _Property_Create ( ) ;
    _Property_Init ( property, name, value, PROPERTY | VARIABLE ) ;
    return property ;
}

void
_Property_SetValue ( Property * property, int32 value )
{
    if ( property ) property->S_Value = value ;
}

int32
_Property_GetValue ( Property * property )
{
    if ( property ) return property->S_Value ;
    else return 0 ;
}

void
CfrTil_Property_New ( )
{
    byte * name = ( byte * ) _DataStack_Pop ( ) ;
    int32 value = ( int32 ) _DataStack_Pop ( ) ;
    Property * property = _Property_New ( name, value ) ;
    _CfrTil_AddSymbol ( (Symbol*) property ) ;
}

Property *
_DObject_FindProperty ( DObject * dobject, byte * name )
{
    Word * word = Word_FindInOneNamespace ( dobject, name ) ;
    if ( word ) return ( (Property*) word ) ;
    else return 0 ;
}

Property *
_DObject_Property_New ( DObject * dobject, byte * name, int32 value )
{
    Property * property = _Property_New ( name, value ) ;
    _Namespace_DoAddSymbol ( dobject, (Symbol*) property ) ;
    dobject->Slots ++ ;
    return property ;
}

int32
_DObject_GetPropertyValue ( DObject * dobject, byte * name )
{
    Property * property = _DObject_FindProperty ( dobject, name ) ;
    return _Property_GetValue ( property ) ;
}

Property *
_DObject_SetPropertyValue ( DObject * dobject, byte * name, int32 value )
{
    Property * property = _DObject_FindProperty ( dobject, name ) ;
    if ( property ) _Property_SetValue ( property, value ) ;
    else property = _DObject_Property_New ( dobject, name, value ) ;
    return property ;
}

Property *
_DObject_FindProperty_BottomUp ( DObject * dobject, byte * name )
{
    Word * word = 0 ;
    do
    {
        if ( ( word = Word_FindInOneNamespace ( dobject, name ) ) ) break ;
    }
    while ( ( dobject = dobject->ContainingNamespace ) ) ;
    if ( word ) return (Property*) ( word ) ;
    else return 0 ;
}
#endif


