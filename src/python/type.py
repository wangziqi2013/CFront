#
# type.py - This function defines types and the type system
#

from symbol_table import Scope, SymbolTable

#####################################################################
# class BaseType and its sub-classes
#####################################################################

class BaseType:
    """
    This class serves as the base class for all types
    that could be used as a base type

    Note that typedef names are not considered as a separate
    type because typedef'ed names may be compatible with
    other types, so we always expand typedef'ed types
    """

    # These are constant values we use to check whether a flag is set or not
    TYPE_SPEC_NONE = 0x00000000
    TYPE_SPEC_CONST = 0x00000001
    TYPE_SPEC_VOLATILE = 0x00000002
    TYPE_SPEC_STATIC = 0x00000004
    TYPE_SPEC_REGISTER = 0x00000008
    TYPE_SPEC_EXTERN = 0x00000010
    TYPE_SPEC_UNSIGNED = 0x00000020
    TYPE_SPEC_AUTO = 0x00000040
    TYPE_SPEC_SIGNED = 0x00000080

    # This is a dict that maps the token type to spec value
    TYPE_SPEC_DICT = {
        # Type qualifier
        "T_CONST": TYPE_SPEC_CONST,
        "T_VOLATILE": TYPE_SPEC_VOLATILE,
        # Storage class specifier
        "T_STATIC": TYPE_SPEC_STATIC,
        "T_REGISTER": TYPE_SPEC_REGISTER,
        "T_EXTERN": TYPE_SPEC_EXTERN,
        "T_UNSIGNED": TYPE_SPEC_UNSIGNED,
        "T_AUTO": TYPE_SPEC_AUTO,
        "T_SIGNED": TYPE_SPEC_SIGNED,
    }

    def __init__(self):
        """
        Initialize the base type attributes which are shared between
        all base types
        """
        self.type_spec = BaseType.TYPE_SPEC_NONE
        return

    def add_spec(self, spec_name):
        """
        Add a specifier to the base type.

          (1) If the spec is not defined then assertion fails
              because this should be regulated by the syntax
          (2) If the spec has already been defined then an exception
              is thrown because the input program is wrong

        :param spec_name: The string object that contains the
                          specification
        :return: None
        """
        # It must be found, otherwise it is implementation error
        mask = BaseType.TYPE_SPEC_DICT.get(spec_name, None)
        assert(mask is not None)

        # If already defined then throw error
        if (self.type_spec & mask) != 0x0:
            raise TypeError("Duplicated type specifier or qualifier: %s" %
                            (spec_name, ))

        # Otherwise just OR it to the specifier bit mask
        self.type_spec |= mask

        return

    @staticmethod
    def get_base_type(spec_node):
        """
        Return a base type with the syntax node that specifies the
        base type and specifiers

        :param spec_node: The T_SPEC_QUAL_LIST or T_DECL_SPEC
        :return: One of the BaseType node
        """
        assert(spec_node.symbol == "T_SPEC_QUAL_LIST" or \
               spec_node.symbol == "T_DECL_SPEC")
        

class IntType(BaseType):
    """
    This class represents arbitrary precision integer types
    It carries the byte length of the integer
    """
    def __init__(self, byte_length):
        """
        Initialize the byte length of the integer
        """
        BaseType.__init__(self)
        self.byte_length = byte_length
        return

class VoidType(BaseType):
    """
    This class represents void type which carries no
    type related information
    """
    def __init__(self):
        """
        Initialize the void type
        """
        BaseType.__init__(self)
        return

class StructType(BaseType):
    """
    This class represents the struct type. Note that struct types
    are not compatible with any other types, and therefore the
    name of the struct suffices as the identifier of the underlying
    struct type. Same is true for unions
    """
    def __init__(self, name):
        """
        Initialize the struct name

        :param name: The name of the struct
        """
        BaseType.__init__(self)
        self.name = name
        return

class UnionType(BaseType):
    """
    This class represents the union type. Union type has the same
    property of struct type, so it only requires a name
    """
    def __init__(self, name):
        """
        Initialize the struct name

        :param name: The name of the struct
        """
        BaseType.__init__(self)
        self.name = name
        return

class BitFieldType(BaseType):
    """
    Bitfield type that has a bit length. The declaration of this
    type is not included, and should be checked when building
    the bit field type (e.g. whether the bit length exceeds
    the declared base length). Sign bit of the bit field is not
    defined and should not be relied on

    Bit field type could not constitute pointers or arrays
    """
    def __init__(self, bit_length):
        """
        Initialize the bitfield with a bit length

        :param bit_length: The bit length of the type
        """
        BaseType.__init__(self)
        self.bit_length = bit_length
        return

#####################################################################
# The following are derivation operations
#####################################################################

class PtrType(BaseType):
    """
    This class represents pointer type. Pointer types need specifiers
    as the type specifier for the current level
    """
    def __init__(self):
        """
        Initialize an empty object denoting the operation
        """
        BaseType.__init__()
        return

class ArrayType(BaseType):
    """
    This class represents an array type. An array type has associated
    data as the static size of the array (which requires static
    evaluation of expressions). If the array size is not known
    then we ignore it
    """
    def __init__(self, array_size):
        """
        Initialize the array type using array size
        :param array_size: The size of the array; Could be None
                           if size is not known
        """
        BaseType.__init__(self)
        self.array_size = array_size
        return

class FuncType(BaseType):
    """
    This class represents a function pointer type. The data it carries
    is function parameter type list, which is a list of types, optionally
    with name bindings if names are specified for functions
    """
    def __init__(self, param_list):
        """
        Initialize the function type using a parameter list

        :param param_list: A list of parameter types
        """
        BaseType.__init__(self)
        self.param_list = param_list
        return

#####################################################################
# class TypeNode
#####################################################################

class TypeNode:
    """
    This class represents base type and type derivation rules
    """
    def __init__(self):
        """
        Initialize the type node with a base type
        """
        # Type derivation rule. We store the operation
        # with the highest precedence before operations
        # with lower precedence
        # Note that the last element must be the base type
        self.rule_list = []

        # We use this as a lazy way of applying operations on
        # a type. All interpretation of the type should start
        # with elements using this index
        self.index = 0

        return

    def __len__(self):
        """
        Returns the length of the actual array (i.e. starting at
        index = 0). The array must have length greater than zero

        :return: int
        """
        assert(len(self.rule_list) > 0)
        assert(0 <= index < len(self.rule_list))

        return len(self.rule_list)

    def expand_typedef_name(self, symbol_table, typedef_name):
        """
        This function expands typedef name into the current type
        node to yield a new type

        If the typedef'ed name does not exist assertion fails,
        because the parser guarantees that typedef'ed names are
        recognized only if they are defined

        :param symbol_table: The symbol table
        :param typedef_name: The type name that is typedef'ed
        :return: None
        """
        t = symbol_table.get((Scope.TYPEDEF, typedef_name), None)
        assert(t is not None)
        assert(isinstance(t, TypeNode))
        # Append the rule list of the typedef'ed name
        # to the current type node. Note that this is only
        # a shallow copy of the array
        self.rule_list += t.rule_list[index:]

        return