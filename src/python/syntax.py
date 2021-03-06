#
# This file generates the C++ parser for a given grammar
#
# We provide four types of parser generators: LL(1), LR(1)
# SLR(1) and LALR(1), and an on-line Earley parser to experiment
# with
#

from common import *
import sys
import json
from lex import CTokenizer, Token
from ast import SyntaxNode

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

    # This is the name of the "eps" symbol
    EMPTY_SYMBOL_NAME = "T_"
    # This is the symbol name indicating the end of input stream
    END_SYMBOL_NAME = CTokenizer.EOF_TOKEN_NAME

    # This is the root of the artificial starting symbol
    # We need it for LR parsing; for LL it is not required, and the
    # root is always referred to as the actual root from the grammar
    ROOT_SYMBOL_NAME = "T__"

    # We must initialize this later because terminal object has
    # not yet been defined
    EMPTY_SYMBOL = None
    END_SYMBOL = None
    ROOT_SYMBOL = None

    @staticmethod
    def init_builtin_symbols():
        """
        This function initializes special built-in symbols after the
        class definition has been compiled. This should be called
        after the class definition and before any routine is called

        :return: None
        """
        Symbol.EMPTY_SYMBOL = Terminal(Symbol.EMPTY_SYMBOL_NAME)
        Symbol.END_SYMBOL = Terminal(Symbol.END_SYMBOL_NAME)
        Symbol.ROOT_SYMBOL = NonTerminal(Symbol.ROOT_SYMBOL_NAME)

        return

    def __init__(self, name):
        """
        Initialize the symbol and its name
        """
        # This is either a terminal macro name or non-terminal name
        self.name = name

        return

    def __hash__(self):
        """
        Hashes the object into a hash code. Note that we compute the
        hash using the name, so each name should be unique no matter
        whether it is terminal or non-terminal

        This is defined for making terminals and non-terminals hashable,
        such that we could use them in a hash or set structure

        :return: hash code
        """
        return hash(self.name)

    def __eq__(self, other):
        """
        Check whether two objects are the same

        :param other: The other object to compare against
        :return: bool
        """
        return self.name == other.name

    def __ne__(self, other):
        """
        Check whether two objects are not the same

        :param other: The other object to compare against
        :return: bool
        """
        return self.name != other.name

    def __lt__(self, other):
        """
        Check whether the current symbol has a name that is
        smaller in alphabetical order

        :param other: The other object
        :return: bool
        """
        return self.name < other.name

    def is_symbol(self):
        """
        Checks whether the item is either a terminal or a
        non-terminal

        :return: bool
        """
        return self.is_terminal() is True or \
               self.is_non_terminal() is True

    def is_terminal(self):
        """
        Whether the node is a terminal object. This function should not
        be overloaded by the derived class, because it checks the class
        type here directly

        :return: bool
        """
        return isinstance(self, Terminal)

    def is_non_terminal(self):
        """
        Whether the node is a non-terminal symbol. This is the counterpart
        of is_terminal()

        :return: bool
        """
        return isinstance(self, NonTerminal)

    def is_empty(self):
        """
        Whether the symbol node is empty terminal, i.e. "eps" in classical
        representations. "eps" is nothing more than a terminal of a special
        name "T_", which could either be shared or a newly created one.

        :return: bool
        """
        # Note that empty node must also be terminal node
        return self.is_terminal() and \
               self.name == Symbol.EMPTY_SYMBOL_NAME

    @staticmethod
    def get_empty_symbol():
        """
        Returns an empty symbol, which is a reference to the shared
        object stored within the class object. Please do not modify
        the result returned by this function as it will affect all
        objects

        Instead of calling this function users could also create their
        own instance of empty symbol using the name "T_". There is no
        difference between creating a new instance and using the shared
        instance (the latter saves memory, though)

        :return: Terminal
        """
        return Symbol.EMPTY_SYMBOL

    @staticmethod
    def get_end_symbol():
        """
        This function returns a reference to the constant END SYMBOL
        which is the "$" symbol in many classical text books

        :return: Terminal
        """
        return Symbol.END_SYMBOL

    @staticmethod
    def get_root_symbol():
        """
        This returns the root symbol which is used for LR parsing.
        Note that this symbol is a non-terminal

        :return: NonTerminal
        """
        return Symbol.ROOT_SYMBOL

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

    def __repr__(self):
        """
        Returns a string representation of the object that could
        be used as its identifier

        :return: str
        """
        return "[T %s]" % (self.name, )

    def __str__(self):
        """
        Returns a string representation

        :return: str
        """
        return self.__repr__()

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
        # They will be computed using recursion + memorization
        self.first_set = set()
        self.follow_set = set()

        # This is the set of all possible symbols if we expand the
        # non-terminal
        # We use this set to determine whether there are hidden
        # left recursions, i.e.
        #
        #  S -> V1 V2 V3
        #  V1 -> V4 V5
        #  V4 -> S V6
        self.first_rhs_set = None

        # This is used to generate name for a new node
        self.new_name_index = 1
        # This is for derived names, e.g. from left recursion elimination
        # or left factorization, because we want to keep the root node
        # for a derived node, in case we need to derive name from a
        # derived node
        self.name_derived_from = self

        # This is used to compute the FIRST and FOLLOW
        # Since we always do these two recursively and iteratively
        # for each iteration we mark the result to True after
        # results are computed, and clear the flag to enable recomputing
        # between different iterations
        self.result_available = False

        return

    def get_first_length(self):
        """
        This function returns the length of the FIRST set. If it is
        None then return 0

        :return: int
        """
        if self.first_set is None:
            return 0

        return len(self.first_set)

    def get_follow_length(self):
        """
        Same as get-first_length() except that it works on FOLLOW set

        :return: int
        """
        if self.follow_set is None:
            return 0

        return len(self.follow_set)

    def clear_result_available(self):
        """
        This function clears the result available flag

        :return: None
        """
        self.result_available = False

        return

    def compute_first(self, path_list):
        """
        This function computes the FIRST set for the non-terminal

        We use a recursive algorithm to compute the first set. This
        process is similar to the one we use to compute the first
        RHS set, but in this case since we care about not only
        the formation but also semantic meaning of production rules,
        we also need to consider the EMPTY SYMBOL

        :return: None
        """
        # If we have already processed this node then return
        if self.result_available is True:
            return
        else:
            self.result_available = True

        # This even works for left-recursive grammar
        # because we may also use this for LR
        if self in path_list:
            return

        path_list.append(self)

        # For all productions A -> B1 B2 .. Bi
        # FIRST(A) is defined as FIRST(B1) union FIRST(Bj)
        # where B1 - Bj - 1 could derive terminal and
        # Bj could not
        for p in self.lhs_set:
            for symbol in p.rhs_list:
                # For terminals just add it and process
                # the next production
                if symbol.is_terminal() is True:
                    # Empty symbol is also added here if it
                    # is derived
                    self.first_set.add(symbol)

                    # Also add it into the production
                    p.first_set.add(symbol)

                    break

                # Recursively compute the FIRST set
                # Since we have processed the symbol == self case here we
                # could call this without checking
                symbol.compute_first(path_list)
                self.first_set = \
                    self.first_set.union(symbol.first_set)

                # This could contain empty symbol
                p.first_set = \
                    p.first_set.union(symbol.first_set)

                # If the empty symbol could not be derived then
                # we do not check the following non-terminals
                if Symbol.get_empty_symbol() not in symbol.first_set:
                    break
            else:
                # This is executed if all symbols are non-terminal
                # and they could all derive empty string
                p.first_set.add(Symbol.get_empty_symbol())
                self.first_set.add(Symbol.get_empty_symbol())

        path_list.pop()

        return

    def compute_follow(self, path_list):
        """
        This functions computes the FOLLOW set. Note that the FOLLOW set
        is also computed recursively, and therefore we use memorization
        to compute it

        Note that this should be run for multiple rounds for correctness
        until no symbol could be added

        :return: None
        """
        # If we are seeking for this node's FOLLOW set and then recursively
        # reached the same node then there must by cyclic grammar:
        #   A -> a B
        #   B -> b C
        #   C -> c A
        # In this case if we compute FOLLOW(A) then we will compute FOLLOW(C)
        # and FOLLOW(B) which comes back to FOLLOW(A)
        #
        # However this structure is very common in left recursion removal:
        #   A -> A a | b
        #   ---
        #   A  -> b A'
        #   A' -> eps | a A'
        # When we compute FOLLOW(A') it is inevitable that this will happen

        # This is how memorization works
        if self.result_available is True:
            return
        else:
            self.result_available = True

        if self in path_list:
            return

        path_list.append(self)

        # For all productions where this terminal appears as a symbol
        for p in self.rhs_set:
            # We allow one symbol to appear in a production for multiple
            # times because that is how for() loop is defined
            index_list = p.get_symbol_index(self)

            for index in index_list:
                # If the symbol appears as the last one in the production
                if index == (len(p.rhs_list) - 1):
                    # This could be a self recursion but we have prevented this
                    # at the beginning of this function
                    p.lhs.compute_follow(path_list)

                    self.follow_set = \
                        self.follow_set.union(p.lhs.follow_set)
                else:
                    # Compute the FIRST set for the substring after the
                    # terminal symbol
                    substr_first_set = p.compute_substring_first(index + 1)

                    # If the string after the non-terminal could be
                    # empty then we also need to add the FOLLOW of the LHS
                    if Symbol.get_empty_symbol() in substr_first_set:
                        p.lhs.compute_follow(path_list)
                        self.follow_set = \
                            self.follow_set.union(p.lhs.follow_set)

                        # Remove the empty symbol because empty could not
                        # appear in FOLLOW set
                        substr_first_set.remove(Symbol.get_empty_symbol())

                    # At last, merge the FIRST() without empty symbol
                    # into the current FOLLOW set
                    self.follow_set = \
                        self.follow_set.union(substr_first_set)

        # Do not forget to remove this in the path set (we know
        # it does not exist before entering this function)
        path_list.pop()

        return

    def get_new_symbol(self):
        """
        This function returns a new non-terminal symbol whose
        name is derived from the name of the current one.

        The new symbol could be used in left recursion elimination
        routines and other transformation. It is of the form:
          original-name + "-" + new name index
        and the index is incremented every time we created a new
        name

        Note that this works even on artificial nodes, because
        each artificial node contains a reference back to the original
        non-artificial node from which its name derives. In
        this case if we call this function on an artificial node,
        it will go directly to the non-artificial node and use
        the name and counter there, i.e. name allocation is centralized
        to non-artificial nodes

        :return: NonTerminal object with a synthesized name
        """
        # We always use the counter in the non-artificial node
        derived_from_node = self.name_derived_from

        # Synthesize a new name
        new_name = derived_from_node.name + \
                   "-" + \
                   str(derived_from_node.new_name_index)

        derived_from_node.new_name_index += 1
        nt = NonTerminal(new_name)

        # Also link it to the root node
        nt.name_derived_from = derived_from_node

        return nt

    def left_factorize(self):
        """
        This function performs left factorization on the LHS
        non-terminal

        The way we perform LF is to build a set of first RHSs
        for all productions for this LHS, if the size of the
        set is smaller than the number of productions then
        we know there are common prefixes

        :return: True if LF is detected and fixed
        """
        # This is the dict we use to group objects
        d = {}
        # This is used to decide whether there are common
        # prefixes
        common_prefix = False
        for p in self.lhs_set:
            assert(len(p.rhs_list) > 0)
            first_rhs = p.rhs_list[0]

            # If the RHS has not yet been added
            # just add it as a list
            if first_rhs not in d:
                d[first_rhs] = [p]
            else:
                # Otherwise just add it into the list
                # and we have found a common prefix
                d[first_rhs].append(p)
                common_prefix = True

        # The number of unique first RHS symbols
        # equals the number of productions
        if common_prefix is False:
            return False

        count = 0
        # Then iterate over all pairs, and skip those
        # with only one element in the list of productions
        for key, value in d.items():
            if len(value) == 1:
                continue
            elif key == self:
                # This should be processed by left recursion
                continue

            pg = None
            # This will create a new artificial name
            new_nt = self.get_new_symbol()
            for p in value:
                # Verify that pgs are identical
                if pg is None:
                    pg = p.pg
                else:
                    assert(id(pg) == id(p.pg))

                # Clear the references for the production
                p.clear()
                # Add new rule: LHS-1 -> RHS[1:]
                # Special case for RHS length being 1 because in this case
                # we just add empty string
                if len(p.rhs_list) == 1:
                    Production(p.pg,
                               new_nt,
                               [Symbol.get_empty_symbol()])
                else:
                    Production(p.pg, new_nt, p.rhs_list[1:])

            # Also link the current symbol with the artificial one
            Production(pg, self, [key, new_nt])
            # Also add the new symbol into terminal set
            pg.non_terminal_set.add(new_nt)

            count += 1

        # Because still we did not process anything
        if count == 0:
            return False

        return True

    def eliminate_left_recursion(self):
        """
        This function eliminates direct left recursion for the
        current non-terminal symbol. We do not deal with
        indirect ones here

        :return: True if there is eliminated left recursion
        """
        # If there is no direct left recursion then return
        # directly
        if self.exists_direct_left_recursion() is False:
            return False

        # Make a backup here because LHS set will be cleared
        # A shallow copy is sufficient here because we just use
        # LHS and RHS references
        p_set = self.lhs_set.copy()

        # This is the set of productions with left recursion
        alpha_set = set()
        # This is the set of productions without left recursion
        beta_set = set()

        pg = None
        # Clear all productions - Note here we could not iterate on
        # self.lhs because it will be changed in the iteration body
        for p in p_set:
            # This removes the production from this object
            # and also removes it from other RHS objects
            p.clear()

            # If this is a left recursion production then add
            # it to alpha set; otherwise to beta set
            if p[0] == self:
                alpha_set.add(p)
            else:
                beta_set.add(p)

            # Check they all come from the same pg instance
            if pg is None:
                pg = p.pg
            else:
                assert(id(pg) == id(p.pg))

        # It must have been cleared
        assert(len(self.lhs_set) == 0)
        assert(len(alpha_set) + len(beta_set) == len(p_set))

        # Create a new symbol and add it into the set
        new_symbol = self.get_new_symbol()
        pg.non_terminal_set.add(new_symbol)

        empty_symbol = Symbol.get_empty_symbol()
        # Also add this into pg's terminal set
        pg.terminal_set.add(empty_symbol)

        # The scheme goes as follows:
        #   For a left recursion like this:
        #     A -> A a1 | A a2 | .. A ai | b1 | b2 | .. | bj
        #   We create a new symbol A' (as above), and add
        #     A  -> b1 A' | b2 A' | ... | bj A'
        #     A' -> a1 A' | a2 A' | ... | ai A' | eps
        #     where eps is the empty symbol
        #
        # There is one exception: if bj is empty string then we
        # just ignore it and add A -> A' instead

        for beta in beta_set:
            # Special case:
            # # If it is "A -> A a1 | A a2 | eps | B1 | B2" then
            # we make it
            #   A -> A' | B1 A' | B2 A'
            #   A' -> a1 A' | a2 A' | eps
            # As long as A', B1 B2 does not have common FIRST() element
            # we are still safe
            if len(beta.rhs_list) == 1 and \
               beta.rhs_list[0] == Symbol.get_empty_symbol():
                rhs_list = []
            else:
                # Make a copy to avoid directly modifying the list
                rhs_list = beta.rhs_list[:]

            rhs_list.append(new_symbol)

            # Add production: A -> bj A'
            Production(pg, self, rhs_list)

        for alpha in alpha_set:
            # Make a slice (which is internally a shallow copy)
            rhs_list = alpha.rhs_list[1:]
            rhs_list.append(new_symbol)

            # Add production: A -> bj A'
            Production(pg, new_symbol, rhs_list)

        # Add the last A' -> eps
        Production(pg, new_symbol, [empty_symbol])

        return True

    def exists_indirect_left_recursion(self):
        """
        This function checks whether there is indirect left recursion
        in the production rules, e.g.

          S -> V1 V2 V3
          V1 -> S V4

        The way we check indirect left recursions is to construct
        a set of all possible left non-terminals in all possible
        derivations of the LHS of a production. This could be done
        recursively, but we also adopt memorization to reduce the
        number of step required from exponential to linear

        :return: bool
        """
        self.build_first_rhs_set()

        # If the node itself is in the first RHS set then
        # we know there is an indirect left recursion
        return self in self.first_rhs_set

    def build_first_rhs_set(self):
        """
        Builds first RHS set as specified in the definition of the
        set. We do this recursively using memorization, i.e. the
        result is saved in self.first_rhs_set and used later

        :return: None
        """
        # If we have visited this symbol before then just return
        # Since the set object is initialized to None on construction
        # this check guarantees we do not revisit symbols
        if self.first_rhs_set is not None:
            return
        else:
            # Initialize it to empty set
            self.first_rhs_set = set()

        # For all productions with this symbol as LHS, recursively
        # add all first RHS symbol into the set of this symbol
        for p in self.lhs_set:
            # The production must have RHS side
            assert(len(p) != 0)
            s = p[0]
            if s.is_terminal() is True:
                continue

            # Recursively build the set for the first RHS
            s.build_first_rhs_set()

            # Then merge these two sets
            self.first_rhs_set = \
                self.first_rhs_set.union(s.first_rhs_set)

            # Also add the first direct RHS into the set
            self.first_rhs_set.add(s)

        return

    def exists_direct_left_recursion(self):
        """
        This function checks whether there is direct left recursion,
        i.e. for non-terminal A checks whether A -> A b exists

        This is a non-recursive process, and we just need to make sure
        that no production for this symbol could derive itself

        :return: bool
        """
        # For each production that this symbol is the LHS
        for p in self.lhs_set:
            # The production must not be an empty one
            # i.e. there must be something on the RHS
            assert(len(p) != 0)
            # Since we have defined operator== for class Symbol
            # we could directly compare two symbol
            if p[0] == self:
                return True

        return False

    def __repr__(self):
        """
        Returns a string representation of the object that could
        be used as its identifier

        :return: str
        """
        return "[NT %s]" % (self.name, )

    def __str__(self):
        """
        Returns a string representation

        :return: str
        """
        return self.__repr__()

