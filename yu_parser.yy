%skeleton "lalr1.cc"
%require  "3.8"
%debug 
%defines 
%define api.namespace {yu}
%define api.parser.class {yu_parser}

%code requires{
#include <string>
#include <cstddef>
#include <istream>
#include <vector>
#include <map>
#include <list>
#include <utility>
#include "yu_expr.hpp"

namespace yu {
   class yu_driver;
   class yu_scanner;
}
}

%parse-param { yu_scanner  &scanner  }
%parse-param { yu_driver  &driver  }

%code{
#include <iostream>
#include <cstdlib>
#include <fstream>

#include "yu_driver.hpp"

#undef yylex
#define yylex scanner.yylex

#define M(x) std::move(x)
#define C(x) yu::expression(x)
}

%define api.value.type variant
%define parse.assert

%token END 0
%token<long> INTEGER
%token<double> FLOAT
%token<std::string> VARIABLE
%token PRINT RETURN LOC VAR
%type <yu::scopes *> exp var_defs params fun_def fun_call args expf param_defs
%type <yu::scopes> var_def1 arg_def1 param1

%right '='
%left '+' '-'
%left '*' '/'
%left '^' '%'
%nonassoc UMINUS

%locations
%start calclist

%%

calclist: %empty
   | calclist stmt ';'
   | calclist PRINT exp ';' { driver.console($3->expr.eval<double>()); }
   | calclist PRINT LOC ';' { 
                              driver.console("parent: " + driver.sc_in->prev->name + ", loc: " + driver.sc_in->name);
                              for (const auto& elem : driver.sc_in->curr_scope)
                              {
                                 std::cout << elem.first << " " << &(elem.second.expr) << " S:";
                                 std::cout << elem.second.curr_scope.size() << ", C:";
                                 std::cout << elem.second.expr.children.size() << ", F:";
                                 std::cout << elem.second.expr.childf.size() << std::endl;
                              }
                              std::cout << "-----" << std::endl;
                            }
   | calclist ob calclist cb
   | calclist fun_def
   ;

ob
   : '{' {
            driver.sc_out = driver.sc_in; 
            driver.sc_in->curr_scope.emplace("local", scopes(driver.sc_in, "local")); 
            driver.sc_in = &(driver.sc_in->curr_scope["local"]); }
   ;

cb
   : '}' {  driver.sc_in = driver.sc_out;
            driver.sc_in->curr_scope.erase("local"); 
            driver.sc_out = driver.sc_in->prev;
         }
   ;

stmt: exp 
   | var_defs
   ;

stmtf: expf 
   | param_defs
   ;

listf: %empty
   | stmtf ';' listf
   | ob listf cb
   | fun_def listf
   ;

fun_def: VARIABLE 
         {
            driver.fun_tmp = driver.defun($1);
            driver.sc_out = driver.sc_in; 
            driver.sc_in->curr_scope.emplace($1, scopes(driver.sc_in, $1)); 
            driver.sc_in = &(driver.sc_in->curr_scope[$1]); 
         }
         params ':' '{' listf 
         RETURN expf ';'
         '}' 
         {  
            driver.fun_tmp->ret = $8;
            driver.sc_in = driver.sc_out;
            driver.sc_out = driver.sc_in->prev;
            $$ = driver.fun_tmp;
         }
   ;

fun_call: VARIABLE {
            driver.fun_tmp = driver.usefun($1);
            //driver.sc_out = driver.sc_in;
            //driver.sc_in = &(driver.fun_tmp->prev->curr_scope[$1]);
         } 
         '(' args ')'
         {
            //std::cout << "inside [" << $1 << "]: C:" << driver.fun_tmp->ret->expr.children.size() << ", S:";
            //std::cout << driver.fun_tmp->curr_scope.size() << std::endl;
            for (const auto& elem : driver.fun_tmp->curr_scope)
            {
               std::cout << elem.first << " " << &(elem.second.expr) << "\n";
            }
            std::cout << "ret: " << &(driver.fun_tmp->ret->expr) << std::endl;
            std::cout << "-----" << std::endl;
            $$ = driver.fun_tmp->ret;
            //driver.sc_in = driver.sc_out;
            //driver.sc_out = driver.sc_in->prev;
         }
   ;

args: %empty
   | arg_def1 { auto tmp = &(driver.fun_tmp->curr_scope[$1.name]); tmp->assign($1.prev); }
   | args ',' arg_def1 { auto tmp = &(driver.fun_tmp->curr_scope[$3.name]); tmp->assign($3.prev); }

