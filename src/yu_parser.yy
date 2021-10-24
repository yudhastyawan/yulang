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
#define C(...) yu::expression(driver.is_debug, __VA_ARGS__ )
#define CS(...) yu::expression(driver.is_debug, __VA_ARGS__, yu::ex::str)
typedef std::list<yu::expression *> listeptr_t;
}

%define api.value.type variant
%define parse.assert

%token END 0
%token<long> INTEGER
%token<double> FLOAT
%token<std::string> VARIABLE STRING
%token PRINT RETURN LOC VAR
%type <yu::scopes *> exp var_defs params fun_def fun_call args expf param_defs print_exps
%type <yu::scopes> var_def1 arg_def1 param1

%right '='
%left '+' '-'
%left '*' '/'
%left '^' '%'
%nonassoc UMINUS
%nonassoc PRINT

%locations
%start calclist

%%

calclist: %empty
   | calclist stmt ';' { if (driver.is_interactive == 1) std::cout << "y> "; }
   | calclist PRINT LOC ';' { 
                              driver.console("parent: " + driver.sc_in->prev->name + ", loc: " + driver.sc_in->name);
                              if (driver.is_debug == 1) {
                              for (const auto& elem : driver.sc_in->curr_scope)
                              {
                                 std::cout << elem.first << " " << &(elem.second.expr) << " S:";
                                 std::cout << elem.second.curr_scope.size() << ", C:";
                                 std::cout << elem.second.expr.children.size() << ", F:";
                                 std::cout << elem.second.expr.childf.size() << std::endl;
                              }
                              std::cout << "-----" << std::endl;
                              }
                              if (driver.is_interactive == 1) std::cout << "y> ";
                            }
   | calclist ob calclist cb { if (driver.is_interactive == 1) std::cout << "y> "; }
   | calclist fun_def { if (driver.is_interactive == 1) std::cout << "y> "; }
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
            driver.tmp_fdef.push_back(driver.defun($1));
            driver.sc_out = driver.sc_in; 
            driver.sc_in->curr_scope.emplace($1, scopes(driver.sc_in, $1)); 
            driver.sc_in = &(driver.sc_in->curr_scope[$1]); 
         }
         params ':' '{' listf 
         RETURN expf ';'
         '}' 
         {  
            driver.tmp_fdef.back()->ret = $8;
            driver.sc_in = driver.sc_out;
            driver.sc_out = driver.sc_in->prev;
            driver.tmp_fdef.back()->param_reset();
            $$ = driver.tmp_fdef.back();
            driver.tmp_fdef.pop_back();
         }
   ;

fun_call: VARIABLE {
            driver.tmp_fcall.push_back(driver.usefun($1));
            driver.sc_out = driver.sc_in;
            driver.sc_in = &(driver.tmp_fcall.back()->prev->curr_scope[$1]);
         } 
         '(' args ')'
         {
            if (driver.is_debug == 1) {
            std::cout << "inside [" << $1 << "]: C:" << driver.tmp_fcall.back()->ret->expr.children.size() << ", S:";
            std::cout << driver.tmp_fcall.back()->curr_scope.size() << std::endl;
            for (const auto& elem : driver.tmp_fcall.back()->curr_scope)
            {
               std::cout << elem.first << " " << &(elem.second.expr) << "\n";
            }
            std::cout << "ret: " << &(driver.tmp_fcall.back()->ret->expr) << std::endl;
            std::cout << "-----" << std::endl;
            }
            $$ = driver.tmp_fcall.back()->ret;
            driver.tmp_fcall.pop_back();
            driver.sc_in = driver.sc_out;
            driver.sc_out = driver.sc_in->prev;
         }
   ;

args: %empty
   | arg_def1 { auto tmp = &(driver.tmp_fcall.back()->curr_scope[$1.name]); tmp->assign($1.prev); }
   | args ',' arg_def1 { auto tmp = &(driver.tmp_fcall.back()->curr_scope[$3.name]); tmp->assign($3.prev); }

arg_def1: VARIABLE ':' expf { $$ = yu::scopes($3, $1); }
   | expf { std::string str_arg =  driver.tmp_fcall.back()->param_pop(); $$ = yu::scopes($1, str_arg); }
   ;

