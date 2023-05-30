#include "parser.hpp"

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
        return ctx.make(ASTIntegerExpression(
            SourceRef(id, begin, position), getIntValue(), getIntSuffixValue()
        ));
    }
    else if (match(Token::True) || match(Token::False))
    {
        return ctx.make(
            ASTBoolExpression(SourceRef(id, begin, position), getTokenValue())
        );
    }
    else if (match(Token::CharLiteral))
    {
        return ctx.make(
            ASTCharExpression(SourceRef(id, begin, position), getTokenValue())
        );
    }
    else if (match(Token::StringLiteral))
    {
        return ctx.make(
            ASTStringExpression(SourceRef(id, begin, position), getTokenValue())
        );
    }
    else if (match(Token::Identifier))
    {
        auto identifier = getTokenValue();
        auto typeArgs = typeArgList();
        if (match(Token::BraceOpen))
        {
            std::vector<std::pair<std::string, ASTExpression*>> values;
            while (!match(Token::BraceClose) && !match(Token::Eof))
            {
                expect(Token::Identifier);
                auto name = getTokenValue();
                expect(Token::Colon);
                values.push_back(std::pair(name, expression()));
                if (!match(Token::Comma))
                {
                    expect(Token::BraceClose);
                    break;
                }
            }
            return ctx.make(ASTStructExpression(
                SourceRef(id, begin, position), identifier, typeArgs, values
            ));
        }
        else
        {
            return ctx.make(ASTIdentifierExpression(
                SourceRef(id, begin, position), identifier, typeArgs
            ));
        }
    }
    else
    {
        return logger.error(SourceRef(id, position), "Invalid L0", nullptr);
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
            e = ctx.make(
                ASTCallExpression(SourceRef(id, begin, position), e, args)
            );
        }
        else if (match(Token::BracketOpen))
        {
            std::vector<ASTExpression*> args;
            while (!match(Token::BracketClose) && !match(Token::Eof))
            {
                args.push_back(expression());
                if (!match(Token::Comma))
                {
                    expect(Token::ParenClose);
                    break;
                }
            }
            e = ctx.make(
                ASTIndexExpression(SourceRef(id, begin, position), e, args)
            );
        }
        else if (match(Token::Dot))
        {
            expect(Token::Identifier);
            e = ctx.make(ASTFieldExpression(
                SourceRef(id, begin, position), e, getTokenValue()
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
        return ctx.make(ASTUnaryExpression(
            SourceRef(id, begin, position), UnaryOperator::Positive, e
        ));
    }
    else if (match(Token::Minus))
    {
        auto e = l2();
        return ctx.make(ASTUnaryExpression(
            SourceRef(id, begin, position), UnaryOperator::Negative, e
        ));
    }
    else if (match(Token::LogicalNot))
    {
        auto e = l2();
        return ctx.make(ASTUnaryExpression(
            SourceRef(id, begin, position), UnaryOperator::LogicalNot, e
        ));
    }
    else if (match(Token::BitwiseNot))
    {
        auto e = l2();
        return ctx.make(ASTUnaryExpression(
            SourceRef(id, begin, position), UnaryOperator::BitwiseNot, e
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
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::Multiply, e, r
            ));
        }
        else if (match(Token::Divide))
        {
            auto r = l2();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::Divide, e, r
            ));
        }
        else if (match(Token::Modulo))
        {
            auto r = l2();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::Modulo, e, r
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
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::Add, e, r
            ));
        }
        else if (match(Token::Minus))
        {
            auto r = l3();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::Subtract, e, r
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
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::ShiftLeft, e, r
            ));
        }
        else if (match(Token::ShiftRight))
        {
            auto r = l4();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::ShiftRight, e, r
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
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::BitwiseAnd, e, r
            ));
        }
        else if (match(Token::BitwiseOr))
        {
            auto r = l5();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::BitwiseOr, e, r
            ));
        }
        else if (match(Token::BitwiseXor))
        {
            auto r = l5();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::BitwiseXor, e, r
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
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::Equal, e, r
            ));
        }
        else if (match(Token::NotEqual))
        {
            auto r = l6();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::NotEqual, e, r
            ));
        }
        else if (match(Token::LessThan))
        {
            auto r = l6();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::LessThan, e, r
            ));
        }
        else if (match(Token::GreaterThan))
        {
            auto r = l6();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position),
                BinaryOperator::GreaterThan,
                e,
                r
            ));
        }
        else if (match(Token::LessOrEqual))
        {
            auto r = l6();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position),
                BinaryOperator::LessOrEqual,
                e,
                r
            ));
        }
        else if (match(Token::GreaterOrEqual))
        {
            auto r = l6();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position),
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
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::LogicalAnd, e, r
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
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::LogicalOr, e, r
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
        return ctx.make(ASTBinaryExpression(
            SourceRef(id, begin, position), BinaryOperator::Assign, e, r
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
    return ctx.make(ASTExpressionStatement(SourceRef(id, begin, position), e));
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
    return ctx.make(
        ASTBlockStatement(SourceRef(id, begin, position), statements)
    );
}

ASTStatement* Parser::variableStatement()
{
    auto begin = trim();
    expect(Token::Let);

    std::vector<std::pair<std::string, ASTExpression*>> items;
    while (!match(Token::NewLine) && match(Token::Identifier))
    {
        auto name = getTokenValue();
        expect(Token::Assign);
        items.push_back(std::pair(name, expression()));
        if (!match(Token::Comma))
        {
            expect(Token::NewLine);
            break;
        }
    }
    return ctx.make(ASTVariableStatement(SourceRef(id, begin, position), items)
    );
}

ASTStatement* Parser::returnStatement()
{
    auto begin = trim();
    expect(Token::Return);

    if (match(Token::NewLine))
    {
        return ctx.make(
            ASTReturnStatement(SourceRef(id, begin, position), nullptr)
        );
    }
    else
    {
        auto e = expression();
        expect(Token::NewLine);
        return ctx.make(ASTReturnStatement(SourceRef(id, begin, position), e));
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
    return ctx.make(
        ASTWhileStatement(SourceRef(id, begin, position), condition, body)
    );
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
        return ctx.make(ASTIfStatement(
            SourceRef(id, begin, position), condition, ifBody, elseBody
        ));
    }
    else
    {
        return ctx.make(ASTIfStatement(
            SourceRef(id, begin, position), condition, ifBody, nullptr
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
    auto name = getTokenValue();

    std::vector<std::pair<std::string, ASTType*>> parameters;
    expect(Token::ParenOpen);
    while (!match(Token::ParenClose) && !match(Token::Eof))
    {
        expect(Token::Identifier);
        auto paramName = getTokenValue();
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

    return ctx.make(ASTConstraintCondition(
        SourceRef(id, begin, position), name, parameters, result
    ));
}

ASTConstraintDeclaration* Parser::constraintDeclaration()
{
    auto begin = trim();
    expect(Token::Constraint);
    expect(Token::Identifier);
    auto constraintName = getTokenValue();
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

    return ctx.make(ASTConstraintDeclaration(
        SourceRef(id, begin, position),
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
    auto structName = getTokenValue();
    auto typeParams = typeParamList();

    expect(Token::BraceOpen);
    std::vector<std::pair<std::string, ASTType*>> fields;
    while (!match(Token::BraceClose) && !match(Token::Eof))
    {
        expect(Token::Identifier);
        auto name = getTokenValue();
        expect(Token::Colon);
        fields.push_back(std::pair(name, typeName()));
        if (!match(Token::Comma))
        {
            expect(Token::BraceClose);
            break;
        }
    }

    return ctx.make(ASTStructDeclaration(
        SourceRef(id, begin, position), structName, typeParams, fields
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
            auto attribute = getTokenValue();
            expect(Token::ParenOpen);
            expect(Token::StringLiteral);
            auto library = getTokenValue();

            attributes.push_back(ctx.make(ASTFunctionAttribute(
                SourceRef(id, begin, position), attribute, { library }
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
        auto paramName = getTokenValue();
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
            (modulePath.empty() ? getTokenValue() : "." + getTokenValue());
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
                (importPath.empty() ? getTokenValue() : "." + getTokenValue());
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
            logger.error(
                SourceRef(id, position),
                "Expected eiter ConstraintDeclaration, StructDeclaration, FunctionDeclaration or ExternFunctionDeclaration"
            );
        }
    }
    return ctx.make(ASTSourceFile(
        SourceRef(id, begin, position), modulePath, imports, declarations
    ));
}

ASTType* Parser::typeName()
{
    auto begin = trim();
    expect(Token::Identifier);
    auto name = getTokenValue();

    auto typeArgs = typeArgList();

    ASTType* type =
        ctx.make(ASTNamedType(SourceRef(id, begin, position), name, typeArgs));
    while (true)
    {
        if (match(Token::Multiply))
        {
            type =
                ctx.make(ASTPointerType(SourceRef(id, begin, position), type));
        }
        else if (match(Token::BracketOpen) && match(Token::BracketClose))
        {
            type = ctx.make(ASTArrayType(SourceRef(id, begin, position), type));
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
            params.push_back(getTokenValue());
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
            auto constraintName = getTokenValue();
            auto typeArgs = typeArgList();
            requirements.push_back(ctx.make(ASTRequirement(
                SourceRef(id, begin, position), constraintName, typeArgs
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