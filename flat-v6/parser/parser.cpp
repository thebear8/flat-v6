#include "parser.hpp"

#include "../util/error_logger.hpp"
#include "../util/graph_context.hpp"

ASTExpression* Parser::l0()
{
    auto begin = trim();
    if (match(Token::ParenOpen))
    {
        auto e = expression();
        expect(Token::ParenClose);
        return e;
    }
    else if (match(Token::Integer))
    {
        return m_ctx.make(ASTIntegerExpression(
            SourceRef(m_id, begin, getPosition()),
            getIntValue(),
            getIntSuffixValue()
        ));
    }
    else if (match(Token::True) || match(Token::False))
    {
        return m_ctx.make(ASTBoolExpression(
            SourceRef(m_id, begin, getPosition()), std::string(getTokenValue())
        ));
    }
    else if (match(Token::CharLiteral))
    {
        return m_ctx.make(ASTCharExpression(
            SourceRef(m_id, begin, getPosition()), std::string(getTokenValue())
        ));
    }
    else if (match(Token::StringLiteral))
    {
        return m_ctx.make(ASTStringExpression(
            SourceRef(m_id, begin, getPosition()), std::string(getTokenValue())
        ));
    }
    else if (match(Token::Identifier))
    {
        auto identifier = std::string(getTokenValue());
        auto typeArgs = typeArgList();
        if (match(Token::BraceOpen))
        {
            std::vector<std::pair<std::string, ASTExpression*>> values;
            while (!match(Token::BraceClose) && !match(Token::Eof))
            {
                expect(Token::Identifier);
                auto name = std::string(getTokenValue());
                expect(Token::Colon);
                values.push_back(std::pair(name, expression()));
                if (!match(Token::Comma))
                {
                    expect(Token::BraceClose);
                    break;
                }
            }
            return m_ctx.make(ASTStructExpression(
                SourceRef(m_id, begin, getPosition()),
                identifier,
                typeArgs,
                values
            ));
        }
        else
        {
            return m_ctx.make(ASTIdentifierExpression(
                SourceRef(m_id, begin, getPosition()), identifier, typeArgs
            ));
        }
    }
    else
    {
        return m_logger.error(
            SourceRef(m_id, getPosition()), "Invalid L0", nullptr
        );
    }
}

ASTExpression* Parser::l1()
{
    auto begin = trim();
    auto e = l0();
    while (true)
    {
        if (match(Token::ParenOpen))
        {
            std::vector<ASTExpression*> args;
            while (!match(Token::ParenClose) && !match(Token::Eof))
            {
                args.push_back(expression());
                if (!match(Token::Comma))
                {
                    expect(Token::ParenClose);
                    break;
                }
            }
            e = m_ctx.make(ASTCallExpression(
                SourceRef(m_id, begin, getPosition()), e, args
            ));
        }
        else if (match(Token::BracketOpen))
        {
            std::vector<ASTExpression*> args;
            while (!match(Token::BracketClose) && !match(Token::Eof))
            {
                args.push_back(expression());
                if (!match(Token::Comma))
                {
                    expect(Token::BracketClose);
                    break;
                }
            }
            e = m_ctx.make(ASTIndexExpression(
                SourceRef(m_id, begin, getPosition()), e, args
            ));
        }
        else if (match(Token::Dot))
        {
            expect(Token::Identifier);
            e = m_ctx.make(ASTFieldExpression(
                SourceRef(m_id, begin, getPosition()),
                e,
                std::string(getTokenValue())
            ));
        }
        else
        {
            return e;
        }
    }
}

ASTExpression* Parser::l2()
{
    auto begin = trim();
    if (match(Token::Plus))
    {
        auto e = l2();
        return m_ctx.make(ASTUnaryExpression(
            SourceRef(m_id, begin, getPosition()), UnaryOperator::Positive, e
        ));
    }
    else if (match(Token::Minus))
    {
        auto e = l2();
        return m_ctx.make(ASTUnaryExpression(
            SourceRef(m_id, begin, getPosition()), UnaryOperator::Negative, e
        ));
    }
    else if (match(Token::LogicalNot))
    {
        auto e = l2();
        return m_ctx.make(ASTUnaryExpression(
            SourceRef(m_id, begin, getPosition()), UnaryOperator::LogicalNot, e
        ));
    }
    else if (match(Token::BitwiseNot))
    {
        auto e = l2();
        return m_ctx.make(ASTUnaryExpression(
            SourceRef(m_id, begin, getPosition()), UnaryOperator::BitwiseNot, e
        ));
    }
    else
    {
        return l1();
    }
}