params: %empty
   | param1 { driver.tmp_fdef.back()->param_push($1.name); $$ = driver.def($1.name); if($1.prev) $$->assign($1.prev);}
   | params ',' param1 { driver.tmp_fdef.back()->param_push($3.name); $$ = driver.def($3.name); if($3.prev) $$->assign($3.prev); }
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

print_exps: exp { $$ = $1; 
                  expr_val extmp = $1->expr.eval<expr_val>(); 
                  if(!extmp.s.empty()) driver.console(extmp.s); 
                  else driver.console(extmp.d);
                  }
   | print_exps ',' exp { $$ = $3; 
                  expr_val extmp = $3->expr.eval<expr_val>(); 
                  if(!extmp.s.empty()) driver.console(extmp.s); 
                  else driver.console(extmp.d);}
   ;

exp: INTEGER { $$ = driver.temp(); $$->expr.assign(C($1)); }
   | FLOAT { $$ = driver.temp(); $$->expr.assign(C($1)); }
   | VARIABLE { $$ = driver.use($1); }
   | STRING { $$ = driver.temp(); $$->expr.assign(CS($1)); }
   | fun_call { $$ = $1; }
   | exp '=' exp { /*$$ = driver.temp(); $$->expr.copy($1->expr); $$->assign($3); $1 = $$;*/ $1->assign($3); $$ = $1; }
   | exp '+' exp { $$ = driver.temp(); $$->expr.assign(C(yu::ex::add, $1->expr, $3->expr)); }
   | exp '-' exp { $$ = driver.temp(); $$->expr.assign(C(yu::ex::subs, $1->expr, $3->expr)); }
   | '-' exp %prec UMINUS { $$ = driver.temp(); $$->expr.assign(C(yu::ex::neg, $2->expr)); }
   | exp '*' exp { $$ = driver.temp(); $$->expr.assign(C(yu::ex::mul, $1->expr, $3->expr)); }
   | exp '/' exp { $$ = driver.temp(); $$->expr.assign(C(yu::ex::div, $1->expr, $3->expr)); }
   | exp '^' exp { $$ = driver.temp(); $$->expr.assign(C(yu::ex::pow, $1->expr, $3->expr)); }
   | exp '%' exp { $$ = driver.temp(); $$->expr.assign(C(yu::ex::mod, $1->expr, $3->expr)); }
   | '(' exp ')' { $$ = $2; }
   | PRINT '(' print_exps ')' { $$ = $3; }
   ;

expf: INTEGER { $$ = driver.temp(); $$->expr.assign(C($1)); }
   | FLOAT { $$ = driver.temp(); $$->expr.assign(C($1)); }
   | VARIABLE { $$ = driver.use($1); }
   | fun_call { $$ = $1; }
   | expf '=' expf { $$ = driver.temp(); $$->expr.copy($1->expr); $$->assign($3); $1 = $$; /*$1->assign($3); $$ = $1;*/ }
   | expf '+' expf { $$ = driver.temp(); $$->expr.assign(C(yu::ex::addf, listeptr_t{&($1->expr), &($3->expr)})); }
   | expf '-' expf { $$ = driver.temp(); $$->expr.assign(C(yu::ex::subsf, listeptr_t{&($1->expr), &($3->expr)})); }
   | '-' expf %prec UMINUS { $$ = driver.temp(); $$->expr.assign(C(yu::ex::negf, listeptr_t{&($2->expr)})); }
   | expf '*' expf { $$ = driver.temp(); $$->expr.assign(C(yu::ex::mulf, listeptr_t{&($1->expr), &($3->expr)})); }
   | expf '/' expf { $$ = driver.temp(); $$->expr.assign(C(yu::ex::divf, listeptr_t{&($1->expr), &($3->expr)})); }
   | expf '^' expf { $$ = driver.temp(); 
                     $$->expr.assign(C(yu::ex::powf, listeptr_t{&($1->expr), &($3->expr)})); }
   | expf '%' expf { $$ = driver.temp(); $$->expr.assign(C(yu::ex::modf, listeptr_t{&($1->expr), &($3->expr)})); }
   | '(' expf ')' { $$ = $2; }
   ;

%%


void 
yu::yu_parser::error( const location_type &l, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
