#
# symbol_table.py - This file defines the symbol table
#                   for both types and identifiers
#

#####################################################################
# class Scope
#####################################################################

class Scope:
    """
    This class represents a scope. It includes a struct
    table that maps struct names to types; it also includes
    a union and typedef table that do the same. Finally it
    also has a identifier table which maps identifiers to
    their types
    """

    # The following constants defines the index of their
    # corresponding tables. Searching routine uses these
    # indices to access different tables rather than
    # implementing a separate routine for each table
    INDEX_BEGIN = 0
    STRUCT = 0
    UNION = 1
    TYPEDEF = 2
    IDENT = 3
    INDEX_END = 3

    def __init__(self):
        """
        Initialize all mapping structures
        """
        # We put them in a list such that we could
        # use an index to access them rather
        # than implement different routines for accessing
        # different tables
        self.symbols = [{}, {}, {}, {}]

        return

    def get_tab

#####################################################################
# class SymbolTable
#####################################################################

class SymbolTable:
    """
    This is the representation of a global symbol table
    which holds a stack of scopes. Each scope has its own
    symbol definitions. When we search names in the symbol
    table, we always start from the topmost scope and descend
    to the bottommost, which is the global scope.
    """
    def __init__(self):
        """
        Initialize the symbol table's stack
        """
        # This is the stack of scopes
        self.scope_stack = []