#####################################################################
# Static Initialization
#####################################################################

# This function initializes built-in symbols inside the static
# member
Symbol.init_builtin_symbols()

#####################################################################
# class Production
#####################################################################

class Production:
    """
    This class represents a single production rule that has a left
    hand side non-terminal symbol and
    """
    def __init__(self, pg, lhs, rhs_list):
        """
        Initialize the production object

        :param pg: The ParserGenerator object
        :param lhs: Left hand side symbol name (string name)
        :param rhs_list: A list of right hand side symbol names
        """
        # Make sure that the lhs is a non terminal symbol
        assert(lhs.is_non_terminal() is True)

        self.pg = pg

        # This is the first set of the production which we use to
        # select rules for the same LHS
        self.first_set = set()

        # Since we defined __setattr__() to prevent setting
        # these two names, we need to set them directly into
        # the underlying dict object
        self.__dict__["lhs"] = lhs
        # We append elements into this list later
        self.__dict__["rhs_list"] = rhs_list

        # Only after this point could we add the production
        # into any set, because the production becomes
        # immutable regarding its identify (i.e. LHS and
        # RHS list)

        # Add a reference to all non-terminal RHS nodes
        for symbol in self.rhs_list:
            assert(symbol.is_symbol() is True)
            if symbol.is_non_terminal() is True:
                # We could add self in this way, because
                # the identify of the set has been fixed
                symbol.rhs_set.add(self)

        # Also add a reference of this production into the LHS
        # set of the non-terminal
        # Note that since we add it into a set, the identity
        # of the production can no longer be changed
        lhs.lhs_set.add(self)

        # Make sure the user does not input duplicated
        # productions
        if self in self.pg.production_set:
            raise KeyError("Production already defined: %s" %
                           (str(self), ))

        # Finally add itself into the production set of the
        # containing pg object
        self.pg.production_set.add(self)

        # This is the table for holding the FIRST set for
        # all prefixes of the production
        # We need this to be computed very fast because LR(1)
        # items heavily rely on it
        self.substring_first_set_list = []

        # This will be set to a tuple if AST rule is given
        # in the syntax definition
        self.ast_rule = None

        return

    def size(self):
        """
        Returns the length of the RHS of this production

        The only exception is that if the production is an empty
        production we return 0 instead of 1

        :return: int
        """
        if self.rhs_list[0] == Symbol.get_empty_symbol():
            return 0

        return len(self.rhs_list)

    def prepare_substring_first(self):
        """
        This function prepares all possible substring FIRST
        set in a table for later use.

        :return: None
        """
        for i in range(0, len(self.rhs_list)):
            self.substring_first_set_list.append(
                self._compute_substring_first(i)
            )

        return

    def get_symbol_index(self, symbol):
        """
        Returns a list of indices a symbol appear in the RHS list

        Note that the symbol could be either terminal or non-terminal
        and we do not check its identify. If the symbol does not
        exist we return an empty list

        :return: list(int)
        """
        # First of all it must be a symbol
        assert(symbol.is_symbol() is True)

        ret = []
        # This tracks the index of the current item
        index = 0
        for s in self.rhs_list:
            if s == symbol:
                ret.append(index)

            index += 1

        return ret

    def compute_substring_first(self, index=0):
        """
        Fast retrieves the substring FIRST set from the production

        :param index: The beginning index
        :return: None
        """
        assert(index < len(self.rhs_list))

        return self.substring_first_set_list[index]

    def add_substring_first(self, index, s):
        """
        Add substring FIRST set into a given set

        :param index: The index of the starting point
        :param s: The set we add symbols to
        :return: None
        """
        assert (index < len(self.rhs_list))

        # Add all symbols
        for symbol in self.substring_first_set_list[index]:
            s.add(symbol)

        return

    def _compute_substring_first(self, index):
        """
        This function computes the FIRST set for a substring. An optional
        index field is also provided to allow the user to start
        computing from a certain starting index

        Note that the FIRST set of the entire production has already been
        computed in compute_first() of a terminal symbol. The algorithm
        there is very similar to the one used here. Nevertheless, the
        result of passing index = 0 and the result computed by the
        non-terminal should be the same

        This function should not be called directly, but rather it is
        used by the initialization routine to populate the internal
        FIRST set table

        :return: set(Terminal)
        """
        assert(index < len(self.rhs_list))

        ret = set()
        for i in range(index, len(self.rhs_list)):
            rhs = self.rhs_list[i]

            # If we have seen a terminal, then add it to the list
            # and then return
            if rhs.is_terminal() is True:
                ret.add(rhs)
                return ret
            else:
                # This makes sure that the first set must already been
                # generated before calling this function
                assert(len(rhs.first_set) > 0)

                # Otherwise just union with the non-terminal's
                # first_set
                ret = ret.union(rhs.first_set)
                # Remove potential empty symbol
                ret.discard(Symbol.get_empty_symbol())

                # If the non-terminal could not derive empty then
                # that's it
                if Symbol.get_empty_symbol() not in rhs.first_set:
                    return ret

        # When we get to here we know that all non-terminals could
        # derive to empty string, and there is no terminal in the
        # sequence, so need also to add empty symbol
        ret.add(Symbol.get_empty_symbol())

        return ret

    def clear(self):
        """
        Clears all references from the production to non-terminal
        objects

        :return: None
        """
        # Remove this production from the LHS's LHS set
        # This happens in-place
        self.lhs.lhs_set.remove(self)
        for symbol in self.rhs_list:
            # If it is a non-terminal then we remove
            # the production from its rhs set
            if symbol.is_non_terminal() is True:
                symbol.rhs_set.remove(self)

        # Then clear itself from the generator's
        self.pg.production_set.remove(self)

        return

    def __setattr__(self, key, value):
        """
        Controls attribute access of this object because we do not allow
        accessing LHS and RHS list of this class directly

        Note that unlink getattr(), this is always called on attribute
        access.

        :param key: The attribute name
        :param value: The attribute value
        :return: None
        """
        if key == "lhs" or key == "rhs_list":
            raise KeyError("Cannot set key %s for class Production" %
                           (key, ))

        # Otherwise directly set the attribute into dict
        self.__dict__[key] = value

        return

    def __getitem__(self, item):
        """
        This mimics the list syntax

        :param item: The index
        :return: The i-th object in the rhs list
        """
        return self.rhs_list[item]

    def __hash__(self):
        """
        This function computes the hash for production object.

        The hash of a production object is defined by each of its
        components: lhs and every symbol in RHS list. We combine
        the hash code of each component using XOR and return

        :return: hash code
        """
        # This computes the hash of the lhs name
        h = hash(self.lhs)

        # Then combine the hash by XOR the hash of each RHS symbol
        for rhs in self.rhs_list:
            h ^= hash(rhs)

        return h

    def __eq__(self, other):
        """
        This function checks whether this production equals another

        We check equality by comparing each component, including LHS
        and each member of the RHS list. This is similar to how
        we compute the hash code for this class

        :param other: The other object
        :return: bool
        """
        # If the length differs then we know they will never
        # be the same. We do this before any string comparison
        if len(self.rhs_list) != len(other.rhs_list):
            return False

        if self.lhs != other.lhs:
            return False

        # Use index to fetch component
        for rhs1, rhs2 in zip(self.rhs_list, other.rhs_list):
            # If there is none inequality then just return
            if rhs1.__eq__(rhs2) is False:
                return False

        return True

    def __ne__(self, other):
        """
        This is the reverse of __eq__()

        :param other: The other object
        :return: bool
        """
        return not self.__eq__(other)

    def __repr__(self):
        """
        Returns a unique representation of the object using
        a string

        :return: str
        """
        s = "[" + self.lhs.name + ' ->'
        for rhs in self.rhs_list:
            s += (' ' + rhs.name)

        return s + ']'

    def __str__(self):
        """
        This function returns the string for printing this object

        :return: str
        """
        return self.__repr__()

    def __len__(self):
        """
        Returns the length of the RHS list

        :return: int
        """
        return len(self.rhs_list)

#####################################################################
# class LRItem
#####################################################################

class LRItem:
    """
    This class represents a LR item, i.e. a production with a position
    indicating the current parsing state of the production

    This class is hashable, and printable. Note also that this class
    does not pertain any state, so we could create arbitrarily many of
    them with the same content
    """
    def __init__(self, p, index):
        """
        Initialize the object with a production object and a position
        as the index into the production

        Note that position i means that the dot is before the i-th
        RHS element of the production, e.g. for S -> A B C ; 2 it
        represents an item:
           S -> A B [dot] C
        Note that it is also allowed for the dot to appear after all
        symbols in the RHS list:
           S -> A B C [dot]
        and in this special case the

        Once p and index are initialized further alternation of these
        two members are prohibited
        """
        self.__dict__["p"] = p
        self.__dict__["index"] = index

        # Whether p is an empty production, because for empty
        # production it is always reduce-able
        self.is_empty_production = False

        # This will be set to True once the closure is computed
        self.closure_computed = False

        # If it is after any symbol then index == len..
        assert(index <= len(p.rhs_list))
        assert(len(p.rhs_list) > 0)

        # If this is an empty production then we also
        # restrict the index to be only 0
        # because empty string does not conceptually take
        # terminals and therefore could not have two separate
        # states
        if p.rhs_list[0] == Symbol.get_empty_symbol():
            assert(index == 0)
            assert(len(p.rhs_list) == 1)
            self.is_empty_production = True

        return

    def advance(self):
        """
        Returns a new item with index ++. This is particularly
        useful for unifying the SLR and LR(1) algorithm under
        the same framework

        :return: LRItem object
        """
        # There must be at least a symbol
        assert(self.get_dotted_symbol() is not None)

        return LRItem(self.p, self.index + 1)

    def could_reduce(self):
        """
        Whether the dot is after all symbols. The special case is for
        empty production: S -> T_ where even if index == 0 for this
        case we still return True

        Empty production always could reduce

        :return: bool
        """
        # Empty production could always reduce
        if self.is_empty_production is True:
            return True
        elif self.index == len(self.p.rhs_list):
            # If the dot is after all symbols then
            # we could also reduce
            return True

        return False

    def get_reduce_symbol(self):
        """
        Return the LHS symbol of the production if this item could
        be reduced. If not assertion fails

        :return: NonTerminal
        """
        assert(self.could_reduce() is True)

        return self.p.lhs

    def get_dotted_symbol(self):
        """
        This function returns the symbol that the dot is on. If
        the dot is after all symbols then return None

        Note that this function may return Terminals and NonTerminals
        and the caller is responsible for filtering out unnecessary
        ones

        For empty string T_ we always return None because it does not
        have any symbol

        :return: Terminal/NonTerminal/None
        """
        # The dot is after all symbols
        if self.could_reduce() is True:
            return None

        return self.p.rhs_list[self.index]

    def compute_closure(self, item_set):
        """
        Computes the closure of the item. The closure for an item is
        defined as returning a set of items for items RHS[index]'s
        production with index = 0. Note that this is not the closure
        for an item set.

        :return: None
        """
        # We must guarantee that this is only called once
        assert(self.closure_computed is False)
        self.closure_computed = True

        symbol = self.get_dotted_symbol()
        if symbol is None:
            return
        elif symbol.is_terminal() is True:
            # Note that the empty symbol is also
            # included
            return

        # Then add every production into the set and return
        for p in symbol.lhs_set:
            item = LRItem(p, 0)
            item_set.add(item)

        return

    def __setattr__(self, key, value):
        """
        Customized definition of setattr(). We need this because we want
        to prevent alternation of p and index fields

        :param key: The attribute name
        :param value: The attribute value
        :return: None
        """
        if key == "p" or key == "index":
            raise KeyError("Could not set read-only attribute %s" %
                           (key, ))

        self.__dict__[key] = value

        return

    def __hash__(self):
        """
        Returns the hash code of the item. We compute hash code using
        the hash of the production XOR-ed by the hash of the index

        :return: hash code
        """
        return id(self.p) ^ self.index

    def __eq__(self, other):
        """
        Checks equality of two item objects

        :param other: The other LRItem object
        :return: bool
        """
        return id(self.p) == id(other.p) and \
               self.index == other.index

    def __ne__(self, other):
        """
        Checks whether this object does not equal another object

        :param other: The other LRItem object
        :return: bool
        """
        return not self.__eq__(other)

    def __repr__(self):
        """
        Returns a string for identifying this object

        :return: str
        """
        return "[%s, %d]" % (str(self.p), self.index)

    def __str__(self):
        """
        Returns a string for printing

        :return: str
        """
        return self.__repr__()

