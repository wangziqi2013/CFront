#
# This file generates the C++ parser for a given grammar
#
# For simplicity purposes our parser is a top-down, predictive one
# that handles LL(1) grammar
#

from common import *

#####################################################################
# class Symbol
#####################################################################

class Symbol:
    """
    This class represents a grammar symbol that is either a terminal
    or non-terminal

    Each symbol has a name, which is the syntax notation we use in the
    grammar definition. For terminal symbols we always assume that the
    name is a macro defined in another CPP file, which is also used
    by the lex to produce the token stream; For terminals their names
    are only used internally by this generator and parser code
    """
    def __init__(self, name):
        """
        Initialize the symbol and its name
        """
        # This is either a terminal macro name or non-terminal name
        self.name = name

        return

#####################################################################
# class Terminal
#####################################################################

class Terminal(Symbol):
    """
    This class represents a terminal symbol object
    """
    def __init__(self, name):
        """
        Initialize the terminal object

        :param name: The name of the macro that defines the terminal
        """
        Symbol.__init__(self, name)

        return

#####################################################################
# class NonTerminal
#####################################################################

class NonTerminal(Symbol):
    """
    This class represents a non-terminal object in a grammar definition
    """
    def __init__(self, name):
        """
        Initialize non-terminal specific data members

        :param name: The name of the non-terminal
        """
        Symbol.__init__(self, name)

        # This is a set of production objects that this symbol
        # appears on the right hand side
        # This is for non-terminals, because we want to track
        # which non-terminal is in which production
        self.rhs_set = set()

        # This is the set of productions that this symbol appears
        # as the left hand side
        self.lhs_set = set()

        # These two are FIRST() and FOLLOW() described in
        # predictive parsers
        self.first_set = set()
        self.follow_set = set()

        return

class Production:
    """
    This class represents a single production rule that has a left
    hand side non-terminal symbol and
    """

class Generator:
    """
    This class is the main class we use to generate the parsing code
    for a given LL(1) syntax
    """
    def __init__(self):
        """
        Initialize the generator object
        """
        # This is a mapping from symbol name to either terminals
        # or non-terminals
        self.symbol_dict = {}

        # This is a mapping from name to terminals
        # All instances must be class Terminal object
        self.terminal_dict = {}

        # This is a mapping from name to non-termimals
        self.non_terminal_dict = {}

        return