ASTExpression* Parser::l3()
{
    auto begin = trim();
    auto e = l2();
    while (true)
    {
        if (match(Token::Multiply))
        {
            auto r = l2();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::Multiply,
                e,
                r
            ));
        }
        else if (match(Token::Divide))
        {
            auto r = l2();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::Divide,
                e,
                r
            ));
        }
        else if (match(Token::Modulo))
        {
            auto r = l2();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::Modulo,
                e,
                r
            ));
        }
        else
        {
            return e;
        }
    }
}

ASTExpression* Parser::l4()
{
    auto begin = trim();
    auto e = l3();
    while (true)
    {
        if (match(Token::Plus))
        {
            auto r = l3();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()), BinaryOperator::Add, e, r
            ));
        }
        else if (match(Token::Minus))
        {
            auto r = l3();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::Subtract,
                e,
                r
            ));
        }
        else
        {
            return e;
        }
    }
}

ASTExpression* Parser::l5()
{
    auto begin = trim();
    auto e = l4();
    while (true)
    {
        if (match(Token::ShiftLeft))
        {
            auto r = l4();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::ShiftLeft,
                e,
                r
            ));
        }
        else if (match(Token::ShiftRight))
        {
            auto r = l4();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::ShiftRight,
                e,
                r
            ));
        }
        else
        {
            return e;
        }
    }
}

ASTExpression* Parser::l6()
{
    auto begin = trim();
    auto e = l5();
    while (true)
    {
        if (match(Token::BitwiseAnd))
        {
            auto r = l5();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::BitwiseAnd,
                e,
                r
            ));
        }
        else if (match(Token::BitwiseOr))
        {
            auto r = l5();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::BitwiseOr,
                e,
                r
            ));
        }
        else if (match(Token::BitwiseXor))
        {
            auto r = l5();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::BitwiseXor,
                e,
                r
            ));
        }
        else
        {
            return e;
        }
    }
}

ASTExpression* Parser::l7()
{
    auto begin = trim();
    auto e = l6();
    while (true)
    {
        if (match(Token::Equal))
        {
            auto r = l6();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::Equal,
                e,
                r
            ));
        }
        else if (match(Token::NotEqual))
        {
            auto r = l6();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::NotEqual,
                e,
                r
            ));
        }
        else if (match(Token::LessThan))
        {
            auto r = l6();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::LessThan,
                e,
                r
            ));
        }
        else if (match(Token::GreaterThan))
        {
            auto r = l6();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::GreaterThan,
                e,
                r
            ));
        }
        else if (match(Token::LessOrEqual))
        {
            auto r = l6();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::LessOrEqual,
                e,
                r
            ));
        }
        else if (match(Token::GreaterOrEqual))
        {
            auto r = l6();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::GreaterOrEqual,
                e,
                r
            ));
        }
        else
        {
            return e;
        }
    }
}

ASTExpression* Parser::l8()
{
    auto begin = trim();
    auto e = l7();
    while (true)
    {
        if (match(Token::LogicalAnd))
        {
            auto r = l7();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::LogicalAnd,
                e,
                r
            ));
        }
        else
        {
            return e;
        }
    }
}

ASTExpression* Parser::l9()
{
    auto begin = trim();
    auto e = l8();
    while (true)
    {
        if (match(Token::LogicalOr))
        {
            auto r = l8();
            e = m_ctx.make(ASTBinaryExpression(
                SourceRef(m_id, begin, getPosition()),
                BinaryOperator::LogicalOr,
                e,
                r
            ));
        }
        else
        {
            return e;
        }
    }
}

ASTExpression* Parser::l10()
{
    auto begin = trim();
    auto e = l9();
    if (match(Token::Assign))
    {
        auto r = l10();
        return m_ctx.make(ASTBinaryExpression(
            SourceRef(m_id, begin, getPosition()), BinaryOperator::Assign, e, r
        ));
    }
    else
    {
        return e;
    }
}