#####################################################################
# class LR1Item
#####################################################################

class LR1Item(LRItem):
    """
    This class represents an LR(1) item, which is an extension based
    on LR(0) item, plus a lookahead set
    """
    def __init__(self, p, index, lookahead_set):
        """
        Initializes the LR(1) item

        In addition to initializing the underlying LR(0) item,
        we also compute the FOLLOW set which exists in LR(1) but
        not LR(0)

        Note that the underlying LR(0) item should be immutable
        after initialization. We should enforce this in LR(1) item
        and do not modify either index or p

        The lookahead set is the set of applicable symbols for the
        LHS of the production. Therefore, no matter how the index
        changes within an item, the lookahead set always does not
        change.
        """
        LRItem.__init__(self, p, index)

        self.lookahead_set = lookahead_set

        return

    def advance(self):
        """
        Go to the next symbol. This function returns a new LR1Item
        object with lookahead set unchanged (so no need to make
        a new set because it will be copied in closure function).

        :return: LR1Item object
        """
        assert(self.get_dotted_symbol() is not None)

        return LR1Item(self.p,
                       self.index + 1,
                       self.lookahead_set)

    def compute_closure(self, item_set):
        """
        This function computes the LR(0) closure with LR(1)
        lookahead. If we derive a new LR1Item using the following
        production rule:
            A -> B [dot] C D
        then the item is all productions with C as LHS, and the
        lookahead is FIRST(D). If FIRST(D) contains empty symbol
        we also add the lookahead of the current item into the
        lookahead set
        
        Note that this is only one iteration in the computation of the 
        closure. The caller of this method must repeat the process on
        the item set for several rounds until no more items can be added

        :param item_set: We add all computed items in this iteration into
                         this set
        :return: None
        """
        # To avoid doing this again and again
        assert (self.closure_computed is False)
        self.closure_computed = True

        symbol = self.get_dotted_symbol()
        if symbol is None:
            return
        elif symbol.is_terminal() is True:
            # Note that the empty symbol is also
            # included
            return

        # We always create a new lookahead set for the new item
        # object to avoid complicated bugs
        if self.index + 1 == len(self.p.rhs_list):
            lookahead_set = self.lookahead_set
        else:
            # Get the FIRST set of the substring after the
            # dot symbol
            lookahead_set = \
                self.p.compute_substring_first(self.index + 1)
            if Symbol.get_empty_symbol() in lookahead_set:
                lookahead_set = lookahead_set.copy()
                lookahead_set.remove(Symbol.get_empty_symbol())
                lookahead_set = \
                    lookahead_set.union(self.lookahead_set)

        # Then add every production into the set and return
        for p in symbol.lhs_set:
            item = LR1Item(p, 0, lookahead_set)
            item_set.add(item)

        return

    def __eq__(self, other):
        """
        Compares two LR(1) items

        :param other: The other LR(1) item
        :return: None
        """
        # First we compare the LR(0) part, and then compare
        # LR(1) part
        return LRItem.__eq__(self, other) and \
               self.lookahead_set == other.lookahead_set

    def __hash__(self):
        """
        Hashes the LR(1) item into a hash code

        :param other: The other LR(1) item
        :return: hash code
        """
        # This is special because the hash
        # of the lookahead symbol set is not easy to
        # compute, so we just use the hash of the underlying
        # LR(0) item
        return LRItem.__hash__(self)

    def __repr__(self):
        """
        Represents the item as a string

        :return: str
        """
        return "[%s, %d, %s]" % (self.p, self.index, self.lookahead_set)

    def __str__(self):
        """
        Returns a string representation

        :return: str
        """
        return self.__repr__()

#####################################################################
# class ItemSet
#####################################################################

class ItemSet:
    """
    This class represents a set of LRItems with additional
    functionality such as computing the closure
    """
    def __init__(self):
        """
        Initialize an empty item set

        Note that after being sealed the item set could not change
        such that the hash and identity remains the same
        """
        # This is the set of items
        self.item_set = set()

        # This maps go to with terminals and non-terminals
        # to the next state
        self.goto_table = {}

        # The union of below two sets is the goto table's key,
        # either Terminal or NonTerminal
        # Note that empty string could not appear in this set
        self.goto_symbol_set = set()

        # This will be set to True if compute_goto() is called
        self.has_goto_flag = False

        # This is the index used for parsing
        # we do not set this member in this class, and instead
        # fix its index after the canonical set has been
        # computed
        self.index = None

        return

    def merge_with(self, other):
        """
        Merges items with items in other.

        This function should only be called if the current
        item set holds LR(1) items, otherwise lookahead is not
        in the items.

        This function changes the current object in-place. We assume
        that both self and other must have the same LR(0) items. We
        simply do a loop to find the corresponding LR(0) item in the 
        other item set for each item in the current item set.

        :param other: Another ItemSet object
        :return: None
        """
        for i in self.item_set:
            # This is the LR(0) equivalence of the destination
            # item (i.e. current item)
            dest_item = LRItem(i.p, i.index)
            # Just use a loop to find a equivalence in LR(0)
            # in the other item set
            for j in other.item_set:
                src_item = LRItem(j.p, j.index)
                # If they have the same production and the same
                # position then just merge them into the dest_item
                if src_item == dest_item:
                    i.lookahead_set = \
                        i.lookahead_set.union(j.lookahead_set)
                    break

        return

    def has_goto_table(self):
        """
        Whether the root table has already been computed
        If this returns true then we do not try to compute its
        GOTO table and also do not add its GOTO target into the
        set

        :return: bool
        """
        return self.has_goto_flag

    def compute_goto(self, identity_dict):
        """
        Computes the GOTO sets of the current item set

        This class maintains a mapping between the symbols that
        could lead to a GOTO (either terminal and non-terminal)
        We also store the set of symbols that could lead to a GOTO
        action in this class for membership testing.

        For newly added ItemSet objects inside goto_table, we always
        compute closure for them such that they could be added
        into a higher level set after this function returns. However
        we do not recursively compute its goto_table since
        there may be an infinite recursion computation.
        
        Note that the GOTO table actually holds transitions for both
        terminals (which is actually a "shift") and non-terminals (
        which is a real "GOTO" in LR terminology). We distinguish between
        them only in parsing table generation phase. Here they are treated
        equally.

        :return: None
        """
        assert(self.has_goto_table() is False)

        for item in self.item_set:
            # If the item could reduce then there is no
            # next symbol
            if item.could_reduce() is True:
                continue

            # Then get the next symbol after the dot
            # and add it into the goto set because this
            # symbol could be used to compute the GOTO
            # item set
            symbol = item.get_dotted_symbol()
            self.goto_symbol_set.add(symbol)

        # Then for every symbol that could lead to a new set
        # we just add it
        for symbol in self.goto_symbol_set:
            # Create a new empty item set and we add items
            # later
            new_item_set = ItemSet()

            for item in self.item_set:
                dotted_symbol = item.get_dotted_symbol()
                # Skip it if we have seen None
                if dotted_symbol is None:
                    continue

                # If this item could be used to GOTO
                if dotted_symbol == symbol:
                    # Then move the dot to the next position
                    # which is guaranteed to be valid
                    # Note that here we could not hard code the
                    # LR item type being used here because it
                    # might as well be an LR1Item
                    new_item = item.advance() #LRItem(item.p, item.index + 1)
                    # Add this into the new item set
                    new_item_set.item_set.add(new_item)

            # This will add other items into the set
            # until no more could be added - add items
            # are newly created
            new_item_set.compute_closure()

            # When computing the closure there may be many items with different
            # lookahead but the same core item (i.e. production + index) added. This
            # way we need to merge these items in a sense that all items with the same
            # core item should be merged into one
            core_item_dict = {}
            merged = False
            for item in new_item_set.item_set:
                # Do not do the following computation
                # for LR(0) items
                if isinstance(item, LR1Item) is False:
                    break

                key = LRItem(item.p, item.index)
                if key not in core_item_dict:
                    core_item_dict[key] = item
                else:
                    merged = True
                    value = core_item_dict[key]
                    # We never modify it in-place, so just fork a
                    # new instance
                    value.lookahead_set = \
                        value.lookahead_set.union(
                            item.lookahead_set)

            # If we did merged items them we need to change
            # the item set because the hash value has changed
            if merged is True:
                new_item_set.item_set = \
                    set(core_item_dict.values())

            # And then put this into the goto table
            # such that we could know how many possible
            # states this state could transit to
            if new_item_set in identity_dict:
                self.goto_table[symbol] = \
                    identity_dict[new_item_set]
            else:
                self.goto_table[symbol] = new_item_set
                identity_dict[new_item_set] = new_item_set

        # Make this instance as having the goto table
        self.has_goto_flag = True

        return

    def compute_closure(self):
        """
        Computes the closure for this set. This function modifies
        self.item_set and therefore must be called before this is
        added to any set

        :return: None
        """
        while True:
            # This is the count of items before the current
            # iteration. If this does not change after the
            # iteration then we know we have had a closure
            prev_count = len(self.item_set)

            # Must make a list first because the set will be changed
            # during the iteration
            item_list = list(self.item_set)

            # Iterator on the item list
            for item in item_list:
                # That is the reason why item sets MUST NOT share item objects
                if item.closure_computed is True:
                    continue

                # Pass the item set into the closure function
                # for adding new items without creating new
                # data structure
                item.compute_closure(self.item_set)

            #print(len(self.item_set))
            # If the size of the set does not change
            # then we have reached a stable state
            if len(self.item_set) == prev_count:
                break

        return

    def __hash__(self):
        """
        Compute the hash code of the set.

        The hash code is defined as the XOR of the hash of all items
        in the item_set

        :return: hash code
        """
        # Since hash itself is integer type
        h = 0
        for item in self.item_set:
            h ^= hash(item)

        return h

    def __eq__(self, other):
        """
        Checks whether two item sets are equal

        This is very simple - we just check whether the two sets
        are equal. Since python already defines equality checking
        for sets (by testing membership for each over another)
        we do not implement our own

        :param other: The other object
        :return: bool
        """
        return self.item_set == other.item_set

#####################################################################
# class ParserGenerator
#####################################################################

