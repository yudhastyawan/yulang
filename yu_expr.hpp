#ifndef __YUEXPR__H
#define __YUEXPR__H

#include <list>
#include <utility>
#include <string>
#include <map>
#include <math.h>
#include <iostream>

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
    std::list<expression> children;
    std::list<expression *> childf;

    template <class... T>
    expression(ex e, T&&... args) : ex_type(e), children {std::forward<T>(args)... } {}
    expression(ex e, std::list<expression *>&& c) : ex_type(e), childf(c) {std::cout << "size: " << childf.size() << std::endl;}
    expression() : ex_type(ex::none) {}
    expression(long v) : ex_type(ex::integer), int_val(v) {}
    expression(double v) : ex_type(ex::flt), flt_val(v) {}
    expression(const std::string& str) : ex_type(ex::ident), id_type(id::variable) {}
    expression(const std::string& str, id t) : ex_type(ex::ident), id_type(t) {}
    void assign(expression&& e)
    {
        if(children.size() != 0) children.clear();
        children.push_back(std::move(e));
    }

    template <class T>
    T eval()
    {
        T v = 0;
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
            default: s = "not handled";
        }
        std::cout << "tree: " << s << " -> C:" << children.size() << ", F:" << childf.size() << " " << this << std::endl;
        #undef PROCESS_VAL
        switch(ex_type)
        {
            case ex::integer: v = int_val; break;
            case ex::flt: v = flt_val; break;
            case ex::add: for( auto i : children ) v += i.eval<T>(); break;
            case ex::subs: v = children.front().eval<T>() - children.back().eval<T>(); break;
            case ex::mul: v = children.front().eval<T>() * children.back().eval<T>(); break;
            case ex::div: v = children.front().eval<T>() / children.back().eval<T>(); break;
            case ex::pow: v = pow(children.front().eval<T>(), children.back().eval<T>()); break;
            case ex::mod: v = (int)children.front().eval<T>() % (int)children.back().eval<T>(); break;
            case ex::neg: v = - children.back().eval<T>(); break;
            case ex::addf: for( auto i : childf ) v += i->eval<T>(); break;
            case ex::subsf: v = childf.front()->eval<T>() - childf.back()->eval<T>(); break;
            case ex::mulf: v = childf.front()->eval<T>() * childf.back()->eval<T>(); break;
            case ex::divf: v = childf.front()->eval<T>() / childf.back()->eval<T>(); break;
            case ex::powf: v = pow(childf.front()->eval<T>(), childf.back()->eval<T>()); break;
            case ex::modf: v = (int)childf.front()->eval<T>() % (int)childf.back()->eval<T>(); break;
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
    std::map<std::string, scopes> curr_scope;

    scopes(scopes *pre, const std::string& n, expression&& e) : name(n), expr(std::move(e)), prev(pre) {}
    scopes(scopes *pre, const std::string& n) : name(n), prev(pre) {}
    scopes() {
        curr_scope.emplace("main", scopes(this, "main"));
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