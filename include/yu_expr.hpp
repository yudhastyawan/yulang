#ifndef __YUEXPR__H
#define __YUEXPR__H

#include <list>
#include <utility>
#include <string>
#include <map>
#include <math.h>
#include <iostream>
#include <string>

struct expr_val{
    double d;
    std::string s;
    expr_val(double i): d(i) {}
    expr_val(std::string ss): s(ss) {}
    expr_val& operator=(double i) {this->d = i; return *this; }
    expr_val& operator+=(double i) {this->d += i; return *this; }
    expr_val& operator+=(const expr_val& ee) {this->d += ee.d; return *this; }
    expr_val& operator-(const expr_val& ee) {this->d -= ee.d; return *this; }
    expr_val& operator*(const expr_val& ee) {this->d *= ee.d; return *this; }
    expr_val& operator/(const expr_val& ee) {this->d /= ee.d; return *this; }
    expr_val& operator+(const expr_val& ee) {this->d += ee.d; return *this; }
    expr_val& pows(const expr_val& ee) {this->d = pow(this->d, ee.d); return *this; }
    expr_val& operator%(const expr_val& ee) {this->d = (int)this->d % (int)ee.d; return *this; }
    expr_val& operator-() {this->d = -(this->d); return *this; }
    expr_val& operator=(const std::string& ss) {this->s = ss; return *this; }
};

namespace yu {

enum class id { undefined, variable, function, parameter };
enum class ex { none, str, integer, flt, ident,
                add, subs, neg, mul, div, mod, pow,
                addf, subsf, negf, mulf, divf, modf, powf,
                cor, cand, loop,
                addrof, deref,
                fcall,
                assign,
                copy,
                comma,
                ret };

struct expression {
    expression * ptrthis = this;
    ex ex_type = ex::none;
    id id_type = id::undefined;
    long int_val = 0;
    double flt_val = 0.;
    std::string str_val;
    int is_debug = 0;
    std::list<expression> children;
    std::list<expression *> childf;

    template <class... T>
    expression(int d, ex e, T&&... args) : ex_type(e), is_debug(d), children {std::forward<T>(args)... } {}
    expression(int d, ex e, std::list<expression *>&& c) : ex_type(e), is_debug(d), childf(c) {}
    expression() : ex_type(ex::none) {}
    expression(int d, long v) : ex_type(ex::integer), int_val(v), is_debug(d) {}
    expression(int d, double v) : ex_type(ex::flt), flt_val(v), is_debug(d) {}
    expression(int d, const std::string& str) : ex_type(ex::ident), id_type(id::variable), is_debug(d) {}
    expression(int d, const std::string& str, id t) : ex_type(ex::ident), id_type(t), is_debug(d) {}
    expression(int d, const std::string& str, ex e) : ex_type(e), id_type(id::variable), str_val(str), is_debug(d) {}
    void assign(expression&& e)
    {
        if(children.size() != 0) children.clear();
        children.push_back(std::move(e));
    }

    void copy(const expression& e) {
        ex_type = e.ex_type;
        id_type = e.id_type;
        int_val = e.int_val;
        flt_val = e.flt_val;
        is_debug = e.is_debug;
        children = e.children;
        childf = e.childf;
    }

    template <class T>
    T eval()
    {
        T v = 0;
        if (is_debug == 1) {
            #define PROCESS_VAL(p) case(p): s = #p; break;
            std::string s;
            switch(ex_type){
                PROCESS_VAL(ex::integer);
                PROCESS_VAL(ex::flt);
                PROCESS_VAL(ex::add);
                PROCESS_VAL(ex::subs);
                PROCESS_VAL(ex::mul);
                PROCESS_VAL(ex::div);
                PROCESS_VAL(ex::pow);
                PROCESS_VAL(ex::mod);
                PROCESS_VAL(ex::neg);
                PROCESS_VAL(ex::ident);
                /* not handled
                { 
                    cor, cand, loop,
                    fcall }; */
                PROCESS_VAL(ex::str);
                PROCESS_VAL(ex::ret);
                PROCESS_VAL(ex::assign);
                PROCESS_VAL(ex::none);
                PROCESS_VAL(ex::comma);
                PROCESS_VAL(ex::copy);
                default: s = "cor, cand, loop, or *f";
            }
            std::cout << "tree: " << s << " -> C:" << children.size() << ", F:" << childf.size() << " " << this << std::endl;
            #undef PROCESS_VAL
        }
        switch(ex_type)
        {
            case ex::integer: v = int_val; break;
            case ex::flt: v = flt_val; break;
            case ex::str: v = str_val; break;
            case ex::add: for( auto i : children ) v += i.eval<T>(); break;
            case ex::subs: v = children.front().eval<T>() - children.back().eval<T>(); break;
            case ex::mul: v = children.front().eval<T>() * children.back().eval<T>(); break;
            case ex::div: v = children.front().eval<T>() / children.back().eval<T>(); break;
            case ex::pow: v = children.front().eval<T>().pows(children.back().eval<T>()); break;
            case ex::mod: v = children.front().eval<T>() % children.back().eval<T>(); break;
            case ex::neg: v = - children.back().eval<T>(); break;
            case ex::addf: for( auto i : childf ) v += i->eval<T>(); break;
            case ex::subsf: v = childf.front()->eval<T>() - childf.back()->eval<T>(); break;
            case ex::mulf: v = childf.front()->eval<T>() * childf.back()->eval<T>(); break;
            case ex::divf: v = childf.front()->eval<T>() / childf.back()->eval<T>(); break;
            case ex::powf: v = childf.front()->eval<T>().pows(childf.back()->eval<T>()); break;
            case ex::modf: v = childf.front()->eval<T>() % childf.back()->eval<T>(); break;
            case ex::negf: v = - childf.back()->eval<T>(); break;
            case ex::ident: v = children.back().eval<T>(); break;
            default: v = 0;
        }
        return v;
    }
};

struct scopes {
    std::string name = "program";
    expression expr;
    scopes *prev = nullptr;
    scopes *ret = nullptr;
    int int_params = 0;
    std::map<std::string, scopes> curr_scope;
    std::vector<std::string> loc_params;

    scopes(scopes *pre, const std::string& n, expression&& e) : name(n), expr(std::move(e)), prev(pre) {}
    scopes(scopes *pre, const std::string& n) : name(n), prev(pre) {}
    scopes() {
        curr_scope.emplace("main", scopes(this, "main"));
    }
    void param_push(std::string s) {
        loc_params.push_back(s);
        int_params++;
    }

    void param_reset()
    {
        int_params = 0;
    }

    std::string param_pop()
    {
        std::string s = loc_params[int_params];
        int_params++;
        return s;
    }

    void assign(scopes * sc)
    {
        expr = sc->expr;
        curr_scope = sc->curr_scope;
    }

    // void assignptr(scopes * sc)
    // {
    //     expr.ex_type = sc->expr.ex_type;

    // }
};

}

#endif