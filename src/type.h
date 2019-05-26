
#ifndef _TYPE_H
#define _TYPE_H

#include <ctype.h>
#include "list.h"
#include "bintree.h"
#include "token.h"
#include "str.h"

#define SCOPE_LEVEL_GLOBAL  0
#define TYPE_PTR_SIZE       8  // A pointer has 8 bytes
#define TYPE_CHAR_SIZE      1
#define TYPE_SHORT_SIZE     2
#define TYPE_INT_SIZE       4
#define TYPE_LONG_SIZE      8
#define TYPE_LLONG_SIZE     16
#define TYPE_UNKNOWN_SIZE   ((size_t)-1) // Array decl without concrete size or struct/union forward decl
#define TYPE_CHAR_MAX       ((1 << (TYPE_CHAR_SIZE * 8 - 1)) - 1)
#define TYPE_UCHAR_MAX      ((1 << (TYPE_CHAR_SIZE * 8)) - 1)

// Used with comp_t init function
#define COMP_NO_DEFINITION   0
#define COMP_HAS_DEFINITION  1

// Arguments to type_gettype
#define TYPE_ALLOW_VOID   0x00000001     // Whether allow void as base type (only allowed for func arg)
#define TYPE_ALLOW_STGCLS 0x00000002     // Whether allow storage class (not for comp type and func arg)

// Used with type_typeof
#define TYPEOF_IGNORE_FUNC_ARG     0x00000001 // Do not check type for function argument
#define TYPEOF_IGNORE_ARRAY_INDEX  0x00000002 // Do not check array index type

// Used with type_cast
#define TYPE_CAST_EXPLICIT         0          // Explicit cast using cast operator
#define TYPE_CAST_IMPLICIT         1          // Implicit cast with array indexing, func arg, and assignment
#define TYPE_CAST_INVALID          0          // Return value: invalid cast
#define TYPE_CAST_SIGN_EXT         1          // Return value: should perform sign extension
#define TYPE_CAST_ZERO_EXT         2          // Return value: should perform zero extension
#define TYPE_CAST_TRUNCATE         3          // Return value: should truncate
#define TYPE_CAST_NO_OP            4          // No special bit operation needed

enum {
  SCOPE_VALUE  = 1, // Named variable that has a value or memory location; Enum constants are put here
  SCOPE_ENUM   = 0, // Enum structures are put here 
  SCOPE_STRUCT = 2,
  SCOPE_UNION  = 3,
  SCOPE_UDEF   = 4, // Type aliasing, i.e. maps a name to a type
  SCOPE_TYPE_COUNT,
};

// We track objects using a centralized list per scope to simplify memory management
// All objects allocated within the scope will be freed when the scope gets poped
// No ownership of memory is therefore enforced, i.e. objects do not own each other.
enum {
  OBJ_TYPE  = 0,
  OBJ_COMP  = 1,
  OBJ_FIELD = 2,
  OBJ_ENUM  = 3,
  OBJ_VALUE = 4,   // Note that not all values are named
  OBJ_TYPE_COUNT,
};

// A statement block creates a new scope. The bottomost scope is the global scope
typedef struct {
  int level;                            // 0 means global
  hashtable_t *names[SCOPE_TYPE_COUNT]; // enum, var, struct, union, udef, i.e. symbol table. Does not own anything
  list_t *objs[OBJ_TYPE_COUNT];      // Objects allocates while processing the scope; Freed on scope free; Owns everything
} scope_t;

typedef void (*obj_free_func_t)(void *);   // Call back handlers for object free; Register one for each type
extern obj_free_func_t obj_free_func_list[OBJ_TYPE_COUNT + 1]; // Registered call back functions for objects

typedef struct {
  stack_t *scopes;
} type_cxt_t;

typedef uint64_t typeid_t;
typedef uint64_t offset_t;

typedef enum {
  LVALUES_BEGIN = 1, 
  ADDR_STACK, ADDR_HEAP, ADDR_GLOBAL,
  LVALUES_END, RVALUES_BEGIN = 10,
  ADDR_TEMP, // Unnamed variable (intermediate node of an expression)
  ADDR_IMM,  // Immediate value (constants)
  RVALUES_END,
} addrtype_t;

struct comp_t_struct;
struct enum_t_struct;

typedef struct type_t_struct {
  decl_prop_t decl_prop;   // Can be BASETYPE_ or TYPE_OP_ or DECL_ series
  struct type_t_struct *next; // If derived type, this points to the next type by applying the op; Do not own
  union {
    struct comp_t_struct *comp; // If base type indicates s/u/e this is a pointer to it; Do not own
    struct enum_t_struct *enu;
  };
  union {
    int array_size;         // If decl_prop is array sub this stores the (optional) size of the array
    struct {
      list_t *arg_list;     // If decl_prop is function call this stores a list of type_t *; Owns memory
      bintree_t *arg_index; // Binary tree using arg name as key; Owns memory
      int vararg;           // Set if varadic argument function
    };
    struct {            // Only valid if base type is BASETYPE_UDEF
      char *udef_name;  // Stores user defined type's name (i.e. the name we use to refer to it)
      struct type_t_struct *udef_type; // Stores actual type for udef'ed names
    };
  };
  size_t size;  // Always check if it is TYPE_UNKNOWN_SIZE which means compile time size unknown or undefined comp
} type_t;

typedef struct { // Integer (builtin type) properties
  int sign;      // 0 means unsigned, 1 means signed
  int size;      // Number of bytes
} int_prop_t;

