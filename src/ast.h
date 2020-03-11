//
// Created by ward on 3/6/20.
//

#pragma once

#include <array>
#include <memory>
#include <vector>

struct AstNode
{
    explicit AstNode() = default;

    friend std::ofstream& operator<<(std::ofstream& stream, const std::unique_ptr<AstNode>& root);

    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual std::string value() const = 0;
    [[nodiscard]] virtual std::vector<AstNode*> children() const = 0;

    [[nodiscard]] virtual std::int32_t get_id() const noexcept = 0;
};

struct Expr : public AstNode
{
    explicit Expr() = default;
};

struct File final : public AstNode
{
    explicit File(std::vector<Expr*> expressions)
    : expressions(std::move(expressions)) {}

	std::vector<Expr*> expressions;

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<AstNode*> children() const final;

    static const std::int32_t ID = 0;
    [[nodiscard]] std::int32_t get_id() const noexcept final { return ID; }
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
    [[nodiscard]] std::vector<AstNode*> children() const override;

    static const std::int32_t ID = 1;
    [[nodiscard]] std::int32_t get_id() const noexcept final { return ID; }
};

struct UnaryExpr final : public Expr
{
    explicit UnaryExpr(std::string operation, Expr* operand)
    : operation(std::move(operation)), operand(operand) {}

	std::string operation;
	Expr* operand;

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<AstNode*> children() const final;

    static const std::int32_t ID = 2;
    [[nodiscard]] std::int32_t get_id() const noexcept final { return ID; }
};

struct Literal : public Expr
{
    Literal() = default;
};

struct Int final : public Literal
{
    explicit Int(const std::string& str)
    : val(std::stoll(str)) {}

    explicit Int(int64_t val)
    : val(val) {}

	int64_t val;

    [[nodiscard]] std::string name() const final;
    [[nodiscard]] std::string value() const final;
    [[nodiscard]] std::vector<AstNode*> children() const final;

    static const std::int32_t ID = 3;
    [[nodiscard]] std::int32_t get_id() const noexcept final { return ID; }
};

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
auto downcast_call(AstNode* node, const Func& func)
{
    switch(node->get_id())
    {
        case File::ID:
            func(static_cast<File*>(node));
            return true;
        case BinaryExpr::ID:
            func(static_cast<BinaryExpr*>(node));
            return true;
        case UnaryExpr::ID:
            func(static_cast<UnaryExpr*>(node));
            return true;
        case Int::ID:
            func(static_cast<Int*>(node));
            return true;
        default:
            return false;
    }
}




