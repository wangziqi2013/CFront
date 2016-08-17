
#pragma once

// Old C headers
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>

// New C++ and C++11 headers
#include <string>
#include <vector>
#include <functional>

#include "token.h"

class SourceFile {
 private:
  // The name of the source file
  std::string filename;
  
  // The actual data of the source file
  // We hold the content of the data in memory until the file
  // has been tokenized
  //
  // Note that this is not the pointer to allocated array after
  // construction since it have been moved forward by 1 byte
  char *data;
  
  // The current position we are reading from
  int current_index;
  
  // The length of the source file, also the length of data array
  // Note that we coule not represent data more than 4GB this way,
  // but such a large source file is not expected
  int file_length;
  
  // These two records how many lines and how many characters in current line
  // we have read
  //
  // NOTE: Both should start with 1 since it is the usually way of counting line
  // and columns
  int current_line;
  int current_column;

 public:

  /*
   * Construct - copy data from an external data source into its own buffer
   */
  SourceFile(const char *p_data, int p_file_length) :
    filename{""},
    data{nullptr},
    current_index{0},
    file_length{p_file_length},
    current_line{1},
    current_column{1} {
    // Allocate two bytes more for EOF sentinel
    data = new char[file_length + 2];
    
    std::memcpy(data + 1, p_data, static_cast<size_t>(file_length));
    
    data[file_length + 1] = EOF;
    data[0] = EOF;
    
    // Make sure that we could always read the previous char of
    // the current one
    data++;
    
    return;
  }
   
  /*
   * Constructor - Opens a file, reads its content into data and return
   */
  SourceFile(std::string p_filename) :
    filename{p_filename},
    data{nullptr},
    current_index{0},
    file_length{0},
    current_line{1},
    current_column{1} {
    // Open the file, and if it fails throw an exception
    FILE *fp = fopen(filename.c_str(), "r");
    if(fp == nullptr) {
      ThrowOpenFileError();
    }
    
    int ret;
    
    ret = fseek(fp, 0L, SEEK_END);
    if(ret != 0) {
      ThrowOtherFileError();
    }
    
    file_length = static_cast<int>(ftell(fp));
    if(file_length == -1) {
      ThrowOtherFileError();
    } else if(file_length == 0) {
      // We do not allow empty file to be opened
      // since they cause trouble for operator new
      ThrowFileEmptyError();
    }

    // Move back to the first byte of the file
    ret = fseek(fp, 0L, SEEK_SET);
    if(ret != 0) {
      ThrowOtherFileError();
    }
    
    // Then allocate data
    // If allocation fails there will be a std::bad_alloc being thrown
    // so we do not need to worry about allocation failure
    //
    // We allocate two more byte to fill it with EOF as sentinels
    // and valid file index is [0, file_length - 1]
    data = new char[file_length + 2];
    
    // Read the entire file into memory
    //
    // We read from offset 1 rather than offset 0
    ret = static_cast<int>(fread(data + 1,   // Read into offset 1
                                 static_cast<size_t>(file_length),
                                 1,
                                 fp));
    if(ret != file_length) {
      ThrowOtherFileError();
    }
    
    ret = fclose(fp);
    if(ret != 0) {
      ThrowOtherFileError();
    }
    
    // Fill the first and last character as EOF to facilitate processing
    // Note that the data array length is file_length + 2
    data[file_length + 1] = EOF;
    data[0] = EOF;
    
    // Let data advance by 1 since we want data[0] to be the first byte
    // in the file
    data++;
    
    return;
  }
  
  /*
   * Destructor - Frees the data array if it has been allocated
   *
   * Note that since the data array has been advanced by 1 byte so we
   * delete the pointer data - 1
   */
  ~SourceFile() {
    // Must use delete[] to match new[] and delete[]
    delete[] (data - 1);
    
    return;
  }
  
  /*
   * Deleted Functions
   *
   * The following constructors AND overloads are deleted since we
   * want to avoid memory leak when copying this object
   */
  SourceFile(const SourceFile &) = delete;
  SourceFile(SourceFile &&) = delete;
  SourceFile &operator=(const SourceFile &) = delete;
  SourceFile &operator=(SourceFile &&) = delete;
  
  ///////////////////////////////////////////////////////////////////
  // Character Handling - Get, Peek, and more
  ///////////////////////////////////////////////////////////////////
  