class ParserGenerator:
    """
    This class represents the abstract parser generator which provides
    common functionality for supporting either LL or LR grammar
    """
    def __init__(self, file_name):
        """
        Open a grammar file and loads its content as terminals, non-terminals
        and productions

        This function loads the file and processes symbols and productions
        so child class do not need to call read_file again

        :param file_name: The name of the grammar file we want to load
        """
        # This is the name of the syntax definition file
        self.file_name = file_name

        # This is a mapping from symbol name to either terminals
        # or non-terminals
        self.symbol_dict = {}

        # This is a set of terminal objects
        self.terminal_set = set()

        # This is a set of non-terminal objects
        self.non_terminal_set = set()

        # This is a set of productions
        self.production_set = set()

        # This is the non-terminal where parsing starts
        self.root_symbol = None

        # Read file and process symbols and productions
        self.read_file(file_name)

        return

    def read_file(self, file_name):
        """
        Read the file into the instance, and do first pass analyze

        The input file is written in the following format:

        # This is a set of production rules:
        # S1 -> V1 V2 V3
        # S1 -> V2 V3
        S1:
          V1 V2 V3
          V2 V3
          ...

        # We use hash tag as comment line indicator
        S2:
          V2 V4
          V1 V3
          ...

        i.e. We treat the line ending with a colon as the left hand side
        of a production rule, and all following lines that do not end
        with a colon as right hand side. Each line represents a separate
        production. All right hand side tokens are separated by spaces,
        and we consider everything that is not space character as token
        names

        The empty symbol "eps" is also a non-terminal, with the special
        name T_ (T + underline)

        There could also be semantic rules associated with each production.
        The semantic rule is defined in the parenthesis pair "()" if there
        is one.

        On example of the semantics rule is given as follows:
          S -> A B C D # $2, $1 $3 $4 T_MARKER
        All contents after the # symbol is treated as the semantic rule
        (Note that if # appears as the first character in a line then it
        it a comment line; However in this case it is after a production
        rule, so it represents the start of semantic rules)

        After the "#" symbol there could be names like $ + number,
        denoting the number-th token in the right hand side. "$2," means
        the second RHS symbol is the root of the returned value, and
        $1 $3 $4 after the comma means the child list contains $1 $3 and $4
        If before the comma we do not use the $ notation, then the symbol
        will be treated as a label and a syntax node with that symbol
        will be created as the returned root. If there is no comma
        then we merely return the syntax node with no children.
        All valid usages are summarized in the following:

          A -> T_MINUS # T_PREFIX_INC
          (Renaming: Create a new node and return as root)
          A -> B C D # T_SOMETHING, $2 $3
          (Create a new node and use C and D as its child and return)
          A -> B C D # $2, $1 $3
          (Use C as root and push B, D as it child and return)

        :param file_name: The name of the file to read the syntax
        :return: None
        """
        fp = open(file_name, "r")
        s = fp.read()
        fp.close()

        # Split the content of the file into lines
        line_list = s.splitlines()

        # Since we need to iterate through this twice, so let's
        # do the filtering for only once
        # We use list comprehension to filter out lines that are
        # empty or starts with '#'
        line_list = \
            [line.strip()
             for line in line_list
             if (len(line.strip()) != 0 and line.strip()[0] != '#')]

        # The following must follow the processing of the
        # syntax file because otherwise the syntax would be
        # incomplete

        # This function recognizes terminals and non-terminals
        # and stores them into the corresponding set structure
        self.process_symbol(line_list)
        # This function adds productions and references between
        # productions and symbols
        self.process_production(line_list)
        # This sets self.root_symbol and throws exception is there
        # is problem finding it
        self.process_root_symbol()

        return

    def process_first_follow(self):
        """
        This function computes FIRST and FOLLOW set
        for all non-terminals. The set of non-terminals
        is not changed during iteration, so we do not
        need to make a copy of the set

        Note that even for grammars with left-recursions, we
        could also compute the FIRST and FOLLOW set, in a sense
        that the FIRST set of S for ruleS -> S does not affect S
        at all

        :return: None
        """
        # We use this to fix the order of iteration
        nt_list = list(self.non_terminal_set)

        dbg_printf("Compute FIRST set")

        # A list of FIRST set sizes; we iterate until this
        # becomes stable
        count_list = [nt.get_first_length() for nt in nt_list]
        index = 0
        while True:
            index += 1
            dbg_printf("    Iteration %d", index)

            for symbol in self.non_terminal_set:
                symbol.clear_result_available()

            for symbol in self.non_terminal_set:
                symbol.compute_first([])

            # This is the vector after iteration
            t = [nt.get_first_length() for nt in nt_list]
            if t == count_list:
                break

            count_list = t

        # After computing the FIRST set for symbols now
        # we prepare the substring FIRST set for all
        # productions
        dbg_printf("Compute suffix FIRST set for productions")
        for p in self.production_set:
            p.prepare_substring_first()

        dbg_printf("Compute FOLLOW set")

        # First add EOF symbol into the root symbol
        # We need to do this before the algorithm converges
        assert (self.root_symbol is not None)
        self.root_symbol.follow_set.add(Symbol.get_end_symbol())

        count_list = [nt.get_follow_length() for nt in nt_list]
        index = 0
        while True:
            index += 1
            dbg_printf("    Iteration %d", index)

            for symbol in self.non_terminal_set:
                symbol.clear_result_available()

            for symbol in self.non_terminal_set:
                # The path list must be empty list
                # because we start from a fresh new state
                # for every iteration
                symbol.compute_follow([])

            t = [nt.get_follow_length() for nt in nt_list]
            if t == count_list:
                break

            count_list = t

        return

    def process_root_symbol(self):
        """
        This function finds root symbols. The root symbol is defined as
        the non-terminal symbol with no reference in its rhs_set, which
        indicates in our rule that the symbol is the starting point of
        parsing

        Note that even in the case that root symbol cannot be found, we
        could always define an artificial root symbol by adding:
            S_ROOT -> S
        if S is supposed to be the root but is referred to in some
        productions. This is guaranteed to work.

        In the case that multiple root symbols are present, i.e. there
        are multiple nodes with RHS list being empty, we report error
        and print all possibilities

        :return: None
        """
        # We use this set to find the root symbol
        # It should only contain 1 element
        root_symbol_list = []
        for symbol in self.non_terminal_set:
            if len(symbol.rhs_set) == 0:
                root_symbol_list.append(symbol)

        # These two are abnormal case
        if len(root_symbol_list) > 1:
            dbg_printf("Multiple root symbols found. " +
                       "Could not decide which one")

            # Print each candidate and exit
            for symbol in root_symbol_list:
                dbg_printf("    Candidate: %s", str(symbol))

            raise ValueError("Multiple root symbols")
        elif len(root_symbol_list) == 0:
            dbg_printf("Root symbol is not found. " +
                       "May be you should define an artificial one")

            raise ValueError("Root symbol not found")

        # This is the normal case - exactly 1 is found
        self.root_symbol = root_symbol_list[0]

        return

    @staticmethod
    def add_ast_rule(p, ast_rule):
        """
        This function parses and adds the AST rule defined after
        the production body (i.e. after the # symbol)

        The AST rule is a tuple:
           (rename_flag,
            root_data,        -> Either index or name
            [child data 1, child data 2, .. , child data N], -> Either index or name or (name, index)
            action name)      -> Name of the action
        If rename_flag is True then root_data is the new name we assign to the
        returned syntax node. Otherwise it is the index in the RHS side
        After the root data is the indices of the child nodes. The index corresponds
        to the index in the RHS list

        If child data is not integer then we create syntax nodes using the name
        stored there; If the name is suffixed by @ and then an integer then we
        in addition add the lexical value of the token to the syntax node. In
        the latter case, the child data is a tuple, and the first element is the
        new syntax node name, and the second element is the index in the production
        rule's RHS list on where the token value is from

        The same applies for root_data is the root data needs a token value

        The child node list could be empty as well if there is an action
        defined but no child list definition

        :param p: The production
        :param ast_rule: The AST rule string
        :return: None
        """
        # If there is no AST rule then we just set it to None in
        # the production
        if ast_rule is None:
            p.ast_rule = None
            return

        # First split the AST rule using comma
        # The first component is root rule
        # The second component is child rule
        # The third component is action
        # The first and second could optionally specify
        #   data that the node needs to carry with
        rule_comma_list = ast_rule.split(",")
        if len(rule_comma_list) == 1:
            root = rule_comma_list[0].strip()
            body = None
            action = None
        elif len(rule_comma_list) == 2:
            root = rule_comma_list[0].strip()
            body = rule_comma_list[1].strip()
            action = None
        elif len(rule_comma_list) == 3:
            root = rule_comma_list[0].strip()
            body = rule_comma_list[1].strip()
            action = rule_comma_list[2].strip()
        else:
            # We have seen more commas which is an error
            raise ValueError("Invalid AST rule: \"%s\"" %
                             (ast_rule,))

        # If the root is empty or body is empty
        # then the rule is invalid
        if len(root) == 0:
            raise ValueError("Invalid AST rule: \"%s\"" %
                             (ast_rule, ))
        elif action is not None and len(action) == 0:
            raise ValueError("Must specify an action after the comma (%s)!" %
                             (ast_rule, ))

        # Since we could have empty body but non-empty action
        # This is necessary
        if body is not None and len(body) == 0:
            body = None

        # If the root is a RHS node then rename is False
        # Otherwise we rename the root and assign new name
        # to root_data
        if root[0] == "$":
            rename_flag = False

            # If the conversion throws exception then the
            # index is wrongly specified
            try:
                root_data = int(root[1:]) - 1
                # Check that the index is valid
                if root_data < 0 or root_data >= len(p.rhs_list):
                    raise ValueError()
            except ValueError:
                raise ValueError("Invalid root node: %s" %
                                 (root, ))
        else:
            rename_flag = True
            root_data = root

            # If the root also carries data
            # the source of the data is from the index
            # specified after "@" character
            index = root_data.find("@")
            if index == -1:
                root_data = root
            else:
                root_name = root[:index]
                root_index = root[index + 1:]
                try:
                    root_index = int(root_index) - 1
                    if root_index < 0 or root_index >= len(p.rhs_list):
                        raise ValueError()
                except ValueError:
                    raise ValueError("Invalid root: %s" %
                                     (root, ))

                root_data = (root_name, root_index)

        # If there is no body then that's it
        if body is None and action is None:
            p.ast_rule = (rename_flag, root_data)
            return

        # This will fall through
        if body is None:
            body = ""

        body_list = body.split()
        body_ret_list = []
        for body_token in body_list:
            # If the body is an index then we just store the
            # integer index
            if body_token[0] == '$':
                # If error happens in converting the index
                # to integer then we know the body is wrong
                try:
                    body_data = int(body_token[1:]) - 1
                    # Check the index, and if it is invalid the
                    # exception will be caught by the outer ValueError
                    if body_data < 0 or body_data >= len(p.rhs_list):
                        raise ValueError()
                except ValueError:
                    raise ValueError("Invalid body: %s" %
                                     (body, ))
                body_ret_list.append(body_data)
            else:
                # Try to find things like T_IDENT@2 which means
                # we create a new node called T_IDENT and its token
                # value is from the 2nd symbol in the production rule
                index = body_token.find("@")
                if index == -1:
                    body_ret_list.append(body_token)
                else:
                    body_token_name = body_token[:index]
                    body_token_index = body_token[index + 1:]
                    try:
                        body_token_index = int(body_token_index) - 1
                        if body_token_index < 0 or \
                           body_token_index >= len(p.rhs_list):
                           raise ValueError()
                    except ValueError:
                        raise ValueError("Invalid body: %s" %
                                         (body,))

                    # In this case we append a tuple to demonstrate both
                    # the name and the index
                    body_ret_list.append((body_token_name,
                                          body_token_index))

        # If there is no action then we return
        if action is None:
            # The third component is the child list template
            p.ast_rule = (rename_flag, root_data, body_ret_list)
            return

        # Otherwise also append the optional action into the tuple
        # and set it as the AST rule of production p
        p.ast_rule = (rename_flag, root_data, body_ret_list, action)

        return

    def process_production(self, line_list):
        """
        This function constructs productions and place them into
        a list of productions. It also establishes referencing relations
        between productions and non-terminal symbols (i.e. which symbol
        is referred to in which production)

        :return: None
        """
        # This is the current non-terminal node, and we change it
        # every time a line with ':' is seen
        current_nt = None
        has_body = True
        for line in line_list:
            # If there are ast rules, we save it for later
            # processing and just process productions in the
            # following first
            index = line.find("#")
            if index != -1:
                ast_rule = line[index + 1:].strip()
                line = line[:index]
                # If after stripping the rule is empty string
                # then still we know there is nothing interesting
                if len(ast_rule) == 0:
                    ast_rule = None
            else:
                # If there is no semantic rule defined just
                # make it as None
                ast_rule = None

            if line[-1] == ':':
                # When we see the start of a production, must make
                # sure that the previous production has been finished
                if has_body is False:
                    raise ValueError(
                        "Production %s does not have a body" %
                        (line, ))
                else:
                    has_body = False

                current_nt_name = line[:-1]
                assert(current_nt_name in self.symbol_dict)

                current_nt = self.symbol_dict[current_nt_name]
                continue

            # There must be a non-terminal node to use
            if current_nt is None:
                raise ValueError("The syntax must start with a non-terminal")
            else:
                assert(current_nt.is_non_terminal() is True)

            # We have seen a production body
            has_body = True

            # This will be passed into the constructor of
            # class Production
            rhs_list = []

            # This is a list of symbol names
            # which have all been converted into terminals
            # or non-terminals in the first pass
            symbol_list = line.split()
            for symbol_name in symbol_list:
                # The key must exist because we already had one
                # pass to add all symbols
                assert(symbol_name in self.symbol_dict)

                symbol = self.symbol_dict[symbol_name]

                # Add an RHS symbol
                # Establishes from non-terminals to productions
                # are established later when we construct the object
                rhs_list.append(symbol)

            # Finally construct an immutable class Production instance
            # the status could no longer be changed after it is
            # constructed
            # This also adds the production into the production
            # set of the generator
            p = Production(self, current_nt, rhs_list)

            # Then add ast rule into the production
            # Note that this is a static function
            ParserGenerator.add_ast_rule(p, ast_rule)

        return

    def process_symbol(self, line_list):
        """
        Recognize symbols, and store them into the dictionary for
        symbols, as well as the sets for terminals and non-terminals

        :param line_list: A list of lines
        :return: None
        """
        # This is a set of names for which we are not certain whether
        # it is a terminal or non-terminal
        in_doubt_set = set()

        for line in line_list:
            # If there are semantic rules just disregard it here
            index = line.find("#")
            if index != -1:
                line = line[:index]

            # We have seen a new LHS of the production rule
            # Also non-terminal is very easy to identify because
            # it must be on the LHS side at least once
            if line[-1] == ':':
                # This is the name of the terminal
                name = line[:-1]
                # Create the non-terminal object and then
                non_terminal = NonTerminal(name)

                # If the non-terminal object already exists then we have
                # seen duplicated definition
                if non_terminal in self.symbol_dict:
                    raise KeyError("Duplication definition of non-terminal: %s" %
                                   (name, ))

                # Add the non-terminal node into both symbol dictionary
                # and the non-terminal set
                self.symbol_dict[name] = non_terminal
                self.non_terminal_set.add(non_terminal)

                # Since we already know that name is a non-terminal
                # we could remove it from this set
                # discard() does not raise error even if the name
                # is not in the set
                in_doubt_set.discard(name)
            else:
                # Split the line using space characters
                name_list = line.split()
                # We do not know whether name is a terminal or non-terminal
                # but we could rule out those that are known to be
                # non-terminals
                for name in name_list:
                    # If the name is already seen and it is a terminal
                    # then we skip it
                    if name in self.symbol_dict:
                        continue

                    # Add the name into in_doubt_set because
                    # we are unsure about its status
                    in_doubt_set.add(name)

        # After this loop, in_doubt_set contains names that do not
        # appear as the left hand side, and must be terminals
        for name in in_doubt_set:
            assert(name not in self.symbol_dict)
            terminal = Terminal(name)
            self.symbol_dict[name] = terminal

            # And then add terminals into the terminal set
            assert(terminal not in self.terminal_set)
            self.terminal_set.add(terminal)

        return

    @staticmethod
    def dump_symbol_set(fp, ss):
        """
        Dumps a set into a given file handle. This function is
        for convenience of printing the FIRST and FOLLOW set

        The set if printed as follows:
           {element, element, .. }

        :param fp: The file handler
        :param ss: The set instance
        :return: None
        """
        first = True
        fp.write("{")

        # Make each iteration produce uniform result
        ss = list(ss)
        ss.sort()

        for i in ss:
            # Must be a symbol element
            assert (i.is_symbol() is True)
            if first is False:
                fp.write(", ")
            else:
                first = False

            fp.write(i.name)

        fp.write("}")

        return

    def dump_terminal_enum(self, file_name):
        """
        This file dumps the terminal enum C file

        :param file_name: The file name that we dump the
          enum definition into
        :return: None
        """
        fp = open(file_name, "w")
        fp.write("enum class TerminalToken {\n")
        symbol_list = [symbol.name for symbol in self.terminal_set]
        symbol_list.sort()

        for name in symbol_list:
            fp.write("  %s,\n" % (name, ))

        fp.write("};\n")

        fp.close()

        return

#####################################################################
# class ParserGeneratorLR
#####################################################################