ASTExpression* Parser::expression()
{
    return l10();
}

ASTStatement* Parser::expressionStatement()
{
    auto begin = trim();
    auto e = expression();
    expect(Token::NewLine);
    return m_ctx.make(
        ASTExpressionStatement(SourceRef(m_id, begin, getPosition()), e)
    );
}

ASTStatement* Parser::blockStatement()
{
    auto begin = trim();
    expect(Token::BraceOpen);

    std::vector<ASTStatement*> statements;
    while (!match(Token::BraceClose) && !match(Token::Eof))
    {
        statements.push_back(statement());
    }
    return m_ctx.make(
        ASTBlockStatement(SourceRef(m_id, begin, getPosition()), statements)
    );
}

ASTStatement* Parser::variableStatement()
{
    auto begin = trim();
    expect(Token::Let);

    std::vector<std::pair<std::string, ASTExpression*>> items;
    while (!match(Token::NewLine) && match(Token::Identifier))
    {
        auto name = std::string(getTokenValue());
        expect(Token::Assign);
        items.push_back(std::pair(name, expression()));
        if (!match(Token::Comma))
        {
            expect(Token::NewLine);
            break;
        }
    }
    return m_ctx.make(
        ASTVariableStatement(SourceRef(m_id, begin, getPosition()), items)
    );
}

ASTStatement* Parser::returnStatement()
{
    auto begin = trim();
    expect(Token::Return);

    if (match(Token::NewLine))
    {
        return m_ctx.make(
            ASTReturnStatement(SourceRef(m_id, begin, getPosition()), nullptr)
        );
    }
    else
    {
        auto e = expression();
        expect(Token::NewLine);
        return m_ctx.make(
            ASTReturnStatement(SourceRef(m_id, begin, getPosition()), e)
        );
    }
}

ASTStatement* Parser::whileStatement()
{
    auto begin = trim();
    expect(Token::While);
    expect(Token::ParenOpen);

    auto condition = expression();
    expect(Token::ParenClose);

    auto body = statement();
    return m_ctx.make(ASTWhileStatement(
        SourceRef(m_id, begin, getPosition()), condition, body
    ));
}

ASTStatement* Parser::ifStatement()
{
    auto begin = trim();
    expect(Token::If);
    expect(Token::ParenOpen);

    auto condition = expression();
    expect(Token::ParenClose);

    auto ifBody = statement();
    if (match(Token::Else))
    {
        auto elseBody = statement();
        return m_ctx.make(ASTIfStatement(
            SourceRef(m_id, begin, getPosition()), condition, ifBody, elseBody
        ));
    }
    else
    {
        return m_ctx.make(ASTIfStatement(
            SourceRef(m_id, begin, getPosition()), condition, ifBody, nullptr
        ));
    }
}

ASTStatement* Parser::statement()
{
    if (peek(Token::BraceOpen))
        return blockStatement();
    else if (peek(Token::Let))
        return variableStatement();
    else if (peek(Token::Return))
        return returnStatement();
    else if (peek(Token::While))
        return whileStatement();
    else if (peek(Token::If))
        return ifStatement();

    return expressionStatement();
}

ASTConstraintCondition* Parser::constraintCondition()
{
    auto begin = trim();
    expect(Token::Function);
    expect(Token::Identifier);
    auto name = std::string(getTokenValue());

    std::vector<std::pair<std::string, ASTType*>> parameters;
    expect(Token::ParenOpen);
    while (!match(Token::ParenClose) && !match(Token::Eof))
    {
        expect(Token::Identifier);
        auto paramName = std::string(getTokenValue());
        expect(Token::Colon);
        auto type = typeName();
        parameters.push_back(std::make_pair(paramName, type));
        if (!match(Token::Comma))
        {
            expect(Token::ParenClose);
            break;
        }
    }

    auto result = (match(Token::Colon) ? typeName() : nullptr);

    return m_ctx.make(ASTConstraintCondition(
        SourceRef(m_id, begin, getPosition()), name, parameters, result
    ));
}

