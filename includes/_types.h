
/*
 *  32 bit classification of the type word : bit fields
 *  distinct types      :   distinct properties         : intersecting properties
 *    6 bits            :       6 bits                  :        52 bits
 * 64 distinct kinds    :  64 distinct properties       :    52 sub-categories 
 * enumerated           :      enumerated               :      shifted bits
*/
#define DISTINCT_TYPE_SIZE              6
#define DISTINCT_PROPERTY_SIZE          6
#define INTERSECTING_PROPERTY_SIZE      52
#define DISTINCT_TYPE_SHIFT_SIZE (DISTINCT_PROPERTY_SIZE + INTERSECTING_PROPERTY_SIZE - 1)
#define DISTINCT_PROPERTY_SHIFT_SIZE (INTERSECTING_PROPERTY_SIZE - 1)
#define dTss DISTINCT_TYPE_SHIFT_SIZE 
#define dPss DISTINCT_PROPERTY_SHIFT_SIZE 

// distinct kinds/types - non-intersecting - not used with other properties or types
#define CLASS_MEMBER_ACCESS             ( (uint64) 1 << dTss )
#define T_CONS                          ( (uint64) 2 << dTss )
#define LIST                            ( (uint64) 3 << dTss )
#define LIST_NODE                       ( (uint64) 4 << dTss )
#define LIST_FUNCTION                   ( (uint64) 5 << dTss )
#define LISP_LITERAL            
#define LISP_OBJECT LISP_LITERAL
#define LIST_QUOTATION LIST_FUNCTION
#define T_LAMBDA_PROC                   ( (uint64) 6 << dTss )
#define T_LAMBDA                        ( (uint64) 7 << dTss )
#define T_LISP_LAMBDA T_LAMBDA       
#define T_LISP_IF                       ( (uint64) 8 << dTss )
#define T_BEGIN                         ( (uint64) 9 << dTss )
#define T_LISP_BEGIN T_BEGIN                         
#define T_QUOTE                         ( (uint64) 10 << dTss )
#define T_LISP_QUOTE T_QUOTE   
#define T_NIL                           ( (uint64) 11 << dTss )
#define T_DEFINE                        ( (uint64) 12 << dTss )
#define T_LISP_DEFINE T_DEFINE
#define T_RAW_STRING                    ( (uint64) 15 << dTss )
#define T_STRING                        ( (uint64) 16 << dTss )
#define T_INT                           ( (uint64) 17 << dTss )
#define T_FLOAT                         ( (uint64) 18 << dTss )
#define T_BIG_INT                       ( (uint64) 19 << dTss )
#define T_BIG_FLOAT                     ( (uint64) 20 << dTss )
#define T_LISP_SET                      ( (uint64) 21 << dTss )

// distinct properties - non-intersecting - not used with other properties or types
#define HISTORY_NODE                    ( (uint64) 12 << dPss )

// pure possibly intersecting properties - used with other properties or types
// these need a distinct bit 
#define OBJECT                          ( (uint64) 1 << 0 )
#define LITERAL OBJECT
#define FUNCTION                        ( (uint64) 1 << 1 )
#define METHOD                          ( (uint64) 1 << 2 )
#define T_SYMBOL                        ( (uint64) 1 << 3 )
#define VARIABLE T_SYMBOL              
#define C_RTL_ARGS                      ( (uint64) 1 << 4 )
#define VOID_RETURN                     ( (uint64) 1 << 5 )
#define INLINE                          ( (uint64) 1 << 6 )
#define PREFIX                          ( (uint64) 1 << 7 )
#define COMPILED_INLINE                 ( (uint64) 1 << 8 )
#define COMPILED_OPTIMIZED              ( (uint64) 1 << 9 )
// optimze types
#define CATEGORY_DUP                    ( (uint64) 1 << 10 )
#define CATEGORY_RECURSIVE              ( (uint64) 1 << 11 )
#define LISP_SPECIAL_WORD               ( (uint64) 1 << 12 )
#define T_LISP_SPECIAL LISP_SPECIAL_WORD 
#define T_HEAD                          ( (uint64) 1 << 13 )
#define T_TAIL                          ( (uint64) 1 << 14 )
#define DEBUG_WORD                      ( (uint64) 1 << 15 )
#define CPRIMITIVE                      ( (uint64) 1 << 16 )
#define CFRTIL_WORD                     ( (uint64) 1 << 17 )
#define DYNAMIC_OBJECT                  ( (uint64) 1 << 18 )
#define DOBJECT DYNAMIC_OBJECT
#define CATEGORY_OP_ORDERED             ( (uint64) 1 << 19 )
#define CATEGORY_OP_UNORDERED           ( (uint64) 1 << 20 )
#define CATEGORY_OP_EQUAL               ( (uint64) 1 << 21 )
#define CATEGORY_STACK                  ( (uint64) 1 << 22 )
#define CATEGORY_OP_STORE               ( (uint64) 1 << 23 )
#define CATEGORY_LOGIC                  ( (uint64) 1 << 24 )
#define CATEGORY_OP_1_ARG               ( (uint64) 1 << 25 )
#define CATEGORY_OP_STACK CATEGORY_STACK
#define CATEGORY_OP_DIVIDE              ( (uint64) 1 << 26 )
#define CATEGORY_OP_LOAD                ( (uint64) 1 << 27 )
#define CLASS                           ( (uint64) 1 << 28 )
#define NAMESPACE                       ( (uint64) 1 << 29 )
#define LISP_WORD                       ( (uint64) 1 << 30 )
#define BLOCK                           ( (uint64) 1 << 31 )
#define ALIAS                           ( (uint64) 1 << 32 )
#define TEXT_MACRO                      ( (uint64) 1 << 33 )
#define STRING_MACRO                    ( (uint64) 1 << 34 )
#define IMMEDIATE                       ( (uint64) 1 << 35 )
#define THIS                            ( (uint64) 1 << 36 )
#define STACK_VARIABLE                  ( (uint64) 1 << 37 )
#define LOCAL_VARIABLE                  ( (uint64) 1 << 38 )
#define REGISTER_VARIABLE               ( (uint64) 1 << 39 )
#define CONSTANT                        ( (uint64) 1 << 40 ) // nb. a constant behave different than a literal
//#define MACRO ( (uint64) 14 << TYPE_SHIFT_BITS )
//#define OPTIMIZE ( (uint64) 15 << TYPE_SHIFT_BITS )

