#ifndef PARSER_H_
#define PARSER_H_

#include <list>

#include "ParserBlock.h"
#include "GlobalParamsAction.h"

// libMesh
#include "getpot.h"
#include "exodusII_io.h"

class MooseMesh;
class MProblem;
class Executioner;


class Parser
{
public:
  static void registerObjects();
protected:
  static bool registered;

public:
  /**
   * Default constructor that initializes the parser and looks for the option to dump
   * the registered objects
   */
  Parser(const std::string &dump_string="--dump");

  /**
   * Destructor to remove the dynamically generated Parse Tree
   */
  virtual ~Parser();

  /**
   * Determines whether a particular parser block is marked as active
   * in the input file
   */
  bool isSectionActive(const std::string & section_name,
                       const std::map<std::string, std::vector<std::string> > & active_lists) const;

  /**
   * Parse an input file consisting of getpot syntax and setup objects
   * in the MOOSE derived application
   */
  void parse(const std::string &input_filename);
  void parse_new(const std::string &input_filename);
  
  /**
   * This function initiates the traversal of the parse block tree which is each block is resposible
   * for creating and filling in various MOOSE based objects.
   */
  void execute();
  void execute_new();

  /**
   * Prints the Parser Block tree
   */
  void printTree();

  /**
   * This function is called to track a parser block that is unable to execute because it's
   * prerequesites have not been satisfied.  The parser object will be in charge of executing
   * this block at a later time.
   */
  inline void deferExecution(ParserBlock * pb_ptr)
  {
    _deferred_execution.push_back(pb_ptr);
  }

  inline std::list<ParserBlock *> & getDeferredList()
  {
    return _deferred_execution;
  }
  
  inline void markExecuted(const std::string & pb_name)
  {
    _executed_blocks.insert(pb_name);
  }

  inline bool isExecuted(const std::string & pb_name)
  {
    return _executed_blocks.find(pb_name) != _executed_blocks.end();
  }

  inline std::set<std::string>::iterator getExecutedSetBegin()
  {
    return _executed_blocks.begin();
  }

  inline std::set<std::string>::iterator getExecutedSetEnd()
  {
    return _executed_blocks.end();
  }

  /**
   * This function will split the passed in string on a set of delimiters appending the substrings
   * to the passed in vector.  The delimiters default to "/" but may be supplied as well
   */
  static void tokenize(const std::string &str,
                       std::vector<std::string> & elements,
                       const std::string &delims = "/");

  /**
   * This function tokenizes a path and checks to see if it contains the string to look for
   */
  static bool pathContains(const std::string &expression,
                           const std::string &string_to_find,
                           const std::string &delims = "/");
    
  /**
   * Return a reference to the getpot object to extract options from the input file
   */
  const GetPot * getPotHandle() const;

  /**
   * Get the executioner
   */
  Executioner * getExecutioner();

  /**
   * This function attempts to extract values from the input file based on the contents of
   * the passed parameters objects.  It handles a number of various types with dynamic casting
   * including vector types
   */
  void extractParams(const std::string & prefix, InputParameters &p);

  /**
   * prints a standard cli usage message
   */
  void printUsage() const;

  ParserBlock * root() { return _input_tree; }

public:
  // data created while running execute()
  MooseMesh *_mesh;
  MProblem * _problem;
  Executioner *_executioner;

  ExodusII_IO *_exreader;                               /// auxiliary object for restart
  bool _loose;                                          /// true if parsing input file with loose syntax

private:
  /**
   * This function inserts blocks into the tree which are optional in the input file but are
   * necessary for the correct execution of MOOSE based applications.
   */
  void fixupOptionalBlocks();

  /**
   * Use MOOSE Factories to construct a full parse tree for documentation. Format
   * parameter specifies how to print the resulting parse tree. Only "dump", the
   * original human readable format, and "yaml" are supported.
   */
  void buildFullTree( const std::string format );

  /**
   * This function check for the existance and readability of the input file and throws a
   * MOOSE error if there is a problem
   */
  void checkInputFile();
  
  /**
   * Helper functions for setting parameters of arbitrary types - bodies are in the .C file
   * since they are colled only from this Object
   */
  template<typename T>
  void setScalarParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<T>* param, bool in_global, GlobalParamsAction *global_block);
  
  template<typename T>
  void setVectorParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<T> >* param, bool in_global, GlobalParamsAction *global_block);

  template<typename T>
  void setTensorParameter(const std::string & full_name, const std::string & short_name, InputParameters::Parameter<std::vector<std::vector<T> > >* param, bool in_global, GlobalParamsAction *global_block);

  /************************************
   * Private Data Members
   ************************************/
  std::string _input_filename;
  std::vector<std::string> _section_names;
  const std::string _dump_string;
  const static std::string _show_tree;

  /**
   * Pointer to the parser block tree built from the call to "parse"
   */
  ParserBlock *_input_tree;
  
  bool _getpot_initialized;
  bool _tree_printed;
  GetPot _getpot_file;

  std::list<ParserBlock *> _deferred_execution;
  std::set<std::string> _executed_blocks;
};

#endif //PARSER_H