class ParserGeneratorLR(ParserGenerator):
    """
    This class is the main class we use to generate a LR parser
    """

    # The following constants defines the action a parser could take
    ACTION_SHIFT = 0
    ACTION_REDUCE = 1
    ACTION_GOTO = 2
    ACTION_ACCEPT = 3

    # Type of LR items we support
    LR_TYPE_SLR = 0
    LR_TYPE_LR1 = 1
    LR_TYPE_LALR = 2

    def __init__(self, file_name, lr_type=LR_TYPE_SLR):
        """
        Read syntax file into the class and analyze its symbols and
        productions

        :param file_name: The file name of the syntax file
        """
        # read file is called inside the constructor
        ParserGenerator.__init__(self, file_name)

        # We build different items based on this
        self.lr_type = lr_type
        if self.lr_type != self.LR_TYPE_LALR and \
           self.lr_type != self.LR_TYPE_SLR and \
           self.lr_type != self.LR_TYPE_LR1:
            raise TypeError("Unknown LR Parser type: %d" % (self.lr_type, ))

        # After computing the item set set we use it to
        # fix the order of items and use the index to refer to
        # the i-th element in this set
        self.item_set_list = []

        # This is an identity mapping - we map an item set to itself
        # The reason for this map is that item set could be identical
        # without being the same object, i.e. we check equality of two
        # items only by comparing their items but not other attributes
        # such as the GOTO table. However we do need to recognize
        # items that already exist and has GOTO table when we have one
        # without GOTO table, and therefore this table provides translation
        # between identity (i.e. the attribute we care for equality)
        # and all contents
        self.item_set_identity_dict = {}

        # It maps a tuple (current state, symbol) to
        #                 (action, data1, data2, i.e. pop_length, data3, i.e. AST rule)
        #    1. If symbol is a terminal then action could be SHIFT or REDUCE
        #       1.1 If action is SHIFT then data 1 is the next state and
        #           there is no data2
        #       1.2 If action is REDUCE then data 1 is the symbol that it
        #           reduces to, and data2 is the length of the production
        #           we use to reduce (pop_length), and data3 is the AST rule
        #           for building the AST
        #    2. If symbol is a non-terminal then action is always GOTO
        #       and next state is the state we enter
        self.parsing_table = {}

        # Add an artificial root symbol for LR parsing
        # This must be done before computing FIRST/FOLLOW set
        fake_root_symbol = Symbol.get_root_symbol()
        self.non_terminal_set.add(fake_root_symbol)

        # Also add end symbol T_EOF (i.e. $ symbol) into the
        # terminal set. This will appear in the FOLLOW set
        self.terminal_set.add(Symbol.get_end_symbol())

        # Use the fake root symbol to derive root symbol
        # because we need it to be the sign of termination
        # We use the fake production also to find starting state
        self.fake_production = \
            Production(self, fake_root_symbol, [self.root_symbol])

        # This is the item set we start parsing and it will
        # be set later
        self.starting_item_set = None
        # This is the state ID from which we start parsing
        self.starting_state = None

        # Compute the follow set of all productions
        # LR generator does not modify the grammar so we could
        # compute the FOLLOW set before everything else
        self.process_first_follow()

        # This function generates the canonical LR set
        self.process_item_set()

        # If the LR type is LALR then we merge LR states using
        # core items (i.e. items without the lookahead symbol)
        if self.lr_type == self.LR_TYPE_LALR:
            self.merge_item_set()

        # Finally...
        # This function will throw conflict exception as KeyError
        # if there is any conflict
        self.generate_parsing_table()

        return

    def merge_item_set(self):
        """
        This function merges LR(1) item sets into LALR item sets
        by using core items to identify states.

        Transforming LR(1) item sets into LALR item sets do not
        introduce extra SHIFT-REDUCE errors because if there is
        any they would also be in LR(1) item sets. However, merging
        LR(0) items may introduce (though very rare) REDUCE-REDUCE
        errors because now the lookahead symbols are merged into one
        state

        Item identity dictionary, starting state will be changed
        by this function

        :return: None
        """
        dbg_printf("Merging item sets for LALR...")

        # This maps from LR(0) item set to LR(1) item set
        new_item_set_dict = {}
        new_item_set_list = []

        # The index of this list is the old index, and the value
        # is the new index
        index_mapping = {}

        new_index = 0
        # These item sets are LR(1) item sets
        # Note that we must iterate on the list because the dict
        # will be invalidated, since we change item sets in-place
        for item_set in self.item_set_list:
            # Build LR(0) Item
            lr0_item_set = ItemSet()
            for item in item_set.item_set:
                # Construct LR(0) item
                lr0_item_set.item_set.add(LRItem(item.p, item.index))

            # If we have not seen the LR item set yet, just add it
            if lr0_item_set not in new_item_set_dict:
                # Then map the old index to newly allocated index which
                # starts at 0
                index_mapping[item_set.index] = new_index
                new_index += 1

                new_item_set_dict[lr0_item_set] = item_set
                new_item_set_list.append(item_set)
            else:
                # This is the item set we merge into
                merge_dest = new_item_set_dict[lr0_item_set]
                merge_dest.merge_with(item_set)

                # Both source and dest should have the same index
                # in the new item set system
                index_mapping[item_set.index] = \
                    index_mapping[merge_dest.index]

        # Fix the GOTO table relation
        for new_item_set in new_item_set_list:
            key_list = list(new_item_set.goto_table.keys())
            for symbol in key_list:
                goto_item_set = new_item_set.goto_table[symbol]
                new_index = index_mapping[goto_item_set.index]
                new_item_set.goto_table[symbol] = \
                    new_item_set_list[new_index]

        # At last fix the index
        for new_item_set in new_item_set_list:
            new_item_set.index = \
                index_mapping[new_item_set.index]

        # Also change starting state
        self.starting_state = \
            index_mapping[self.starting_state]

        # Change item set list
        self.item_set_list = new_item_set_list

        # Change identify dict
        d = {}
        for item_set in new_item_set_list:
            d[item_set] = item_set
        self.item_set_identity_dict = d

        dbg_printf("Merge complete. There are now %d states",
                   len(self.item_set_list))

        return

    @staticmethod
    def json_unicode_to_str(obj):
        """
        This function converts all unicode objects to strings
        in a structure returned by JSON

        :param obj: The object
        :return: None
        """
        if isinstance(obj, list) is False:
            return

        for i in xrange(0, len(obj)):
            if isinstance(obj[i], unicode):
                obj[i] = str(obj[i])
            else:
                ParserGeneratorLR.json_unicode_to_str(obj[i])

        return obj


    def load_parsing_table(self, file_name):
        """
        This function loads the parsing table in the format we dump it
        (i.e. Two python strings separated by arrow "->")

        Note that since JSON will convert all tuples into strings, we
        should convert it back to tuples for them to be used as dict
        keys.

        :param file_name: The file name of the dumped parsing table
        :return: None
        """
        fp = open(file_name, "r")

        first_line = True
        entry_count = 0
        # For each line just decompose it into a key and a value field
        for line in fp:
            # If it is the first line, then this is the
            # starting state and we just recover it
            if first_line is True:
                first_line = False
                self.starting_state = int(line)

                dbg_printf("    Recovered starting state: %d",
                           self.starting_state)
                continue
            else:
                # This is how many entries we read from the file
                entry_count += 1

            # Strip off the trailing '\n' character
            line = line.strip()
            # Then locate the arrow symbol. If there is not one then
            # we know the format is invalid
            index = line.find("->")
            if index == -1:
                raise ValueError("Invalid parsing table file: \"%s\"" %
                                 (line, ))

            key_value = line.split("->")
            if len(key_value) != 2:
                raise ValueError("Invalid parsing table file: \"%s\"" %
                                 (line,))

            # Convert it into tuple key and tuple value
            key = tuple(
                ParserGeneratorLR.json_unicode_to_str(
                    json.loads(key_value[0])))
            value = tuple(
                ParserGeneratorLR.json_unicode_to_str(
                    json.loads(key_value[1])))

            # Finally store them inside the parsing table
            self.parsing_table[key] = value

        fp.close()

        dbg_printf("Successfully recovered %d entries", entry_count)

        return

    def dump_parsing_table(self, file_name):
        """
        Dumps the parsing table into a file. The format of the file is specified as
        follows:

        The file is dumped as a set of lines, and each line represents an entry in
        the parsing table. The format of the line is like the following:
            start_state               # This must be the first line of the file
                                      # And it is an integer
            key -> value              # Both key and value are JSON representation
                                      # of the parsing table key and value

        It is worth to mention that since JSON does not support tuples, all
        tuple objects will be dumped as a list. Therefore, when we loads the
        parsing table back to the parser, we need to convert the list
        object back to tuple to be used as dict keys.

        Note that since we use -> to separate key and value in the parsing
        table, the terminals and non-terminals could not contain the arrow
        symbol "->". It is recommended that both terminals and non-terminals
        should have T_ prefix and should not use special characters to avoid
        misinterpretation

          e.g. tuple([1, 2, 3, 4, 5]) returns (1, 2, 3, 4, 5)
               list((1, 2, 3, 4, 5)) returns [1, 2, 3, 4, 5]
        :param file_name: The file to dump the parsing table into
        :return: None
        """
        dbg_printf("Dumping parsing table into file: %s", file_name)

        fp = open(file_name, "w")

        # Write the starting state first
        fp.write("%d\n" % (self.starting_state, ))

        # For each key value pair dump them into the file
        for key, value in self.parsing_table.items():
            fp.write("%s -> %s\n" % (json.dumps(key), json.dumps(value)))

        fp.close()

        return

    @staticmethod
    def print_item_set(item_set, ident=0):
        """
        This function prints an item set on stdout. The printing is of
        the following format:
            item 1
            item 2
            ..
            item k
        with optional space characters specified in identifier

        :param item_set: The item set we print
        :return: None
        """
        for item in item_set.item_set:
            s = " " * ident
            s += ("%s" % (item, ))
            dbg_printf("%s", s)

        return

    def generate_parsing_table(self):
        """
        This function generates a parsing table for the LR parser

        :return: None
        """
        dbg_printf("Generating LR parsing table...")
        if self.lr_type == self.LR_TYPE_SLR:
            dbg_printf("    Type SLR")
        elif self.lr_type == self.LR_TYPE_LR1:
            dbg_printf("    Type LR1")
        elif self.lr_type == self.LR_TYPE_LALR:
            dbg_printf("    Type LALR")
        else:
            assert False

        # Iterate over all items
        for item_set in self.item_set_identity_dict:
            for symbol, goto_item_set in \
                    item_set.goto_table.items():
                # Current state, symbol
                k = (item_set.index, symbol.name)

                # We have not added REDUCE entry so there
                # would not be any SHIFT-REDUCE conflict
                assert(k not in self.parsing_table)

                # If it is terminal then we do SHIFT
                if symbol.is_terminal() is True:
                    # Shift to the next state
                    self.parsing_table[k] = \
                        (self.ACTION_SHIFT, goto_item_set.index)
                else:
                    self.parsing_table[k] = \
                        (self.ACTION_GOTO, goto_item_set.index)

            #
            # Then add reduce
            #

            for item in item_set.item_set:
                # We only process those that can reduce
                if item.could_reduce() is False:
                    continue

                # This is the LHS of the production that could be reduced
                lhs = item.p.lhs

                # If the LHS is fake root then we know if we see an
                # EOF we could finish parsing; However if it is not EOF
                # then parsing may continue
                if lhs == self.fake_production.lhs:
                    dbg_printf("Setting finish state for fake root symbol")

                    # Note that we use the name of T_EOF as the key here
                    k = (item_set.index, Symbol.get_end_symbol().name)
                    assert(k not in self.parsing_table)

                    self.parsing_table[k] = (self.ACTION_ACCEPT, )

                    # Do not compute using fake root's FOLLOW set
                    continue

                if self.lr_type == self.LR_TYPE_SLR:
                    lookahead_set = lhs.follow_set
                elif self.lr_type == self.LR_TYPE_LR1 or \
                     self.lr_type == self.LR_TYPE_LALR:
                    lookahead_set = item.lookahead_set
                else:
                    assert False

                # The look-ahead symbol is the follow set of the
                # LHS symbol of the item
                for symbol in lookahead_set:
                    assert(symbol.is_terminal() is True)

                    # Key and value in the mapping table
                    k = (item_set.index, symbol.name)

                    # Note 1: the AST rule is also contained in the reduce
                    # action descriptor
                    # Note 2: We use the non-terminal's name instead of the object
                    # as the table entry
                    v = (self.ACTION_REDUCE,
                         lhs.name,
                         # Note: If item.p is empty production this should
                         # be 0 instead of 1
                         item.p.size(),
                         item.p.ast_rule)

                    # If the entry exists and the value is different
                    # then we know there is a conflict
                    if k in self.parsing_table and \
                       self.parsing_table[k] != v:
                        conflict_action = self.parsing_table[k][0]

                        # Could be both because REDUCE adds multiple
                        # symbols
                        assert(conflict_action == self.ACTION_SHIFT or
                               conflict_action == self.ACTION_REDUCE)
                        if conflict_action == self.ACTION_SHIFT:
                            dbg_printf("SHIFT-REDUCE conflict on symbol %s, state %d",
                                       symbol,
                                       item_set.index)

                            # Print it out with ident = 4
                            ParserGeneratorLR.print_item_set(item_set, 4)

                            dbg_printf("Reduce: %s", item)
                            dbg_printf("    FOLLOW: %s", item.p.lhs.follow_set)

                            # In favor of shift
                            continue
                            #raise KeyError("SHIFT-REDUCE conflict")
                        elif conflict_action == self.ACTION_REDUCE:
                            dbg_printf("REDUCE-REDUCE conflict on symbol %s",
                                       symbol)
                            dbg_printf("    ItemSet %s", item_set.item_set)

                            raise KeyError("REDUCE-REDUCE conflict")

                    # Then set reduce action
                    self.parsing_table[k] = v

        dbg_printf("    Parsing table has %d entries", len(self.parsing_table))

        return

    def process_item_set(self):
        """
        This function computes the canonical LR item sets

        :return: None
        """
        new_item_set = ItemSet()

        # Fake item is the starting point
        if self.lr_type == ParserGeneratorLR.LR_TYPE_SLR:
            dbg_printf("Compute LR(0) set with lookahead")
            new_item = LRItem(self.fake_production, 0)
        elif self.lr_type == ParserGeneratorLR.LR_TYPE_LR1 or \
             self.lr_type == ParserGeneratorLR.LR_TYPE_LALR:
            dbg_printf("Compute canonical LR set")
            new_item = LR1Item(self.fake_production,
                               0,
                               set([Symbol.get_end_symbol()]))
        else:
            raise TypeError("Unknown LR parser type: %d" %
                            (self.lr_type, ))

        new_item_set.item_set.add(new_item)
        # Adds the new item set after computing its closure
        # and then we begin iteration
        new_item_set.compute_closure()

        # Set it here such that we could find it later
        self.starting_item_set = new_item_set

        # Map identity to contents
        self.item_set_identity_dict[new_item_set] = \
            new_item_set

        # Number of iterations it takes to build the LR
        # canonical set
        iteration = 0
        while True:
            iteration += 1

            prev_count = len(self.item_set_identity_dict)

            dbg_printf("    Iteration %d; states %d",
                       iteration,
                       prev_count)

            # We must iterate on the list because the item set
            # will be changed
            item_set_list = list(self.item_set_identity_dict.keys())

            t = 0
            # For each item set computes its GOTO table
            # if it does not have one
            for item_set in item_set_list:
                # If we have already processed this then
                # skip it because it will not add any new item set
                if item_set.has_goto_table() is True:
                    continue

                # Compute GOTO item set. If the computed GOTO
                # is an existing one (we do this by testing it
                # against the current item set set) then just
                # replace it with the current one
                item_set.compute_goto(self.item_set_identity_dict)

                t += 1
                dbg_printf("Finished computing %d GOTO set\r",
                           t,
                           no_newline=True,
                           flush=True)

            dbg_printf("", no_header=True)

            # If the set does not change, i.e. all newly computed
            # GOTO sets are already in the set then we know we
            # have done
            if prev_count == len(self.item_set_identity_dict):
                break

        # From now on we use the item set list
        self.item_set_list = list(self.item_set_identity_dict.keys())
        for i in range(0, len(self.item_set_list)):
            # We start parsing on starting state
            if self.item_set_list[i] == self.starting_item_set:
                self.starting_state = i

            # Also set the index of the item set
            assert(self.item_set_list[i].index is None)
            self.item_set_list[i].index = i

        # Debug: find who GOTO to #248, the SHIFT-REDUCE conflict
        # state on T_COLON to decide whether to parse a labelled statement
        # or an identifier primary expression
        """
        for i in self.item_set_list:
            for j in i.goto_table.values():
                if j.index == 248:
                    dbg_printf("========================", )
                    ParserGeneratorLR.print_item_set(i, 4)
        """

        assert(self.starting_state is not None)

        dbg_printf("Computed the canonical set in %d steps",
                   iteration)
        dbg_printf("    There are %d elements in the canonical set",
                   len(self.item_set_identity_dict))

        return