ASTConstraintDeclaration* Parser::constraintDeclaration()
{
    auto begin = trim();
    expect(Token::Constraint);
    expect(Token::Identifier);
    auto constraintName = std::string(getTokenValue());
    auto typeParams = typeParamList();
    auto requirements = requirementList();

    expect(Token::BraceOpen);
    std::vector<ASTConstraintCondition*> conditions;
    while (!match(Token::BraceClose) && !match(Token::Eof))
    {
        conditions.push_back(constraintCondition());
        if (!match(Token::Comma))
        {
            expect(Token::BraceClose);
            break;
        }
    }

    return m_ctx.make(ASTConstraintDeclaration(
        SourceRef(m_id, begin, getPosition()),
        constraintName,
        typeParams,
        requirements,
        conditions
    ));
}

ASTStructDeclaration* Parser::structDeclaration()
{
    auto begin = trim();
    expect(Token::Struct);
    expect(Token::Identifier);
    auto structName = std::string(getTokenValue());
    auto typeParams = typeParamList();

    expect(Token::BraceOpen);
    std::vector<std::pair<std::string, ASTType*>> fields;
    while (!match(Token::BraceClose) && !match(Token::Eof))
    {
        expect(Token::Identifier);
        auto name = std::string(getTokenValue());
        expect(Token::Colon);
        fields.push_back(std::pair(name, typeName()));
        if (!match(Token::Comma))
        {
            expect(Token::BraceClose);
            break;
        }
    }

    return m_ctx.make(ASTStructDeclaration(
        SourceRef(m_id, begin, getPosition()), structName, typeParams, fields
    ));
}

ASTFunctionDeclaration* Parser::functionDeclaration()
{
    auto begin = trim();
    auto isExtern = false;

    std::vector<ASTFunctionAttribute*> attributes;
    while (match(Token::At))
    {
        if (match(Token::NoMangle))
        {
            attributes.push_back(ctx.make(ASTFunctionAttribute(
                SourceRef(id, begin, position), getTokenValue(), {}
            )));
        }
        else if (match(Token::Test))
        {
            attributes.push_back(ctx.make(ASTFunctionAttribute(
                SourceRef(id, begin, position), getTokenValue(), {}
            )));
        }
        else if (match(Token::Extern))
        {
            isExtern = true;
            attributes.push_back(ctx.make(ASTFunctionAttribute(
                SourceRef(id, begin, position), getTokenValue(), {}
            )));
        }
        else
        {
            logger.error(
                SourceRef(id, position),
                "Expected eiter no_mangle, test or extern"
            );
        }
    }

    expect(Token::Function);
    expect(Token::Identifier);
    auto name = getTokenValue();
    auto typeParams = (isExtern ? std::vector<std::string>() : typeParamList());
    auto requirements =
        (isExtern ? std::vector<ASTRequirement*>() : requirementList());

    std::vector<std::pair<std::string, ASTType*>> parameters;
    expect(Token::ParenOpen);
    while (!match(Token::ParenClose) && !match(Token::Eof))
    {
        expect(Token::Identifier);
        auto paramName = std::string(getTokenValue());
        expect(Token::Colon);
        auto type = typeName();
        parameters.push_back({ paramName, type });
        if (!match(Token::Comma))
        {
            expect(Token::ParenClose);
            break;
        }
    }

    auto result =
        (isExtern ? (expect(Token::Colon), typeName())
                  : (match(Token::Colon) ? typeName() : nullptr));
    auto body = (isExtern ? nullptr : blockStatement());
    return ctx.make(ASTFunctionDeclaration(
        SourceRef(id, begin, position),
        name,
        attributes,
        typeParams,
        parameters,
        result,
        requirements,
        body
    ));
}