  /*
   * PeekNextChar() - Returns the current character without moving the reading
   *                  pointer
   *
   * Note that for this function the current position must be valid or one
   * after vaid range
   */
  inline char PeekNextChar() const noexcept {
    assert(current_index <= file_length);
    
    return data[current_index];
  }
  
  /*
   * PeekChar() - Returns a character on a given offset
   *
   * The offset could be positive or negative. When the character
   * if positive the target offset is checked against the end of file
   * and if it exceeds end of file EOF is returned. Otherwise the character
   * on the target position is returned
   *
   * NOTE: For negative offset no checking is performed, so the caller
   * should check whether the target index is valid or not
   */
  inline char PeekChar(int offset) const noexcept {
    int read_offset = current_index + offset;
    
    // valid range is [0, file_length - 1]
    return (read_offset < file_length) ? data[read_offset] : EOF;
  }
  
  /*
   * GetNextChar() - Returns current character and return
   *
   * If we have reached the end of file return EOF. Otherwise return the char
   * at current read position, and then increament the read position by 1
   *
   * This function could also be used merely to advance the pointer
   * in that case the return value will be discarded and could hopefully be
   * optimized out by the compiler
   *
   * NOTE: It is important that only this function is called for moving
   * the current read position, since we need this function moving the cursor
   * one by one to detect new line and count column number
   *
   * NOTE: This functions does not stop advancing the pointer after we have
   * reached file length. But it returns EOF when the last byte in the file
   * has been passed. So caller should check whether the byte returned
   * by this function is EOF or not, and should not advance the pointer
   * blindly
   */
  char GetNextChar() noexcept {
    // Current index could not be greater than the file length, since we
    // could only increase the index by 1 each time
    assert(current_index < file_length);
    
    // This even works with the first character in the file since we filled
    // the byte before the 1st byte of the file as EOF
    bool is_start_of_line = (data[current_index - 1] == '\n');
    
    // Update line and column
    current_line += static_cast<int>(is_start_of_line);
    current_column = is_start_of_line ? 1 : current_column + 1;
    
    return data[current_index++];
  }
  
  /*
   * GetNextCharOrEof() - Gets next char if there is one, or EOF
   *
   * This function returns EOF if we have reached end of file, or
   * otherwise calls GerNextChar() to extract next char and advance the
   * pointer by 1
   */
  char GetNextCharOrEof() noexcept {
    if(IsEof() == true) {
      return EOF;
    }

    return GetNextChar();
  }
  
  ///////////////////////////////////////////////////////////////////
  // Token Type Judgement
  ///////////////////////////////////////////////////////////////////
  
  /*
   * IsEof() - Returns true if there is nothing more to read
   *
   * Please note that since there are two sentinels wrapping the data
   * the valid range for data is [1, file_length], and file_length + 1
   * is explicitly filled with EOF to denote end of file
   */
  inline bool IsEof() const noexcept {
    assert(current_index <= file_length);

    return current_index == file_length;
  }
  
  /*
   * IsNumeric() - Returns whether the current character is the numeric
   *
   * If the current character is between 0 and 9 then return true
   * Otherwise return false
   *
   * NOTE: This function could correctly detect decimal, octal and hex (0x)
   * but negative numbers are not detected, since unary "-" is treated as
   * an operator, we do not need to recognize negative constant
   */
  inline bool IsNumeric() const noexcept {
    char ch = PeekNextChar();
    
    return (ch >= '0') && (ch <= '9');
  }
  
  /*
   * IsIdentifier() - Returns whether the current character starts a keyword
   *                  or user-defined identifier
   *
   * It checks for a-z, A-Z and _. Note that it does not check for numerics
   * which is also valid inside an identifier as long as it is not the first
   * one
   */
  inline bool IsIdentifier() const noexcept {
    char ch = PeekNextChar();
    
    return ((ch >= 'a') && (ch <= 'z')) || \
           ((ch >= 'A') && (ch <= 'Z')) ||
           (ch == '_');
  }
  
  /*
   * IsSpace() - Returns whether the current character is space
   *
   * Space includes ' ', '\n' '\r' and '\t'
   */
  inline bool IsSpace() const noexcept {
    char ch = PeekNextChar();

    return (ch == ' ') || (ch == '\n') || (ch == '\t') || (ch == '\r');
  }
  
