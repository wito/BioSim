#ifndef READ_PARAMETERS_H
#define READ_PARAMETERS_H

/** @file read_parameters.h
    Declares class ReadParams for reading of parameters from text files.

    @note This file contains an implementation of ReadParameters that
    provides a crude work-around for gcc bug 4672. The problem is the
    failure to resolve calls to @c ReadParameters::register_param()
    unambiguously if called with a @c list<T> parameter and only two
    arguments.  See Vandervoorde&Josuttis, C++ Templates, ch 12, for
    details.  As a work-around, this version of ReadParams has a
    member @c register_list_param() which is to be used for list
    parameters. Use this file if you have gcc 4.0.0 and
    earlier. Otherwise, use @c read_parameters_correct.h .

    This file is part of BioSim.

    (C) 2006 Hans Ekkehard Plesser <hans.ekkehard.plesser@umb.no>

    This file is public software under GPL2.

*/

#include <list>
#include <map>
#include <string>
#include <stdexcept>
#include <istream>
#include <cassert>
#include <limits>

namespace toolbox {

  /** Class for reading parameters from files.
   
      This class reads parameters from text files. Files must contain
      one parameter per line; each line contains the parameter label
      followed by the parameter value.
   
      Before ReadParams can be used, parameters must be registered,
      supplying the parameter name and a reference to the variable to
      be set.  Parameter names are strings not containing any
      whitespace. There must be one and only one value per parameter,
      in such a way that @c operator>>() can read the value.
   
      Parameters can be containers. In this case, the container will
      be cleared and filled with all parameter values found. For non-
      container parameters, it is an error if one parameter is found
      twice in the same file.
   
      Default values can be specified for parameters. If no default
      value is specified, it is an error if the parameter is not found
      in the file.  For container parameters, no default value can be
      specified, but a minimum and maximum size can be specified. It
      is an error if the number of container elements is outside this
      range once reading is complete.

      Comments are permitted in parameter files; they must be on lines
      on their own, and the first character on a commment line must be
      the user-definable comment character.
   
      Any unknown parameter names are treated as errors.
   
      ReadParam::read() will read the entire parameter file first. If
      no error has occured, all parameter values will be set using the
      registered mutator functions. Thus, no incomplete data sets will
      be entered (unless errors occur during the setting process).
   
      Typical usage:
   
      @code
      class Foo {									       
      public:										       
        Foo();               // constructor, registers parameters w reader		       
        void init(const string&);  // initializes, including param reading		       
        void dump();									       
      										       
      private:									       
        ReadParameters  param_reader_;  // default comment character			       
        int              n_;								       
        double           x_;								       
        list<string>     ls_;								       
        static list<int> li_;								       
      };										       
         										       
      Foo::Foo() 									       
      {										       
        param_reader_.register_param("n", n_);					       
        param_reader_.register_param("x", x_, 0.5);        // default value 0.5		       
        param_reader_.register_list_param("s", ls_);       // minimum 1 element		       
        param_reader_.register_list_param("i", li_, 0, 3); // 0..3 elements		       
      }										       
      										       
      void Foo::init(const string& fname) 						       
      {										       
        param_reader_.read(fname);  // reads file & sets values			       
        return;									       
      }										       
      										       
      void Foo::dump() {								       
        cout << "\nn : " << n_ << "\nx : " << x_ << "\ns : ";				       
        for ( list<string>::const_iterator it = ls_.begin() ; it != ls_.end() ; ++it )       
          cout << *it << ' ';								       
        cout << "\ni : ";								       
        for ( list<int>::const_iterator it = li_.begin() ; it != li_.end() ; ++it )	       
          cout << *it << ' ';								       
        cout << endl;									       
        return;									       
       }										       
      										       
      list<int> Foo::li_;  // static member definition				       
      										       
      int main()									       
      {										       
        Foo foo;									       
        										       
        try {										       
          foo.init("params1.dat");							       
          cout << "\nFoo:init() succeeded" << endl;					       
        }										       
        catch ( std::runtime_error &e ) {						       
          cerr << "Error: " << e.what();						       
        }										       
      										       
        foo.dump();  // show values							       
      										       
        return 0;  									       
      }                                                                                      
      @endcode
   
      @todo At present, @c std::list<T> is the only container type
      that is implemented. Attempting to register other container
      types as parameters will fail on compilation. To implement other
      container types, one needs a suitable equivalent to 
      @c ReadParameters::ListToken<T>.

      @warning Feeding references to the variables to be set is messy
      and violates containment. But at present it seems the only way
      that allows both static and member variables to be set. 

      @ingroup toolbox
   */

