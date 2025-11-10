/**
 * @file evaluation.cpp
 * @brief Expression evaluation implementation for the Scheme interpreter
 * @author luke36
 *
 * This file implements evaluation methods for all expression types in the Scheme
 * interpreter. Functions are organized according to ExprType enumeration order
 * from Def.hpp for consistency and maintainability.
 */
#include <sstream>
#include "value.hpp"
#include "expr.hpp"
#include "RE.hpp"
#include "syntax.hpp"
#include <cstring>
#include <vector>
#include <map>
#include <climits>

extern std::map<std::string, ExprType> primitives;
extern std::map<std::string, ExprType> reserved_words;
Value createPrimitiveProcedure(ExprType type);

Value Fixnum::eval(Assoc &e)
{ // evaluation of a fixnum
    return IntegerV(n);
}

Value RationalNum::eval(Assoc &e)
{ // evaluation of a rational number
    return RationalV(numerator, denominator);
}

Value StringExpr::eval(Assoc &e)
{ // evaluation of a string
    return StringV(s);
}

Value True::eval(Assoc &e)
{ // evaluation of #t
    return BooleanV(true);
}

Value False::eval(Assoc &e)
{ // evaluation of #f
    return BooleanV(false);
}

Value MakeVoid::eval(Assoc &e)
{ // (void)
    return VoidV();
}

Value Exit::eval(Assoc &e)
{ // (exit)
    return TerminateV();
}

Value Unary::eval(Assoc &e)
{ // evaluation of single-operator primitive
    return evalRator(rand->eval(e));
}

Value Binary::eval(Assoc &e)
{ // evaluation of two-operators primitive
    return evalRator(rand1->eval(e), rand2->eval(e));
}

Value Variadic::eval(Assoc &e)
{
    std::vector<Value> tmp;
    for (auto i : rands)
    {
        tmp.push_back(i->eval(e));
    }
    return evalRator(tmp);
} // 实现了一些eval求值