  /*
   * IsLineComment() - Returns true if the next two characters starts
   *                   line comment
   */
  inline bool IsLineComment() const noexcept {
    char ch = PeekNextChar();
    char ch2 = PeekChar(1);
    
    // If the next two characters form "//" then it is a line comment
    return (ch == '/') && (ch2 == '/');
  }
  
  /*
   * IsBlockComment() - Returns true if the next two characters starts
   *                    a block comment of /* and  * /
   *
   * Please note that block commens are not nested
   */
  inline bool IsBlockComment() const noexcept {
    char ch = PeekNextChar();
    char ch2 = PeekChar(1);

    // If the next two characters form "//" then it is a line comment
    return (ch == '/') && (ch2 == '*');
  }
  
  /*
   * IsStringLiteral() - Returns true if the current position has an "\""
   */
  inline bool IsStringLiteral() const noexcept {
    return PeekNextChar() == '\"';
  }
  
  /*
   * IsCharLiternal() - Returns true if the current position has '\''
   */
  inline bool IsCharLiteral() const noexcept {
    return PeekNextChar() == '\'';
  }
  
  /*
   * IsHexDigit() - Returns whether the current digit is hex digit
   */
  inline bool IsHexDigit() const noexcept {
    char ch = PeekNextChar();
    
    return (ch >= '0' && ch <= '9') || \
           (ch >= 'a' && ch <= 'f') || \
           (ch >= 'A' && ch <= 'F');
  }
  
  /*
   * IsDecDigit() - Returns whether the current digit is decimal digit
   */
  inline bool IsDecDigit() const noexcept {
    char ch = PeekNextChar();
    
    return ch >= '0' && ch <= '9';
  }
  
  /*
   * IsOctDigit() - Returns whether the current digit is oct digit
   */
  inline bool IsOctDigit() const noexcept {
    char ch = PeekNextChar();
    
    return ch >= '0' && ch <= '7';
  }
  
  /*
   * IsHexValueLiteral() - Determine whether current stream has a hex value
   *                       literal
   *
   * We detect hex value literal using "0x" - If there is a "0x" then it is hex
   * Also there must be at least 1 numeric after the 0x notation
   */
  inline bool IsHexValueLiteral() const noexcept {
    char ch = PeekNextChar();
    char ch2 = PeekChar(1);
    
    return (ch == '0' && ch2 == 'x');
  }
  
  /*
   * IsOctValueLiteral() - Determine whether current stream is an oct value
   *                       literal
   *
   * It checks whether the leading numeric digit is '0'. Note that in the
   * caller we should not call GetNextChar() to jump over the leading 0, since
   * if the literal is a single "0" then there is no digit after that
   */
  inline bool IsOctValueLiteral() const noexcept {
    return PeekNextChar() == '0';
  }
  
  ///////////////////////////////////////////////////////////////////
  // Token Handling - Skip
  ///////////////////////////////////////////////////////////////////
  
  /*
   * SkipSpace() - Skips all space characters including ' ', '\n', '\t'
   *
   * It is possible that after this function returns, the current position
   * exceeds file length. It is a totally legal state.
   */
  void SkipSpace() noexcept {
    while(IsSpace() == true) {
      GetNextChar();
    }
    
    return;
  }
  
  /*
   * SkipLine() - Jumps to the starting of the next line or EOF
   *
   * This function is usually used to jump over line comment
   * If there is not a next line because we have reached EOF then this
   * function just returns on seeing EOF, and it should be detected
   * by the next loop inside the caller typically
   */
  void SkipLine() noexcept {
    // This function must be used to skip line comment
    assert(IsLineComment() == true);
    
    // If either of this is true then we know we have seen the end of line
    // Note that IsEof() is necessary since GetNextChar() could not be called
    // when IsEof() == true mainly for performance reasons
    while((IsEof() == false) && (GetNextChar() != '\n')) {}
    
    return;
  }
  