#####################################################################
# class ParserGeneratorLL1
#####################################################################

class ParserGeneratorLL1(ParserGenerator):
    """
    This class is the main class we use to generate the parsing code
    for a given LL(1) syntax
    """
    def __init__(self, file_name):
        """
        Initialize the generator object
        """
        ParserGenerator.__init__(self, file_name)

        # It is a directory structure using (NonTerminal, Terminal)
        # pair as keys, and production rule object as value (note
        # that we only allow one production per key)
        self.parsing_table = {}

        # As suggested by name
        self.process_left_recursion()
        # This must be done after left recursion elimination
        # because left recursion may expose common prefix
        self.process_common_prefix()

        # Compute the first and follow set
        # Note that this function is defined in the base class
        # and we call this after we have processed left recursion
        # and common prefix
        self.process_first_follow()

        # Check feasibility of LL(1)
        self.verify()

        # Then generates the parsing table
        self.generate_parsing_table()

        return

    def generate_parsing_table(self):
        """
        This function generates the parsing table with (A, alpha)
        as keys and productions to use as values

        :return: None
        """
        for p in self.production_set:
            lhs = p.lhs
            for i in p.first_set:
                # Do not add empty symbol
                if i == Symbol.get_empty_symbol():
                    continue

                pair = (lhs, i)
                if pair in self.parsing_table:
                    raise KeyError(
                        "Duplicated (A, FIRST) entry for %s" %
                        (str(pair), ))

                self.parsing_table[pair] = p

            # If the production produces empty string
            # then we also need to add everything in FOLLOW(lhs)
            # into the jump table
            # Since we already verified that no FIRST in other
            # productions could overlap with LHS's FOLLOW set
            # this is entirely safe
            if Symbol.get_empty_symbol() in p.first_set:
                for i in lhs.follow_set:
                    pair = (lhs, i)
                    if pair in self.parsing_table:
                        raise KeyError(
                            "Duplicated (A, FOLLOW) entry for %s" %
                            (str(pair), ))

                    self.parsing_table[pair] = p

        return

    def dump(self, file_name):
        """
        This file dumps the contents of the parser generator into a file
        that has similar syntax as the .syntax input file.

        We dump the following information:
           (1) Revised rules
           (2) FIRST and FOLLOW set for each rule

        :param file_name: The file name we dump into
        :return: None
        """
        dbg_printf("Dumping modified syntax to %s", file_name)

        fp = open(file_name, "w")

        # Construct a list and sort them in alphabetical order
        # Since we have already defined the less than function for
        # non-terminal objects this is totally fine
        nt_list = list(self.non_terminal_set)
        nt_list.sort()
        # We preserve these symbols
        for symbol in nt_list:
            fp.write("%s: " % (symbol.name, ))

            ParserGenerator.dump_symbol_set(fp, symbol.first_set)
            fp.write(" ")
            ParserGenerator.dump_symbol_set(fp, symbol.follow_set)

            # End the non-terminal line
            fp.write("\n")

            for p in symbol.lhs_set:
                fp.write("   ")
                for rhs in p.rhs_list:
                    fp.write(" %s" % (rhs.name, ))

                fp.write("; ")
                ParserGenerator.dump_symbol_set(fp, p.first_set)

                fp.write("\n")

            fp.write("\n")

        fp.close()
        return

    def dump_parsing_table(self, file_name):
        """
        This function dumps a parsing table into a specified file.
        The table is dumped in a format like the following:

           (NonTerminal, Terminal): Production Rule

        And for each entry in the table there is a line like this.
        Empty strings should not appear (they are always the default
        case), and EOF is displayed as T_EOF

        :param file_name: The name of the dumping file
        :return: None
        """
        dbg_printf("Dumping parsing table into %s", file_name)

        fp = open(file_name, "w")

        # Sort the list of keys such that NonTerminals group together
        # and then terminals group together
        key_list = self.parsing_table.keys()
        key_list.sort()

        prev_key = None
        for k in key_list:
            # If the key changes we also print a new line
            if prev_key is None:
                prev_key = k[0]
            elif prev_key != k[0]:
                fp.write("\n")
                prev_key = k[0]

            p = self.parsing_table[k]
            fp.write("(%s, %s): %s\n" %
                     (k[0].name, k[1].name, str(p)))

        fp.close()

        return

    def verify(self):
        """
        Verify the validity of the grammar as LL(1) for generating
        predictive recursive descent parser

        We check the following properties:
          (1) There is no direct left recursion
          (2) There is no indirect left recursion
          The above two are checked together because indirect
          left recursion includes direct left recursion in our case
          (3) For LHS symbol S, all of its production must have
              disjoint FIRST sets
          (4) For A -> a | b if a derives empty string then FIRST(b)
              and FOLLOW(A) are disjoint sets
          (5) For all productions, if T_ appears then it must be
              A -> T_
          (6) For all non-terminals S, it must only appear once in
              all productions where it appears
          (7) Empty symbol does not appear in all FOLLOW sets

        (This list is subject to change)

        If any of these are not satisfied we throw an exception to
        indicate the user that the grammar needs to be changed

        :return: None
        """
        # This checks both condition
        for symbol in self.non_terminal_set:
            if symbol.exists_indirect_left_recursion() is True:
                raise ValueError("Left recursion is detected" +
                                 " @ symbol %s" %
                                 (str(symbol), ))

        # This checks condition 5
        for p in self.production_set:
            for symbol in p.rhs_list:
                if symbol == Symbol.get_empty_symbol():
                    if len(p.rhs_list) != 1:
                        raise ValueError("Empty string in the" +
                                         " middle of production")

        # This checks condition 6
        for symbol in self.non_terminal_set:
            for p in symbol.rhs_set:
                # This is a list of indices that this symbol
                # appears in the production
                ret = p.get_symbol_index(symbol)

                # Make sure each non-terminal only appears once
                # in all productions
                assert(len(ret) == 1)

        # This checks condition 7
        for symbol in self.non_terminal_set:
            assert(Symbol.get_empty_symbol() not in symbol.follow_set)

        # This checks condition 3
        for symbol in self.non_terminal_set:
            # Make it a list to support enumeration
            lhs = list(symbol.lhs_set)
            size = len(lhs)

            for i in range(1, size):
                for j in range(0, i):
                    # This is the intersection of both sets
                    s = lhs[i].first_set.intersection(lhs[j].first_set)
                    if len(s) != 0:
                        raise ValueError(
                            ("The intersection of %s's first_set is not empty\n" +
                             "  %s (%s)\n  %s (%s)") %
                            (str(symbol),
                             str(lhs[i]),
                             str(lhs[i].first_set),
                             str(lhs[j]),
                             str(lhs[j].first_set)))

        # This checks condition 4
        for symbol in self.non_terminal_set:
            lhs = list(symbol.lhs_set)
            size = len(lhs)

            for i in range(1, size):
                for j in range(0, i):
                    # These two are two productions
                    pi = lhs[i]
                    pj = lhs[j]

                    # If pi could derive empty string then FIRST(pj)
                    # and follow A are disjoint
                    if Symbol.get_empty_symbol() in pi.first_set:
                        t = pj.first_set.intersection(symbol.follow_set)
                        if len(t) != 0:
                            raise ValueError(
                                "FIRST/FOLLOW conflict for %s on: \n  %s\n  %s" %
                                (str(symbol),
                                 str(pi),
                                 str(pj)))

                    if Symbol.get_empty_symbol() in pj.first_set:
                        t = pi.first_set.intersection(symbol.follow_set)
                        if len(t) != 0:
                            raise ValueError(
                                "FIRST/FOLLOW conflict for %s on: \n  %s\n  %s" %
                                (str(symbol),
                                 str(pi),
                                 str(pj)))

        return

    def process_common_prefix(self):
        """
        This function iteratively eliminates all common left
        prefixes by applying left factorization using artificial
        non-terminal nodes. This process must be performed by multiple
        rounds until no more left factorization could be done

        :return: None
        """
        total_count = 0
        # This counts the number of iteration
        it_count = 0
        while True:
            it_count += 1

            count = 0
            # Always make a new copy here because LF will add
            # new terminals
            temp = self.non_terminal_set.copy()

            for symbol in temp:
                ret = symbol.left_factorize()
                if ret is True:
                    count += 1

            if count == 0:
                break
            else:
                total_count += count

        dbg_printf("Left factorized for %d symbols in %d iterations",
                   total_count,
                   it_count)

        return

    def process_left_recursion(self):
        """
        This function removes left recursion for all rules

        :return: None
        """
        # Make a copy to avoid changing the size of the set
        temp = self.non_terminal_set.copy()

        # This is the number of left recursions we have removed
        count = 0

        # Remove left recursion. Note we should iterate
        # on the set above - it is guaranteed that the
        # newly added symbols will not create left recursion
        for symbol in temp:
            ret = symbol.eliminate_left_recursion()
            if ret is True:
                count += 1

        # Verify that we have eliminated all left recursions
        for symbol in self.non_terminal_set:
            assert(symbol.exists_direct_left_recursion() is False)

        dbg_printf("Removed left recursion for %d non-terminals",
                   count)

        return

####################################################################
# class ParserLR
####################################################################