extern int_prop_t ints[11];           // An array of integer properties for conversion
extern type_t type_builtin_ints[11];  // An array of built in integer types
extern type_t type_builtin_const_char;
extern type_t type_builtin_string_template;

typedef struct value_t_struct {
  type_t *type;         // Do not own
  addrtype_t addrtype;
  union {
    int8_t   charval;   // The following are for constant evaluation; We do not support long long
    uint8_t  ucharval;
    int16_t  shortval;
    uint16_t ushortval;
    int32_t  intval;
    uint32_t uintval;
    int64_t  longval;
    uint64_t ulongval;
    int64_t  offset;    // If it represents an address then this is the offset (relative to data segment, stack, etc.)
    uint64_t ptrval;
  };
} value_t;

// Represents composite type
typedef struct comp_t_struct {
  char *source_offset;    // If name is not NULL this is the token's offset
  char *name;             // NULL if no name; Does not own memory
  list_t *field_list;     // A list of field *; Does not contain promoted comp types; Owns memory;
  bintree_t *field_index; // These two provides both fast named access, and ordered storage; Owns memory
  size_t size;
  int has_definition;     // Whether it is a forward definition (0 means yes)
} comp_t;

// Single field within the composite type
typedef struct {
  char *source_offset; // If name is not NULL this is the token's offset
  char *name;          // NULL if anonymous field; Does not own memory
  int bitfield_size;   // Set if bit field; -1 if not
  int bitfield_offset; // Bit offset within the integer; Must not be larger than size
  int offset;          // Offset within the composite structure
  size_t size;         // Number of bytes occupied by the actual storage including padding
  type_t *type;        // Type of this field; Do not own memory
} field_t;

typedef struct enum_t_struct {
  char *offset;            // If name is not NULL this is the token's offset
  char *name;              // NULL if unnamed enum
  list_t *field_list;      
  bintree_t *field_index;  // Same as above
  size_t size;             // Fixed size - same as integer
} enum_t;

static inline void type_error_not_supported(const char *offset, decl_prop_t decl_prop) {
  error_row_col_exit(offset, "Sorry, type \"%s\" not yet supported\n", token_decl_print(decl_prop));
}
// Returns 1 if it is integer types. Applies to any type object
static inline int type_is_integer(type_t *type) {
  return BASETYPE_GET(type->decl_prop) >= BASETYPE_CHAR && BASETYPE_GET(type->decl_prop) <= BASETYPE_ULLONG;
}
// Returns 1 if type is signed; only valid for integer types; undefined for others
static inline int type_is_signed(type_t *type) {
  assert(type_is_integer(type));
  assert(BASETYPE_INDEX(type->decl_prop) >= 1 && BASETYPE_INDEX(type->decl_prop) <= 10);
  return ints[BASETYPE_INDEX(type->decl_prop)].sign;
}
static inline int type_is_const(type_t *type) { // Returns 1 if the type has const modifier
  return !!(type->decl_prop & DECL_CONST_MASK);
}
static inline int type_is_volatile(type_t *type) { // Returns 1 if the type has const modifier
  return !!(type->decl_prop & DECL_VOLATILE_MASK);
}
// Returns 1 if it is struct or union type. Applies to any type object
static inline int type_is_comp(type_t *type) {
  return BASETYPE_GET(type->decl_prop) == BASETYPE_STRUCT || BASETYPE_GET(type->decl_prop) == BASETYPE_UNION;
}
static inline const char *type_printable_name(const char *name) { return name ? name : "<No Name>"; }

type_t *type_get_strliteral(type_cxt_t *cxt, size_t full_size); // Returns a const char[full_size] type object

str_t *type_print(type_t *type, const char *name, str_t *s, int print_comp_body, int level);

scope_t *scope_init(int level);
void scope_free(scope_t *scope);
void init_builtin_types();
type_cxt_t *type_sys_init();
void type_sys_free(type_cxt_t *cxt); 

hashtable_t *scope_atlevel(type_cxt_t *cxt, int level, int domain);
hashtable_t *scope_top_name(type_cxt_t *cxt, int domain);
int scope_numlevel(type_cxt_t *cxt);
void scope_recurse(type_cxt_t *cxt);
void scope_decurse(type_cxt_t *cxt);
void *scope_top_find(type_cxt_t *cxt, int domain, void *key);
void *scope_top_insert(type_cxt_t *cxt, int domain, void *key, void *value);
void *scope_search(type_cxt_t *cxt, int domain, void *name);
void scope_top_obj_insert(type_cxt_t *cxt, int domain, void *obj); // Adding an object into the topmost scope for memory mgmt

type_t *type_init(type_cxt_t *cxt);
void type_free(void *ptr);
comp_t *comp_init(type_cxt_t *cxt, char *name, char *source_offset, int has_definition);
void comp_free(void *ptr);
field_t *field_init(type_cxt_t *cxt);
void field_free(void *ptr);
enum_t *enum_init(type_cxt_t *cxt);
void enum_free(void *ptr);
value_t *value_init(type_cxt_t *cxt);
void value_free(void *ptr);

// Returns a type * object given a T_DECL node and optionally base type
type_t *type_gettype(type_cxt_t *cxt, token_t *decl, token_t *basetype, uint32_t flags); 
comp_t *type_getcomp(type_cxt_t *cxt, token_t *token, int is_forward);
enum_t *type_getenum(type_cxt_t *cxt, token_t *token);

int type_cast(type_t *to, type_t *from, int cast_type, char *offset);
type_t *type_typeof(type_cxt_t *cxt, token_t *exp, uint32_t options); // Evaluate the type of an expression

#endif