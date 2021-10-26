#ifndef __MCDRIVER_HPP__
#define __MCDRIVER_HPP__ 1

#include <string>
#include <cstddef>
#include <istream>
#include <vector>
#include <map>
#include <list>
#include <utility>

#include "yu_scanner.hpp"
#include "yu_parser.tab.hh"
#include "yu_expr.hpp"

namespace color {
const std::string red   = "\033[1;31m";
const std::string blue  = "\033[1;36m";
const std::string norm  = "\033[0m";
}

namespace yu{

class yu_driver{
public:
   yu_driver() = default;

   virtual ~yu_driver();
   
   /** 
    * parse - parse from a file
    * @param filename - valid string with input file
    */
   void parse( const char * const filename );
   /** 
    * parse - parse from a c++ input stream
    * @param is - std::istream&, valid input stream
    */
   void parse( std::istream &iss );

   template <typename A>
   std::ostream& console_basic(const A& arg){
        return (std::cout << "print: " << arg << std::endl);
   }

   std::ostream& console(double arg);
   std::ostream& console(int arg);
   std::ostream& console(std::string arg);

   std::ostream& errcon(const std::string& err);

   yu::scopes* define(const std::string& name, yu::expression&& e);
   yu::scopes* def(const std::string& name);
   yu::scopes* defun(const std::string& name);
   yu::scopes* use(const std::string& name);
   yu::scopes* temp();
   yu::scopes* usefun(const std::string& name);

   yu::scopes* scope = new scopes();
   yu::scopes* sc_in = &(scope->curr_scope["main"]);
   yu::scopes* sc_out = &(scope->curr_scope["main"]);
   std::vector<yu::scopes*> tmp_fdef;
   std::vector<yu::scopes*> tmp_fcall;

   int is_interactive = 0;
   int is_debug = 0;
   
private:

   void parse_helper( std::istream &stream );

   yu::yu_parser  *parser  = nullptr;
   yu::yu_scanner *scanner = nullptr;

   unsigned tempcounter = 0;

};

} /* end namespace MC */

#endif /* END __MCDRIVER_HPP__ */