class ParserLR(ParserGeneratorLR):
    """
    This class is a simple demo of how LR shift-reduce
    parser works
    """

    def process_decl_body(self, ast_root):
        """
        This function processes typedef and adds name defined by
        typedef into the current scope. If the name is already
        defined then just throw an error

        :param ast_root: The root of the AST
        :return: Same root
        """
        symbol_stack = self.symbol_stack

        # First check whether typedef in indeed
        # part of the declaration
        specifier_list = symbol_stack[-1]
        for specifier in specifier_list:
            if specifier == "T_TYPEDEF":
                break
        else:
            # If we did not break from the loop
            # then it must be because there is no typedef
            return ast_root

        # We can never see an empty list
        assert(ast_root.size() != 0)
        # We only process the last one because this function
        # has already been invoked for the previous ones
        last_init_decl = ast_root[-1]
        assert(last_init_decl == "T_INIT_DECL")

        decl_body = last_init_decl[0]
        assert(decl_body == "T_DECL_BODY")
        assert(decl_body[0] == "T_IDENT")

        # Obtain the name which must be the first element
        # of the declaration
        name = decl_body[0].data
        if self.add_typedefed_name(name) is False:
            raise ValueError("Name %s has already been typedef'ed" %
                             (name, ))

        return ast_root

    def preprocess_token(self, token):
        """
        Does not pre-processing of the token before it is used
        for parsing

        :param token: The token object
        :return: Same or modified token object
        """
        assert(isinstance(token, Token))
        # Change this to change the return value
        ret_token = token
        if token.name == "T_IDENT":
            if self.is_typedefed(token.data) is False:
                return token

            print "Rename %s to typedef name" % (token.data, )

            # Change it to T_TYPEDEF_NAME
            ret_token = Token("T_TYPEDEF_NAME", token.data)
            ret_token.index = token.index

        return ret_token

    def process_enter_scope(self, ast_root):
        """
        This function process enter_scope action in the AST rule

        :param ast_root: The root of the AST subtree
        :return: Same AST node
        """
        self.enter_scope()
        return ast_root

    def process_leave_scope(self, ast_root):
        """
        This function process enter_scope action in the AST rule

        :param ast_root: The root of the AST subtree
        :return: Same AST node
        """
        self.leave_scope()
        return ast_root

    # This is a dictionary recording actions when a
    # reduce is performed
    # The key is the action string specified in the AST rule
    # The return value of the call back function will be
    # passed to the parent as the parsed AST
    REDUCE_ACTION_DICT = {
        "enter_scope": process_enter_scope,
        "leave_scope": process_leave_scope,
        "add_decl_body": process_decl_body,
    }

    def __init__(self, file_name):
        """
        Initialize the LR parsing table

        :param file_name: The file of the dumped parsing table or
         the syntax file depending on the second argument
        """
        # This is to avoid PyCharm warning
        if False:
            ParserGeneratorLR.__init__(self, None)

        self.terminal_set = set()
        self.non_terminal_set = set()
        self.symbol_dict = {}
        self.production_set = set()

        # These two are really used
        self.starting_state = -1
        self.parsing_table = {}

        self.load_parsing_table(file_name)

        # This is the stack of symbol tables in order to
        # parse typedef'ed name - we use a set object
        # to represent names that has been typedef'ed
        self.scope_stack = [set()]

        # These two are used during parsing
        self.state_stack = None
        self.symbol_stack = None

        return

    def enter_scope(self):
        """
        Enters a new scope. This is implemented as pushing a new
        set into the scope_stack

        :return: None
        """
        print "enter scope"
        self.scope_stack.append(set())
        return

    def leave_scope(self):
        """
        Leaves the current scope. Note that it is impossible for the
        program to underflow the scope stack because our grammar
        rule makes sure that each scope must be terminated properly
        if parsing is successful

        :return: None
        """
        print "leave scope"
        assert(len(self.scope_stack) != 0)
        self.scope_stack.pop()
        print "Scope stack after leaving:", self.scope_stack

        return

    def is_typedefed(self, name):
        """
        Whether a symbol (i.e. string representation of the identifier)
        has been defined in the current scope. If it is not then we
        search up the scope stack until the first one

        :return: bool
        """
        # Iterate from the back of the list
        i = len(self.scope_stack) - 1
        while i >= 0:
            symbol_set = self.scope_stack[i]
            # If the name exists in any of the symbol set then
            # just return True otherwise it does not exist
            if name in symbol_set:
                return True

            i -= 1

        return False

    def add_typedefed_name(self, name):
        """
        This function adds a typedefed name into the current scope
        i.e. the top level scope

        If the name already exists in the top level scope then we
        know we have seen a name conflict that could not be resolved
        and this function returns False to signal the caller

        :param name: The name we need to add
        :return: bool. False if the name already exists
        """
        assert(len(self.scope_stack) != 0)
        top_level = self.scope_stack[-1]
        # If the name exists in the top level scope
        # then we have redefined it and this is an error
        if name in top_level:
            return False

        # Otherwise just add it into the symbol set
        top_level.add(name)
        print "added typedef name", name

        return True

    @staticmethod
    def load_token_list(file_name):
        """
        This function loads a token list and build a list of
        terminals for the parser to consume

        The token list should be a file with lines:
            TokenType = [Here goes terminal name]    ; ...
        Since we only care about the terminal name but not
        their contents, everything after the semicolon will be ignored

        :param file_name: The file name of the token list file
        :return: list(Terminal)
        """
        fp = open(file_name, "r")
        s = fp.read()
        fp.close()

        # A list of terminals
        ret_list = []

        # A list of non-empty lines
        line_list = [line.strip() for
                     line in
                     s.splitlines() if
                     len(line.strip()) != 0]
        for line in line_list:
            # Skip commented lines
            if line[0] == '#':
                continue

            if line.startswith("TokenType = ") is False:
                raise ValueError("Illegal line: %s" %
                                 (line, ))

            index = line.find(";")
            if index == -1:
                raise ValueError("Illegal line: %s" %
                                 (line, ))

            # Cut the token
            token = \
                line[line.find("=") + 1:line.find(";")].strip()

            ret_list.append(Terminal(token))

        dbg_printf("Read %d tokens from %s",
                   len(ret_list),
                   file_name)

        return ret_list

    def parse(self, file_name):
        """
        Start parsing a file

        :param file_name: The file name
        :return: The final root symbol (i.e. the top symbol after
         acceptance)
        """
        tk = CTokenizer.read_file(file_name)

        # Save then into self to allow action call backs
        # to also access the symbol stack
        self.state_stack = [self.starting_state]
        self.symbol_stack = []

        state_stack = self.state_stack
        symbol_stack = self.symbol_stack

        token = tk.get_next_token()

        while True:
            top_state = state_stack[-1]

            token = self.preprocess_token(token)
            #if token.name == "T_IDENT" or token.name == "T_TYPEDEF_NAME":
            #    print "Probe using", token.data
            k = (top_state, token.name)
            t = self.parsing_table[k]
            action = t[0]

            # If we shift then consume a terminal and
            # goto the next state
            if action == ParserGeneratorLR.ACTION_SHIFT:
                assert(isinstance(t[1], int))
                state_stack.append(t[1])
                symbol_stack.append(token)
                #if token.name == "T_IDENT" or token.name == "T_TYPEDEF_NAME":
                #    print "Shift", token.data

                token = tk.get_next_token()
            elif action == ParserGeneratorLR.ACTION_REDUCE:
                # This is a string denoting the name of the
                # non-terminal; it is not the non-terminal object
                reduce_to = t[1]
                reduce_length = t[2]
                ast_rule = t[3]

                #if reduce_to == "compound-statement":
                #    print t, top_state, token.name

                if ast_rule is None:
                    sn = SyntaxNode(reduce_to)
                    # The third component is the pop length
                    sn.child_list = symbol_stack[-reduce_length:]
                else:
                    # Whether the root node is a new string or
                    # an existing node
                    rename_flag = ast_rule[0]
                    # This is the name of the root node
                    root_name = ast_rule[1]

                    # This is a list of children
                    if len(ast_rule) == 2:
                        child_list = None
                        ast_action = None
                    elif len(ast_rule) == 3:
                        child_list = ast_rule[2]
                        ast_action = None
                    elif len(ast_rule) == 4:
                        child_list = ast_rule[2]
                        ast_action = ast_rule[3]
                    else:
                        raise ValueError("Invalid AST rule")

                    # This fixes the root node
                    if rename_flag is False:
                        assert (isinstance(root_name, int))
                        sn = symbol_stack[-reduce_length + root_name]
                    else:
                        # If it is just a name without token value index
                        # then we just create syntax node for it
                        if isinstance(root_name, str):
                            sn = SyntaxNode(root_name)
                        else:
                            # Otherwise need to get token value also
                            sn = SyntaxNode(root_name[0])
                            sn.data = \
                                symbol_stack[-reduce_length +
                                             root_name[1]].data

                    # Then add its children nodes
                    if child_list is not None:
                        for child in child_list:
                            # If this is a symbol from the stack then
                            # add it as the child node
                            if isinstance(child, int) is True:
                                node = symbol_stack[-reduce_length + child]

                                # Make sure it has no parent and we assign
                                # a parent to it
                                assert(node.parent is None)
                                node.parent = sn

                                sn.append(node)
                            elif isinstance(child, str) is True:
                                new_node = SyntaxNode(child)
                                new_node.parent = sn
                                # If it is a new name then add it also
                                sn.append(new_node)
                            else:
                                # Otherwise it is a new symbol but we need the
                                # token data also
                                new_node = SyntaxNode(child[0])
                                # Get the data on the corresponding location
                                new_node.data = \
                                    symbol_stack[-reduce_length + child[1]].data
                                # Assign the parent node
                                new_node.parent = sn
                                sn.append(new_node)

                # Remove the same number of elements
                for _ in range(0, reduce_length):
                    symbol_stack.pop()
                    state_stack.pop()

                # This is the state we use to compute GOTO
                top_state = state_stack[-1]

                goto_tuple = self.parsing_table[(top_state, reduce_to)]
                assert(goto_tuple[0] == ParserGeneratorLR.ACTION_GOTO)

                # If the AST action is defined then we run the action
                if ast_action is not None:
                    # Process the transformed AST using call backs
                    # defined above
                    callback = ParserLR.REDUCE_ACTION_DICT.get(ast_action, None)
                    # If the call back is defined then we call it with the instance
                    # and also with the AST root
                    # Otherwise throw an exception
                    if callback is not None:
                        sn = callback(self, sn)
                    else:
                        raise ValueError("Could not find action: %s" %
                                         (ast_action, ))

                #if sn.symbol in temp:
                #    print sn.symbol, goto_tuple[1]

                # Push new non-terminal into the list
                symbol_stack.append(sn)

                # Push the new symbol after reduction into the list
                state_stack.append(goto_tuple[1])
            elif action == ParserGeneratorLR.ACTION_ACCEPT:
                break
            else:
                raise ValueError("Unknown parser state: %s" %
                                 (t[0],))

        # Symbol stack and state stack should all have one element
        dbg_printf("Symbol stack: %s", symbol_stack)
        dbg_printf("State stack: %s", state_stack)
        dbg_printf("Scope stack: %s", self.scope_stack)

        assert(len(self.scope_stack) == 1)

        return symbol_stack[0]

#####################################################################
# class ParserEarley - Earley parser implementation
#####################################################################

class ParserEarley(ParserGenerator):
    """
    This class implements the earley parser, which is used for parse a given
    string online - which means that we could start from any non-terminal
    as the root symbol and does not rely on the preset LR states
    """
    def __init__(self, syntax_file_name):
        """
        Read the syntax file by calling super class constructor
        
        :param syntax_file_name: The file name of the syntax file 
        """
        # This reads the syntax file and parses the AST rules
        # We do not call compute first follow routine though, which
        # is traditionally done by LL and LR parser generators
        ParserGenerator.__init__(self, syntax_file_name)

        return

    #################################################################
    # class EarleyState
    #################################################################

    class EarleyState:
        """
        This class represents earley state which is a combination of a set and a list.
        The set is used to maintain uniqueness of states, while the list is used to 
        perform iteration since we do not iterate on items that has already been
        processed
        """
        def __init__(self, index):
            """
            Initialize the Earley state object
            
            :param index: On which position does this state is in
            """
            self.index = index
            # We iterate on this list while appending elements to it
            self.item_list = []
            self.item_set = set()

            return

        def append(self, item):
            """
            This function appends an item if it is not already in the item set
            The item is both added into the set and also appended to the end of the list.
            If this is called all iterators on the item list object will become devoid
            and therefore we should use index to iterate on the list
            
            :param item: The EarleyItem object
            :return: None
            """
            assert(isinstance(item, ParserEarley.EarleyItem) is True)
            if item not in self.item_set:
                self.item_set.add(item)
                self.item_list.append(item)
                # This must hold
                assert(len(self.item_list) == len(self.item_set))

            return

        def __len__(self):
            """
            Returns the number of items stored in this object. Since we maintained
            a consistent item count across the set and the list, any of these two is
            sufficient
            
            :return: int
            """
            assert (len(self.item_list) == len(self.item_set))
            return len(self.item_list)

        def __getitem__(self, index):
            """
            Returns the i-th item in the item list given the index i
            Note that if index out of bound then assertion fails
             
            :param index: The integral index of the item in the item list 
            :return: EarleyItem object
            """
            assert(index < len(self))
            return self.item_list[index]

        def __str__(self):
            """
            Returns a string form of this object
            :return: str
            """
            ret = "[EarleyState[%d]%s]"
            item_str = ""
            for item in self.item_list:
                if item.token_index == 0 and item.could_reduce() is True:
                    item_str += (" " + str(item))

            return ret % (len(self.item_list), item_str)

        __repr__ = __str__

    #################################################################
    # class EarleyItem
    #################################################################

    class EarleyItem(LRItem):
        """
        This class represents an Earley item, which is essentially an LR(0)
        item with an extra index recording its position in the input file
        (index of the token). Note that not all LR item function is callable 
        for this object, and we should only use the most basic ones
        """
        def __init__(self, p, index, token_index, state_index, template_item=None):
            """
            Initialize the base class with a production and index in the production
            
            :param p: The production object 
            :param index: The index of the current position within the production
            :param token_index: The index of the token
            :param state_index: The index of the state this item is appended to
            :param template_item: When advancing an item we need the older item's
                                  child list. If this is None then we initialize
                                  empty child list
            """
            LRItem.__init__(self, p, index)
            self.token_index = token_index

            # This is the index of the state this item is added to
            self.state_index = state_index

            # One of the child it predicts that reduced successfully
            # We also prepare a list of list of possible parse trees
            # for each symbol in the RHS. We will search this forest for
            # possible parsing states
            if template_item is None:
                # This initializes a list object for each slot
                self.child_list_list = [list() for i in range(0, len(self.p))]
            else:
                self.child_list_list = []
                # Also duplicate the child list list
                for i in range(0, len(template_item.child_list_list)):
                    self.child_list_list.append(list())
                    for j in template_item.child_list_list[i]:
                        self.child_list_list[i].append(j)

            return

        def advance(self, state_index=-1):
            """
            Returns a new item object with the dot symbol advanced
            
            :param state_index: The state index for the new item
            :return: EarleyItem
            """
            # There must be something after the dot
            assert (self.get_dotted_symbol() is not None)
            # Make sure there is always a state index
            # while maintaining the same signature as the parent class
            assert(state_index != -1)

            # Use self as a template for copying child lists
            ret = self.__class__(self.p,
                                 self.index + 1,
                                 self.token_index,
                                 state_index,
                                 self)

            # NOTE: DO NOT DO THIS - ALWAYS COPY
            #ret.child_list_list = self.child_list_list

            return ret

        def __hash__(self):
            """
            Need also to hash the index
            :return: int 
            """
            return LRItem.__hash__(self) ^ self.token_index

        def __eq__(self, other):
            """
            Check equality of two objects
            :param other: The other Earley item object
            :return: bool
            """
            return LRItem.__eq__(self, other) and \
                   (self.token_index == other.token_index)

        def __str__(self):
            """
            This function returns a string representation of the object
            
            :return: str 
            """
            ret = "[EarleyItem %s, %s, %s]" % (self.p, self.token_index, self.state_index)
            return ret

        __repr__= __str__

    def parse(self, root_name, s, is_filename):
        """
        Start parsing the given file. We use a tokenizer to tokenize the given
        file
        
        :param root_name: The symbolic name of the root symbol; we will later resolve
                          it to the actual non-terminal object
        :param s: Either the file name or the string to be parsed, depending on
                  the next argument
        :param is_filename: Whether the previous argument is a file name or a 
                            string to be parsed.
        :return: AST root node if success; None if fail
        """
        # if the flag denotes a file name then read the file
        if is_filename is True:
            tokenizer = CTokenizer.read_file(s)
        else:
            # Otherwise just directly construct the tokenizer object
            # using the given string
            tokenizer = CTokenizer(s)

        # This list holds tokens we have processed
        token_list = []

        # First extract all tokens and make them a list
        while True:
            token = tokenizer.get_next_token()
            if token.name == CTokenizer.EOF_TOKEN_NAME:
                break
            else:
                token_list.append(token)

        token_count = len(token_list)
        dbg_printf("Extracted %d tokens from the input", token_count)

        # Corner case should be avoided
        if token_count == 0:
            raise ValueError("Empty input")

        # This is a list of Earley states; the length of this list equals
        # the length of the token list + 1, and we use list comprehension
        # to build the list of states
        state_list = [self.EarleyState(i) for i in range(0, token_count + 1)]

        # Read the root symbol from the symbol dict and do basic checking
        root_symbol = self.symbol_dict.get(root_name, None)
        if root_symbol is None:
            raise ValueError("Non-terminal \"%s\" does not exist" %
                             (root_name, ))
        elif isinstance(root_symbol, NonTerminal) is False:
            raise ValueError("Symbol \"%s\" is not a non-terminal" %
                             (root_name, ))

        # Add all production originating from this symbol into the first
        # state object as initialization
        for p in root_symbol.lhs_set:
            # Append items with dot position at 0 and token index at 0
            state_list[0].append(self.EarleyItem(p, 0, 0, 0))

        # This is the current token index we will scan
        for current_token_index in range(0, token_count + 1):
            # Token index is also state index
            current_state = state_list[current_token_index]

            # If there is a next state then grab the next state object
            # Otherwise we know this is the last state and just set it to
            # None. The None value also serves as an indication to later
            # procedures that we are at the last state
            if current_token_index != token_count:
                next_state = state_list[current_token_index + 1]
            else:
                next_state = None

            # Could not use for loop to increment it since
            # the length of the list will change
            list_index = 0
            # Note that we should check the length each time, as the
            # length of the list will change during this iteration
            while list_index < len(current_state):
                item = current_state[list_index]

                # Three possible outcome:
                #   1. Non-terminal - predict (GOTO)
                #   2. Terminal - scan (SHIFT)          -> Only do this when we are
                #                                          NOT in the last state
                #   3. None object - complete (REDUCE)
                dotted_symbol = item.get_dotted_symbol()
                if isinstance(dotted_symbol, NonTerminal) is True:
                    # Predict all productions and add them into the current state
                    for p in dotted_symbol.lhs_set:
                        current_state.append(
                            self.EarleyItem(p, 0, current_token_index, current_token_index))
                elif next_state is not None and \
                     isinstance(dotted_symbol, Terminal) is True and \
                     token_list[current_token_index].name == dotted_symbol.name:
                    # Advance the dot and add it into the next state
                    # Note that we should set state index accordingly
                    next_state.append(item.advance(current_token_index + 1))
                elif dotted_symbol is None:
                    # This disallows empty reduction as there must be
                    # at least one symbol at the right hand side of the production
                    # we will reduce
                    if item.token_index == current_token_index:
                        raise ValueError("Do not allow empty reduction (\"%s\")" %
                                         (str(item.p), ))

                    # This is the state where the reduced symbol is first predicted
                    from_state = state_list[item.token_index]

                    # This is the symbol that will be reduced
                    reduce_symbol = item.get_reduce_symbol()
                    assert(isinstance(reduce_symbol, NonTerminal) is True)

                    # Could directly iterate here since we do not modify it
                    for from_item in from_state.item_list:
                        # This is the dotted symbol where the reduced symbol is from
                        from_item_dotted_symbol = from_item.get_dotted_symbol()
                        # GOTO using the reduced symbol
                        if isinstance(from_item_dotted_symbol, NonTerminal) is True and \
                           reduce_symbol.name == from_item_dotted_symbol.name:
                            # This is a possible subtree of the symbol we just reduced
                            from_item.child_list_list[from_item.index].append(item)
                            # This must be done after we added the subtree. Also the new
                            # item inherits the subtree we just added
                            # Note that the state index is current state index
                            current_state.append(from_item.advance(current_token_index))

                list_index += 1

        # This contains the finished parsing
        dbg_printf("Last state: %s", str(state_list[-1]))

        # Recover the parse tree from the state list, token list and the
        # root name and return the tree or None
        tree, last_index = self.build_unique_tree(state_list, token_list, root_name)

        # If we did not manage to the last token then parsing fails
        if tree is not None and \
           last_index != token_count:
            dbg_printf("Returned a partial syntax tree, but not all tokens are used")
            return None

        return tree

    @classmethod
    def build_unique_tree(cls, state_list, token_list, root_name):
        """
        This function recovers the parse tree from given states and tokens
        
        :param state_list: The state list as Early states
        :param token_list: The token list from the source
        :param root_name: The string name of the root symbol
        :return: None or SyntaxNode
        """
        # Avoid editor warning
        del token_list

        last_state = state_list[-1]

        # Check last state for an eligible item that completes all
        # symbols in the production with the root symbol and uses
        # all tokens
        for item in last_state.item_list:
            if item.could_reduce() is True and \
               item.get_reduce_symbol().name == root_name and \
               item.token_index == 0:
                # Print the decomposition first
                # This function is totally readonly
                cls.print_state_decomposition(item, 0)

                return cls._build_unique_tree(item, 0)

        dbg_printf("Did not find eligible parsed item in the final state")
        return None, -1

    @classmethod
    def _build_unique_tree(cls, item, next_token_index, depth=0):
        """
        Given an item object, build a list of subtrees using this object. This
        function is called recursively
        
        :param item: The EarleyItem object
        :param next_token_index: The index of the next token. This is recursive variable
        :param depth: For debugging purposes
        :return: tuple(SyntaxTree object or None if tree not unique, next token index)
        """
        dbg_printf("Recovering tree for %s @ token index %d, depth %d",
                   str(item),
                   next_token_index,
                   depth)
        # Add a syntax node and use the LHS of the production as the
        # label of the symbol
        sn = SyntaxNode(item.get_reduce_symbol().name)

        # This is the index of RHS nodes in the production
        rhs_index = 0
        for child_list in item.child_list_list:
            # If it is a terminal then just append it to the syntax node
            if isinstance(item.p[rhs_index], Terminal) is True:
                sn.append(SyntaxNode(item.p[rhs_index].name))
                # Consumed one non-terminal
                next_token_index += 1
                # Also consider the next slot in the production
                rhs_index += 1
                continue

            # These two stores the maximum child length
            max_child_length = -1
            max_child = None

            for child in child_list:
                child_dotted_symbol = child.get_dotted_symbol()
                if child.token_index == next_token_index and \
                   child_dotted_symbol is None:
                    child_length = child.state_index - child.token_index
                    if child_length > max_child_length:
                        max_child = child

            if max_child is None:
                dbg_printf("Did not find a matching item for LHS %s @ token index %d",
                           str(item.p.lhs),
                           next_token_index)
                return None, -1

            # Recursively build subtree
            # Also we pass the next token index
            child_node, next_token_index = \
                cls._build_unique_tree(max_child,
                                       next_token_index,
                                       depth + 1)

            # If no unique subtree then return None
            # Otherwise just append it as child node
            if child_node is None:
                return None, -1
            else:
                sn.append(child_node)

            rhs_index += 1

        dbg_printf("Function returns")
        return sn, next_token_index

    @classmethod
    def print_state_decomposition(cls, current_item, depth=0):
        """
        This function prints the parse sub-trees of a state. It is mainly
        used for debugging purposes.
        
        Note that this function prints out all possible decomposition plans.
        In practice we only select the plan based on a maximum-length principle.
        That is, we select items from left-to-right, and whenever there are
        multiple candidates we always select the one with the longest token
        sequence. This matches the heuristics we as human adopts when reading the 
        source code and are thus convenient.
        
        :param current_item: The item we are decomposing
        :param depth: Recursion variable
        :return: None
        """
        index = 0
        for child_list in current_item.child_list_list:
            # This is the symbol on the index indicated by the index
            current_symbol = current_item.p[index]
            # For terminals there must be no child, so skip it
            if isinstance(current_symbol, Terminal) is True:
                dbg_printf("%sIndex %d Terminal %s", depth * " ", index, current_symbol.name)
            else:
                # Then for each child print this
                for child in child_list:
                    # The item as child must be an EarleyItem object
                    assert(isinstance(child, cls.EarleyItem) is True)

                    dbg_printf("%sIndex %d %s", depth * " ", index, child)
                    cls.print_state_decomposition(child, depth + 1)

            # This is always called
            index += 1

        return

