//
// Created by ward on 3/6/20.
//

#pragma once

#include <array>
#include <memory>
#include <vector>

namespace Ast
{

struct Node
{
    explicit Node() = default;

    friend std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<Node>& root);

    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual std::string value() const = 0;
    [[nodiscard]] virtual std::vector<Node*> children() const = 0;
};

struct Expr : public Node
{
    explicit Expr() = default;
};

struct File final : public Node
{
    explicit File(std::vector<Expr*> expressions)
    : expressions(std::move(expressions)) {}

	std::vector<Expr*> expressions;

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
};

struct BinaryExpr final : public Expr
{
    explicit BinaryExpr(std::string operation, Expr* lhs, Expr* rhs)
    : operation(std::move(operation)), lhs(lhs), rhs(rhs) {}

	std::string operation;

	Expr* lhs;
    Expr* rhs;

    [[nodiscard]] std::string name() const override;
    [[nodiscard]] std::string value() const override;
    [[nodiscard]] std::vector<Node*> children() const override;
};

struct UnaryExpr final : public Expr
{
    explicit UnaryExpr(std::string operation, Expr* operand)
    : operation(std::move(operation)), operand(operand) {}

	std::string operation;
	Expr* operand;

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<Node*> children() const final;
};

struct Literal : public Expr
{
    Literal() = default;
};

template<typename Type>
struct ArithmeticType : public Literal
{
    explicit ArithmeticType(Type val) : val(val) {}

    [[nodiscard]] std::string name() const final { return "literal"; }
    [[nodiscard]] std::string value() const final { return ""; }
    [[nodiscard]] std::vector<Node*> children() const final { return {}; }

    ArithmeticType* operator+ (ArithmeticType* rhs) { return ArithmeticType(val +  rhs->val); }
    ArithmeticType* operator- (ArithmeticType* rhs) { return ArithmeticType(val -  rhs->val); }
    ArithmeticType* operator* (ArithmeticType* rhs) { return ArithmeticType(val *  rhs->val); }
    ArithmeticType* operator/ (ArithmeticType* rhs) { return ArithmeticType(val /  rhs->val); }
    ArithmeticType* operator% (ArithmeticType* rhs) { return ArithmeticType(val %  rhs->val); }
    ArithmeticType* operator< (ArithmeticType* rhs) { return ArithmeticType(val <  rhs->val); }
    ArithmeticType* operator> (ArithmeticType* rhs) { return ArithmeticType(val >  rhs->val); }
    ArithmeticType* operator<=(ArithmeticType* rhs) { return ArithmeticType(val <= rhs->val); }
    ArithmeticType* operator>=(ArithmeticType* rhs) { return ArithmeticType(val >= rhs->val); }
    ArithmeticType* operator==(ArithmeticType* rhs) { return ArithmeticType(val == rhs->val); }
    ArithmeticType* operator!=(ArithmeticType* rhs) { return ArithmeticType(val != rhs->val); }
    ArithmeticType* operator&&(ArithmeticType* rhs) { return ArithmeticType(val && rhs->val); }
    ArithmeticType* operator||(ArithmeticType* rhs) { return ArithmeticType(val || rhs->val); }

    Type val;
};

using Int = ArithmeticType<int>;
using Float = ArithmeticType<float>;
using Char = ArithmeticType<char>;

struct Variable : public Expr
{
    Variable(std::string name) : name(std::move(name)) {}

    std::string name;
};

}

////////////////////////

namespace detail
{
    template <class... Fs>
    struct overload;

    template <class F>
    struct overload<F> : public F
    {
        explicit overload(F f) : F(f) {}
    };

    template <class F, class... Fs>
    struct overload<F, Fs...> : public overload<F>, overload<Fs...>
    {
        explicit overload(F f, Fs... fs) : overload<F>(f), overload<Fs...>(fs...) {}

        using overload<F>::operator();
        using overload<Fs...>::operator();
    };
}  // namespace detail

template <class... F>
auto overloaded(F... f)
{
    return detail::overload<F...>(f...);
}

template<typename Func>
void downcast_call(Ast::Node* node, const Func& func)
{
    if     (auto* res = dynamic_cast<Ast::File*      >(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::BinaryExpr*>(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::UnaryExpr* >(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::Int*       >(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::Float*     >(node)) func(res);
    else if(auto* res = dynamic_cast<Ast::Char*      >(node)) func(res);
}




