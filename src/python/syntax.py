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

        return

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
    def __init__(self, pg, lhs, rhs_list=[]):
        """
        Initialize the production object

        :param pg: The ParserGenerator object
        :param lhs: Left hand side symbol name (string name)
        :param rhs_list: A list of right hand side symbol names
        """
        # Make sure that the lhs is a non terminal symbol
        assert(lhs.is_non_terminal() is True)
        # Also make sure that every object in RHS is a symbol
        # object
        for rhs in rhs_list:
            assert(rhs.is_terminal() is True or
                   rhs.is_non_terminal() is True)

        self.pg = pg
        self.lhs = lhs
        self.rhs_list = rhs_list

        return

    def __getitem__(self, item):
        """
        This mimics the list syntax

        :param item: The index
        :return: The i-th object in the rhs list
        """
        return self.rhs[item]

    def append(self, item):
        """
        This mimics the list syntax of appending a new element
        at the back of rhs_list. We also do extra checking to
        make sure that the item is a symbol object

        :param item: The new syntax node
        :return: None
        """
        assert(iten.is_symbol() is True)
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
        for line in line_list:
            if line[-1] == ':':
                current_nt = line[:-1]
                continue

            # There must be a non-terminal node to use
            if current_nt is None:
                raise ValueError("The syntax must start with a non-terminal")

            # Otherwise we know this is a new production
            production = Production(current_nt)

            # This is a list of symbol names
            # which have all been converted into terminals
            # or non-terminals in the first pass
            symbol_list = line.split()
            for symbol in



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

        # Check the identity of symbols
        for i in pg.terminal_set:
            assert(i.is_terminal() is True)

        for i in pg.non_terminal_set:
            assert(i.is_non_terminal() is True)

        return

if __name__ == "__main__":
    ParserGeneratorTestCase()