Value Var::eval(Assoc &e)
{
    Value matched_value = find(x, e);
    if (matched_value.get() != nullptr)
    {
        return matched_value;
    }
    if (primitives.count(x))
    {
        return createPrimitiveProcedure(primitives[x]);
    }
    throw RuntimeError("Undefined variable: " + x);
}
static int gcd(int a, int b)
{

    if (a == 0 && b == 0) return 1;
    if (a == 0) return (b < 0) ? -b : b;
    if (b == 0) return (a < 0) ? -a : a;
    
    a = (a < 0) ? -a : a;
    b = (b < 0) ? -b : b;
    
    while (b != 0)
    {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}
Value plus(const Value &rand1, const Value &rand2)
{
    ValueType type1 = rand1->v_type;
    ValueType type2 = rand2->v_type;
    if ((type1 != V_RATIONAL && type1 != V_INT) || (type2 != V_RATIONAL && type2 != V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    else if (type1 == V_RATIONAL && type2 == V_RATIONAL)
    {
        auto ptr1 = dynamic_cast<Rational *>(rand1.get());
        auto ptr2 = dynamic_cast<Rational *>(rand2.get());
        if (ptr1->denominator == 0 || ptr2->denominator == 0)
        {
            throw(RuntimeError("Division by zero"));
        }
        int a = ptr1->numerator * ptr2->denominator + ptr1->denominator * ptr2->numerator;
        int b = ptr1->denominator * ptr2->denominator;
        int g = gcd(a, b);
        a = a / g;
        b = b / g;
        if (b != 1)
        {
            return RationalV(a, b);
        }
        else
        {
            return IntegerV(a);
        }
    }
    else if (type1 == V_INT && type2 == V_INT)
    {
        auto ptr1 = static_cast<Integer *>(rand1.get());
        auto ptr2 = static_cast<Integer *>(rand2.get());
        int num = (ptr1->n) + (ptr2->n);
        return IntegerV(num);
    }
    else if (type1 == V_INT && type2 == V_RATIONAL)
    {
        auto ptr1 = static_cast<Integer *>(rand1.get());
        auto ptr2 = static_cast<Rational *>(rand2.get());
        if (ptr2->denominator == 0)
        {
            throw(RuntimeError("Division by zero"));
        }
        int a = ptr1->n * ptr2->denominator + ptr2->numerator;
        int b = ptr2->denominator;
        int g = gcd(a, b);
        a = a / g;
        b = b / g;
        if (b != 1)
        {
            return RationalV(a, b);
        }
        else
            return IntegerV(a);
    }
    else if (type1 == V_RATIONAL && type2 == V_INT)
    {
        auto ptr1 = static_cast<Rational *>(rand1.get());
        auto ptr2 = static_cast<Integer *>(rand2.get());
        if (ptr1->denominator == 0)
        {
            throw(RuntimeError("Division by zero"));
        }
        int a = ptr2->n * ptr1->denominator + ptr1->numerator;
        int b = ptr1->denominator;
        int g = gcd(a, b);
        a = a / g;
        b = b / g;
        if (b != 1)
        {
            return RationalV(a, b);
        }
        else
            return IntegerV(a);
    }
    return VoidV();
}
Value minus(const Value &rand1, const Value &rand2)
{ // -
    ValueType type1 = rand1->v_type;
    ValueType type2 = rand2->v_type;
    if ((type1 != V_RATIONAL && type1 != V_INT) || (type2 != V_RATIONAL && type2 != V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    else if (type1 == V_RATIONAL && type2 == V_RATIONAL)
    {
        auto ptr1 = dynamic_cast<Rational *>(rand1.get());
        auto ptr2 = dynamic_cast<Rational *>(rand2.get());
        if (ptr1->denominator == 0 || ptr2->denominator == 0)
        {
            throw RuntimeError("Division by zero");
        }
        int a = ptr1->numerator * ptr2->denominator - ptr1->denominator * ptr2->numerator;
        int b = ptr1->denominator * ptr2->denominator;
        int g = gcd(a, b);
        a = a / g;
        b = b / g;
        if (b != 1)
        {
            return RationalV(a, b);
        }
        else
            return IntegerV(a);
    }
    else if (type1 == V_INT && type2 == V_INT)
    {
        auto ptr1 = static_cast<Integer *>(rand1.get());
        auto ptr2 = static_cast<Integer *>(rand2.get());
        int num = (ptr1->n) - (ptr2->n);
        return IntegerV(num);
    }
    else if (type1 == V_INT && type2 == V_RATIONAL)
    {
        auto ptr1 = static_cast<Integer *>(rand1.get());
        auto ptr2 = static_cast<Rational *>(rand2.get());
        if (ptr2->denominator == 0)
        {
            throw RuntimeError("Division by zero");
        }
        int a = ptr1->n * ptr2->denominator - ptr2->numerator;
        int b = ptr2->denominator;
        int g = gcd(a, b);
        a = a / g;
        b = b / g;
        if (b != 1)
        {
            return RationalV(a, b);
        }
        else
            return IntegerV(a);
    }
    else if (type1 == V_RATIONAL && type2 == V_INT)
    {
        auto ptr1 = static_cast<Rational *>(rand1.get());
        auto ptr2 = static_cast<Integer *>(rand2.get());
        if (ptr1->denominator == 0)
        {
            throw RuntimeError("Division by zero");
        }
        int a = ptr2->n * ptr1->denominator - ptr1->numerator;
        int b = ptr1->denominator;
        int g = gcd(a, b);
        a = a / g;
        b = b / g;
        if (b != 1)
        {
            return RationalV(a, b);
        }
        else
            return IntegerV(a);
    }
    return VoidV();
}
Value mult(const Value &rand1, const Value &rand2)
{ // *
    ValueType type1 = rand1->v_type;
    ValueType type2 = rand2->v_type;
    if ((type1 != V_RATIONAL && type1 != V_INT) || (type2 != V_RATIONAL && type2 != V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    else if (type1 == V_RATIONAL && type2 == V_RATIONAL)
    {
        auto ptr1 = dynamic_cast<Rational *>(rand1.get());
        auto ptr2 = dynamic_cast<Rational *>(rand2.get());
        if (ptr1->denominator == 0 || ptr2->denominator == 0)
        {
            throw RuntimeError("Division by zero");
        }
        int a = ptr1->numerator * ptr2->numerator;
        int b = ptr1->denominator * ptr2->denominator;
        int g = gcd(a, b);
        a = a / g;
        b = b / g;
        if (b != 1)
        {
            return RationalV(a, b);
        }
        else
            return IntegerV(a);
    }
    else if (type1 == V_INT && type2 == V_INT)
    {
        auto ptr1 = static_cast<Integer *>(rand1.get());
        auto ptr2 = static_cast<Integer *>(rand2.get());
        int num = (ptr1->n) * (ptr2->n);
        return IntegerV(num);
    }
    else if (type1 == V_INT && type2 == V_RATIONAL)
    {
        auto ptr1 = static_cast<Integer *>(rand1.get());
        auto ptr2 = static_cast<Rational *>(rand2.get());
        if (ptr2->denominator == 0)
        {
            throw RuntimeError("Division by zero");
        }
        int a = ptr1->n * ptr2->numerator;
        int b = ptr2->denominator;
        int g = gcd(a, b);
        a = a / g;
        b = b / g;
        if (b != 1)
        {
            return RationalV(a, b);
        }
        else
        {
            return IntegerV(a);
        }
    }
    else if (type1 == V_RATIONAL && type2 == V_INT)
    {
        auto ptr1 = static_cast<Rational *>(rand1.get());
        auto ptr2 = static_cast<Integer *>(rand2.get());
        if (ptr1->denominator == 0)
        {
            throw RuntimeError("Division by zero");
        }
        int a = ptr2->n * ptr1->numerator;
        int b = ptr1->denominator;
        int g = gcd(a, b);
        a = a / g;
        b = b / g;
        if (b != 1)
        {
            return RationalV(a, b);
        }
        else
        {
            return IntegerV(a);
        }
    }
    return VoidV();
}
Value div(const Value &rand1, const Value &rand2)
{ // /
    ValueType type1 = rand1->v_type;
    ValueType type2 = rand2->v_type;
    if ((type1 != V_RATIONAL && type1 != V_INT) || (type2 != V_RATIONAL && type2 != V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    else if (type1 == V_RATIONAL && type2 == V_RATIONAL)
    {
        auto ptr1 = dynamic_cast<Rational *>(rand1.get());
        auto ptr2 = dynamic_cast<Rational *>(rand2.get());
        if (ptr1->denominator == 0 || ptr2->numerator == 0)
        {
            throw RuntimeError("Division by zero");
        }
        int a = ptr1->numerator * ptr2->denominator;
        int b = ptr1->denominator * ptr2->numerator;
        int g = gcd(a, b);
        a = a / g;
        b = b / g;
        if (b != 1)
        {
            return RationalV(a, b);
        }
        else
        {
            return IntegerV(a);
        }
    }
    else if (type1 == V_INT && type2 == V_INT)
    {
        int n1 = dynamic_cast<Integer *>(rand1.get())->n;
        int n2 = dynamic_cast<Integer *>(rand2.get())->n;
        if (n2 == 0)
            throw RuntimeError("Division by zero");
        if (n1 % n2 == 0)
        {
            return IntegerV(n1 / n2);
        }
        else
        {
            int g = gcd(n1, n2);
            return RationalV(n1 / g, n2 / g);
        }
    }
    else if (type1 == V_INT && type2 == V_RATIONAL)
    {
        auto ptr1 = static_cast<Integer *>(rand1.get());
        auto ptr2 = static_cast<Rational *>(rand2.get());
        if (ptr2->denominator == 0||ptr2->numerator == 0)
        {
            throw RuntimeError("Division by zero");
        }
        int a = ptr1->n * ptr2->denominator;
        int b = ptr2->numerator;
        int g = gcd(a, b);
        a = a / g;
        b = b / g;
        if (b != 1)
        {
            return RationalV(a, b);
        }
        else
        {
            return IntegerV(a);
        }
    }
    else if (type1 == V_RATIONAL && type2 == V_INT)
{
    Rational* r1 = dynamic_cast<Rational*>(rand1.get());
    int n2 = dynamic_cast<Integer*>(rand2.get())->n;
    if (n2 == 0) throw RuntimeError("Division by zero");
    
    int numerator = r1->numerator;
    int denominator = r1->denominator * n2;
    int g = gcd(numerator, denominator);
    numerator /= g;
    denominator /= g;
    
    if (denominator == 1) return IntegerV(numerator);
    else return RationalV(numerator, denominator);
}
    return VoidV();
}
Value Plus::evalRator(const Value &rand1, const Value &rand2)
{
    return plus(rand1, rand2);
}

Value Minus::evalRator(const Value &rand1, const Value &rand2)
{
    return minus(rand1, rand2);
}

Value Mult::evalRator(const Value &rand1, const Value &rand2)
{
    return mult(rand1, rand2);
}

Value Div::evalRator(const Value &rand1, const Value &rand2)
{
    return div(rand1, rand2);
}

Value Modulo::evalRator(const Value &rand1, const Value &rand2)
{ // modulo
    if (rand1->v_type == V_INT && rand2->v_type == V_INT)
    {
        int dividend = dynamic_cast<Integer *>(rand1.get())->n;
        int divisor = dynamic_cast<Integer *>(rand2.get())->n;
        if (divisor == 0)
        {
            throw(RuntimeError("Division by zero"));
        }
        return IntegerV(dividend % divisor);
    }
    throw(RuntimeError("modulo is only defined for integers"));
}

Value PlusVar::evalRator(const std::vector<Value> &args)
{
    if (args.size() == 0)
    {
        return IntegerV(0);
    }
    else if (args.size() == 1)
    {
        auto ptr = dynamic_cast<Rational *>(args[0].get());
        if (ptr != nullptr)
        {
            if (ptr->denominator == 0)
            {
                throw RuntimeError("Division by zero");
            }
        }
        return args[0];
    }
    else if (args.size() == 2)
    {
        return plus(args[0], args[1]);
    }
    else
    {
        Value sum = plus(args[0], args[1]);
        for (int i = 2; i < args.size(); ++i)
        {
            sum = plus(sum, args[i]);
        }
        return sum;
    }
}

Value MinusVar::evalRator(const std::vector<Value> &args)
{
    if (args.size() == 0)
    {
        throw RuntimeError("minus:num needed");
    } // - with multiple args
    else if (args.size() == 1)
    {
        if (args[0]->v_type == V_INT)
        {
            auto ptr = dynamic_cast<Integer *>(args[0].get());
            return IntegerV(-(ptr->n));
        }
        else
        {
            auto ptr = dynamic_cast<Rational *>(args[0].get());
            if (ptr->denominator == 0)
            {
                throw RuntimeError("Division by zero");
            }
            return RationalV(-(ptr->numerator), ptr->denominator);
        }
    }
    else if (args.size() == 2)
    {
        return minus(args[0], args[1]);
    }
    else
    {
        Value sum = minus(args[0], args[1]);
        for (int i = 2; i < args.size(); ++i)
        {
            sum = minus(sum, args[i]);
        }
        return sum;
    }
}

Value MultVar::evalRator(const std::vector<Value> &args)
{
    if (args.size() == 0)
    {
        return IntegerV(1);
    } // * with multiple args
    else if (args.size() == 1)
    {
        auto ptr = dynamic_cast<Rational *>(args[0].get());
        if (ptr != nullptr)
        {
            if (ptr->denominator == 0)
            {
                throw RuntimeError("Division by zero");
            }
        }
        return args[0];
    }
    else if (args.size() == 2)
    {
        return mult(args[0], args[1]);
    }
    else
    {
        Value sum = mult(args[0], args[1]);
        for (int i = 2; i < args.size(); ++i)
        {
            sum = mult(sum, args[i]);
        }
        return sum;
    }
}

Value DivVar::evalRator(const std::vector<Value> &args)
{
    if (args.size() == 0)
    {
        throw RuntimeError("div:num needed");
    }
    else if (args.size() == 1)
    {
        Value arg = args[0];
        if (arg->v_type == V_INT)
        {
            int n = dynamic_cast<Integer *>(arg.get())->n;
            if (n == 0)
                throw RuntimeError("Division by zero");
            return RationalV(1, n);
        }
        else if (arg->v_type == V_RATIONAL)
        {
            Rational *r = dynamic_cast<Rational *>(arg.get());
            if (r->numerator == 0)
                throw RuntimeError("Division by zero");
            return RationalV(r->denominator, r->numerator);
        }
    }
    else if (args.size() == 2)
    {
        return div(args[0], args[1]);
    }
    else
    {
        Value sum = div(args[0], args[1]);
        for (int i = 2; i < args.size(); ++i)
        {
            sum = div(sum, args[i]);
        }
        return sum;
    }
}

Value Expt::evalRator(const Value &rand1, const Value &rand2)
{ // expt
    if (rand1->v_type == V_INT && rand2->v_type == V_INT)
    {
        int base = dynamic_cast<Integer *>(rand1.get())->n;
        int exponent = dynamic_cast<Integer *>(rand2.get())->n;

        if (exponent < 0)
        {
            throw(RuntimeError("Negative exponent not supported for integers"));
        }
        if (base == 0 && exponent == 0)
        {
            throw(RuntimeError("0^0 is undefined"));
        }

        long long result = 1;
        long long b = base;
        int exp = exponent;

        while (exp > 0)
        {
            if (exp % 2 == 1)
            {
                result *= b;
                if (result > INT_MAX || result < INT_MIN)
                {
                    throw(RuntimeError("Integer overflow in expt"));
                }
            }
            b *= b;
            if (b > INT_MAX || b < INT_MIN)
            {
                if (exp > 1)
                {
                    throw(RuntimeError("Integer overflow in expt"));
                }
            }
            exp /= 2;
        }

        return IntegerV((int)result);
    }
    throw(RuntimeError("Wrong typename"));
}

// A FUNCTION TO SIMPLIFY THE COMPARISON WITH INTEGER AND RATIONAL NUMBER
int compareNumericValues(const Value &v1, const Value &v2)
{
    if (v1->v_type == V_INT && v2->v_type == V_INT)
    {
        int n1 = dynamic_cast<Integer *>(v1.get())->n;
        int n2 = dynamic_cast<Integer *>(v2.get())->n;
        return (n1 < n2) ? -1 : (n1 > n2) ? 1
                                          : 0;
    }
    else if (v1->v_type == V_RATIONAL && v2->v_type == V_INT)
    {
        Rational *r1 = dynamic_cast<Rational *>(v1.get());
        int n2 = dynamic_cast<Integer *>(v2.get())->n;
        // 检查分母是否为0
        if (r1->denominator == 0)
        {
            throw RuntimeError("Division by zero");
        }
        long long left = (long long)r1->numerator;
        long long right = (long long)n2 * r1->denominator;
        return (left < right) ? -1 : (left > right) ? 1
                                                    : 0;
    }
    else if (v1->v_type == V_INT && v2->v_type == V_RATIONAL)
    {
        int n1 = dynamic_cast<Integer *>(v1.get())->n;
        Rational *r2 = dynamic_cast<Rational *>(v2.get());
        // 检查分母是否为0
        if (r2->denominator == 0)
        {
            throw RuntimeError("Division by zero");
        }
        long long left = (long long)n1 * r2->denominator;
        long long right = (long long)r2->numerator;
        return (left < right) ? -1 : (left > right) ? 1
                                                    : 0;
    }
    else if (v1->v_type == V_RATIONAL && v2->v_type == V_RATIONAL)
    {
        Rational *r1 = dynamic_cast<Rational *>(v1.get());
        Rational *r2 = dynamic_cast<Rational *>(v2.get());
        // 检查分母是否为0
        if (r1->denominator == 0 || r2->denominator == 0)
        {
            throw RuntimeError("Division by zero");
        }
        long long left = (long long)r1->numerator * r2->denominator;
        long long right = (long long)r2->numerator * r1->denominator;
        return (left < right) ? -1 : (left > right) ? 1
                                                    : 0;
    }
    throw RuntimeError("Wrong type for numeric comparison");
}

Value Less::evalRator(const Value &rand1, const Value &rand2)
{ // <
    if ((rand1->v_type != V_RATIONAL && rand1->v_type != V_INT) || (rand2->v_type != V_RATIONAL && rand2->v_type != V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    if (compareNumericValues(rand1, rand2) == -1)
    {
        return BooleanV(true);
    }
    else
    {
        return BooleanV(false);
    }
}

Value LessEq::evalRator(const Value &rand1, const Value &rand2)
{ // <=
    if ((rand1->v_type != V_RATIONAL && rand1->v_type != V_INT) || (rand2->v_type != V_RATIONAL && rand2->v_type != V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    if (compareNumericValues(rand1, rand2) == -1 || compareNumericValues(rand1, rand2) == 0)
    {
        return BooleanV(true);
    }
    else
    {
        return BooleanV(false);
    }
}

Value Equal::evalRator(const Value &rand1, const Value &rand2)
{ // =
    if ((rand1->v_type != V_RATIONAL && rand1->v_type != V_INT) || (rand2->v_type != V_RATIONAL && rand2->v_type != V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    if (compareNumericValues(rand1, rand2) == 0)
    {
        return BooleanV(true);
    }
    else
    {
        return BooleanV(false);
    }
}

Value GreaterEq::evalRator(const Value &rand1, const Value &rand2)
{    
    if ((rand1->v_type != V_RATIONAL && rand1->v_type != V_INT) || (rand2->v_type != V_RATIONAL && rand2->v_type != V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    if (compareNumericValues(rand1, rand2) == 0 || compareNumericValues(rand1, rand2) == 1)
    {
        return BooleanV(true);
    }
    else
    {
        return BooleanV(false);
    }
}

Value Greater::evalRator(const Value &rand1, const Value &rand2)
{ // >
    if  ((rand1->v_type != V_RATIONAL && rand1->v_type != V_INT) || (rand2->v_type != V_RATIONAL && rand2->v_type != V_INT))
    {
        throw(RuntimeError("Wrong typename"));
    }
    if (compareNumericValues(rand1, rand2) == 1)
    {
        return BooleanV(true);
    }
    else
    {
        return BooleanV(false);
    }
}

Value LessVar::evalRator(const std::vector<Value> &args)
{
    for (int i = 0; i < args.size(); ++i)
    {
        if (args[i]->v_type != V_RATIONAL && args[i]->v_type != V_INT)
        {
            throw(RuntimeError("Wrong typename"));
        }
    } // < with multiple args
    if (args.empty()||args.size()==1)
    {
        throw RuntimeError("Invalid params");
    }
    for (int i = 0; i < args.size() - 1; ++i)
    {
        if (compareNumericValues(args[i], args[i + 1]) != -1)
        {
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value LessEqVar::evalRator(const std::vector<Value> &args)
{
    for (int i = 0; i < args.size(); ++i)
    {
        if (args[i]->v_type != V_RATIONAL && args[i]->v_type != V_INT)
        {
            throw(RuntimeError("Wrong typename"));
        }
    } // <= with multiple args
    if (args.empty()||args.size()==1)
    {
       throw RuntimeError("Invalid params");
    }
    for (int i = 0; i < args.size() - 1; ++i)
    {
        if (compareNumericValues(args[i], args[i + 1]) == 1)
        {
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value EqualVar::evalRator(const std::vector<Value> &args)
{
    for (int i = 0; i < args.size(); ++i)
    {
        if (args[i]->v_type != V_RATIONAL && args[i]->v_type != V_INT)
        {
            throw(RuntimeError("Wrong typename"));
        }
    } 
    if (args.empty()||args.size()==1)
    {
        throw RuntimeError("Invalid params");
    }// = with multiple args
    for (int i = 0; i < args.size() - 1; ++i)
    {
        if (compareNumericValues(args[i], args[i + 1]) != 0)
        {
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value GreaterEqVar::evalRator(const std::vector<Value> &args)
{
    for (int i = 0; i < args.size(); ++i)
    {
        if (args[i]->v_type != V_RATIONAL && args[i]->v_type != V_INT)
        {
            throw(RuntimeError("Wrong typename"));
        }
    }
    if (args.empty()||args.size()==1)
    {
        throw RuntimeError("Invalid params");
    } // >= with multiple args
    for (int i = 0; i < args.size() - 1; ++i)
    {
        if (compareNumericValues(args[i], args[i + 1]) == -1)
        {
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value GreaterVar::evalRator(const std::vector<Value> &args)
{
    for (int i = 0; i < args.size(); ++i)
    {
        if (args[i]->v_type != V_RATIONAL && args[i]->v_type != V_INT)
        {
            throw(RuntimeError("Wrong typename"));
        }
    }
    if (args.empty()||args.size()==1)
    {
        throw RuntimeError("Invalid params");
    } // > with multiple args
    for (int i = 0; i < args.size() - 1; ++i)
    {
        if (compareNumericValues(args[i], args[i + 1]) != 1)
        {
            return BooleanV(false);
        }
    }
    return BooleanV(true);
}

Value Cons::evalRator(const Value &rand1, const Value &rand2)
{ // cons
    return PairV(rand1, rand2);
}

Value ListFunc::evalRator(const std::vector<Value> &args)
{
    if (args.empty())
    {
        return NullV();
    }
    Value result = NullV();
    for (int i = args.size() - 1; i >= 0; --i)
    {
        result = PairV(args[i], result);
    }
    return result;
}

Value IsList::evalRator(const Value &rand)
{ // list?
    if (rand->v_type == V_NULL)
    {
        return BooleanV(true);
    }
    else if (rand->v_type != V_PAIR)
    {
        return BooleanV(false);
    }
    else
    {
        auto ptr = dynamic_cast<Pair *>(rand.get());
        while (ptr->cdr->v_type == V_PAIR)
        {
            ptr = dynamic_cast<Pair *>(ptr->cdr.get());
        }
        if (ptr->cdr->v_type == V_NULL)
        {
            return BooleanV(true);
        }
        else
        {
            return BooleanV(false);
        }
    }
}

Value Car::evalRator(const Value &rand)
{ // car
    if (rand->v_type != V_PAIR)
    {
        throw RuntimeError("car: expected pair");
    }
    else
    {
        auto ptr = dynamic_cast<Pair *>(rand.get());
        return ptr->car;
    }
}

Value Cdr::evalRator(const Value &rand)
{ // cdr
    if (rand->v_type != V_PAIR)
    {
        throw RuntimeError("cdr: expected pair");
    }
    else
    {
        auto ptr = dynamic_cast<Pair *>(rand.get());
        return ptr->cdr;
    }
}

Value SetCar::evalRator(const Value &rand1, const Value &rand2)
{ // set-car!
    if (rand1->v_type != V_PAIR)
    {
        throw RuntimeError("SetCar!:required pair");
    }
    auto ptr = dynamic_cast<Pair *>(rand1.get());
    if (ptr == nullptr)
    {
        throw RuntimeError("SetCar!:Invalid Pair");
    }
    ptr->car = rand2;
    return VoidV();
}

Value SetCdr::evalRator(const Value &rand1, const Value &rand2)
{ // set-cdr!
    if (rand1->v_type != V_PAIR)
    {
        throw RuntimeError("SetCdr!:required pair");
    }
    auto ptr = dynamic_cast<Pair *>(rand1.get());
    if (ptr == nullptr)
    {
        throw RuntimeError("SetCdr!:Invalid Pair");
    }
    ptr->cdr = rand2;
    return VoidV();
}

Value IsEq::evalRator(const Value &rand1, const Value &rand2)
{ // eq?
    if (rand1->v_type == V_INT && rand2->v_type == V_INT)
    {
        return BooleanV((dynamic_cast<Integer *>(rand1.get())->n) == (dynamic_cast<Integer *>(rand2.get())->n));
    }
    else if (rand1->v_type == V_BOOL && rand2->v_type == V_BOOL)
    {
        return BooleanV((dynamic_cast<Boolean *>(rand1.get())->b) == (dynamic_cast<Boolean *>(rand2.get())->b));
    }
    else if (rand1->v_type == V_SYM && rand2->v_type == V_SYM)
    {
        return BooleanV((dynamic_cast<Symbol *>(rand1.get())->s) == (dynamic_cast<Symbol *>(rand2.get())->s));
    }
    else if ((rand1->v_type == V_NULL && rand2->v_type == V_NULL) ||
             (rand1->v_type == V_VOID && rand2->v_type == V_VOID))
    {
        return BooleanV(true);
    }
    else
    {
        return BooleanV(rand1.get() == rand2.get());
    }
}

Value IsBoolean::evalRator(const Value &rand)
{ // boolean?
    return BooleanV(rand->v_type == V_BOOL);
}

Value IsFixnum::evalRator(const Value &rand)
{ // number?
    return BooleanV(rand->v_type == V_INT);
}

Value IsNull::evalRator(const Value &rand)
{ // null?
    return BooleanV(rand->v_type == V_NULL);
}

Value IsPair::evalRator(const Value &rand)
{ // pair?
    return BooleanV(rand->v_type == V_PAIR);
}

Value IsProcedure::evalRator(const Value &rand)
{
    // procedure?
    return BooleanV(rand->v_type == V_PROC);
}

Value IsSymbol::evalRator(const Value &rand)
{ // symbol?
    return BooleanV(rand->v_type == V_SYM);
}

Value IsString::evalRator(const Value &rand)
{ // string?
    return BooleanV(rand->v_type == V_STRING);
}

Value Begin::eval(Assoc &e)
{
    if (es.empty())
    {
        return VoidV();
    }
    Value result = VoidV();
    for (auto &expr : es)
    {
        result = expr->eval(e);
    }
    return result;
}
Value syntaxToQuotedValue(const Syntax &syntax)
{
    if (auto num = dynamic_cast<Number *>(syntax.get()))
    {
        return IntegerV(num->n);
    }
    else if (auto rational = dynamic_cast<RationalSyntax *>(syntax.get()))
    {
        return RationalV(rational->numerator, rational->denominator);
    }
    else if (auto sym = dynamic_cast<SymbolSyntax *>(syntax.get()))
    {
        return SymbolV(sym->s);
    }
    else if (auto str = dynamic_cast<StringSyntax *>(syntax.get()))
    {
        return StringV(str->s);
    }
    else if (dynamic_cast<TrueSyntax *>(syntax.get()))
    {
        return BooleanV(true);
    }
    else if (dynamic_cast<FalseSyntax *>(syntax.get()))
    {
        return BooleanV(false);
    }
    else if (auto list = dynamic_cast<List *>(syntax.get()))
    {
        if (list->stxs.empty())
        {
            return NullV();
        }
        if (list->stxs.size() == 3)
        {
            auto dot = dynamic_cast<SymbolSyntax *>(list->stxs[1].get());
            if (dot && dot->s == ".")
            {
                Value car = syntaxToQuotedValue(list->stxs[0]);
                Value cdr = syntaxToQuotedValue(list->stxs[2]);
                return PairV(car, cdr);
            }
        }
        for (size_t i = 0; i < list->stxs.size(); i++)
        {
            auto dot = dynamic_cast<SymbolSyntax *>(list->stxs[i].get());
            if (dot && dot->s == ".")
            {
                if (i == 0 || i == list->stxs.size() - 1)
                {
                    throw RuntimeError("Invalid dotted pair syntax");
                }
                Value car_part = NullV();
                for (int j = i - 1; j >= 0; j--)
                {
                    Value elem = syntaxToQuotedValue(list->stxs[j]);
                    car_part = PairV(elem, car_part);
                }
                Value cdr_part = syntaxToQuotedValue(list->stxs[i + 1]);
                Value current = car_part;
                while (auto pair = dynamic_cast<Pair *>(current.get()))
                {
                    if (dynamic_cast<Null *>(pair->cdr.get()))
                    {
                        const_cast<Pair *>(pair)->cdr = cdr_part;
                        return car_part;
                    }
                    current = pair->cdr;
                }
                return car_part;
            }
        }
        Value result = NullV();
        for (int i = list->stxs.size() - 1; i >= 0; i--)
        {
            Value element = syntaxToQuotedValue(list->stxs[i]);
            result = PairV(element, result);
        }
        return result;
    }
    throw RuntimeError("Unsupported syntax in quote");
}
Value Quote::eval(Assoc &e)
{
    return syntaxToQuotedValue(s);
}

Value AndVar::eval(Assoc &e)
{
    if (rands.empty())
    {
        return BooleanV(true);
    } // and with short-circuit evaluation
    for (auto expr : rands)
    {
        Value value = expr->eval(e);
        auto ptr = dynamic_cast<Boolean *>(value.get());
        if (ptr != nullptr)
        {
            if (ptr->b == false)
            {
                return BooleanV(false);
            }
        }
    }
    return rands.back()->eval(e);
}

Value OrVar::eval(Assoc &e)
{
    if (rands.empty())
    {
        return BooleanV(false);
    } // or with short-circuit evaluation
    for (auto expr : rands)
    {
        Value value = expr->eval(e);
        auto ptr = dynamic_cast<Boolean *>(value.get());
        if (ptr == nullptr)
        {
            return value;
        }
        else if (ptr != nullptr)
        {
            if (ptr->b != false)
            {
                return value;
            }
        }
    }
    return rands.back()->eval(e);
}

Value Not::evalRator(const Value &rand)
{ // not
    auto ptr = dynamic_cast<Boolean *>(rand.get());
    if (ptr == nullptr)
    {
        return BooleanV(false);
    }
    else if (ptr->b == false)
    {
        return BooleanV(true);
    }
    else if (ptr->b == true)
    {
        return BooleanV(false);
    }
    return VoidV();
}

Value If::eval(Assoc &e)
{
    Value condValue = cond->eval(e);
    auto ptr = dynamic_cast<Boolean *>(condValue.get());
    if (ptr == nullptr)
    {
        return conseq->eval(e);
    }
    else
    {
        if (ptr->b == true)
        {
            return conseq->eval(e);
        }
        else if (ptr->b == false)
        {
            return alter->eval(e);
        }
    }
    return VoidV();
}

Value Cond::eval(Assoc &env)
{
    if (clauses.empty())
    {
        return VoidV();
    }
    for (auto &clause : clauses)
    {
        if (clause.empty())
        {
            continue;
        }
        bool is_else = false;
        if (auto var = dynamic_cast<Var *>(clause[0].get()))
        {
            if (var->x == "else")
            {
                is_else = true;
            }
        }
        bool condition = is_else;
        Value condition_value(nullptr);
        if (is_else == false)
        {
            condition_value = clause[0]->eval(env);
            condition = true;
            auto ptr = dynamic_cast<Boolean *>(condition_value.get());
            if (ptr != nullptr)
            {
                if (ptr->b == false)
                {
                    condition = false;
                }
            }
        }
        if (condition)
        {
            if (clause.size() == 1)
            {
                return is_else ? VoidV() : condition_value;
            }
            else
            {
                Value result = VoidV();
                for (size_t i = 1; i < clause.size(); ++i)
                {
                    result = clause[i]->eval(env);
                }
                return result;
            }
        }
    }
    return VoidV();
}

Value Lambda::eval(Assoc &env)
{
    return ProcedureV(x, e, env);
} // 只负责捕获，不修改当前环境



// 接下来的部分和环境有关
Value Define::eval(Assoc &env)
{
    if (primitives.count(var))
    {
        throw RuntimeError("Cannot redefine primitive: " + var);
    }
    if (reserved_words.count(var))
    {
        throw RuntimeError("Cannot use reserved word as variable: " + var);
    }

    // 使用更安全的环境查找
    Value existing = find(var, env);
    if (existing.get() != nullptr)
    {
        // 重新定义
        Value value = e->eval(env);
        modify(var, value, env);
    }
    else
    {
        // 新定义
        env = extend(var, VoidV(), env);
        Value value = e->eval(env);
        modify(var, value, env);
    }
    return VoidV();
}

Value Let::eval(Assoc &env)
{
    std::vector<Value> tmp;
    for (auto &binding : bind)
    {
        Value value = binding.second->eval(env);
        tmp.push_back(value);
    }
    Assoc new_env = env;
    for (int i = 0; i < tmp.size(); ++i)
    {
        new_env = extend(bind[i].first, tmp[i], new_env);
    }
    Value result = body->eval(new_env);
    return result;
}

Value Letrec::eval(Assoc &env)
{
    if (bind.empty())
    {
        return body->eval(env);
    }
    Assoc new_env = env;
    for (auto &binding : bind)
    {
        new_env = extend(binding.first, VoidV(), new_env);
    }
    for (auto &binding : bind)
    {
        Value value = binding.second->eval(new_env);
        modify(binding.first, value, new_env);
    }
    return body->eval(new_env);
}

Value Set::eval(Assoc &env)
{
    try
    {
        find(var, env);
    }
    catch (const RuntimeError &)
    {
        throw RuntimeError("Variable not found: " + var);
    }
    Value new_value = e->eval(env);
    modify(var, new_value, env);
    return new_value;
}

Value Display::evalRator(const Value &rand)
{ // display function
    if (rand->v_type == V_STRING)
    {
        String *str_ptr = dynamic_cast<String *>(rand.get());
        std::cout << str_ptr->s;
    }
    else
    {
        rand->show(std::cout);
    }
    return VoidV();
}
//---------------将原始过程转换-------------------//
//----------------------------------------------//
Value createPrimitiveProcedure(ExprType type)
{
    switch (type)
    {

    // 算术操作
    case E_PLUS:
        return ProcedureV({}, Expr(new PlusVar({})), empty());
    case E_MINUS:
        return ProcedureV({}, Expr(new MinusVar({})), empty());
    case E_MUL:
        return ProcedureV({}, Expr(new MultVar({})), empty());
    case E_DIV:
        return ProcedureV({}, Expr(new DivVar({})), empty());
    case E_MODULO:
        return ProcedureV({"a", "b"}, Expr(new Modulo(Expr(new Var("a")), Expr(new Var("b")))), empty());
    case E_EXPT:
        return ProcedureV({"a", "b"}, Expr(new Expt(Expr(new Var("a")), Expr(new Var("b")))), empty());

    // 比较操作
    case E_LT:
        return ProcedureV({}, Expr(new LessVar({})), empty());
    case E_LE:
        return ProcedureV({}, Expr(new LessEqVar({})), empty());
    case E_EQ:
        return ProcedureV({}, Expr(new EqualVar({})), empty());
    case E_GE:
        return ProcedureV({}, Expr(new GreaterEqVar({})), empty());
    case E_GT:
        return ProcedureV({}, Expr(new GreaterVar({})), empty());

    // 列表操作
    case E_CONS:
        return ProcedureV({"a", "b"}, Expr(new Cons(Expr(new Var("a")), Expr(new Var("b")))), empty());
    case E_CAR:
        return ProcedureV({"p"}, Expr(new Car(Expr(new Var("p")))), empty());
    case E_CDR:
        return ProcedureV({"p"}, Expr(new Cdr(Expr(new Var("p")))), empty());
    case E_LIST:
        return ProcedureV({}, Expr(new ListFunc({})), empty());
    case E_SETCAR:
        return ProcedureV({"p", "v"}, Expr(new SetCar(Expr(new Var("p")), Expr(new Var("v")))), empty());
    case E_SETCDR:
        return ProcedureV({"p", "v"}, Expr(new SetCdr(Expr(new Var("p")), Expr(new Var("v")))), empty());

    // 逻辑操作
    case E_NOT:
        return ProcedureV({"v"}, Expr(new Not(Expr(new Var("v")))), empty());
    case E_AND:
        return ProcedureV({}, Expr(new AndVar({})), empty());
    case E_OR:
        return ProcedureV({}, Expr(new OrVar({})), empty());

    // 类型谓词
    case E_EQQ:
        return ProcedureV({"a", "b"}, Expr(new IsEq(Expr(new Var("a")), Expr(new Var("b")))), empty());
    case E_BOOLQ:
        return ProcedureV({"v"}, Expr(new IsBoolean(Expr(new Var("v")))), empty());
    case E_INTQ:
        return ProcedureV({"v"}, Expr(new IsFixnum(Expr(new Var("v")))), empty());
    case E_NULLQ:
        return ProcedureV({"v"}, Expr(new IsNull(Expr(new Var("v")))), empty());
    case E_PAIRQ:
        return ProcedureV({"v"}, Expr(new IsPair(Expr(new Var("v")))), empty());
    case E_PROCQ:
        return ProcedureV({"v"}, Expr(new IsProcedure(Expr(new Var("v")))), empty());
    case E_SYMBOLQ:
        return ProcedureV({"v"}, Expr(new IsSymbol(Expr(new Var("v")))), empty());
    case E_LISTQ:
        return ProcedureV({"v"}, Expr(new IsList(Expr(new Var("v")))), empty());
    case E_STRINGQ:
        return ProcedureV({"v"}, Expr(new IsString(Expr(new Var("v")))), empty());

    // I/O 操作
    case E_DISPLAY:
        return ProcedureV({"v"}, Expr(new Display(Expr(new Var("v")))), empty());

    // 特殊值
    case E_VOID:
        return ProcedureV({}, Expr(new MakeVoid()), empty());
    case E_EXIT:
        return ProcedureV({}, Expr(new Exit()), empty());

    default:
        throw RuntimeError("Unknown primitive procedure type: " + std::to_string(type));
    }
}
