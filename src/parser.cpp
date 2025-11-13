/**
 * @file parser.cpp
 * @brief Parsing implementation for Scheme syntax tree to expression tree conversion
 * 
 * This file implements the parsing logic that converts syntax trees into
 * expression trees that can be evaluated.
 * primitive operations, and function applications.
 */

#include "RE.hpp"
#include "Def.hpp"
#include "syntax.hpp"
#include "value.hpp"
#include "expr.hpp"
#include <map>
#include <string>
#include <iostream>

using std::string;
using std::vector;
using std::pair;

extern std::map<std::string, ExprType> primitives;
extern std::map<std::string, ExprType> reserved_words;
extern Syntax readSyntax(std::istream &);
/**
 * @brief Default parse method (should be overridden by subclasses)
 */
Expr Syntax::parse(Assoc &env) {
    throw RuntimeError("Unimplemented parse method");
}

Expr Number::parse(Assoc &env) {
    return Expr(new Fixnum(n));
}

Expr RationalSyntax::parse(Assoc &env) {
    return Expr(new RationalNum(numerator,denominator));
}

Expr SymbolSyntax::parse(Assoc &env) {
    return Expr(new Var(s));
}

Expr StringSyntax::parse(Assoc &env) {
    return Expr(new StringExpr(s));
}

Expr TrueSyntax::parse(Assoc &env) {
    return Expr(new True());
}

Expr FalseSyntax::parse(Assoc &env) {
    return Expr(new False());
}

