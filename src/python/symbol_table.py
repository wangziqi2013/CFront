#
# symbol_table.py - This file defines the symbol table
#                   for both types and identifiers
#

from debug import dbg_printf, Argv, DebugRunTestCaseBase

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
    TABLE_TYPE_INDEX_BEGIN = 0
    TABLE_TYPE_STRUCT = 0
    TABLE_TYPE_UNION = 1
    TABLE_TYPE_TYPEDEF = 2
    TABLE_TYPE_IDENT = 3
    TABLE_TYPE_INDEX_END = 3

    # The following defines the type of the scope
    SCOPE_TYPE_INDEX_BEGIN = 100
    # This is the global scope (top level scope)
    SCOPE_TYPE_GLOBAL = 100
    # Functional level
    SCOPE_TYPE_FUNCTION = 101
    # Local scope inside a function
    SCOPE_TYPE_LOCAL = 102
    # Inside a struct or union definition because name conflict
    # can still occur at this level
    SCOPE_TYPE_STRUCT = 103
    SCOPE_TYPE_INDEX_END = 103

    def __init__(self, scope_type):
        """
        Initialize all mapping structures
        
        :param scope_type: Enum constants defined above
        """
        # We put them in a list such that we could
        # use an index to access them rather
        # than implement different routines for accessing
        # different tables
        self.symbols = [{}, {}, {}, {}]

        # The scope type must be a valid one
        assert(self.SCOPE_TYPE_INDEX_BEGIN <=
               scope_type <=
               self.SCOPE_TYPE_INDEX_END)

        # Save the type of the scope for later inspection
        self.scope_type = scope_type

        return

    def get_table(self, t):
        """
        Return a table given a type

        :param t: The type constant defined above
        :return: the table instance
        """
        assert(Scope.INDEX_BEGIN <= t <= Scope.INDEX_END)

        return self.symbols[t]

    def get_type(self):
        """
        This function returns the type of the current scope
        :return: Scope type constant
        """
        return self.scope_type

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
        # By default there is a global scope at initialization
        # and the type is set as global scope type
        self.scope_stack = [Scope(Scope.SCOPE_TYPE_GLOBAL)]

        return

    def enter_scope(self, scope_type):
        """
        Enters a new scope by pushing a new scope object into the
        stack of tables

        :param scope_type: The type of the scope defined in class Scope
        :return: None
        """
        # Use the given type to define a new scope
        self.scope_stack.append(Scope(scope_type))

        return

    def leave_scope(self):
        """
        Leave the current scope by popping from the end of the list

        :return: None
        """
        assert(len(self.scope_stack) != 0)
        self.scope_stack.pop()
        return

    def get_current_scope_type(self):
        """
        This function returns the type of the current (i.e. topmost) scope
        
        :return: scope type constant
        """
        assert(len(self.scope_stack) != 0)

        return self.scope_stack[-1].get_type()

    def get_depth(self):
        """
        Get the current depth of the symbol table (i.e. the length of the list)
        Note that depth starts from 1
        
        :return: int 
        """
        return len(self.scope_stack)

    def get(self, key, ret):
        """
        Searches for a given name in the given type. If we could not
        find the name in all scopes then return the alternative

        :param key: Tuple(type, name)
        :param ret: Alternative value if name not found
        :return: Any object
        """
        i = len(self.scope_stack)
        while i >= 0:
            scope = self.scope_stack[i]
            # If the key exists then return the value
            # we do not need get() here since it is
            # guaranteed to exist
            if key in scope:
                return scope[key]

        # If we could not find the value in all scopes
        # then just return the alternative value
        return ret

    def __contains__(self, item):
        """
        Checks whether a value exists in the symbol table

        :param item: Tuple(type, name)
        :return: bool
        """
        i = len(self.scope_stack)
        while i >= 0:
            scope = self.scope_stack[i]
            if key in scope:
                return True

        return False

    def __getitem__(self, item):
        """
        Returns an item in all scopes if there is one. Note that
        for this function if the name is not defined for all
        scopes we need to assert False, and the caller should
        avoid that

        :param item: Tuple(type, name)
        :return: Any object
        """
        i = len(self.scope_stack)
        while i >= 0:
            scope = self.scope_stack[i]
            if key in scope:
                return scope[key]

        assert False

    def __setitem__(self, key, value):
        """
        This function sets the name in the topmost
        scope because that is how scope works

        :param key: Tuple(type, name)
        :param value: Any object
        :return: None
        """
        assert(len(self.scope_stack) != 0)
        # Use index = -1 to address the topmost scope
        self.scope_stack[-1][key] = value
        return

#####################################################################
# Unit test cases
#####################################################################

class ScopeTestCase(DebugRunTestCaseBase):
    """
    Unit tests for symbol table
    """
    def __init__(self):
        """
        This function calls the base class constructor
        """
        super(self.__class__, self).__init__()

        # This is required for running the test case
        argv = Argv()
        # This calls the base class method and hence runs the test case
        self.run_tests(argv)

        return

    @staticmethod
    @TestNode()
    def test_basic(argv, **kwargs):
        """
        This function tests whether basic symbol table works
        
        :param argv: Unused argv
        :param kwargs: Keyword arguments
        :return: None 
        """
        del argv
        del kwargs

        # Build a symbol table, and the type must be global
        st = SymbolTable()
        assert(st.get_current_scope_type() == Scope.SCOPE_TYPE_GLOBAL)

        return




