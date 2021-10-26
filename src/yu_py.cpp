#include <iostream>
#include <sstream>

#include "yu_driver.hpp"
#include "yu_py.hpp"

#define YU_VERSION "0.0.1"
#define YU_STABILITY 0

#define INTRO_STR \
std::cout << "Yu Language " << YU_VERSION << " (" << (YU_STABILITY == 0 ? "unstable" : "stable") << ", "; \
std::cout << __DATE__ << ", " << __TIME__ << ")" << std::endl; \

void parse(const char* input)
{
    yu::yu_driver driver;
    std::istringstream istr(input);
    driver.parse(istr);
}

void read(const char* filename)
{
    yu::yu_driver driver;
    driver.parse(filename);
}

void console()
{
    INTRO_STR
    yu::yu_driver driver;
    driver.is_interactive = 1;
    std::cout << "interactive:" << std::endl << "y> ";
    driver.parse( std::cin );
}