arg_def1: VARIABLE '=' expf { $$ = yu::scopes($3, $1);}
   ;

params: %empty
   | param1 { $$ = driver.def($1.name); if($1.prev) $$->assign($1.prev);}
   | params ',' param1 { $$ = driver.def($3.name); if($3.prev) $$->assign($3.prev); }
   ;

param_defs: VAR param1 { $$ = driver.def($2.name); if($2.prev) $$->assign($2.prev);}
   | param_defs ',' param1 { $$ = driver.def($3.name); if($3.prev) $$->assign($3.prev); }
   ;

param1: VARIABLE '=' expf { $$ = yu::scopes($3, $1);}
   | VARIABLE { $$ = yu::scopes(nullptr, $1); }
   ;

var_defs: VAR var_def1 { $$ = driver.def($2.name); if($2.prev) $$->assign($2.prev);}
   | var_defs ',' var_def1 { $$ = driver.def($3.name); if($3.prev) $$->assign($3.prev); }
   ;

var_def1: VARIABLE '=' exp { $$ = yu::scopes($3, $1);}
   | VARIABLE { $$ = yu::scopes(nullptr, $1); }
   ;

exp: INTEGER { $$ = driver.temp(); $$->expr.assign(C($1)); }
   | FLOAT { $$ = driver.temp(); $$->expr.assign(C($1)); }
   | VARIABLE { $$ = driver.use($1); }
   | fun_call { $$ = $1; }
   | exp '=' exp { $1->assign($3); $$ = $1; }
   | exp '+' exp { $$ = driver.temp(); $$->expr.assign(yu::expression(yu::ex::add, M($1->expr), M($3->expr))); }
   | exp '-' exp { $$ = driver.temp(); $$->expr.assign(yu::expression(yu::ex::subs, M($1->expr), M($3->expr))); }
   | '-' exp %prec UMINUS { $$ = driver.temp(); $$->expr.assign(yu::expression(yu::ex::neg, M($2->expr))); }
   | exp '*' exp { $$ = driver.temp(); $$->expr.assign(yu::expression(yu::ex::mul, M($1->expr), M($3->expr))); }
   | exp '/' exp { $$ = driver.temp(); $$->expr.assign(yu::expression(yu::ex::div, M($1->expr), M($3->expr))); }
   | exp '^' exp { $$ = driver.temp(); $$->expr.assign(yu::expression(yu::ex::pow, M($1->expr), M($3->expr))); }
   | exp '%' exp { $$ = driver.temp(); $$->expr.assign(yu::expression(yu::ex::mod, M($1->expr), M($3->expr))); }
   ;

expf: INTEGER { $$ = driver.temp(); $$->expr.assign(C($1)); }
   | FLOAT { $$ = driver.temp(); $$->expr.assign(C($1)); }
   | VARIABLE { $$ = driver.use($1); }
   | fun_call { $$ = $1; }
   | expf '=' expf { printf("assign\n"); $1->assign($3); $$ = $1; }
   | expf '+' expf { $$ = driver.temp(); $$->expr.assign(yu::expression(yu::ex::addf, std::list<expression *>{&($1->expr), &($3->expr)})); }
   | expf '-' expf { $$ = driver.temp(); $$->expr.assign(yu::expression(yu::ex::subsf, std::list<expression *>{&($1->expr), &($3->expr)})); }
   | '-' expf %prec UMINUS { $$ = driver.temp(); $$->expr.assign(yu::expression(yu::ex::negf, std::list<expression *>{&($2->expr)})); }
   | expf '*' expf { $$ = driver.temp(); $$->expr.assign(yu::expression(yu::ex::mulf, std::list<expression *>{&($1->expr), &($3->expr)})); }
   | expf '/' expf { $$ = driver.temp(); $$->expr.assign(yu::expression(yu::ex::divf, std::list<expression *>{&($1->expr), &($3->expr)})); }
   | expf '^' expf { $$ = driver.temp(); 
                     $$->expr.assign(yu::expression(yu::ex::powf, std::list<expression *>{&($1->expr), &($3->expr)})); }
   | expf '%' expf { $$ = driver.temp(); $$->expr.assign(yu::expression(yu::ex::modf, std::list<expression *>{&($1->expr), &($3->expr)})); }
   ;

%%


void 
yu::yu_parser::error( const location_type &l, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}