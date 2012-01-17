#include "filename.h"

#include <iomanip>
#include <sstream>
#include <string>

using std::string;

// --------------------------------------------------

toolbox::Filename::Filename()
{}

// --------------------------------------------------

toolbox::Filename::Filename(const std::string& filename)
{
  set_fullname(filename);
}

// --------------------------------------------------

void toolbox::Filename::set_fullname(const std::string& filename)
{
  // pathological case: path name containing only .. and / or \,
  // -> no file name and suffix 
  if ( filename.find_first_not_of("./\\") == string::npos ) 
  {
    path_ = filename;
    string::size_type spos = filename.find_last_of("/\\");
    if ( spos != string::npos )
      sep_ = filename[spos];
    return;
  }
  
  string::size_type len  = filename.size();
  string::size_type dot  = filename.rfind('.');
  string::size_type spos = filename.find_last_of("/\\");

  // store separator
  if ( spos != string::npos )
    sep_ = filename[spos];

  if ( dot == string::npos ) 
    {
      if ( spos == string::npos ) 
        name_ = filename;      // no path, no suffix
      else 
	{                        // path, no suffix
	  path_ = filename.substr(0, spos);
	  if ( spos + 1 < len )
	    name_ = filename.substr(spos+1);
	}
    }
  else
    {
      if ( spos == string::npos )
	{                        // no path, suffix
	  name_ = filename.substr(0, dot);
	  if ( dot + 1 < len )
	    suffix_ = filename.substr(dot+1);
	}
      else if ( spos < dot )
	{                        // path, suffix
	  path_ = filename.substr(0, spos);
	  name_ = filename.substr(spos+1, dot-(spos+1));
	  if ( dot + 1 < len )
	    suffix_ = filename.substr(dot + 1);
	}
      else
	{                        // path containing ., no suffix
	  path_ = filename.substr(0, spos);
	  if ( spos + 1 < len )
	    path_ = filename.substr(spos + 1);
	}
    }
}

// --------------------------------------------------

std::string toolbox::Filename::get_fullname() const
{
  string fn = path_;  // no problem if path is empty

  // add separator if there is a path and the last
  // character of the path is not the separator
  if ( !fn.empty() )
  {
    // find position of separator from back
    string::size_type seppos = fn.rfind(sep_);

    if ( seppos == string::npos ||           // no separator present
	 seppos + sep_.size() < fn.size() )  // separator not at end
      fn += sep_;
  }
  
  fn += name_;

  // add dot only if something follows
  if ( !suffix_.empty() )
    fn += '.';

  fn += suffix_;
  
  return fn;
}

// --------------------------------------------------

string toolbox::Filename::num_name(int n, unsigned int width, char fill) const
{
  std::ostringstream os;
    
  os << get_fullname() << '.'
     << std::setw(width) << std::setfill(fill)
     << n;

  return os.str();
}

int toolbox::Filename::explode(std::vector<std::string> &store)
{
  size_t start = 0; 
  int len = 0;
  int i = 0;
  std::vector<std::string> temp; 

  std::string delimiter = std::string("/");
  std::string line = get_path();

  store.clear(); 
  len = delimiter.length();

  if ((line.find(delimiter) == std::string::npos)) {
    store.push_back( line );

    return 1;
  }

  do {
    start = line.find(delimiter);
    store.push_back( line.substr( 0, start) );
    line.erase( 0, start+len );
    i++;
  } while (start != std::string::npos);

  return i;
}
