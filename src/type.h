
#ifndef _TYPE_H
#define _TYPE_H

#include <ctype.h>
#include "list.h"
#include "bintree.h"
#include "token.h"
#include "str.h"

#define SCOPE_LEVEL_GLOBAL  0
#define TYPE_PTR_SIZE       8  // A pointer has 8 bytes
#define TYPE_UNKNOWN_SIZE   ((size_t)-1) // Array decl without concrete size or struct/union forward decl

// Arguments to type_gettype
#define TYPE_ALLOW_VOID   0x00000001     // Whether allow void as base type (only allowed for func arg)
#define TYPE_ALLOW_STGCLS 0x00000002     // Whether allow storage class (not for comp type and func arg)

enum {
  SCOPE_VAR    = 1, // Variable that has a value or memory location; Enum constants are put here
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

typedef struct value_t_struct {
  type_t *type;         // Do not own
  addrtype_t addrtype;
  union {
    uint8_t ucharval;
    uint16_t ushortval;
    uint32_t uintval;
    uint64_t ulongval;
    offset_t offset;    // If it represents an address then this is the offset
  };
} value_t;

#define COMP_NO_DEFINITION   0
#define COMP_HAS_DEFINITION  1

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

// Returns 1 if it is integer types. Applies to any type object
static inline int type_is_integer(type_t *type) {
  return BASETYPE_GET(type->decl_prop) >= BASETYPE_CHAR && BASETYPE_GET(type->decl_prop) <= BASETYPE_ULLONG;
}
// Returns 1 if it is struct or union type. Applies to any type object
static inline int type_is_comp(type_t *type) {
  return BASETYPE_GET(type->decl_prop) == BASETYPE_STRUCT || BASETYPE_GET(type->decl_prop) == BASETYPE_UNION;
}

static inline const char *type_printable_name(const char *name) { return name ? name : "<No Name>"; }

typedef struct { // Integer (builtin type) properties
  int sign;      // 0 means unsigned, 1 means signed
  int size;      // Number of bytes
} int_prop_t;

extern int_prop_t ints[11]; // An array of integer properties

str_t *type_print(type_t *type, const char *name, str_t *s, int print_comp_body, int level);

scope_t *scope_init(int level);
void scope_free(scope_t *scope);
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

// Returns a type * object given a T_DECL node and optionally base type
type_t *type_gettype(type_cxt_t *cxt, token_t *decl, token_t *basetype, uint32_t flags); 
comp_t *type_getcomp(type_cxt_t *cxt, token_t *token, int is_forward);
enum_t *type_getenum(type_cxt_t *cxt, token_t *token);

#endif