  class ReadParameters {

  private:

    /** Base class for all Token classes.
     */
    class Token 
    {

    public:

      virtual ~Token() {}

      /** Read value from file.
	  The value must be read in Token, since ReadParameters does
	  not know the data type of the token.
	  @param strm stream to read from
	  @throws runtime_error if reading error or value has been read before
      */
      virtual std::istream& read(std::istream& strm) =0;

      /** Returns true if Token is ready for writing to target.

	  A non-container Token is ready for storage if it has a
	  default value or it has been read.

	  A container Token is ready for storage if its number of
	  elements is between the given limits for the number of 
	  elements.

	  @note Must be defined in Token classes, since
	  logic differs between normal and container classes.
      */
      virtual bool is_ready() const =0;

      /** Transfer value to target.
       */
      virtual void write_target() const =0;

      /** Clears token for another reading.
	  value_ is reset to default_ if necessary and read_ flag
	  is reset.
      */
      virtual void reset() =0;     

    private:
      /** Assignment operator not needed, make it private 
	  @note We must provide a dummy implementation, otherwise
          the linker cannot build the vtable. */
      virtual Token& operator=(const Token&)
	{ assert(false); return *this; }

    };  // Token

    /** Class for non-container parameter Tokens.

        A Token represents a single parameter. The associative map in
        the ReadParameters class connects parameter names with
        parameter Tokens. 

	@note Token instances are created by register(),
	not by read().

	@note Template instantiation hinges on the signature of the
	mutator function passed to the constructor.
     */
    template <typename T>
    class PlainToken : public Token
    {
    public:
      /** Constructor without default value.
	  @param name parameter name 
	  @param target variable to be set
      */
      PlainToken(const std::string& name, T& target);

      /** Constructor with default value.
	  @param name parameter name 
	  @param target variable to be set
	  @param deflt default parameter value
      */
      PlainToken(const std::string& name, T& target, T deflt);

      /** Read value from file.
	  The value must be read in Token, since ReadParameters does
	  not know the data type of the token.
	  @param strm stream to read from
	  @throws runtime_error if reading error or value has been read before
      */
      std::istream& read(std::istream& strm);

      /** Transfer value to target.
       */
      void write_target() const;

      bool is_ready() const;   //!< redefined from AbstractToken

      /** Clears token for another reading.
	  value_ is reset to default_ if necessary and read_ flag
	  is reset.
      */
      void reset();     

    private:
      const std::string name_;  //!< parameter name, for error messages
      const bool has_default_;  //!< whether token has default value
      const T    default_;      //!< default token value

      bool    read_;            //!< whether token has been read

      /** Actual value.
	  @note Must be set to default on construction and reset.
      */
      T  value_;      

      T& target_; //!< destination of data

    private:
      /** Copy constructor not needed, make it private */
      PlainToken(const PlainToken&);

    }; // PlainToken


    /** Class for list parameter tokens.

        For container parameter types, data has to be stored using
        push_back(). Also, construction is different, since minimum
        and maximum element numbers can be given.

	@note Use this as starting point for other container parameter tokes.
	@note @c T is the type of the elements in the list.
     */
    template <typename T>
    class ListToken : public Token
    {
    private:
      typedef typename std::list<T>::size_type list_sz;

    public:
      /** Constructor for list parameter token.
	  @param name   parameter name
	  @param target container variable to be set
	  @param min_sz minimum size of container
	  @param max_sz maximum size of container
      */
      ListToken(const std::string& name, std::list<T>& target, 
		list_sz min_sz, list_sz max_sz);

      /** Read value from file.
	  The value must be read in Token, since ReadParameters does
	  not know the data type of the token.
	  @param strm stream to read from
	  @throws runtime_error if reading error or value has been read before
      */
      std::istream& read(std::istream& strm);

      /** Transfer container to target.
       */
      void write_target() const;

