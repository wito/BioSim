#include "read_parameters.h"
#include "skip_comment.h"
#include <string>
#include <fstream>
#include <stdexcept>
#include <cassert>

// -----------------------------------------------------------

toolbox::ReadParameters::ReadParameters(char comment_char) :
  comment_char_(comment_char),
  parameters_()
{
}

// -----------------------------------------------------------

void toolbox::ReadParameters::read(const std::string& fname)
{
  reset_();  // reset all tokens
  

  // open file
  std::ifstream istrm(fname.c_str());
  if ( !istrm )
    throw std::runtime_error("ReadParameters::read(): Could not open " + fname);

  while ( istrm )
  {
    skip_comment(istrm, comment_char_);

    // line is not a comment
    std::string pname;
    istrm >> pname;

    if ( istrm.eof() ) // at end of file, ok here
      break;
    else if ( istrm.fail() ) 
      throw std::runtime_error("ReadParameters::read(): read error on par name");

    // read value
    Tokenmap::iterator it = parameters_.find(pname);
    if ( it == parameters_.end() )
      throw std::runtime_error("ReadParameters::read(): unknown parameter name " 
			       + pname);

    assert(it->second != 0);  // never use pointer without it
    it->second->read(istrm);  // errors will pass through
 
  } // while

  // all is read, time to write; better catch errors from there 
  // & recast, since write_target_() is private
  try {
    write_target_();
  } catch ( std::runtime_error& e ) {
    throw std::runtime_error("ReadParameters::read(): writing failed "
			     "with message:\n  " + std::string(e.what()));
  }

  return;
}

// -----------------------------------------------------------

void toolbox::ReadParameters::reset_()
{
  // reset all tokens
  for ( Tokenmap::iterator it = parameters_.begin() ;
	it != parameters_.end() ; ++it )
  {
    assert(it->second != 0);  // never use a pointer without it
    it->second->reset();
  }

  return;
}

// -----------------------------------------------------------

void toolbox::ReadParameters::write_target_() const
{
  // ensure that all tokens are ready
  bool all_ok = true;
  for ( Tokenmap::const_iterator it = parameters_.begin() ;
	it != parameters_.end() ; ++it )
  {
    assert(it->second != 0);
    all_ok = all_ok && it->second->is_ready();
  }

  if ( !all_ok )
      throw std::runtime_error("ReadParameters::write_target_():"
			       "not all parameters ready for writing.");

  // all is fine, now write; skip assert, did that above
  for ( Tokenmap::const_iterator it = parameters_.begin() ;
	it != parameters_.end() ; ++it )
    it->second->write_target();

  return;
}
