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

#####################################################################
# class Tokenizer
#####################################################################

class Tokenizer:
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
        self.s = s
        self.index = 0

        return

    @staticmethod
    def read_file(file_name):
        """
        This function opens a file and returns an instance of the
        tokenizer initialized using the string read from the file

        :return: Tokenizer
        """
        # Open the file, read the string and then close
        fp = open(file_name)
        s = fp.read()
        fp.close()

        return Tokenizer(s)

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