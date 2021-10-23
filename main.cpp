#include <iostream>
#include <cstdlib>
#include <cstring>
#include <chrono>

#include "yu_driver.hpp"
#include "yu_date.hpp"

#define YU_VERSION "0.0.1"
#define YU_STABILITY 0

#define INTRO_STR \
std::cout << "Yu Language " << YU_VERSION << " (" << (YU_STABILITY == 0 ? "unstable" : "stable") << ", "; \
std::cout << __DATE__ << ", " << __TIME__ << ")" << std::endl; \

#define HELP_STR \
std::cout << "use:\t" << color::blue << "yulang -c" << color::norm << "\t\t= interactive console\n"; \
std::cout << "\t" << color::blue << "yulang <filename>" << color::norm << "\t= running the script file\n"; \
std::cout << "\t" << color::blue << "yulang -v" << color::norm << "\t\t= version\n"; \
std::cout << "\t" << color::blue << "yulang -h" << color::norm << "\t\t= this menu\n"; \
std::cout << "\t" << color::blue << "(...) -d [end line]" << color::norm << "\t= developer/debug monitor\n";

int 
main( const int argc, const char **argv )
{
   /** check for the right # of arguments **/
   if( argc >= 2  && std::strncmp( argv[ 1 ], "-d", 2 ) != 0 )
   {
      yu::yu_driver driver;
      if ( std::strncmp( argv[ argc - 1 ], "-d", 2 ) == 0 )
      {
         driver.is_debug = 1;
      }
      /** example for piping input from terminal, i.e., using cat **/ 
      if( std::strncmp( argv[ 1 ], "-c", 2 ) == 0 )
      {
         INTRO_STR
         driver.is_interactive = 1;
         std::cout << "interactive:" << std::endl << "y> ";
         driver.parse( std::cin );
      }
      /** simple help menu **/
      else if( std::strncmp( argv[ 1 ], "-h", 2 ) == 0 )
      {
         HELP_STR
         return( EXIT_SUCCESS );
      }
      else if( std::strncmp( argv[ 1 ], "-v", 2 ) == 0 )
      {
         std::cout << YU_VERSION << std::endl;
         return( EXIT_SUCCESS );
      }
      /** example reading input from a file **/
      else
      {
         /** assume file, prod code, use stat to check **/
         driver.parse( argv[1] );
      }
   }
   else
   {
      INTRO_STR
      HELP_STR
      return( EXIT_SUCCESS );
   }
   return( EXIT_SUCCESS );
}
