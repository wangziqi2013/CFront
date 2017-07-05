#
# basic_type.py - This file defines primitives types for the language. We use basic types
#                 to build more complicated types (e.g. arrays, structs, unions, etc.) and also
#                 to perform static evaluation of expressions.
#
# We need to support basic types for static evaluation, i.e. integer types that have different length
#

class BaseType:
    """
    This class is the common interface for any type. It implements type system's most
    fundamental functionality such as sizeof() operator.
    """
    def __init__(self):
        """
        Initialize the base type object
        
        :param length: Number of bytes this type occupies. Note that this is the real
                       storage requirement, and does not contain paddeds value
        """
        return

    def sizeof(self):
        """
        Returns the size of the type. This must be overridden to avoid exception
        :return: None
        """
        raise RuntimeError("Sizeof operator of a base type must be overridden")

class IntegerType:
    """
    This class represents integer types of arbitrary precision. The length of an integer
    type is an attribute of the 
    """