      bool is_ready() const;   //!< redefined from AbstractToken

      /** Clears token for another reading.
	  container_ is cleared().
      */
      void reset();     

    private:
      const std::string name_;  //!< parameter name, for error messages

      const list_sz min_sz_;   //!< minimal allowed size
      const list_sz max_sz_;   //!< maximal allowed size

      //! token value, for temp storage during reading
      std::list<T> container_;  
      
      //! destination of data
      std::list<T>& target_; 

    private:
      /** Copy constructor not needed, make it private */
      ListToken(const ListToken&);

    }; // ListToken

    
  public:
    /** Construct parameter reader with empty map.
	@param comment_char Lines with this character in position 1
               will be skipped on reading. 
    */
    ReadParameters(char comment_char = '%');

    /** Register parameter with reader without default value.
	This register() function is for non-container types.
	@param name  parameter name
	@param target  variable to be read to
	@throw runtime_error if registration fails
	@note See class documentation for ReadParameters for
	information on required properties for parameter name and
	parameters.
    */
    template <typename T> 
    void register_param(const std::string& name, T& target);

    /** Register parameter with reader with default value.
	This register() function is for non-container types.
	@param name  parameter name
	@param target  variable to be read to
	@param deflt Default parameter value
	@throw runtime_error if registration fails
	@note See class documentation for ReadParameters for
	information on required properties for parameter name and
	parameters.
    */
    template <typename T> 
    void register_param(const std::string& name, T& target,
			T deflt);
    
    /** Register containter parameter with reader.
	This register() function is for container types.
	@param name  parameter name
	@param target  variable to be read to
	@param min_sz minimum size of container, default @c 1
	@param max_sz maximum size of container, default @c max<size_type>
	@throw runtime_error if registration fails
	@note See class documentation for ReadParameters for
	information on required properties for parameter name and
	parameters.
	@bug This function is called @c register_list_param() instead
	of @c register_param() only to work around GCC bug 4672.
    */
    template <typename T> 
    void register_list_param(const std::string& name, typename std::list<T>& target,
			typename std::list<T>::size_type min_sz = 1, 
			typename std::list<T>::size_type max_sz = 
			std::numeric_limits<typename std::list<T>::size_type>::max());

    /** Function reading parameters.
	This function performs the actual parameter reading.
	@param fname Name of file to be read
	@throws runtime_error in case of reading errors
    */
    void read(const std::string& fname);

  private:
    const char comment_char_;  //!< this character mark comment lines

    /** Shorthand for map type
	@todo Could one use Token& here?
    */
    typedef std::map<std::string, Token*> Tokenmap;
    
    /** Connection between names and parameter Tokens.
     */
    Tokenmap parameters_;

    /** Reset all tokens.
     */
    void reset_();

    /** Write all tokens to their targets.
	Write all tokens to their targets after checking that all are ready.
	@throw runtime_error if not all tokens are ready
    */
    void write_target_() const;

  }; // ReadParameters

} // namespace toolbox


// -- PlainToken ---------------------------------------------------

template <typename T>
toolbox::ReadParameters::PlainToken<T>::PlainToken(const std::string& name, 
						   T& target) :
  name_(name),
  has_default_(false),
  default_(T()),
  read_(false),
  value_(T()),
  target_(target)
{
}

template <typename T>
toolbox::ReadParameters::PlainToken<T>::PlainToken(const std::string& name, 
						   T& target, T deflt) :
  name_(name),
  has_default_(true),
  default_(deflt),
  read_(false),
  value_(default_),
  target_(target)
{
}

template <typename T>
std::istream& toolbox::ReadParameters::PlainToken<T>::read(std::istream& strm)
{
  if ( read_ )
    throw std::runtime_error("PlainToken::read(): " + name_ 
			     + " has been read before.");

  strm >> value_;
  if ( strm.fail() )
    throw std::runtime_error("PlainToken::read(): Reading " + name_ 
			     + " failed.");

  read_ = true;

  return strm;
}

template <typename T>
void toolbox::ReadParameters::PlainToken<T>::write_target() const
{
  if ( !is_ready() )
    throw std::runtime_error("PlainToken::write_target(): Token "
			     + name_ + " is not ready for writing.");

  target_ = value_;
  return;
}

