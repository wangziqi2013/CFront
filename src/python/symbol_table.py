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

    def get_table(self, t):
        """
        Return a table given a type

        :param t: The type constant defined above
        :return: the table instance
        """
        assert(Scope.INDEX_BEGIN <= t <= Scope.INDEX_END)

        return self.symbols[t]

    def __getitem__(self, item):
        """
        Fetches an item from the scope's symbol table. The item
        is a tuple specifying the dict and the name

        :param item: Tuple(type, name)
        :return: Item stored in the table
        """
        t = self.get_table(item[0])
        return t[item[1]]

    def __contains__(self, item):
        """
        Same as __getitem__ except that it checks for membership

        :param item: Tuple(type, name)
        :return: bool
        """
        t = self.get_table(item[0])
        return item[1] in t

    def __setitem__(self, key, value):
        """
        Same as __getitem__ except that it sets a value with the
        given type and name

        :param key: Tuple(type, name)
        :param value: Any value
        :return: None
        """
        t = self.get_table(key[0])
        t[key[1]] = value
        return

    def get(self, key, ret):
        """
        This one mimics the behavior of dict.get() which returns
        the alternative value if the desired value does not exist

        :param key: Tuple(type, name)
        :param ret: Alternative value if the name does not exist
        :return: Any value
        """
        t = self.get_table(key[0])
        return t.get(key[1], ret)

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