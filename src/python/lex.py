#
# This file includes a C tokenizer
#

import sys
from common import *

#####################################################################
# class Token
#####################################################################

class Token:
    """
    This file represents a token from the C source file
    """
    def __init__(self, name, data):
        """
        Initializes a token using a name and an optional data field

        :param name: The name of the token, e.g. "T_IDENT"
        :param data: The data associated with the token. For symbol
         and keyword types this is None. For identifiers and literals
         this contains the string literal, the semantics of which will
         be resolved later
        """
        self.name = name
        self.data = data

        return

    def __repr__(self):
        """
        Returns the string representation of the object

        :return: str
        """
        if self.data is None:
            return "[Token %s]" % (self.name, )
        else:
            return "[Token %s %s]" % (self.name, self.data)

    def __str__(self):
        """
        Return string representation of this object

        :return: str
        """
        return self.__repr__()


#####################################################################
# class Tokenizer
#####################################################################

class Tokenizer:
    # This represents the EOF character, and it is returned
    # from the peek function. We use 0x00 such that it would fail
    # all comparisons to printable characters
    EOF_CHAR = chr(0x00)

    """
    This class represents the tokenizer that decomposes a C source
    program into a series of tokens
    """
    def __init__(self, s):
        """
        Initializes the tokenizer with a string which is the
        source code

        :param s: The C program
        """
        # Add a new line to the end of the input always
        if s[-1] != '\n':
            self.s = s + '\n'
        else:
            self.s = s

        self.index = 0

        # This will be updated while we scan the file
        self.row = 1
        self.col = 1

        return

    @classmethod
    def read_file(cls, file_name):
        """
        This function opens a file and returns an instance of the
        tokenizer initialized using the string read from the file

        Note that this is made a class method such that the child
        class instance will be returned if called through the
        child class

        :return: Tokenizer
        """
        # Open the file, read the string and then close
        fp = open(file_name)
        s = fp.read()
        fp.close()

        return cls(s)

    @staticmethod
    def is_ident_char(ch, first_char):
        """
        Returns whether a character is valid character for
        constituting an identifier, i.e. alphabet or underline

        :param ch: The character (one char string)
        :param first: Whether this is the first char of
                      an ident
        :return: bool
        """
        return ch.isalpha() or (not first_char and ch.isdigit()) or ch == "_"

    @staticmethod
    def is_dec_digit(ch):
        """
        Returns whether the character is valid digit

        :param ch: The character
        :return: bool
        """
        return ch.isdigit()

    @staticmethod
    def is_hex_digit(ch):
        """
        Returns whether a character is valid character for
        a hex digit

        :param ch: The character
        :return: bool
        """
        return ch.isdigit() or \
               'A' <= ch <= 'F' or \
               'a' <= ch <= 'f'

    @staticmethod
    def is_oct_digit(ch):
        """
        Returns whether a character is a valid OCT digit

        :param ch: The character
        :return: bool
        """
        return '0' <= ch <= '7'

    def update_row_col(self, prev_index):
        """
        This function counts the number of new line characters
        between the prev_index and current index. Note that current
        index is not considered

        :param prev_index: Starting point
        :return: int
        """
        assert(0 <= prev_index < len(self.s))

        i = prev_index
        while i < self.index:
            ch = self.s[i]
            if ch == '\n':
                self.row += 1
                self.col = 1
            else:
                self.col += 1

            i += 1

        return

    def advance(self, offset=1):
        """
        Advance the index by offset. If the resulting index is
        invalid an exception is thrown

        If the offset is not given then by default we move the
        pointer by 1

        :param offset: The offset we need to move the pointer
        :return: None
        """
        new_index = self.index + offset
        # If it is before the input or after the end of input
        # then we raise an exception
        # Valid range is [0, len - 1] however len itself should
        # also be a valid position
        if new_index < 0 or new_index > len(self.s):
            raise ValueError("Could not move the pointer beyond" +
                             " the input stream")

        # Save it to update row and col later
        prev_index = self.index
        # Move the pointer
        self.index = new_index

        # Update row and column based on how many new lines
        # and how many characters we have jumped over
        self.update_row_col(prev_index)

        return

    def get_next_char(self):
        """
        Returns the next unread character, and advance the index by 1.
        If the index is already on the end an exception will be thrown

        :return: str
        """
        # Index could not pass even the end of the string
        assert(self.index <= len(self.s))
        if self.index == len(self.s):
            raise ValueError("Already reached the end of the input")

        # Get the character and advance the index
        ch = self.s[self.index]
        index += 1

        # Since we just consumed one character
        self.update_row_col(index - 1)

        return ch

    def peek_char(self, offset):
        """
        Peek character from the current position given an offset

        :param offset: The offset
        :return: Character in the input, or EOF
        """
        assert(self.index <= len(self.s))
        # This is the index we peek into
        peek_index = self.index + offset
        if peek_index >= len(self.s) or peek_index < 0:
            return Tokenizer.EOF

        return self.s[peek_index]

    def starts_with(self, pattern):
        """
        Check whether the current read position starts with
        the given pattern

        Even if the pattern is longer than the input string
        we still perform the check and returns False

        :param pattern: A string
        :return: bool
        """
        offset = 0
        # If one character in pattern is not matched we just
        # return False
        for ch in pattern:
            if ch != self.peek_char(offset):
                return False
            offset += 1

        return True

    def scan_until(self, callback):
        """
        This function scans the input till the given call back
        evaluates to True, or end of input is reached

        The call back receives "self" as its argument. If it
        returns True then we terminate scanning the input and
        return. The call back could also modify "self".

        If the end of file is reached before call back returns
        True, we also return.

        If call back causes an exception to be thrown, we do
        not catch it and let it propagate to the upper level

        This function also updates row and col if the pattern is
        matched successfully. Otherwise the row and col is
        not updated

        :return: False if end of input is reached; True if the
                 pattern is matched
        """
        assert(self.index <= len(self.s))

        # Save this for updating row and col
        prev_index = self.index

        while self.index < len(self.s):
            if callback(self) is True:
                self.update_row_col(prev_index)

                return True

            self.index += 1

        # Note that we do not update row and col if
        # this happens
        return False

    def scan_until_pattern(self, pattern, inclusive=False):
        """
        Scans the input until the pattern is met.

        The return value is the same as scan_until(). If inclusive
        is set to True, then after the pattern is matched we also
        jump the pattern (if the pattern is not matched we just
        return because the file has been exhausted)

        Row and col is not updated if pattern is not matched

        :param pattern: The string pattern we need to match
        :param inclusive: Whether the pattern should be skipped
                          if it is matched
        :return: The same as scan_until()
        """
        # Note that pattern is in the closure of the lambda
        # so we could use it directly
        ret = self.scan_until(lambda tk: tk.starts_with(pattern))

        # If the pattern is not matched just return False
        if ret is False:
            return False

        # If the pattern is matched then we check the flag
        # and skip the pattern itself. This is safe since we know
        # the pattern has been matched
        if inclusive is True:
            self.advance(len(pattern))

        return True

    def skip_space(self):
        """
        This function skips space characters in the input. It
        is guaranteed for this function to return without throwing
        an exception because we know the end character must be
        a new line character

        :return: None
        """
        # This stops at the first non-space character
        # which might be EOF
        ret = \
            self.scan_until(lambda tk: not tk.s[tk.index].isspace())
        assert(ret is True)

        return


