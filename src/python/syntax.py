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
    # This is the name of the "eps" symbol
    EMPTY_SYMBOL_NAME = "T_"

    # We must initialize this later because terminal object has
    # not yet been defined
    EMPTY_SYMBOL = None

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
        if Symbol.EMPTY_SYMBOL is None:
            Symbol.EMPTY_SYMBOL = \
                Terminal(Symbol.EMPTY_SYMBOL_NAME)

        return Symbol.EMPTY_SYMBOL

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

        :return: NonTerminal object with a synthesized name
        """
        # Synthesize a new name
        new_name = self.name + "-" + str(self.new_name_index)
        self.new_name_index += 1

        return NonTerminal(new_name)

    def eliminate_left_recursion(self):
        """
        This function eliminates direct left recursion for the
        current non-terminal symbol. We do not deal with
        indirect ones here

        :return: None
        """
        # If there is no direct left recursion then return
        # directly
        if self.exists_direct_left_recursion() is False:
            return

        return

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
# class Production
#####################################################################

class Production:
    """
    This class represents a single production rule that has a left
    hand side non-terminal symbol and
    """
    def __init__(self, pg, lhs):
        """
        Initialize the production object

        :param pg: The ParserGenerator object
        :param lhs: Left hand side symbol name (string name)
        :param rhs_list: A list of right hand side symbol names
        """
        # Make sure that the lhs is a non terminal symbol
        assert(lhs.is_non_terminal() is True)

        self.pg = pg
        self.lhs = lhs

        # We append elements into this list later
        self.rhs_list = []

        return

    def __getitem__(self, item):
        """
        This mimics the list syntax

        :param item: The index
        :return: The i-th object in the rhs list
        """
        return self.rhs_list[item]

    def append(self, item):
        """
        This mimics the list syntax of appending a new element
        at the back of rhs_list. We also do extra checking to
        make sure that the item is a symbol object

        :param item: The new syntax node
        :return: None
        """
        assert(item.is_symbol() is True)
        self.rhs_list.append(item)

        return

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
# class ParserGenerator
#####################################################################

class ParserGenerator:
    """
    This class is the main class we use to generate the parsing code
    for a given LL(1) syntax
    """
    def __init__(self, file_name):
        """
        Initialize the generator object
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

        # Reading the file
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

        # This function recognizes terminals and non-terminals
        # and stores them into the corresponding set structure
        self.process_symbol(line_list)
        # This function adds productions and references between
        # productions and symbols
        self.process_production(line_list)
        # This sets self.root_symbol and throws exception is there
        # is problem finding it
        self.process_root_symbol()

        # TODO: Add transformation here
        #self.verify()

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

            # Otherwise we know this is a new production
            production = Production(self, current_nt)

            # This is a list of symbol names
            # which have all been converted into terminals
            # or non-terminals in the first pass
            symbol_list = line.split()
            for symbol_name in symbol_list:
                # The key must exist because we already had one
                # pass to add all symbols
                assert(symbol_name in self.symbol_dict)

                symbol = self.symbol_dict[symbol_name]
                production.append(symbol)

                # If a production rule refers to a non-terminal
                # then we need to also add the production back
                # to the non-terminal as a backward reference
                if symbol.is_non_terminal() is True:
                    symbol.rhs_set.add(production)

            # The same production must not appear twice
            # otherwise it is an input error
            if production in self.production_set:
                raise ValueError("Duplicated production: %s" %
                                 (str(production), ))

            # After appending all nodes we also add the production
            # into the set pf productions
            self.production_set.add(production)
            current_nt.lhs_set.add(production)

        return

    def process_symbol(self, line_list):
        """
        Recognize symbols, and store them into the dictionary for
        symbols, as well as the sets for terminals and non-terminals

        :param line_list: A list of lines
        :return:
        """
        # This is a set of names for which we are not certain whether
        # it is a terminal or non-terminal
        in_doubt_set = set()

        for line in line_list:
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

    @TestNode()
    def test_read_file(self, argv):
        """
        This function tests whether the input file could be read
        and parsed correctly

        :return: None
        """
        print_test_name()

        # The first argument is the file name
        file_name = argv.arg_list[0]
        dbg_printf("Opening file: %s", file_name)

        # Initialize the object - it will read the file
        # and parse its contents
        self.pg = ParserGenerator(file_name)
        pg = self.pg

        dbg_printf("Terminals: %s", str(pg.terminal_set))
        dbg_printf("Non-Terminals: %s", str(pg.non_terminal_set))
        dbg_printf("Productions:")
        for symbol in pg.non_terminal_set:
            for p in symbol.lhs_set:
                dbg_printf("%s", str(p))

        dbg_printf("Root symbol: %s", pg.root_symbol)

        # Check the identity of symbols
        for i in pg.terminal_set:
            assert(i.is_terminal() is True)

        for i in pg.non_terminal_set:
            assert(i.is_non_terminal() is True)

        # Manually build the set and print them
        for symbol in pg.non_terminal_set:
            symbol.build_first_rhs_set()
            dbg_printf("RHS set for %s: %s",
                       str(symbol),
                       str(symbol.first_rhs_set))

        return

if __name__ == "__main__":
    ParserGeneratorTestCase()