Expr List::parse(Assoc &env) {
    if (stxs.empty()) {
        return Expr(new ListFunc(vector<Expr>()));
    }
    SymbolSyntax *id = dynamic_cast<SymbolSyntax*>(stxs[0].get());

    if (id == nullptr) {
        vector<Expr> parameters;
        for(int i=1;i<stxs.size();++i){
            parameters.push_back(stxs[i]->parse(env));
        }
        Expr rat=stxs[0]->parse(env);
        return Expr(new Apply(rat,parameters));
    }else{
    string op = id->s;
    if (find(op, env).get() != nullptr) {
        Value found = find(op, env);
        if (found.get() != nullptr) {
        vector<Expr> parameters;
        for (int i = 1; i < stxs.size(); ++i) {
            parameters.push_back(stxs[i]->parse(env));
        }
            Expr rat = stxs[0]->parse(env);
            return Expr(new Apply(rat, parameters));
        }
    }
    if (reserved_words.count(op) != 0) {
        if(op=="begin"){
            vector<Expr> parameters;
            for(int i=1;i<stxs.size();++i){
                parameters.push_back(stxs[i]->parse(env));
            }
            return Expr(new Begin(parameters));
        }
        if(op=="quote"){
            if (stxs.size() != 2) {
                throw RuntimeError("quote: requires 1 argument");
            }
            return Expr(new Quote(stxs[1]));
        }
        if(op=="if"){
            if (stxs.size() <3||stxs.size()>4) {
                throw RuntimeError("if: requires 2 or 3 argument");
            }
            vector<Expr> parameters;
            for(int i=1;i<stxs.size();++i){
                parameters.push_back(stxs[i]->parse(env));
            }
            Expr alter = stxs.size()==4 ? parameters[2] : Expr(new MakeVoid());
            return Expr(new If(parameters[0],parameters[1],alter));
        }
        if(op=="cond"){
            vector<vector<Expr>> tmp;
            for (int i=1;i<stxs.size();++i){
                auto clause_list=dynamic_cast<List*>(stxs[i].get());
                if(clause_list==nullptr){throw RuntimeError("cond:list needed");}
                vector<Expr> clauses;
                for(auto& item : clause_list->stxs){
                    clauses.push_back(item->parse(env));
                }
                tmp.push_back(clauses);
            }
            return Expr(new Cond(tmp));
        }
        if(op=="lambda"){
            if (stxs.size() < 3) {
                throw RuntimeError("lambda: requires parameter list and body");
            }
            auto paramList = dynamic_cast<List*>(stxs[1].get());
            if (paramList==nullptr) {
                throw RuntimeError("lambda: parameter list must be a list");
            }
            vector<string> params;
            for (auto& param : paramList->stxs) {
                auto sym = dynamic_cast<SymbolSyntax*>(param.get());
                if (sym==nullptr) {
                    throw RuntimeError("lambda: parameters must be symbols");
                }
                params.push_back(sym->s);
            }
            Assoc new_env = env;
            for (const auto& param : params) {
                new_env = extend(param, NullV(), new_env);
            }
            vector<Expr> bodyExprs;
            for (int i = 2; i < stxs.size(); ++i) {
                bodyExprs.push_back(stxs[i]->parse(new_env));
            }
            Expr body(nullptr);
            if (bodyExprs.size() == 1) {
                body = bodyExprs[0];
            } else {
                body = Expr(new Begin(bodyExprs));
            }
            return Expr(new Lambda(params, body));
        }   
        if(op=="define"){
            if(stxs.size()<3){
                throw RuntimeError("define: requires at least 2 augments");
            }
            auto varSym = dynamic_cast<SymbolSyntax*>(stxs[1].get());
            if(varSym!=nullptr){
                vector<Expr> parameters;
                for(int i=2;i<stxs.size();++i){
                    parameters.push_back(stxs[i]->parse(env));
                }
                if(parameters.size()!=1){throw RuntimeError("define: only 1 var can be defined");}
                return Expr(new Define(varSym->s,parameters[0]));
            }//定义的是一个变量
            else{
                auto funcDef = dynamic_cast<List*>(stxs[1].get());
                if (funcDef==nullptr || funcDef->stxs.empty()) {
                    throw RuntimeError("define: invalid function definition");
                }
                auto funcNameSym = dynamic_cast<SymbolSyntax*>(funcDef->stxs[0].get());
                if (funcNameSym==nullptr) {
                    throw RuntimeError("define: function name must be a symbol");
                }
                vector<string> params;
                for (int i = 1; i < funcDef->stxs.size(); ++i) {
                    auto paramSym = dynamic_cast<SymbolSyntax*>(funcDef->stxs[i].get());
                    if (paramSym==nullptr) {
                        throw RuntimeError("define: parameters must be symbols");
                    }
                    params.push_back(paramSym->s);
                }
                vector<Expr> bodyExprs;
                for(int i=2;i<stxs.size();++i){
                    bodyExprs.push_back(stxs[i]->parse(env));
                }
                Expr body(nullptr);
                if(bodyExprs.size()==1){
                    body=bodyExprs[0];
                }else{
                    body=Expr(new Begin(bodyExprs));
                }
                Expr lambda = Expr(new Lambda(params, body));
                return Expr(new Define(funcNameSym->s, lambda));
            }//定义的是一个函数
        }
        if(op=="set!"){
            if (stxs.size() != 3) {
                throw RuntimeError("set!: requires 2 arguments");
            }
            auto varSym = dynamic_cast<SymbolSyntax*>(stxs[1].get());
            if (varSym==nullptr) {
                throw RuntimeError("set!: first argument must be a variable");
            };
            Expr value =stxs[2]->parse(env);
            return Expr(new Set(varSym->s, value));
        }     
        if (op == "let") {
            if (stxs.size() < 2) {
                throw RuntimeError("let: requires bindings and body");
            }
            auto bindings_list = dynamic_cast<List*>(stxs[1].get());
            if (bindings_list==nullptr) {
                throw RuntimeError("let: bindings must be a list");
            }
            std::vector<std::pair<std::string, Expr>> bindings;
            for (auto& binding_syntax : bindings_list->stxs) {
                auto binding = dynamic_cast<List*>(binding_syntax.get());
                if (binding==nullptr || binding->stxs.size() != 2) {
                    throw RuntimeError("let: each binding must be (variable value)");
                }
                auto var_sym = dynamic_cast<SymbolSyntax*>(binding->stxs[0].get());
                if (var_sym==nullptr) {
                    throw RuntimeError("let: variable must be a symbol");
                }
                Expr value_expr = binding->stxs[1]->parse(env);
                bindings.push_back({var_sym->s, value_expr});
            }
            Assoc new_env = env;
            for (const auto& binding : bindings) {
                new_env = extend(binding.first, NullV(), new_env);
            }
            std::vector<Expr> body_exprs;
            for (size_t i = 2; i < stxs.size(); i++) {
                body_exprs.push_back(stxs[i]->parse(new_env));
            }
            Expr body(nullptr);
            if (body_exprs.size() == 1) {
                body = body_exprs[0];
            } else {
                body = Expr(new Begin(body_exprs));
            }
            return Expr(new Let(bindings, body));
        }
        if (op == "letrec") {
            if (stxs.size() < 2) {
                throw RuntimeError("letrec: requires bindings and body");
            }
            auto bindings_list = dynamic_cast<List*>(stxs[1].get());
            if (bindings_list==nullptr) {
                throw RuntimeError("letrec: bindings must be a list");
            }
    
            std::vector<std::pair<std::string, Expr>> bindings;
            for (auto& binding_syntax : bindings_list->stxs) {
                auto binding = dynamic_cast<List*>(binding_syntax.get());
                if (binding==nullptr || binding->stxs.size() != 2) {
                    throw RuntimeError("letrec: each binding must be (variable value)");
                }
        
                auto var_sym = dynamic_cast<SymbolSyntax*>(binding->stxs[0].get());
                if (var_sym==nullptr) {
                    throw RuntimeError("letrec: variable must be a symbol");
                }
        
                Expr value_expr = binding->stxs[1]->parse(env);
                bindings.push_back({var_sym->s, value_expr});
            }
            std::vector<Expr> body_exprs;
            for (size_t i = 2; i < stxs.size(); i++) {
                body_exprs.push_back(stxs[i]->parse(env));
            }
            Expr body(nullptr);
            if (body_exprs.size() == 1) {
                body = body_exprs[0];
            } else {
                body = Expr(new Begin(body_exprs));
            }
            return Expr(new Letrec(bindings, body));
        } 
    }
    if(primitives.count(op)){
        if(op=="void"){
            if(stxs.size()>1){throw RuntimeError("no params intended");}
            return Expr(new MakeVoid());
        }
        if(op=="exit"){
            if(stxs.size()>1){throw RuntimeError("no params intended");}
            else {return Expr(new Exit());}
        }
        if(op=="and"){
            vector<Expr> parameters;
            for(int i=1;i<stxs.size();++i){
                parameters.push_back(stxs[i]->parse(env));
            }
            return Expr(new AndVar(parameters));
        }
        if(op=="or"){
            vector<Expr> parameters;
            for(int i=1;i<stxs.size();++i){
                parameters.push_back(stxs[i]->parse(env));
            }
            return Expr(new OrVar(parameters));
        }
        if(op=="not"){
            Expr parameter(nullptr);
            if(stxs.size()!=2){throw RuntimeError("not:one param needed");}
            parameter=stxs[1]->parse(env);
            return Expr(new Not(parameter));
        }
        if(op=="set-car!"){
            vector<Expr> parameters;
            if(stxs.size()!=3){throw RuntimeError("set-car:two params needed");}
            for(int i=1;i<=2;++i){
                parameters.push_back(stxs[i]->parse(env));
            }
            return Expr(new SetCar(parameters[0],parameters[1]));
        }
        if(op=="set-cdr!"){
            vector<Expr> parameters;
            if(stxs.size()!=3){throw RuntimeError("set-cdr:two params needed");}
            for(int i=1;i<=2;++i){
                parameters.push_back(stxs[i]->parse(env));
            }
            return Expr(new SetCdr(parameters[0],parameters[1]));
        }
    }
        vector<Expr> parameters;
        for(int i=1;i<stxs.size();++i){
            parameters.push_back(stxs[i]->parse(env));
        }
        Expr rat=stxs[0]->parse(env);
        return Expr(new Apply(rat,parameters));
    }
}