ASTSourceFile* Parser::sourceFile()
{
    auto begin = trim();

    std::string modulePath;
    expect(Token::Module);
    while (!match(Token::Eof))
    {
        expect(Token::Identifier);
        modulePath +=
            (modulePath.empty() ? std::string(getTokenValue())
                                : "." + std::string(getTokenValue()));
        if (!match(Token::Dot))
            break;
        modulePath += ".";
    }

    std::vector<std::string> imports;
    while (match(Token::Import))
    {
        std::string importPath;
        while (!match(Token::Eof))
        {
            expect(Token::Identifier);
            importPath +=
                (importPath.empty() ? std::string(getTokenValue())
                                    : "." + std::string(getTokenValue()));
            if (!match(Token::Dot))
                break;
            importPath += ".";
        }
        imports.push_back(importPath);
    }

    std::vector<ASTDeclaration*> declarations;
    while (!match(Token::Eof))
    {
        if (peek(Token::Constraint))
            declarations.push_back(constraintDeclaration());
        else if (peek(Token::Struct))
            declarations.push_back(structDeclaration());
        else if (peek(Token::Function) || peek(Token::At))
            declarations.push_back(functionDeclaration());
        else
        {
            m_logger.error(
                SourceRef(m_id, getPosition()),
                "Expected eiter ConstraintDeclaration, StructDeclaration, FunctionDeclaration or ExternFunctionDeclaration"
            );
        }
    }
    return m_ctx.make(ASTSourceFile(
        SourceRef(m_id, begin, getPosition()), modulePath, imports, declarations
    ));
}

ASTType* Parser::typeName()
{
    auto begin = trim();
    expect(Token::Identifier);
    auto name = std::string(getTokenValue());

    auto typeArgs = typeArgList();

    ASTType* type = m_ctx.make(
        ASTNamedType(SourceRef(m_id, begin, getPosition()), name, typeArgs)
    );
    while (true)
    {
        if (match(Token::Multiply))
        {
            type = m_ctx.make(
                ASTPointerType(SourceRef(m_id, begin, getPosition()), type)
            );
        }
        else if (match(Token::BracketOpen) && match(Token::BracketClose))
        {
            type = m_ctx.make(
                ASTArrayType(SourceRef(m_id, begin, getPosition()), type)
            );
        }
        else
        {
            return type;
        }
    }
}

//

std::vector<std::string> Parser::typeParamList()
{
    std::vector<std::string> params;
    if (match(Token::LessThan))
    {
        while (!match(Token::GreaterThan) && !match(Token::Eof))
        {
            expect(Token::Identifier);
            params.push_back(std::string(getTokenValue()));
            if (!match(Token::Comma))
            {
                expect(Token::GreaterThan);
                break;
            }
        }
    }
    return params;
}

std::vector<ASTType*> Parser::typeArgList()
{
    std::vector<ASTType*> args;
    if (match(Token::LessThan))
    {
        while (!match(Token::GreaterThan) && !match(Token::Eof))
        {
            args.push_back(typeName());
            if (!match(Token::Comma))
            {
                expect(Token::GreaterThan);
                break;
            }
        }
    }
    return args;
}

std::vector<ASTRequirement*> Parser::requirementList()
{
    std::vector<ASTRequirement*> requirements;
    if (match(Token::BracketOpen))
    {
        while (!match(Token::BracketClose) && !match(Token::Eof))
        {
            auto begin = trim();
            expect(Token::Identifier);
            auto constraintName = std::string(getTokenValue());
            auto typeArgs = typeArgList();
            requirements.push_back(m_ctx.make(ASTRequirement(
                SourceRef(m_id, begin, getPosition()), constraintName, typeArgs
            )));
            if (!match(Token::Comma))
            {
                expect(Token::BracketClose);
                break;
            }
        }
    }
    return requirements;
}

size_t Parser::trim()
{
    return m_lexer.trim();
}

TokenInfo Parser::match(Token token)
{
    size_t position = m_lexer.getPosition();
    auto next = m_lexer.match(token);
    if (next.token == Token::Error)
    {
        m_logger.error(SourceRef(m_id, next.begin, next.end), "Unknown token");
    }

    return next;
}

TokenInfo Parser::expect(Token token)
{
    auto next = match(token);
    if (!next)
    {
        m_logger.error(
            SourceRef(m_id, next.begin, next.end),
            std::string("Unexpected token ") + TokenNames[(size_t)next.token]
                + " `" + std::string(next.value) + "`\n  expected "
                + TokenNames[(size_t)token]
        );
    }

    return next;
}

TokenInfo Parser::lookahead(Token token)
{
    auto next = m_lexer.lookahead(token);
    if (!next)
    {
        m_logger.error(SourceRef(m_id, next.begin, next.end), "Unknown token");
    }

    return next;
}