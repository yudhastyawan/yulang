#include <cctype>
#include <fstream>
#include <cassert>

#include "yu_driver.hpp"

yu::yu_driver::~yu_driver()
{
   delete(scanner);
   scanner = nullptr;
   delete(parser);
   parser = nullptr;
}

void 
yu::yu_driver::parse( const char * const filename )
{
   assert( filename != nullptr );
   std::ifstream in_file( filename );
   if( ! in_file.good() )
   {
       exit( EXIT_FAILURE );
   }
   parse_helper( in_file );
   return;
}

void
yu::yu_driver::parse( std::istream &stream )
{
   if( ! stream.good()  && stream.eof() )
   {
       return;
   }
   //else
   parse_helper( stream ); 
   return;
}


void 
yu::yu_driver::parse_helper( std::istream &stream )
{
   
   delete(scanner);
   try
   {
      scanner = new yu::yu_scanner( &stream );
   }
   catch( std::bad_alloc &ba )
   {
      std::cerr << "Failed to allocate scanner: (" <<
         ba.what() << "), exiting!!\n";
      exit( EXIT_FAILURE );
   }
   
   delete(parser); 
   try
   {
      parser = new yu::yu_parser( (*scanner) /* scanner */, 
                                  (*this) /* driver */ );
   }
   catch( std::bad_alloc &ba )
   {
      std::cerr << color::red << "Failed to allocate parser: (" << 
         ba.what() << "), exiting!!" << color::norm << std::endl;
      exit( EXIT_FAILURE );
   }
   const int accept( 0 );
   if( parser->parse() != accept )
   {
      std::cerr << color::red << "Parse failed!!" << color::norm << std::endl;
   }
   return;
}

std::ostream& yu::yu_driver::console(double arg) {
   return console_basic<double>(arg);
}

std::ostream& yu::yu_driver::console(int arg){
   return console_basic<int>(arg);
}
std::ostream& yu::yu_driver::console(std::string arg){
   return console_basic<std::string>(arg);
}

std::ostream& yu::yu_driver::errcon(const std::string& err){
   return (std::cerr << color::red << err << color::norm << std::endl);
}

yu::scopes* yu::yu_driver::define(const std::string& name, yu::expression&& e)
{
   auto r = sc_in->curr_scope.emplace(name, scopes(sc_in, name, std::move(e)));
   if(!r.second) yu::yu_driver::errcon("duplicate definition " + name + ".");
   return &(r.first->second);
}

yu::scopes* yu::yu_driver::def(const std::string& name)
{
   return yu::yu_driver::define(name, yu::expression(is_debug, name));
}

yu::scopes* yu::yu_driver::defun(const std::string& name)
{
   return yu::yu_driver::define(name, yu::expression(is_debug, name, yu::id::function));
}

static int useref(yu::scopes* in, yu::scopes* out, const std::string& name, yu::scopes** ret)
{
   if(out == nullptr) return 0;
   else if (auto i = in->curr_scope.find(name); i != in->curr_scope.end())
   {
      *ret = &(i->second); 
      return 1; 
   }
   else {
      in = out;
      out = in->prev;
      return useref(in, out, name, ret);
   }
}

static int usefunref(yu::scopes* in, yu::scopes* out, const std::string& name, yu::scopes** ret)
{
   if(out == nullptr) return 0;
   else if (auto i = in->curr_scope.find(name); i->second.expr.id_type == yu::id::function)
   {
      *ret = &(i->second); 
      return 1; 
   }
   else {
      in = out;
      out = in->prev;
      return useref(in, out, name, ret);
   }
}

yu::scopes* yu::yu_driver::use(const std::string& name)
{
   yu::scopes* ret = nullptr;
   if(useref(sc_in, sc_out, name, &ret)) return ret;
   yu::yu_driver::errcon("undefined variable " + name + ".");
   exit(EXIT_FAILURE);
}

yu::scopes* yu::yu_driver::temp()
{
   std::string name = "$I" + std::to_string(tempcounter++);
   return yu::yu_driver::def(name);
}

yu::scopes* yu::yu_driver::usefun(const std::string& name)
{
   yu::scopes* ret = nullptr;
   if(usefunref(sc_in, sc_out, name, &ret)) return ret;
   yu::yu_driver::errcon("undefined function " + name + ".");
   exit(EXIT_FAILURE);
}