#####################################################################
# class CTokenizer
#####################################################################

class CTokenizer(Tokenizer):
    """
    This class is the C language tokenizer that recognizes the input
    into different tokens
    """
    def __init__(self, s):
        """
        Initialize the base class

        :param s: The string to tokenize
        """
        Tokenizer.__init__(self, s)

        return

    def skip_line_comment(self):
        """
        Skips line comments. Line comments end with new line
        character or EOF

        This function does not throw any exception. This function
        also skips the preceding "//" automatically

        :return: None
        """
        assert(self.peek_char(0) == '/' and self.peek_char(1) == '/')

        # Because we need to skip "//"
        self.advance(2)

        # This must always succeed because we always append "\n" to
        # the end of the input
        ret = self.scan_until_pattern("\n", True)
        assert(ret is True)

        return

    def skip_block_comment(self):
        """
        This function skips a block comment. If the input ends
        prematurely then an exception will be thrown

        This function also skips the leading /*

        :return: None
        """
        assert(self.peek_char(0) == '/' and self.peek_char(1) == '*')
        self.advance(2)

        # Also eat the comment end mark
        ret = self.scan_until_pattern("*/", True)
        # It is possible for block comments to pass the
        # end of the input, so check return value here
        if ret is False:
            raise ValueError("Block comment passes" +
                             " the end of the input")

        return

    def clip_string_literal(self):
        """
        This function clips a string literal from the input

        If the string is not closed we throw an exception

        :return: Token with the string literal's literal as data
                 and type being T_STRING_CONST
        """
        assert(self.peek_char(0) == "\"")
        self.advance(1)
        prev_index = self.index

        # Scan the input until we see an unescaped "\""
        ret = self.scan_until(lambda tk: tk.peek_char(0) == "\"" and \
                                         tk.peek_char(-1) != "\\")
        if ret is False:
            raise ValueError("Unclosed string literal")
        else:
            # Skip the closing "\""
            self.advance(1)

        return Token("T_STRING_CONST", self.s[prev_index:self.index - 1])

    def clip_char_literal(self):
        """
        This function clips a char literal from the input.
        If the char is not closed we throw an exception.

        This function is just copied from clip_string_literal()
        with minor modification

        :return: Token with the char's literal as data and type
                 being T_CHAR_CONST
        """
        assert (self.peek_char(0) == "\'")
        self.advance(1)
        prev_index = self.index

        # Scan the input until we see an unescaped "\'"
        ret = self.scan_until(lambda tk: tk.peek_char(0) == "\'" and \
                                         tk.peek_char(-1) != "\\")
        if ret is False:
            raise ValueError("Unclosed char literal")
        else:
            # Skip the closing "\'"
            self.advance(1)

        return Token("T_CHAR_CONST", self.s[prev_index:self.index - 1])