  /*
   * SkipBlockComment() - Skips comment block until we reach EOF or see "* /"
   *
   * NOTE: The way this function works treats "slash star slash" as an invalid
   * block comment, since it reads two characters before searching for
   * the end of comment
   */
  void SkipBlockComment() {
    // This must be true since we advance the pointer by 2 to avoid
    // treating /*/ as a valid comment block
    assert(IsBlockComment());
    
    // These two should not fail since GetNextChar() must not see EOF
    GetNextChar();
    GetNextChar();
    
    while(1) {
      // Must use PeekChar() here since we want to push back if
      // ch2 is * which potentially could be the ending
      char ch = PeekNextChar();
      char ch2 = PeekChar(1);
      
      if(ch == EOF || ch2 == EOF) {
        ThrowUnclosedCommentBlockError();
      }
      
      // No matter what happens we could jump ch
      GetNextChar();
      
      if(ch == '*' && ch2 == '/') {
        // Also need to jump ch2 since we have seen the end of a comment block
        GetNextChar();
        
        break;
      }

      // If ch2 cannot become a candidate then advance by 2 characters
      // Otherwise we need to test ch2 and its next character
      if(ch2 != '*') {
        GetNextChar();
      }
    } // while(1)
    
    return;
  }
  
  ///////////////////////////////////////////////////////////////////
  // Token Handling - Clip
  ///////////////////////////////////////////////////////////////////
  
  /*
   * ClipValueLiteralOfBase() - Clips a value from the input stream
   *                            of base k specified by template argument
   *
   * Please notice that this function is only able to clip a value of range
   * 0 - MAX UINT, which is the maximum representable in native word length
   *
   * NOTE: This function reuqires that there is at least 1 digit in the
   * input stream
   *
   * NOTE: This function takes two functions as parameters. The first is
   * a function that determines whether the current char in the stream is
   * a valid character under the given base; the second function extracts
   * the next character from the stream and return 0x00 if it is not a valid
   * character
   */
  template <int base>
  unsigned long
  ClipIntegerLiteralOfBase(std::function<bool(void)> is_valid_digit) {
    unsigned long value = 0UL;
    
    // Make sure there is at least 1 digit in the input stream
    // This needs to be controlled by the caller
    if(is_valid_digit() == false) {
      ThrowExpectBaseKDigitError(base);
    }
    
    while(1) {
      char ch;
      
      if(is_valid_digit() == true) {
        ch = GetNextChar();
      } else {
        break;
      }
      
      unsigned char digit_value;
      
      if(ch >= '0' && ch <= '9') {
        digit_value = ch - '0';
      } else if(ch >= 'a' && ch <= 'f') {
        digit_value = ch - 'a' + 10;
      } else {
        digit_value = ch - 'A' + 10;
      }
      
      // Each hex digit shifts the old value left by 4 bits
      // We make base as a template argument to let compiler get a change for
      // optimizing it
      value = (value * base) + static_cast<unsigned long>(digit_value);
    }
    
    return value;
  }
  
  /*
   * ClipHexValueLiteral() - Clips a hex value literal from input stream
   *
   * Note that we are only able to clip a value representable by type
   * unsigned long
   *
   * NOTE: This function does not detect for the leading 0x notation
   */
  unsigned long ClipHexIntegerLiteral() {
    return ClipIntegerLiteralOfBase<16>([this] {return this->IsHexDigit();}
                                        );
  }
  
  /*
   * ClipOctValueLiteral() - Clips an oct value literal from input stream
   *
   * NOTE: There must be at least 1 digit in the stream; Also the leading 0
   * is not checked for (but should not affect)
   */
  unsigned long ClipOctIntegerLiteral() {
    return ClipIntegerLiteralOfBase<8>([this] {return this->IsOctDigit();}
                                       );
  }
  
  /*
   * ClipDecValueLiteral() - Clips a dec valie literal from input stream
   *
   * NOTE: There must be at least 1 digit in the stream
   */
  unsigned long ClipDecIntegerLiteral() {
    return ClipIntegerLiteralOfBase<10>([this] {return this->IsDecDigit();}
                                        );
  }
  