#####################################################################
#####################################################################
#####################################################################
# class ParserGeneratorTestCase - Test cases
#####################################################################
#####################################################################
#####################################################################

class ParserGeneratorTestCase(DebugRunTestCaseBase):
    """
    This is the test case class
    """
    def __init__(self):
        """
        Initialize the testing environment and start testing
        """
        # Note that we did not adopt the new style class definition
        # and therefore must directly refer to the base class
        DebugRunTestCaseBase.__init__(self)

        # Processing command lines
        argv = Argv()

        # Initialize data members that will be used across
        # different test cases
        self.pg = None

        self.run_tests(argv)

        return

    @staticmethod
    def print_parse_tree(t, ident=0):
        """
        This function prints the parse tree recursively

        :param t: The SyntaxNode object
        :param ident: The number of spaces
        :return: None
        """
        prefix = " " * ident

        if not isinstance(t, SyntaxNode):
            print prefix + str(t)
        else:
            print prefix + str(t)
            for symbol in t.child_list:
                ParserGeneratorTestCase.print_parse_tree(symbol,
                                                         ident + 1)

        return

    @TestNode("test_lr")
    def test_lr_parse(self, argv):
        """
        Tests whether LR parse works

        :param argv: Argument vector
        :return: None
        """
        if argv.has_keys("lr") is False:
            dbg_printf("Please use --lr to test parsing")
            return

        if argv.has_keys("token-file") is False:
            dbg_printf("Please use --token-file to specify lex output")
            return

        pg = self.pg
        assert(pg is not None)

        token_file_name = argv.get_all_values("token-file")[0]

        # Then load the token file and parse
        # If accepted the return value is the top symbol
        # on the stack which is also the root of the syntax tree
        root_symbol = pg.parse(token_file_name)

        # This one tests AST transformation
        #root_symbol = walk_ast(root_symbol)
        assert(root_symbol is not None)

        # Print the tree after transformation
        ParserGeneratorTestCase.print_parse_tree(root_symbol)

        return

    @TestNode("test_ll")
    def test_ll_parse(self, argv):
        """
        Interactive mode to display how a string is parsed

        :param argv: Argument vector
        :return: None
        """
        if argv.has_keys("ll") is False:
            dbg_printf("Please use --ll to test LL(1) parser generator")
            return

        pg = self.pg
        pt = pg.parsing_table

        # It's like the following:
        # *p + a * b ? func(1, c * *q) : 2

        test_str = [Terminal("T_STAR"),
                    Terminal("T_IDENT"),
                    Terminal("T_PLUS"),
                    Terminal("T_IDENT"),
                    Terminal("T_STAR"),
                    Terminal("T_IDENT"),
                    Terminal("T_QMARK"),
                    Terminal("T_IDENT"),
                    Terminal("T_LPAREN"),
                    Terminal("T_INT_CONST"),
                    Terminal("T_COMMA"),
                    Terminal("T_IDENT"),
                    Terminal("T_STAR"),
                    Terminal("T_STAR"),
                    Terminal("T_IDENT"),
                    Terminal("T_RPAREN"),
                    Terminal("T_COLON"),
                    Terminal("T_INT_CONST"),
                    Symbol.get_end_symbol()]

        index = 0
        step = 1

        # We use a stack to mimic the behavior of the parser
        stack = [NonTerminal("expression")]
        while len(stack) > 0:
            print step, stack

            step += 1
            top = stack.pop()

            if top.is_terminal() is True:
                if top == Symbol.get_empty_symbol():
                    # Empty symbol does not consume any
                    # tokens in the token stream
                    continue
                elif top == test_str[index]:
                    index += 1
                    continue
                else:
                    raise ValueError("Could not match token: %s @ %d" %
                                     (str(test_str[index]), index))

            pair = (top, test_str[index])
            if pair not in pt:
                dbg_printf("Pair %s (index %d) not in parsing table",
                           pair,
                           index)
                raise ValueError("Could not find entry in parsing table")

            p = pt[pair]
            for i in reversed(p.rhs_list):
                stack.append(i)

        return

    @TestNode()
    def test_ll(self, argv):
        """
        This function tests whether the input file could be read
        and parsed correctly

        :return: None
        """
        if argv.has_keys("ll") is False:
            dbg_printf("Please use --ll to test LL(1) parser generator")
            return

        # The first argument is the file name
        file_name = argv.arg_list[0]
        dbg_printf("Opening file: %s", file_name)

        # Initialize the object - it will read the file
        # and parse its contents
        self.pg = ParserGeneratorLL1(file_name)
        pg = self.pg

        dbg_printf("Root symbol: %s", pg.root_symbol)

        # Check the identity of symbols
        for i in pg.terminal_set:
            assert(i.is_terminal() is True)

        for i in pg.non_terminal_set:
            assert(i.is_non_terminal() is True)

        for p in pg.production_set:
            if p.first_set != p.compute_substring_first():
                print p
                print p.first_set
                print p.compute_substring_first()

            assert(p.first_set == p.compute_substring_first())

        # Finally dump the resulting file
        pg.dump(file_name + ".dump")
        pg.dump_parsing_table(file_name + ".table")

        return

    @TestNode()
    def test_lr(self, argv):
        """
        This function tests LR parser generator

        :param argv: Argument vector
        :return: None
        """
        if argv.has_keys("slr", "lr1", "lalr", "lr") is False:
            dbg_printf("Please use --lr1 or --slr or --lalr to" +
                       " test LR parser generator; or --lr to test" +
                       " parsing")
            return

        # The first argument is the file name
        file_name = argv.arg_list[0]
        dbg_printf("Opening file: %s", file_name)

        # Initialize the object - it will read the file
        # and parse its contents
        # If no parser type is specified then we just think
        # the file given is the parsing table
        if argv.has_keys("slr"):
            self.pg = ParserGeneratorLR(file_name,
                                        ParserGeneratorLR.LR_TYPE_SLR)
        elif argv.has_keys("lr1"):
            self.pg = ParserGeneratorLR(file_name,
                                        ParserGeneratorLR.LR_TYPE_LR1)
        elif argv.has_keys("lalr"):
            self.pg = ParserGeneratorLR(file_name,
                                        ParserGeneratorLR.LR_TYPE_LALR)
        elif argv.has_keys("lr"):
            # If no parser type is specified just load the parsing
            # table
            self.pg = ParserLR(file_name)

        pg = self.pg

        dbg_printf("Starting state: %d", pg.starting_state)

        # If there is request to dump the parsing table
        if argv.has_keys("dump-file"):
            dump_file_name = argv.get_all_values("dump-file")[0]
            pg.dump_parsing_table(dump_file_name)

        pg.dump_terminal_enum("symbols.h")

        return

    @classmethod
    @TestNode()
    def test_earley_parse(cls, argv):
        """
        This function tests Earley parsing
        
        :param argv: Argument vector
        :return: None
        """
        if argv.has_keys("earley") is False:
            dbg_printf("Please use --earley to run Earley parser")
            return
        elif argv.has_keys("token-file") is False:
            dbg_printf("Please use --token-file to specify the input source file")
            return
        elif len(argv.arg_list) < 1:
            dbg_printf("Please specify the syntax file as the first argument")
            return

        # This is the syntax file name
        syntax_file_name = argv.arg_list[0]
        source_file_name = argv.get_all_values("token-file")[0]
        dbg_printf("Syntax file: %s", syntax_file_name)
        dbg_printf("Source file: %s", source_file_name)

        pe = ParserEarley(syntax_file_name)
        tree = pe.parse("declaration", "typedef int *(*a[])(void);", False)

        if tree is None:
            dbg_printf("Failed to parse")
        else:
            cls.print_parse_tree(tree)

        pe = ParserEarley(syntax_file_name)
        tree = pe.parse("root", "int main() {const char *p = &(\"This is ia string\") + 1; printf(\"Hello World!\\n\"); return 0;}", False)

        if tree is None:
            dbg_printf("Failed to parse")
        else:
            cls.print_parse_tree(tree)

        return

if __name__ == "__main__":
    ParserGeneratorTestCase()
