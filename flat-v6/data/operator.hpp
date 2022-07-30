#pragma once
#include <string>
#include <unordered_map>

#include "enum.hpp"
#include "token.hpp"

#define UNARY_OPERATOR_LIST(ENTRY) \
ENTRY(Positive) \
ENTRY(Negative) \
ENTRY(BitwiseNot) \
ENTRY(LogicalNot)
DEFINE_ENUM(UnaryOperator, UNARY_OPERATOR_LIST)

#define BINARY_OPERATOR_LIST(ENTRY) \
ENTRY(Add) \
ENTRY(Subtract) \
ENTRY(Multiply) \
ENTRY(Divide) \
ENTRY(Modulo) \
ENTRY(BitwiseAnd) \
ENTRY(BitwiseOr) \
ENTRY(BitwiseXor) \
ENTRY(ShiftLeft) \
ENTRY(ShiftRight) \
ENTRY(LogicalAnd) \
ENTRY(LogicalOr) \
ENTRY(Equal) \
ENTRY(NotEqual) \
ENTRY(LessThan) \
ENTRY(GreaterThan) \
ENTRY(LessOrEqual) \
ENTRY(GreaterOrEqual) \
ENTRY(Assign)
DEFINE_ENUM(BinaryOperator, BINARY_OPERATOR_LIST)

#define OPERATOR_CATEGORY_LIST(ENTRY) \
ENTRY(UnaryArithmetic) \
ENTRY(UnaryBitwise) \
ENTRY(UnaryLogic) \
ENTRY(BinaryArithmetic) \
ENTRY(BinaryBitwise) \
ENTRY(BinaryLogic) \
ENTRY(BinaryEquality) \
ENTRY(BinaryComparison) \
ENTRY(BinaryAssign)
DEFINE_ENUM(OperatorCategory, OPERATOR_CATEGORY_LIST)

struct OperatorInfo
{
	std::string name;
	std::string symbol;
	OperatorCategory category;
};

static const inline std::unordered_map<UnaryOperator, OperatorInfo> unaryOperators =
{
	{ UnaryOperator::Positive, { "__pos__", "+", OperatorCategory::UnaryArithmetic } },
	{ UnaryOperator::Negative, { "__neg__", "-", OperatorCategory::UnaryArithmetic } },
	{ UnaryOperator::BitwiseNot, { "__not__", "!", OperatorCategory::UnaryBitwise } },
	{ UnaryOperator::LogicalNot, { "__lnot__", "~", OperatorCategory::UnaryLogic } },
};

static const inline std::unordered_map<BinaryOperator, OperatorInfo> binaryOperators =
{
	{ BinaryOperator::Add, { "__add__", "+", OperatorCategory::BinaryArithmetic } },
	{ BinaryOperator::Subtract, { "__sub__", "-", OperatorCategory::BinaryArithmetic } },
	{ BinaryOperator::Multiply, { "__mul__", "*", OperatorCategory::BinaryArithmetic } },
	{ BinaryOperator::Divide, { "__div__", "/", OperatorCategory::BinaryArithmetic } },
	{ BinaryOperator::Modulo, { "__mod__", "%", OperatorCategory::BinaryArithmetic } },

	{ BinaryOperator::BitwiseAnd, { "__and__", "&", OperatorCategory::BinaryBitwise } },
	{ BinaryOperator::BitwiseOr, { "__or__", "|", OperatorCategory::BinaryBitwise } },
	{ BinaryOperator::BitwiseXor, { "__xor__", "^", OperatorCategory::BinaryBitwise } },
	{ BinaryOperator::ShiftLeft, { "__shl__", "<<", OperatorCategory::BinaryBitwise } },
	{ BinaryOperator::ShiftRight, { "__shr__", ">>", OperatorCategory::BinaryBitwise } },

	{ BinaryOperator::LogicalAnd, { "__land__", "&&", OperatorCategory::BinaryLogic } },
	{ BinaryOperator::LogicalOr, { "__lor__", "||", OperatorCategory::BinaryLogic } },

	{ BinaryOperator::Equal, { "__eq__", "==", OperatorCategory::BinaryEquality } },
	{ BinaryOperator::NotEqual, { "__neq__", "!=", OperatorCategory::BinaryEquality } },

	{ BinaryOperator::LessThan, { "__lt__", "<", OperatorCategory::BinaryComparison } },
	{ BinaryOperator::GreaterThan, { "__gt__", ">", OperatorCategory::BinaryComparison } },
	{ BinaryOperator::LessOrEqual, { "__ltoreq__", "<=", OperatorCategory::BinaryComparison } },
	{ BinaryOperator::GreaterOrEqual, { "__gtoreq__", ">=", OperatorCategory::BinaryComparison } },

	{ BinaryOperator::Assign, { "__assign__", "=", OperatorCategory::BinaryAssign } },
};

/*static const inline std::unordered_map<Token, OperatorInfo> unaryOperators =
{
	{ Token::Plus, { "__pos__", "+", OperatorCategory::UnaryArithmetic } },
	{ Token::Minus, { "__neg__", "-", OperatorCategory::UnaryArithmetic } },
	{ Token::BitwiseNot, { "__not__", "!", OperatorCategory::UnaryBitwise } },
	{ Token::LogicalNot, { "__lnot__", "~", OperatorCategory::UnaryLogic } },
};

static const inline std::unordered_map<Token, OperatorInfo> binaryOperators =
{
	{ Token::Plus, { "__add__", "+", OperatorCategory::BinaryArithmetic } },
	{ Token::Minus, { "__sub__", "-", OperatorCategory::BinaryArithmetic } },
	{ Token::Multiply, { "__mul__", "*", OperatorCategory::BinaryArithmetic } },
	{ Token::Divide, { "__div__", "/", OperatorCategory::BinaryArithmetic } },
	{ Token::Modulo, { "__mod__", "%", OperatorCategory::BinaryArithmetic } },

	{ Token::BitwiseAnd, { "__and__", "&", OperatorCategory::BinaryBitwise } },
	{ Token::BitwiseOr, { "__or__", "|", OperatorCategory::BinaryBitwise } },
	{ Token::BitwiseXor, { "__xor__", "^", OperatorCategory::BinaryBitwise } },
	{ Token::ShiftLeft, { "__shl__", "<<", OperatorCategory::BinaryBitwise } },
	{ Token::ShiftRight, { "__shr__", ">>", OperatorCategory::BinaryBitwise } },

	{ Token::LogicalAnd, { "__land__", "&&", OperatorCategory::BinaryLogic } },
	{ Token::LogicalOr, { "__lor__", "||", OperatorCategory::BinaryLogic } },

	{ Token::Equal, { "__eq__", "==", OperatorCategory::BinaryEquality } },
	{ Token::NotEqual, { "__neq__", "!=", OperatorCategory::BinaryEquality } },

	{ Token::LessThan, { "__lt__", "<", OperatorCategory::BinaryComparison } },
	{ Token::GreaterThan, { "__gt__", ">", OperatorCategory::BinaryComparison } },
	{ Token::LessOrEqual, { "__ltoreq__", "<=", OperatorCategory::BinaryComparison } },
	{ Token::GreaterOrEqual, { "__gtoreq__", ">=", OperatorCategory::BinaryComparison } },

	{ Token::Assign, { "__assign__", "=", OperatorCategory::BinaryAssign } },
};
*/