template <typename T>
bool toolbox::ReadParameters::PlainToken<T>::is_ready() const
{
  return has_default_ || read_;
}

template <typename T>
void toolbox::ReadParameters::PlainToken<T>::reset()
{
  value_ = default_; // no harm in this even if no default given
  read_  = false;
  return;
}


// -- ListToken ---------------------------------------------------

template <typename T>
toolbox::ReadParameters::ListToken<T>::ListToken(const std::string& name,
						 std::list<T>& target, 
						 list_sz min_sz,  
						 list_sz max_sz) :
  name_(name),
  min_sz_(min_sz),
  max_sz_(max_sz),
  container_(),
  target_(target)
{
}
 
template <typename T>
std::istream& toolbox::ReadParameters::ListToken<T>::read(std::istream& strm)
{
  if ( container_.size() >= max_sz_ )
    throw std::runtime_error("ListToken::read(): No further instances of " 
			     + name_ + " allowed.");

  T value;
  strm >> value;
  if ( strm.fail() )
    throw std::runtime_error("ListToken::read(): Reading " + name_ 
			     + " failed.");
  
  container_.push_back(value);

  return strm;
}

template <typename T>
void toolbox::ReadParameters::ListToken<T>::write_target() const
{
  if ( !is_ready() )
    throw std::runtime_error("PlainToken::write_target(): Token "
			     + name_ + " is not ready for writing.");

  target_ = container_;
  return;
}

template <typename T>
bool toolbox::ReadParameters::ListToken<T>::is_ready() const
{
  typename std::list<T>::size_type sz = container_.size();

  return min_sz_ <= sz && sz <= max_sz_;
}

template <typename T>
void toolbox::ReadParameters::ListToken<T>::reset()
{
  container_.clear();
  return;
}


// -- ReadParameters -----------------------------------------------

template <typename T> 
void toolbox::ReadParameters::register_param(const std::string& name, 
					     T& target)
{
  // check if name exists already
  if ( parameters_.find(name) != parameters_.end() )
    throw std::runtime_error("ReadParameters::register_param(): parameter "
			     + name + " registered previously.");

  // create token
  PlainToken<T> *tok = new PlainToken<T>(name, target);
  assert(tok != 0);

  // build label-value pair
  std::pair<std::string, PlainToken<T>* > pt(name, tok);

  // insert
  std::pair<Tokenmap::iterator, bool> res = parameters_.insert(pt);

  if ( !res.second )
    throw std::runtime_error("ReadParameters::register_param(): insertion of "
			     + name + " failed.");

  return;
}

template <typename T> 
void toolbox::ReadParameters::register_param(const std::string& name, 
					     T& target,	T deflt)
{
  // check if name exists already
  if ( parameters_.find(name) != parameters_.end() )
    throw std::runtime_error("ReadParameters::register_param(): parameter "
			     + name + " registered previously.");

  // create token
  PlainToken<T> *tok = new PlainToken<T>(name, target, deflt);
  assert(tok != 0);

  // build label-value pair
  std::pair<std::string, PlainToken<T>* > pt(name, tok);

  // insert
  std::pair<Tokenmap::iterator, bool> res = parameters_.insert(pt);

  if ( !res.second )
    throw std::runtime_error("ReadParameters::register_param(): insertion of "
			     + name + " failed.");

  return;
}

template <typename T> 
void toolbox::ReadParameters::register_list_param(const std::string& name, 
					     typename std::list<T>& target,
					     typename std::list<T>::size_type min_sz, 
					     typename std::list<T>::size_type max_sz)
{
  // check if name exists already
  if ( parameters_.find(name) != parameters_.end() )
    throw std::runtime_error("ReadParameters::register_param(): parameter "
			     + name + " registered previously.");

  // create token
  ListToken<T> *tok = new ListToken<T>(name, target, min_sz, max_sz);
  assert(tok != 0);

  // build label-value pair
  std::pair<std::string, ListToken<T>* > pt(name, tok);

  // insert
  std::pair<Tokenmap::iterator, bool> res = parameters_.insert(pt);

  if ( !res.second )
    throw std::runtime_error("ReadParameters::register_param(): insertion of "
			     + name + " failed.");

  return;
}

#endif