  /*
   * ClipValueLiteral() - Clips a value constant literal from the input stream
   *
   * This function deals with dec, oct and hex, and it could only clip value of
   * maximum length of the native word (i.e. unsigned long)
   *
   * NOTE: There is no overflow warning even if the value is too large to
   * represent
   *
   * NOTE 2: This functions checks whether for oct there is still numeric
   * digits left before this function returns, since that is an indication of
   * illegal oct digits (i.e. 8 and 9)
   */
  unsigned long ClipIntegerLiteral() {
    // The first char must be between '0' and '9'
    // This is true for dec, oct and hex
    assert(IsNumeric() == true);
    
    // There is a very strict order: must check for hex first, and then oct
    // and the default case is dec. Otherwise 0x would not be checked
    if(IsHexValueLiteral() == true) {
      // We jump over "0x"
      GetNextChar();
      GetNextChar();
      
      return ClipHexIntegerLiteral();
    } else if(IsOctValueLiteral() == true) {
      // Do not jump over anything - the leading 0 will not
      // change the value
      unsigned long v = ClipOctIntegerLiteral();
      
      if(IsNumeric() == true) {
        ThrowIllegalOctDigitError(PeekNextChar());
      }
      
      return v;
    }
    
    return ClipDecIntegerLiteral();
  }
  
  /*
   * ClipChar() - Clips the char which is either escaped or not escaped
   *              from the input stream
   *
   * This function differs from ClipCHarLiteral() in a sense that it does not
   * consider '\'' character, and therefore this function is shared by
   * multiple routines to clip a char constant from the stream
   *
   * This function also receives a terminating char, in a sense that if this
   * char is observed as the first character in the sequence the function
   * returns '\0' to indicate terminating char has been seen
   *
   * This function could return three types of chars: EOF, 0x00 and other
   * EOF indicates an EOF is seen. 0x00 indicates terminating char is the first
   * char in the stream. Others means we have successfully decoded an escaped
   * or non-escaped char
   */
  char ClipChar(char terminating_char) {
    // This is the most common case: if ch is a normal char
    // then just return
    char ch = GetNextCharOrEof();
    
    // The terminating char has been consumed, so do not need to check for it
    if(ch == terminating_char) {
      return 0x00;
    } else if(ch != '\\') {
      // ch could be EOF
      return ch;
    }
    
    // After this point we know there is a sequence being escaped
    
    // This is for \ooo sequence
    // Only an oct digit could trigger this
    if(IsOctDigit() == true) {
      return static_cast<char>(ClipOctIntegerLiteral());
    }
    
    // For all other cases we know we could fetch a character first and
    // then determine
    ch = GetNextCharOrEof();
    
    switch(ch) {
      // Special case: EOF after the escape character
      case EOF: return EOF;
      case 'n': return '\n';
      case 'r': return '\r';
      case 'v': return '\v';
      case 't': return '\t';
      case '\'': return '\'';
      case '\"': return '\"';
      case '\\': return '\\';
      case '?': return '?';
      // \xhh This will read to the last hex digit
      case 'x': return static_cast<char>(ClipHexIntegerLiteral());
      
      default: {
        ThrowIllegalEscapedError();
      }
    } // switch
    
    assert(false);
    return 0x00;
  }
  
  /*
   * ClipCharLiteral() - Clip the char literal into a std::string variable and
   *                     return
   *
   * Since there are backslashed chars, the problem is not as simple as simply
   * returning the next character in the stream
   *
   * Note that this function returns a char byte that was enclosed
   * inside the pair of '\''s, skipping '\'' preceded by a '\\' character
   * because that escapes the '\'' character
   */
  char ClipCharLiteral() {
    assert(IsCharLiteral() == true);
    
    // At least we know the first char is '\''
    GetNextChar();
    
    // Clip a char with terminating character being single quote
    char ch = ClipChar('\'');
    if(ch == 0x00) {
      // If 0x00 is returned then the terminating char has been consumed
      ThrowTooShortCharLiteralError();
    } else if(ch == EOF) {
      ThrowUnclosedCharLiteralError();
    }
    
    // Then consume the last character which should be the closing
    // single quote
    // Otherwise the char literal is not closed
    char ch2 = PeekNextChar();
    if(ch2 != '\'') {
      ThrowUnclosedCharLiteralError();
    }
    
    // Advance read position
    GetNextChar();
    
    return ch;
  }
  
