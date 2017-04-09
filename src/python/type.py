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

    # This defines the length of integers
    # Note that the length of long is 8 rather than 4
    TYPE_LENGTH_CHAR = 1
    TYPE_LENGTH_SHORT = 2
    TYPE_LENGTH_INT = 4
    TYPE_LENGTH_LONG = 8

    def __init__(self):
        """
        Initialize the base type attributes which are shared between
        all base types
        """
        self.type_spec = BaseType.TYPE_SPEC_NONE
        return

    def add_spec_list(self, spec_list):
        """
        Add specs from a spec list

        :param spec_list: T_DECL_SPEC OR T_SPEC_QUAL_LIST
        :return: None
        """
        assert(spec_list.symbol == "T_DECL_SPEC" or
               spec_list.symbol == "T_SPEC_QUAL_LIST")

        # For each node in the spec list, add the specifier
        for node in spec_list.child_list:
            self.add_spec(node.symbol)

        return

    def add_spec(self, spec_name):
        """
        Add a specifier to the base type.

          (1) If the spec is not defined then we ignore it
              because there might be other information
          (2) If the spec has already been defined then an exception
              is thrown because the input program is wrong

        :param spec_name: The string object that contains the
                          specification
        :return: None
        """
        # It must be found, otherwise it is implementation error
        # Mask could be None because there might be other
        # information such as the base type
        mask = BaseType.TYPE_SPEC_DICT.get(spec_name, None)

        # If already defined then throw error
        if (self.type_spec & mask) != 0x0:
            raise TypeError("Duplicated type specifier or qualifier: %s" %
                            (spec_name, ))

        # Otherwise just OR it to the specifier bit mask
        self.type_spec |= mask

        return

#####################################################################
# class IntType, VoidType, StructType, UnionType, BitFieldType
#####################################################################

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
    def __init__(self, param_type_list, is_vararg):
        """
        Initialize the function type using a parameter list

        :param param_type_list: A list of parameter types
        :param is_vararg: Whether the function is vararg
        """
        BaseType.__init__(self)
        self.param_type_list = param_type_list
        self.is_vararg = is_vararg
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

    def add_derivation(self, spec_body_node):
        """
        This function processes a given derivation body
        and adds them to the rule list, from the highest
        precedence to the lowest precedence

        :param spec_body_node: T_DECL_BODY or T_ABS_DECL_BODY
        :return: None
        """
        assert(spec_body_node.symbol == "T_DECL_BODY" or
               spec_body_node.symbol == "T_ABS_DECL_BODY")

        i = 0
        # Make sure it always has
        while i < len(spec_body_node):
            child = spec_body_node[i]
            child_name = child.symbol
            if child_name == "T_PTR":
                # There might be multiple levels of pointers
                # We add specifier for each level
                for ptr in child.child_list:
                    ptr_type = PtrType()
                    # Then it must be a specifier list
                    if ptr.symbol != "T_":
                        ptr_type.add_spec_list(ptr)

                    self.rule_list.append(ptr_type)

                # It only takes one slot
                i += 1
            elif child_name == "T_IDENT":
                # It also only takes one slot
                i += 1
            elif child_name == "T_ARRAY_SUB":
                sub = spec_body_node[i + 1]

                if sub.symbol != "T_":
                    raise NotImplementedError("Static evaluation of array sizes")
                else:
                    array_type = ArrayType()

                self.rule_list.append(array_type)
                # It takes two slots
                i += 2
            elif child_name == "T_FUNC_CALL":
                sub = spec_body_node[i + 1]
                if sub.symbol != "T_":
                    if sub.symbol == "T_IDENT_LIST":
                        raise TypeError("Old-style function declaration" +
                                        " no longer supported")
                    raise NotImplementedError("Type for function arguments")
                else:
                    # Empty list for arguments
                    func_type = FuncType([], [])
            else:
                # Do not know what is the type
                assert False

        return

    @staticmethod
    def report_conflict_base_type(integer, struct, union, typedef):
        """
        Reports conflict types there must be 1 and only 1 setting to
        True in the four arguments.

        This function never returns

        :param integer: Whether base is integer
        :param struct: Whether base is struct
        :param union: Whether base is union
        :param typedef: Whether base is typedef'ed name
        :return: None
        """
        # Exactly two of these could be true
        assert(int(integer) +
               int(struct) +
               int(union) +
               int(typedef) == 2)

        if integer and struct:
            raise TypeError("Conflicting types: integer and struct")
        elif integer and union:
            raise TypeError("Conflicting types: integer and union")
        elif integer and typedef:
            raise TypeError("Conflicting types: integer and typedef'ed type")
        elif struct and union:
            raise TypeError("Conflicting types: struct and union")
        elif struct and typedef:
            raise TypeError("Conflicting types: struct and typedef'ed type")
        elif union and typedef:
            raise TypeError("Conflicting types: union and typedef'ed type")
        else:
            assert False

    @staticmethod
    def report_conflict_integer_length(char, short, long):
        """
        This function repoerts conflict integer length, and
        it never returns

        :param char: Whether char is specified
        :param short: Whether short is specified
        :param long: Whether long is specified
        :return: None
        """
        assert(int(char) + int(short) + int(long) == 2)
        if char and short:
            raise ValueError("Conflicting integer types: char and short")
        elif char and long:
            raise ValueError("Conflicting integer types: char and long")
        elif short and long:
            raise ValueError("Conflicting integer types: short and long")
        else:
            assert False

    def add_base_type_node(self, spec_node):
        """
        Return a base type TypeNode with the syntax node that
        specifies the base type and specifiers

        :param spec_node: The T_SPEC_QUAL_LIST or T_DECL_SPEC
        :return: One of the BaseType node
        """
        assert (spec_node.symbol == "T_SPEC_QUAL_LIST" or
                spec_node.symbol == "T_DECL_SPEC")

        # The following should be
        # Whether we have seen "int"
        integer = False
        # Whether we have seen struct, union, and typedef
        struct = False
        union = False
        typedef = False

        # This points to the base node data (i.e. for
        # struct union and typedef this is the identifier)
        data = None

        # This is for changing integer type
        long = False
        short = False
        char = False

        for node in spec_node.child_list:
            name = node.symbol
            if name == "T_INT":
                integer = True
                if struct or union or typedef:
                    TypeNode.report_conflict_base_type(integer,
                                                       struct,
                                                       union,
                                                       typedef)
            elif name == "T_LONG":
                integer = True
                long = True