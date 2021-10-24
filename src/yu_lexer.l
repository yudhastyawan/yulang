%{
#include <string>
#include <fstream>
#include <vector>
#include <utility>
#include <regex>

#include "yu_scanner.hpp"
#undef  YY_DECL
#define YY_DECL int yu::yu_scanner::yylex( yu::yu_parser::semantic_type * const lval, yu::yu_parser::location_type * loc )

using token = yu::yu_parser::token;
#define yyterminate() return( token::END )
#define YY_NO_UNISTD_H
#define YY_USER_ACTION loc->step(); loc->columns(yyleng);
std::vector<YY_BUFFER_STATE> inc_vec;
std::vector<std::ifstream*> fil_vec;
%}

%option debug
%option nodefault
%option yyclass="yu::yu_scanner"
%option noyywrap
%option c++

%x IFILE
%x COMMENTS

%%
%{          /** Code executed at the beginning of yylex **/
            yylval = lval;
%}
import:[ \t]*[\"<]+ { BEGIN(IFILE); }
<IFILE>[^\t\n\">]+ {
    {
        int c;
        while((c = yyinput()) && c != '\n');
    }
    std::ifstream* f = new std::ifstream(yytext);
    fil_vec.push_back(std::move(f));
    if(fil_vec.back()->good())
    {
    inc_vec.push_back(YY_CURRENT_BUFFER);
    yy_switch_to_buffer(yy_create_buffer(fil_vec.back(), YY_BUF_SIZE));
    }
    BEGIN(INITIAL);
}

<<EOF>> {   
            if(inc_vec.size() == 0) yyterminate();
            else {
            if(fil_vec.back()->good())
            {
            fil_vec.back()->close();
            yy_delete_buffer(YY_CURRENT_BUFFER);
            delete fil_vec.back();
            yy_switch_to_buffer(inc_vec.back());
            inc_vec.pop_back();
            fil_vec.pop_back();
            }
            }
        }


"/>" { BEGIN(COMMENTS); }
<COMMENTS>{
    [ \t\n]+ {}
    "</" { BEGIN(INITIAL); }
    . { }
}


["]([^"\\\n]|\\(.|\n))*["] { 
std::string text(yytext);
text = std::regex_replace(text, std::regex("[\"]"), "");
text = std::regex_replace(text, std::regex(R"(\\n)"), "\n");
text = std::regex_replace(text, std::regex(R"(\\t)"), "\t");
text = std::regex_replace(text, std::regex(R"(\\\\)"), "\\");
yylval->emplace<std::string>(std::string(text)); return token::STRING;
}

"+" |
"-" |
"*" |
";" |
"(" |
")" |
"=" |
"{" |
"}" |
":" |
"^" |
"%" |
"," |
"/" { return yytext[0]; }
"exit" { exit(EXIT_SUCCESS); }
"print" { return token::PRINT; }
"return" { return token::RETURN; }
"loc" { return token::LOC; }
"var" { return token::VAR; }
"//".* /* ignore comments */
[0-9]+ { yylval->emplace<long>(atol(yytext)); return token::INTEGER; }
[0-9]+"."[0-9]* |
[0-9]*"."[0-9]+ { yylval->emplace<double>(std::stod(yytext)); return token::FLOAT; }
[a-zA-Z][a-zA-Z0-9_-]* { yylval->emplace<std::string>(yytext); return token::VARIABLE; }
[ \t\n]+ {  }
. {  }
%%