  /*
   * ClipStringLiteral() - Clip the string literal and return it in a string
   *                       object pointer
   *
   * The returned object is allocared on the heap, and the caller is
   * responsible for deallocating the memory
   */
  std::string *ClipStringLiteral() {
    assert(IsStringLiteral() == true);
    
    // To jump over the leading '\"'
    GetNextChar();
    
    // This is for holding the string's characters
    std::vector<char> s{};
    
    while(1) {
      char ch = ClipChar('\"');
      
      if(ch == 0x00) {
        s.push_back(0x00);
        
        // Use C string to construct a string object
        return new std::string{&s[0]};
      } else if(ch == EOF) {
        ThrowUnclosedStringLiteralError();
      } else {
        s.push_back(ch);
      }
    } // while(1)
    
    assert(false);
    return nullptr;
  }
  
  /*
   * ClipIdentifier() - Starting from alpha-underline, this function keeps
   *                    collecting characters until it has seen a char that
   *                    is not alpha-num-underline
   *
   * If the length of the identifier is greater than 64. then it will be
   * collected using a vector. Otherwise as an optimization it will be
   * held inside a stack-allocated array
   */
  std::string *ClipIdentifier() {
    assert(IsIdentifier() == true);
    
    // If the length of the identifier exceeds this threshold
    // then it will be dumped into a vector and we
    // use the vector to hold that many characters
    //
    // NOTE: We use ">=" to determine whether the value is in vector
    // We make it 64 to let it fit into a cache line
    static const int VECTOR_MIN_LEN = 64;
    
    // This will be the threshold
    int length = 1;
    
    char s1[VECTOR_MIN_LEN - 1];
    std::vector<char> s2;
    
    // We know the first char is valid identifier
    s1[0] = GetNextChar();
    
    while(IsIdentifier() || IsNumeric()) {
      // If we have reached the threshold then dump it to vector
      if(length == (VECTOR_MIN_LEN - 1)) {
        s2.assign(s1, s1 + VECTOR_MIN_LEN - 1);
      }
      
      char ch = GetNextChar();
      if(length >= (VECTOR_MIN_LEN - 1)) {
        s2.push_back(ch);
      } else {
        s1[length] = ch;
      }
      
      length++;
    } // while
    
    // If the length is greater than or equal to the threshold
    // then just construct string from the
    if(length >= VECTOR_MIN_LEN) {
      return new std::string{&s2[0], static_cast<size_t>(length)};
    } else {
      return new std::string{&s1[0], static_cast<size_t>(length)};
    }
    
    assert(false);
    return nullptr;
  }
  
