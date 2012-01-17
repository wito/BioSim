#include "skip_comment.h"
#include <string>
#include <stdexcept>

std::istream& toolbox::skip_comment(std::istream& istrm, char comment_char)
{
  // read as long as file is good
  // once a non-comment is detected, return from inside the loop
  // return conditions are: EOF or non-comment non-white char

  while ( istrm ) 
  {
    // check if first char on line is comment char
    char c;
    istrm >> c;  // >> gobbles all whitespace

    if ( istrm.eof() ) // at end of file, ok 
      return istrm;
    else if ( istrm.fail() ) 
      throw std::runtime_error("skip_comment(): read error");
    else if ( c == comment_char )
    {
      // comment line, gobble
      std::string foo;
      std::getline(istrm, foo);
      
      // string reading may fail at EOF, so we have
      // to check for eof() first, then for fail()
      if ( istrm.eof() ) 
	return istrm;
      else if ( istrm.fail() ) 
	throw std::runtime_error("skip_comment(): read error on comment gobble");
    }
    else
    {
      istrm.putback(c);      // was no comment after all
      return istrm;
    }
  }

  // should never get here, but for safety's sake
  return istrm;
}
