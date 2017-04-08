#
# type.py - This function defines types and the type system
#

#####################################################################
# class BaseType and its sub-classes
#####################################################################

class BaseType:
    """
    This class serves as the base class for all types
    that could be used as a base type
    """
    def __init__(self):
        return

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
    

#####################################################################
# class TypeNode
#####################################################################

class TypeNode:
    """
    This class represents base type and type derivation rules
    """
    def __init__(self, base_type):
        """
        Initialize the type node with a base type
        """
        # This is the base type after all derivations
        self.base_type = base_type
        # Type derivation rule. We store the operation
        # with the highest precedence before operations
        # with lower precedence
        self.rule_list = []

        return