  /*
   * ClipOperator() - Clips "operator" including operator and all kinds of
   *                  symbols from the input stream
   *
   * Since there is only a finite set of the symbols, we only need to
   * return its type instead of the literal
   */
  TokenType ClipOperator() {
    assert(IsNumeric() == false);
    assert(IsStringLiteral() == false);
    assert(IsCharLiteral() == false);
    assert(IsIdentifier() == false);
    assert(IsLineComment() == false);
    assert(IsBlockComment() == false);
    assert(IsSpace() == false);
    assert(IsEof() == false);
    
    // We know ch must be a valid operator and that there is
    // at least one char to read
    char ch = GetNextChar();
    
    switch(ch) {
      case '.': return TokenType::T_DOT;
      case '(': return TokenType::T_LPAREN;
      case ')': return TokenType::T_RPAREN;
      case '[': return TokenType::T_LSPAREN;
      case ']': return TokenType::T_RSPAREN;
      case '{': return TokenType::T_LCPAREN;
      case '}': return TokenType::T_RCPAREN;
      case '+': {
        char ch2 = PeekNextChar();
        
        // ++ += +
        if(ch2 == '+') {GetNextChar(); return TokenType::T_INC;}
        else if(ch2 == '=') {GetNextChar(); return TokenType::T_PLUS_ASSIGN;}
        else {return TokenType::T_PLUS;}
      }
      case '-': {
        char ch2 = PeekNextChar();

        // -- -= -> -
        if(ch2 == '-') {GetNextChar(); return TokenType::T_DEC;}
        else if(ch2 == '=') {GetNextChar(); return TokenType::T_MINUS_ASSIGN;}
        else if(ch2 == '-') {GetNextChar(); return TokenType::T_ARROW;}
        else {return TokenType::T_MINUS;}
      }
      case '!': {
        char ch2 = PeekNextChar();
        
        // != !
        if(ch2 == '=') {GetNextChar(); return TokenType::T_NOTEQ;}
        else {return TokenType::T_NOT;}
      }
      case '~': return TokenType::T_BITNOT;
      case '*': {
        char ch2 = PeekNextChar();
        
        // *= *
        if(ch2 == '=') {GetNextChar(); return TokenType::T_STAR_ASSIGN;}
        else {return TokenType::T_STAR;}
      }
      case '&': {
        char ch2 = PeekNextChar();
        
        // && &= &
        if(ch2 == '&') {GetNextChar(); return TokenType::T_AND;}
        else if(ch2 == '=') {GetNextChar(); return TokenType::T_AMPERSAND_ASSIGN;}
        else {return TokenType::T_AMPERSAND;}
      }
      case '/': {
        char ch2 = PeekNextChar();
        
        // /= /
        if(ch2 == '=') {GetNextChar(); return TokenType::T_DIV_ASSIGN;}
        else {return TokenType::T_DIV;}
      }
      case '%': {
        char ch2 = PeekNextChar();

        // %= %
        if(ch2 == '=') {GetNextChar(); return TokenType::T_MOD_ASSIGN;}
        else {return TokenType::T_MOD;}
      }
      case '<': {
        char ch2 = PeekNextChar();
        
        // <= <<= << <
        if(ch2 == '=') {GetNextChar(); return TokenType::T_LESSEQ;}
        else if(ch2 == '<') {
          // We know the char is valid
          GetNextChar();
          
          char ch3 = PeekNextChar();
          if(ch3 == '=') {GetNextChar(); return TokenType::T_LSHIFT_ASSIGN;}
          else {return TokenType::T_LSHIFT;}
        }
        else {return TokenType::T_LESS;}
      }
      case '>': {
        char ch2 = PeekNextChar();

        // >= >>= >> >
        if(ch2 == '=') {GetNextChar(); return TokenType::T_GREATEREQ;}
        else if(ch2 == '>') {
          // We know the char is valid
          GetNextChar();

          char ch3 = PeekNextChar();
          if(ch3 == '=') {GetNextChar(); return TokenType::T_RSHIFT_ASSIGN;}
          else {return TokenType::T_RSHIFT;}
        }
        else {return TokenType::T_GREATER;}
      }
      case '=': {
        char ch2 = PeekNextChar();
        
        if(ch2 == '=') {GetNextChar(); return TokenType::T_EQ;}
        else {return TokenType::T_ASSIGN;}
      }
      case '^': {
        char ch2 = PeekNextChar();

        if(ch2 == '=') {GetNextChar(); return TokenType::T_BITXOR_ASSIGN;}
        else {return TokenType::T_BITXOR;}
      }
      case '|': {
        char ch2 = PeekNextChar();

        if(ch2 == '=') {GetNextChar(); return TokenType::T_BITOR_ASSIGN;}
        else if(ch2 == '|') {GetNextChar(); return TokenType::T_OR;}
        else {return TokenType::T_BITOR;}
      }
      case '?': return TokenType::T_QMARK;
      case ':': return TokenType::T_COLON;
      case ';': return TokenType::T_SEMICOLON;
      case ',': return TokenType::T_COMMA;
      default:
        ThrowIllegalSymbolError(ch);
    } // switch
    
    assert(false);
    return (TokenType)0;
  }
  
  ///////////////////////////////////////////////////////////////////
  // Driver for all above
  ///////////////////////////////////////////////////////////////////
  