#####################################################################
#####################################################################
#####################################################################
# class TokenizerTestCase - Test cases
#####################################################################
#####################################################################
#####################################################################

class TokenizerTestCase(DebugRunTestCaseBase):
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

        # Start running test cases
        self.run_tests(argv)

        return

    @staticmethod
    @TestNode()
    def test_scan(_):
        """
        This function tests scan

        :param _: Unused argv
        :return: None
        """
        # This is a call back that judges whether we have seen
        # an end of comment
        end_of_comment_cb = lambda t: t.peek_char(0) == '*' and \
                                      t.peek_char(1) == '/'

        tk = Tokenizer("/*       /* **\     **/ This is a comment")
        ret = tk.scan_until(end_of_comment_cb)
        assert(ret is True)
        assert(tk.s[tk.index:] == "*/ This is a comment\n")

        tk.index = 0
        ret = tk.scan_until_pattern("*/", True)
        assert (ret is True)
        assert (tk.s[tk.index:] == " This is a comment\n")

        tk = Tokenizer("/*       /* **\     ** This is a comment")
        ret = tk.scan_until(end_of_comment_cb)
        assert(ret is False)
        assert(tk.index == len(tk.s))

        tk.index = 0
        ret = tk.scan_until_pattern("*/", True)
        assert (ret is False)
        assert (tk.index == len(tk.s))

        return

    @staticmethod
    @TestNode("test_scan")
    def test_skip(_):
        """
        This function tests whether various skip function works

        :param _: Unused argv
        :return: None
        """
        s = "\n    \n\r\n\v\r\n     EOF "
        tk = CTokenizer(s)
        tk.skip_space()
        dbg_printf("Tokenizer now @ row %d col %d", tk.row, tk.col)
        assert(tk.row == 5 and tk.col == 6)

        s = "// This is a line comment\n "
        tk = CTokenizer(s)
        tk.skip_line_comment()
        dbg_printf("Tokenizer now @ row %d col %d", tk.row, tk.col)
        assert (tk.row == 2 and tk.col == 1)

        s = "/* This is a block comment \n\n With two new lines */ "
        tk = CTokenizer(s)
        tk.skip_block_comment()
        dbg_printf("Tokenizer now @ row %d col %d", tk.row, tk.col)
        assert (tk.row == 3 and tk.col == 23)

        return

    @staticmethod
    @TestNode("test_skip")
    def test_clip_literal(_):
        """
        This function tests whether we could clip literals

        :param _: Unused argv
        :return: None
        """
        s = "\"This is a string \\n literal \\\" literal \\\" \"\n  "
        tk = CTokenizer(s)
        token = tk.clip_string_literal()
        dbg_printf("%s", token)
        dbg_printf("Tokenizer now @ row %d col %d", tk.row, tk.col)
        assert (tk.row == 1 and tk.col == 45)

        s = "'ch\\''"
        tk = CTokenizer(s)
        token = tk.clip_char_literal()
        dbg_printf("%s", token)
        dbg_printf("Tokenizer now @ row %d col %d", tk.row, tk.col)
        assert (tk.row == 1 and tk.col == 7)

        return

if __name__ == "__main__":
    TokenizerTestCase()