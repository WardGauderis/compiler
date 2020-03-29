//============================================================================
// @author      : Thomas Dooms
// @date        : 3/21/20
// @copyright   : BA2 Informatica - Thomas Dooms - University of Antwerp
//============================================================================

#include <IRVisitor/irVisitor.h>
#include "expressions.h"

namespace
{
template <typename Type>
bool assign_fold(Type*& elem)
{
    if(auto* res = elem->fold())
    {
        elem = res;
        return true;
    }
    return false;
}

template <typename Type0, typename Type1>
Ast::Literal* fold_modulo(Type0 lhs, Type1 rhs, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
{
    if constexpr(std::is_integral_v<Type0> and std::is_integral_v<Type1>)
    {
        return new Ast::Literal(lhs % rhs, std::move(table), line, column);
    }
    else
    {
        throw InternalError("modulo on floating points while folding", line, column);
    }
}

template <typename Type0, typename Type1>
Ast::Literal*
fold_binary(Type0 lhs, Type1 rhs, BinaryOperation operation, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
{
    if(operation.isDivisionModulo() and rhs == 0) return nullptr;

    if(operation == BinaryOperation::Add)
        return new Ast::Literal(lhs + rhs, std::move(table), line, column);
    else if(operation == BinaryOperation::Sub)
        return new Ast::Literal(lhs - rhs, std::move(table), line, column);
    else if(operation == BinaryOperation::Mul)
        return new Ast::Literal(lhs * rhs, std::move(table), line, column);
    else if(operation == BinaryOperation::Div)
        return new Ast::Literal(lhs / rhs, std::move(table), line, column);
    else if(operation == BinaryOperation::Mod)
        return fold_modulo(lhs, rhs, std::move(table), line, column);
    else if(operation == BinaryOperation::Lt)
        return new Ast::Literal(lhs < rhs, std::move(table), line, column);
    else if(operation == BinaryOperation::Gt)
        return new Ast::Literal(lhs > rhs, std::move(table), line, column);
    else if(operation == BinaryOperation::Le)
        return new Ast::Literal(lhs <= rhs, std::move(table), line, column);
    else if(operation == BinaryOperation::Ge)
        return new Ast::Literal(lhs >= rhs, std::move(table), line, column);
    else if(operation == BinaryOperation::Eq)
        return new Ast::Literal(lhs == rhs, std::move(table), line, column);
    else if(operation == BinaryOperation::Neq)
        return new Ast::Literal(lhs != rhs, std::move(table), line, column);
    else if(operation == BinaryOperation::And)
        return new Ast::Literal(lhs && rhs, std::move(table), line, column);
    else if(operation == BinaryOperation::Or)
        return new Ast::Literal(lhs || rhs, std::move(table), line, column);
    else
        throw InternalError("unknown binary operation", line, column);
}

template <typename Type>
Ast::Literal*
fold_prefix(Type operand, PrefixOperation operation, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
{
    if(operation == PrefixOperation::Plus)
        return new Ast::Literal(operand, std::move(table), line, column);
    else if(operation == PrefixOperation::Neg)
        return new Ast::Literal(-operand, std::move(table), line, column);
    else if(operation == PrefixOperation::Not)
        return new Ast::Literal(!operand, std::move(table), line, column);
    else if(operation == PrefixOperation::Incr)
        return new Ast::Literal(operand + 1, std::move(table), line, column);
    else if(operation == PrefixOperation::Decr)
        return new Ast::Literal(operand - 1, std::move(table), line, column);
    else
        throw InternalError("unknown prefix expression", line, column);
}

template <typename Type>
Ast::Literal*
fold_postfix(Type operand, PostfixOperation operation, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
{
    if(operation == PostfixOperation::Incr)
        return new Ast::Literal(operand + 1, std::move(table), line, column);
    else if(operation == PostfixOperation::Decr)
        return new Ast::Literal(operand - 1, std::move(table), line, column);
    else
        throw InternalError("unknown postfix expression", line, column);
}

template <typename Type>
Ast::Literal*
fold_cast(Type operand, const std::string& operation, std::shared_ptr<SymbolTable> table, size_t line, size_t column)
{
    if(operation == "float")
        return new Ast::Literal((float)operand, std::move(table), line, column);
    else if(operation == "double")
        return new Ast::Literal((double)operand, std::move(table), line, column);
    else if(operation == "char")
        return new Ast::Literal((char)operand, std::move(table), line, column);
    else if(operation == "short")
        return new Ast::Literal((short)operand, std::move(table), line, column);
    else if(operation == "int")
        return new Ast::Literal((int)operand, std::move(table), line, column);
    else if(operation == "long")
        return new Ast::Literal((long)operand, std::move(table), line, column);
    else
        throw InternalError("unknown type for conversion: " + operation, line, column);
}

bool check_const(const std::shared_ptr<SymbolTable>& table,
                 const std::string& identifier,
                 const std::string& operation,
                 size_t line,
                 size_t column)
{
    if(table->lookup_const(identifier))
    {
        std::cout << ConstError(operation, identifier, line, column);
        return false;
    }
    return true;
}
bool check_uninit(const std::shared_ptr<SymbolTable>& table, const std::string& identifier, size_t line, size_t column)
{
    if(not table->lookup_initialized(identifier))
    {
        std::cout << UninitializedWarning(identifier, line, column);
        table->set_initialized(identifier); // this is used so no other warnings are given
    }
    return true;
}

} // namespace

namespace Ast
{
std::string Expr::color () const
{
    return "#ced6eb"; // light blue
}

std::string Comment::name () const
{
    return "comment";
}

std::string Comment::value () const
{
    return "...";
}

std::vector<Node*> Comment::children () const
{
    return {};
}

std::string Comment::color () const
{
    return "#d5ceeb"; // light purple
}

Literal* Comment::fold ()
{
    return nullptr;
}

void Comment::visit(IRVisitor& visitor)
{

}

	std::string Literal::name () const
{
    return "literal";
}

std::string Literal::value () const
{
    return std::visit ([&] (const auto& val) { return std::to_string (val); }, literal);
}

std::vector<Node*> Literal::children () const
{
    return {};
}

Literal* Literal::fold ()
{
    return this;
}

Type Literal::type () const
{
    return Type (true, static_cast<BaseType> (literal.index ()));
}

void Literal::visit(IRVisitor& visitor)
{
	visitor.visitLiteral(*this);
}

std::string Variable::name () const
{
    return identifier;
}

std::string Variable::value () const
{
    return (*table->lookup (identifier))->second.type.string ();
}

std::vector<Node*> Variable::children () const
{
    return {};
}

std::string Variable::color () const
{
    return "#ebe6ce";
}

Literal* Variable::fold ()
{
    const auto& literal = table->get_literal (name ());
    if (literal.has_value ())
    {
        return new Ast::Literal (literal.value (), table, line, column);
    }
    else
        return nullptr;
}

bool Variable::check () const
{
    check_uninit (table, identifier, line, column);
    if (not table->lookup (identifier).has_value ())
    {
        std::cout << UndeclaredError (identifier, line, column);
        return false;
    }
    return true;
}

Type Variable::type () const
{
    const auto temp = table->lookup (identifier);
    if (temp.has_value ()) return (*temp)->second.type;
    else return Type ();
}

void Variable::visit(IRVisitor& visitor)
{
	visitor.visitVariable(*this);
}

	std::string BinaryExpr::name () const
{
    return "binary expression";
}

std::string BinaryExpr::value () const
{
    return operation.string ();
}

std::vector<Node*> BinaryExpr::children () const
{
    return { lhs, rhs };
}

Literal* BinaryExpr::fold ()
{
    auto* new_lhs = lhs->fold ();
    auto* new_rhs = rhs->fold ();

    const auto set_folded = [&] () -> Ast::Literal* {
        if (new_lhs) lhs = new_lhs;
        if (new_rhs) rhs = new_rhs;
        return nullptr;
    };

    if (new_lhs and new_rhs)
    {
        const auto lambda = [&] (const auto& lhs, const auto& rhs) {
            auto* res = fold_binary (lhs, rhs, operation, table, line, column);
            if (res) return res;
            else
                return set_folded ();
        };
        // TODO: deletus feetus, memory leakus
        return std::visit (lambda, new_lhs->literal, new_rhs->literal);
    }
    else
    {
        return set_folded ();
    }
}

bool BinaryExpr::check () const
{
    return Type::combine (operation, lhs->type (), rhs->type (), line, column).has_value ();
}

Type BinaryExpr::type () const
{
    try
    {
        return Type::combine (operation, lhs->type (), rhs->type (), 0, 0, false).value ();
    }
    catch (...)
    {
        return Type ();
    }
}

void BinaryExpr::visit(IRVisitor& visitor)
{
	visitor.visitBinaryExpr(*this);
}

	std::string PrefixExpr::name () const
{
    return "prefix expression";
}

std::string PrefixExpr::value () const
{
    return operation.string () + std::visit ([&] (auto val) { return val->value (); }, operand);
}

std::vector<Node*> PrefixExpr::children () const
{
    const auto node = std::visit ([&] (auto val) { return static_cast<Node*> (val); }, operand);
    return { node };
}

Literal* PrefixExpr::fold ()
{
    auto* new_operand = std::visit ([&] (auto val) { return val->fold (); }, operand);
    const auto lambda
    = [&] (const auto& val) { return fold_prefix (val, operation, table, line, column); };

    // TODO: deletus feetus, memory leakus
    if (new_operand) return std::visit (lambda, new_operand->literal);
    return nullptr;
}

bool PrefixExpr::check () const
{
    if (operand.index () == 0 and operation.isIncrDecr ())
    {
        auto* variable = std::get<Variable*> (operand);
        const auto error = check_const (table, variable->name (), operation.string (), line, column);
        if (not error) return false;
    }
    if (operand.index () == 1 and operation.isIncrDecr ())
    {
        throw SemanticError ("assigning to an r-value in prefix expression", line, column);
    }

    const auto type = std::visit ([&] (auto val) { return val->type (); }, operand);
    return Type::unary (operation, type, line, column).has_value ();
}

Type PrefixExpr::type () const
{
    try
    {
        const auto type = std::visit ([&] (auto val) { return val->type (); }, operand);
        return Type::unary (operation, type, 0, 0, false).value ();
    }
    catch (...)
    {
        return Type ();
    }
}

void PrefixExpr::visit(IRVisitor& visitor)
{
	visitor.visitPrefixExpr(*this);
}

	std::string PostfixExpr::name () const
{
    return "prefix expression";
}

std::string PostfixExpr::value () const
{
    return operation.string () + variable->value ();
}

std::vector<Node*> PostfixExpr::children () const
{
    return { variable };
}

Literal* PostfixExpr::fold ()
{
    auto* new_operand = variable->fold ();
    const auto lambda
    = [&] (const auto& val) { return fold_postfix (val, operation, table, line, column); };

    // TODO: deletus feetus, memory leakus
    if (new_operand) return std::visit (lambda, new_operand->literal);
    return nullptr;
}

bool PostfixExpr::check () const
{
    return check_const (table, variable->name (), operation.string (), line, column);
}

Type PostfixExpr::type () const
{
    return variable->type ();
}

void PostfixExpr::visit(IRVisitor& visitor)
{
	visitor.visitPostfixExpr(*this);
}

	std::string CastExpr::name () const
{
    return "cast expression";
}

std::string CastExpr::value () const
{
    return '(' + cast.string () + ')';
}

std::vector<Node*> CastExpr::children () const
{
    return { operand };
}

Literal* CastExpr::fold ()
{
    auto* new_operand = operand->fold ();
    const auto lambda
    = [&] (const auto& val) { return fold_cast (val, cast.string (), table, line, column); };

    if (new_operand) return std::visit (lambda, new_operand->literal);
    else
        return nullptr;
}

bool CastExpr::check () const
{
    return Type::convert (operand->type (), cast, true, line, column);
}

Type CastExpr::type () const
{
    return cast;
}

void CastExpr::visit(IRVisitor& visitor)
{
	visitor.visitCastExpr(*this);
}

	std::string Assignment::name () const
{
    return "assignment";
}

std::string Assignment::value () const
{
    return "";
}

std::vector<Node*> Assignment::children () const
{
    return { variable, expr };
}

Literal* Assignment::fold ()
{
    assign_fold (expr);
    return nullptr;
}

bool Assignment::check () const
{
    if (table->lookup_const (variable->name ()))
    {
        std::cout << ConstError ("assignment", variable->name (), line, column);
        return false;
    }
    table->set_initialized (variable->name ());
    return Type::convert (expr->type (), variable->type (), false, line, column);
}

Type Assignment::type () const
{
    return variable->type ();
}

void Assignment::visit(IRVisitor& visitor)
{
	visitor.visitAssignment(*this);
}

	std::string PrintfStatement::name() const
{
    return "printf";
}

std::string PrintfStatement::value() const
{
    return "";
}

std::vector<Node*> PrintfStatement::children() const
{
    return { expr };
}

Literal* PrintfStatement::fold()
{
    if(auto* res = expr->fold()) expr = res;
    return nullptr;
}

Type PrintfStatement::type () const
{
    return Type(false, BaseType::Int);
}

void PrintfStatement::visit(IRVisitor& visitor)
{
	visitor.visitPrintfStatement(*this);
}
} // namespace Ast