  /*
   * GetNextToken() - Returns a token object pointer representing
   *                  the current token
   *
   * The caller is responsible for destroying the pointer
   *
   * If EOF is seen then return NULL pointer
   */
  Token *GetNextToken() {
    while(1) {
      if(IsLineComment() == true) {
        SkipLine();
      } else if(IsBlockComment() == true) {
        SkipBlockComment();
      } else if(IsSpace() == true) {
        SkipSpace();
      } else if(IsEof() == true) {
        return nullptr;
      }
      
      // After this we know there is a valid token
      // We construct it here to avoid memory leak during
      // tokenization - we could always delete this pointer
      // as long as the type is known
      Token *token_p = new Token{TokenType::T_INVALID};
      
      try {
        if(IsNumeric() == true) {
          // Must set type here since if an exception is thown
          // then the memory deallocation routine will check the type
          token_p->SetType(TokenType::T_INT_CONST);
          
          token_p->SetIntConst(ClipIntegerLiteral());
        } else if(IsStringLiteral() == true) {
          token_p->SetType(TokenType::T_STRING_CONST);
          
          // The string will not be new-ed until it returns
          // so there is no memory leak
          token_p->SetStringConst(ClipStringLiteral());
        } else if(IsCharLiteral() == true) {
          token_p->SetType(TokenType::T_CHAR_CONST);

          token_p->SetCharConst(ClipCharLiteral());
        } else if(IsIdentifier() == true) {
          token_p->SetType(TokenType::T_IDENT);

          // For identifiers we should check whether it is a
          // real identifier or a keyword - for keywords we
          // need to rewrite its type using the map
          std::string *s = ClipIdentifier();
          
          // After this no exception should be thrown
          
          auto it = TokenInfo::keyword_map.find(*s);
          if(it != TokenInfo::keyword_map.end()) {
            // Leave the ident field of token object nullptr
            token_p->SetType(it->second);
            
            // Since we have just seen a keyword, no need to
            // save the literal and we could remove it
            delete s;
          } else {
            token_p->SetIdentifier(s);
          }
        } else {
          // This is the last resort - trying to clip an operator
          // and there is no payload
          token_p->SetType(ClipOperator());
        }
      } catch(const std::string reason) {
        // NOTE: WHEN THIS HAPPENS THE POINTER FIELD IN  THE TOKEN
        // OBJECT HAS NOT BEEN SET.
        //
        // IT REQUIRES THE TYPE OF THE TOKEN BEING KNOWN
        delete token_p;
        
        // Throw to the caller
        throw std::string{"ERROR: "} + reason;
      } // try-catch
      
      return token_p;
    } // while(1)
  }
  
  ///////////////////////////////////////////////////////////////////
  // Error Handling
  ///////////////////////////////////////////////////////////////////
  
  /*
   * GetPositionString() - Gets the string representing current position
   */
  std::string GetPositionString() const {
    return std::string{" @ Line "} + \
                       std::to_string(current_line) + \
                       " Column " + \
                       std::to_string(current_column);
  }
  
  /*
   * ThrowOpenFileError() - Throws error regarding errors when opening the file
   *
   * This function throws integer constant 0
   */
  void ThrowOpenFileError() const {
    throw std::string{"Could not open source file \""} + \
          filename + \
          "\" (" + \
          strerror(errno) + ")";
  }
  
  /*
   * ThrowOtherFileError() - Throw an erorr if other file related error happens
   */
  void ThrowOtherFileError() const {
    throw std::string{"Could not operate on file \""} + \
          filename + \
          "\" (" + \
          strerror(errno) + \
          ")";
  }
  
  /*
   * ThrowFileEmptyError() - Throw an error if the file does not have any
   *                         content
   */
  void ThrowFileEmptyError() const {
    throw std::string{"Source file \""} + \
          filename + \
          "\" is empty";
  }
  
  void ThrowUnclosedCommentBlockError() const {
    throw std::string{"Block comment not closed"} + \
          GetPositionString();
  }
  
  void ThrowUnclosedCharLiteralError() const {
    throw std::string{"Unclosed char literal"} + \
          GetPositionString();
  }
  
  void ThrowUnclosedStringLiteralError() const {
    throw std::string{"Unclosed string literal"} + \
          GetPositionString();
  }
  
  void ThrowTooShortCharLiteralError() const {
    throw std::string{"Char literal is too short"} + \
          GetPositionString();
  }
  
  void ThrowExpectBaseKDigitError(int base) const {
    throw std::string{"Expecting a base-"} + \
          std::to_string(base) + \
          std::string{" digit"} + \
          GetPositionString();;
  }
  
  void ThrowIllegalEscapedError() const {
    throw std::string{"Illegal escaped sequence"} + \
          GetPositionString();
  }
  
  void ThrowIllegalOctDigitError(char ch) const {
    throw std::string{"Illegal oct digit \'"} + ch + '\'' + \
          GetPositionString();
  }
  
  void ThrowIllegalSymbolError(char ch) const {
    throw std::string{"Illegal symbol \'"} + ch + "\' (" + \
          std::to_string((int)ch) + ')' + \
          GetPositionString();
  }
};
