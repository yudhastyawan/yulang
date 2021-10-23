#ifndef __YUSCANNER_HPP__
#define __YUSCANNER_HPP__ 1

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "yu_parser.tab.hh"
#include "location.hh"

namespace yu{

class yu_scanner : public yyFlexLexer{
public:
   
   yu_scanner(std::istream *in) : yyFlexLexer(in)
   {
   };
   virtual ~yu_scanner() {
   };

   //get rid of override virtual function warning
   using FlexLexer::yylex;

   virtual
   int yylex( yu::yu_parser::semantic_type * const lval, 
              yu::yu_parser::location_type * location );
   // YY_DECL defined in mc_lexer.l
   // Method body created by flex in mc_lexer.yy.cc

private:
   /* yyval ptr */
   yu::yu_parser::semantic_type *yylval = nullptr;
};

} /* end namespace MC */

#endif /* END __MCSCANNER_HPP__ */
