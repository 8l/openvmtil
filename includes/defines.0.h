
/*
 *  32 bit classification of the type word : bit fields
 *  distinct types      :   distinct properties         : intersecting properties
 *    6 bits            :       6 bits                  :        20 bits
 * 64 distinct kinds    :  64 distinct properties       :    20 sub-categories 
 * enumerated           :      enumerated               :      shifted bits
*/
#define DISTINCT_TYPE_SIZE              6
#define DISTINCT_PROPERTY_SIZE          6
#define INTERSECTING_PROPERTY_SIZE      20
#define DISTINCT_TYPE_SHIFT_SIZE (DISTINCT_PROPERTY_SIZE + INTERSECTING_PROPERTY_SIZE - 1)
#define DISTINCT_PROPERTY_SHIFT_SIZE (INTERSECTING_PROPERTY_SIZE - 1)

// distinct kinds/types - non-intersecting 
#define THIS ( 1 << 20 )
#define CLASS_MEMBER_ACCESS ( 1 << 21 )
#define T_CONS ( 1 << 37 )
#define LIST ( 1 << 38 )
#define LIST_NODE ( 1 << 51 )
#define LIST_FUNCTION ( 1 << 39 )
#define LISP_LITERAL 
#define LISP_OBJECT LISP_LITERAL
#define LIST_QUOTATION LIST_FUNCTION
#define T_LAMBDA_PROC ( 1 << 18 )
#define T_LAMBDA ( 1 << 62 )
#define T_LISP_IF ( 1 << 43 )
#define T_BEGIN ( 1 << 55 )
#define T_QUOTE ( 1 << 56 )
#define T_NIL ( 1 << 54 )
#define T_DEFINE ( 1 << 60 )
#define T_HEAD ( 1 << 61 )
#define T_TAIL ( 1 << 7 )
#define T_RAW_STRING ( 1 << 44 )
#define T_STRING ( 1 << 46 )
#define T_INT ( 1 << 47 )
#define T_FLOAT ( 1 << 48 )
#define T_BIG_INT ( 1 << 49 )
#define T_BIG_FLOAT ( 1 << 50 )

// distinct properties - non-intersecting 
#define LISP_SPECIAL_WORD ( 1 << 63 )
#define CPRIMITIVE ( 1 << KIND_SHIFT_BITS )
#define CFRTIL_WORD ( 2 << KIND_SHIFT_BITS )
#define LISP_WORD ( 3 << KIND_SHIFT_BITS )
#define BLOCK ( 2 << KIND_SHIFT_BITS )
#define OBJECT ( 3 << KIND_SHIFT_BITS )
#define LITERAL OBJECT
#define CONSTANT LITERAL
#define FUNCTION ( 5 << KIND_SHIFT_BITS )
#define METHOD ( 6 << KIND_SHIFT_BITS )
#define CLASS ( 1 << KIND_SHIFT_BITS )
#define NAMESPACE ( 2 << KIND_SHIFT_BITS )
#define T_SYMBOL ( 1 << 12 )
#define HISTORY_NODE ( 3 << 3_BYTES )
#define DYNAMIC_OBJECT ( 7 << 3_BYTES )
#define DOBJECT DYNAMIC_OBJECT
#define STACK_VARIABLE ( 1 << 10 )
#define LOCAL_VARIABLE ( 1 << 11 )
#define REGISTER_VARIABLE ( 1 << 17 )
#define CATEGORY_OP_ORDERED ( 1 << 25 )
#define CATEGORY_OP_UNORDERED ( 1 << 31 )
#define CATEGORY_OP_EQUAL ( 1 << 27 )
#define CATEGORY_STACK ( 1 << 16 )
#define CATEGORY_OP_STORE ( 1 << 23 )
#define CATEGORY_LOGIC ( 1 << 28 )
#define CATEGORY_OP_1_ARG ( 1 << 29 )
#define CATEGORY_OP_STACK CATEGORY_STACK
#define CATEGORY_OP_DIVIDE ( 1 << 33 )
#define CATEGORY_OP_LOAD ( 1 << 26 )

// pure possibly intersecting properties
// these need a distinct bit
#define DEBUG_WORD ( 1 << 22 )
#define ALIAS ( 3 << TYPE_SHIFT_BITS )
#define TEXT_MACRO ( 4 << TYPE_SHIFT_BITS )
#define STRING_MACRO ( 5 << TYPE_SHIFT_BITS )
#define C_RTL_ARGS ( 6 << TYPE_SHIFT_BITS )
#define VOID_RETURN ( 7 << TYPE_SHIFT_BITS )
#define IMMEDIATE ( 9 << TYPE_SHIFT_BITS )
#define INLINE ( 10 << TYPE_SHIFT_BITS )
#define PREFIX ( 11 << TYPE_SHIFT_BITS )
#define COMPILED_INLINE ( 12 << TYPE_SHIFT_BITS )
#define COMPILED_OPTIMIZED ( 13 << TYPE_SHIFT_BITS )
// optimze types
#define CATEGORY_DUP ( 1 << 24 )
#define CATEGORY_RECURSIVE ( 1 << 30 )
//#define MACRO ( 14 << TYPE_SHIFT_BITS )
//#define OPTIMIZE ( 15 << TYPE_SHIFT_BITS )

