#ifndef FILENAME_H
#define FILENAME_H

/** @file filename.h
    Utility functions for handling of file names.

    This file contains utility functions for handling of file names.

    This file is part of BioSim

    @author (C) 2003-2006 Hans Ekkehard Plesser <hans.ekkehard.plesser@umb.no>

    This file is public software under GPL2.
*/

#include <string>
#include <vector>

namespace toolbox {

  /** Provide convenient handling of file names.
      
  This class represents file names and provides access to
  the individual parts of the file name, as well as enumerated
  file names.

  @par Example:
  @code
  Filename fn("/home/plesser/INF210/Biosim/examples/Bjarnoya.geo");

  cout << fn.get_path() << endl
       << fn.get_name() << endl
       << fn.get_suffix() << endl
       << fn.num_name(10) << endl;
  @endcode
  yields output
  @code
  /home/plesser/INF210/Biosim/examples
  Bjarnoya
  geo
  /home/plesser/INF210/Biosim/examples/Bjarnoya.geo.00010
  @endcode

  The filename may also be set later:
  @code
  Filename fn;
  fn.set_fullname("test/data.dat");
  ofstream data(fn.num_name(5).c_str());
  @endcode

  @par Parsing of directory names
  - @c test/data will be parsed as path @c test and name @c data
  - @c test/data/ will be parsed as path @c test/data and empty name

  @note Windows filenames beginning with a drive letter are not handled correctly
  under all circumstances. @c C:\\abc should work, though.

  @ingroup toolbox
  */

  class Filename {
    
  public:
    /** Construct object without filename.
	A filename must be set using @c set_fullname() before any @c get_*() calls
	can be applied.
    */
    Filename();

    /** Construct object based on given file name.
	@param filename  Object will represent this filename.
    */
    Filename(const std::string& filename);
    
    /** Set file name.
     */
    void set_fullname(const std::string& filename);

    //! Return path, ie, all before the filename proper.
    std::string get_path() const;
    
    //! Return filename proper, without suffix.
    std::string get_name() const;
 
    //! Return complete file name
    std::string get_fullname() const;
   
    //! Return suffix.
    std::string get_suffix() const;
    
    /** Return complete filename with appended number.
	@param n      number to append
	@param width  width of number suffix
	@param fill   fill character for number
    */
    std::string num_name(int n, unsigned int width = 5, char fill = '0') const;

    //! Gets the separate path components, not including filename
    int explode(std::vector<std::string> &store);
    
  private:
    
    std::string path_;    //!< all up to, but not including, last @c /
    std::string name_;    //!< from last @c / to last @c ., not including
    std::string suffix_;  //!< after last @c .
    std::string sep_;     /*< separator between path components, must be 
			    @c string since it may be empty. 
			  */

  };  // class Filename
  
}  // namespace toolbox

inline
std::string toolbox::Filename::get_path() const
{
  return path_;
}

inline
std::string toolbox::Filename::get_name() const
{
  return name_;
}

inline
std::string toolbox::Filename::get_suffix() const
{
  return suffix_;
}

#endif
