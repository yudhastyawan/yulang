#include <iostream>
#include <cstdlib>
#include <cstring>

#include "yu_driver.hpp"

#define HELP_STR \
std::cout << "use " << color::blue << "yulang -c" << color::norm << " for an interactive console\n"; \
std::cout << "use " << color::blue << "yulang <filename>" << color::norm << " to run the script file\n"; \
std::cout << "use " << color::blue << "yulang -h" << color::norm << " to get this menu\n";

int 
main( const int argc, const char **argv )
{
   /** check for the right # of arguments **/
   if( argc == 2 )
   {
      yu::yu_driver driver;
      /** example for piping input from terminal, i.e., using cat **/ 
      if( std::strncmp( argv[ 1 ], "-c", 2 ) == 0 )
      {
         driver.parse( std::cin );
      }
      /** simple help menu **/
      else if( std::strncmp( argv[ 1 ], "-h", 2 ) == 0 )
      {
         HELP_STR
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
      HELP_STR
      return( EXIT_SUCCESS );
   }
   return( EXIT_SUCCESS );
}
