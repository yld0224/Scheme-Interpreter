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
        return Expr(new Quote(Syntax(new List())));
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
    if (primitives.count(op) != 0) {
        vector<Expr> parameters;
        for(int i=1;i<stxs.size();++i){
            parameters.push_back(stxs[i]->parse(env));
        }
        ExprType op_type = primitives[op];
        if (op_type == E_PLUS) {
            if (parameters.size() == 2) {
                return Expr(new Plus(parameters[0], parameters[1])); 
            } else {
                throw RuntimeError("Wrong number of arguments for +");
            }
        } else if (op_type == E_MINUS) {
           if (parameters.size() == 2) {
                return Expr(new Minus(parameters[0], parameters[1])); 
            } else {
                throw RuntimeError("Wrong number of arguments for -");
            }
        } else if (op_type == E_MUL) {
           if (parameters.size() == 2) {
                return Expr(new Mult(parameters[0], parameters[1])); 
            } else {
                throw RuntimeError("Wrong number of arguments for *");
            }
        }  else if (op_type == E_DIV) {
           if (parameters.size() == 2) {
                return Expr(new Div(parameters[0], parameters[1])); 
            } else {
                throw RuntimeError("Wrong number of arguments for /");
            }
        } else if (op_type == E_MODULO) {
            if (parameters.size() != 2) {
                throw RuntimeError("Wrong number of arguments for modulo");
            }
            return Expr(new Modulo(parameters[0], parameters[1]));
        } else if (op_type == E_LIST) {
            return Expr(new ListFunc(parameters));
        } else if (op_type == E_LT) {
            if(parameters.size()==2){return Expr(new Less(parameters[0],parameters[1]));}
            else {return Expr(new LessVar(parameters));}
        } else if (op_type == E_LE) {
            if(parameters.size()==2){return Expr(new LessEq(parameters[0],parameters[1]));}
            return Expr(new LessEqVar(parameters));
        } else if (op_type == E_EQ) {
            if(parameters.size()==2){return Expr(new Equal(parameters[0],parameters[1]));}
            return Expr(new EqualVar(parameters));
        } else if (op_type == E_GE) {
            if(parameters.size()==2){return Expr(new GreaterEq(parameters[0],parameters[1]));}
           return Expr(new GreaterEqVar(parameters));
        } else if (op_type == E_GT) {
            if(parameters.size()==2){return Expr(new Greater(parameters[0],parameters[1]));}
            return Expr(new GreaterVar(parameters));
        } else if (op_type == E_AND) {
            return Expr(new AndVar(parameters));
        } else if (op_type == E_OR) {
            return Expr(new OrVar(parameters));
        } else if (op_type == E_NOT) {
            if(parameters.size()==1){
                return Expr(new Not(parameters[0]));
            }
            else {throw "Wrong number for not";}
        }else {
            throw RuntimeError ("Unknown primitive:" + op);
        }
    }

    if (reserved_words.count(op) != 0) {
        vector<Expr> parameters;
        for(int i=1;i<stxs.size();++i){
            parameters.push_back(stxs[i]->parse(env));
        }
        if(op=="begin"){return Expr(new Begin(parameters));}
        if(op=="quote"){
            if (stxs.size() != 2) {
                throw RuntimeError("quote: requires 1 argument");
            }
            return Expr(new Quote(stxs[1]));
        }
        if(op=="if"){
            if (stxs.size() != 4) {
                throw RuntimeError("if: requires 3 argument");
            }
            return Expr(new If(parameters[1],parameters[2],parameters[3]));
        }
        if(op=="cond"){
            vector<vector<Expr>> tmp;
            tmp.emplace_back(parameters);
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
            vector<Expr> bodyExprs;
            for (int i = 2; i < stxs.size(); ++i) {
                bodyExprs.push_back(stxs[i]->parse(env));
            }
            Expr body(nullptr);
            if (bodyExprs.size() == 1) {
                body = bodyExprs[0];
            } else {
                body = Expr(new Begin(bodyExprs));
            }//这里可能有多个表达式
            return Expr(new Lambda(params, body));
        }
        if(op=="define"){
            if(stxs.size()!=3){
                throw RuntimeError("define: requires 2 augments");
            }
            auto varSym = dynamic_cast<SymbolSyntax*>(stxs[1].get());
            if(varSym!=nullptr){
                return Expr(new Define(varSym->s,parameters[1]));
            }//定义的是一个变量
            else{
                auto funcDef = dynamic_cast<List*>(stxs[1].get());
                if (funcDef==nullptr || funcDef->stxs.empty()) {
                    throw RuntimeError("define: malformed function definition");
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
                Expr body = stxs[2]->parse(env);
                Expr lambda = Expr(new Lambda(params, body));
                return Expr(new Define(funcNameSym->s, lambda));
            }//定义的是一个函数
        }
        if(op=="set!"){
            if (parameters.size() != 2) {
                throw RuntimeError("set!: requires 2 arguments");
            }
            auto varSym = dynamic_cast<Var*>(parameters[0].get());
            if (varSym==nullptr) {
                throw RuntimeError("set!: first argument must be a variable");
            }
            return Expr(new Set(varSym->x, parameters[1]));
        }
        throw RuntimeError("Unknown reserved word: " + op);
    }//还有几个没有完成，待修改
        vector<Expr> parameters;
        for(int i=1;i<stxs.size();++i){
            parameters.push_back(stxs[i]->parse(env));
        }
        Expr rat=stxs[0]->parse(env);
        return Expr(new Apply(rat,parameters));
    }
}
