#include "parser.hpp"

ASTExpression* Parser::l0() {
    auto begin = trim();
    if (match(Token::ParenOpen)) {
        auto e = expression();
        expect(Token::ParenClose);
        return e;
    } else if (match(Token::Integer)) {
        return ctx.make(ASTIntegerExpression(
            SourceRef(id, begin, position), getIntValue(), getIntSuffixValue()
        ));
    } else if (match(Token::True) || match(Token::False)) {
        return ctx.make(
            ASTBoolExpression(SourceRef(id, begin, position), getTokenValue())
        );
    } else if (match(Token::CharLiteral)) {
        return ctx.make(
            ASTCharExpression(SourceRef(id, begin, position), getTokenValue())
        );
    } else if (match(Token::StringLiteral)) {
        return ctx.make(
            ASTStringExpression(SourceRef(id, begin, position), getTokenValue())
        );
    } else if (match(Token::Identifier)) {
        auto identifier = getTokenValue();
        if (match(Token::BraceOpen)) {
            std::vector<std::pair<std::string, ASTExpression*>> values;
            while (!match(Token::BraceClose) && !match(Token::Eof)) {
                expect(Token::Identifier);
                auto name = getTokenValue();
                expect(Token::Colon);
                values.push_back(std::pair(name, expression()));
                if (!match(Token::Comma)) {
                    expect(Token::BraceClose);
                    break;
                }
            }
            return ctx.make(ASTStructExpression(
                SourceRef(id, begin, position), identifier, values
            ));
        } else {
            return ctx.make(ASTIdentifierExpression(
                SourceRef(id, begin, position), identifier
            ));
        }
    } else {
        return logger.error(SourceRef(id, position), "Invalid L0", nullptr);
    }
}

ASTExpression* Parser::l1() {
    auto begin = trim();
    auto e = l0();
    while (true) {
        if (match(Token::ParenOpen)) {
            std::vector<ASTExpression*> args;
            while (!match(Token::ParenClose) && !match(Token::Eof)) {
                args.push_back(expression());
                match(Token::Comma);
            }
            e = ctx.make(
                ASTCallExpression(SourceRef(id, begin, position), e, args)
            );
        } else if (match(Token::BracketOpen)) {
            std::vector<ASTExpression*> args;
            while (!match(Token::BracketClose) && !match(Token::Eof)) {
                args.push_back(expression());
                match(Token::Comma);
            }
            e = ctx.make(
                ASTIndexExpression(SourceRef(id, begin, position), e, args)
            );
        } else if (match(Token::Dot)) {
            expect(Token::Identifier);
            e = ctx.make(ASTFieldExpression(
                SourceRef(id, begin, position), e, getTokenValue()
            ));
        } else {
            return e;
        }
    }
}

ASTExpression* Parser::l2() {
    auto begin = trim();
    if (match(Token::Plus)) {
        auto e = l2();
        return ctx.make(ASTUnaryExpression(
            SourceRef(id, begin, position), UnaryOperator::Positive, e
        ));
    } else if (match(Token::Minus)) {
        auto e = l2();
        return ctx.make(ASTUnaryExpression(
            SourceRef(id, begin, position), UnaryOperator::Negative, e
        ));
    } else if (match(Token::LogicalNot)) {
        auto e = l2();
        return ctx.make(ASTUnaryExpression(
            SourceRef(id, begin, position), UnaryOperator::LogicalNot, e
        ));
    } else if (match(Token::BitwiseNot)) {
        auto e = l2();
        return ctx.make(ASTUnaryExpression(
            SourceRef(id, begin, position), UnaryOperator::BitwiseNot, e
        ));
    } else {
        return l1();
    }
}

ASTExpression* Parser::l3() {
    auto begin = trim();
    auto e = l2();
    while (true) {
        if (match(Token::Multiply)) {
            auto r = l2();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::Multiply, e, r
            ));
        } else if (match(Token::Divide)) {
            auto r = l2();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::Divide, e, r
            ));
        } else if (match(Token::Modulo)) {
            auto r = l2();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::Modulo, e, r
            ));
        } else {
            return e;
        }
    }
}

ASTExpression* Parser::l4() {
    auto begin = trim();
    auto e = l3();
    while (true) {
        if (match(Token::Plus)) {
            auto r = l3();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::Add, e, r
            ));
        } else if (match(Token::Minus)) {
            auto r = l3();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::Subtract, e, r
            ));
        } else {
            return e;
        }
    }
}

ASTExpression* Parser::l5() {
    auto begin = trim();
    auto e = l4();
    while (true) {
        if (match(Token::ShiftLeft)) {
            auto r = l4();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::ShiftLeft, e, r
            ));
        } else if (match(Token::ShiftRight)) {
            auto r = l4();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::ShiftRight, e, r
            ));
        } else {
            return e;
        }
    }
}

ASTExpression* Parser::l6() {
    auto begin = trim();
    auto e = l5();
    while (true) {
        if (match(Token::BitwiseAnd)) {
            auto r = l5();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::BitwiseAnd, e, r
            ));
        } else if (match(Token::BitwiseOr)) {
            auto r = l5();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::BitwiseOr, e, r
            ));
        } else if (match(Token::BitwiseXor)) {
            auto r = l5();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::BitwiseXor, e, r
            ));
        } else {
            return e;
        }
    }
}

ASTExpression* Parser::l7() {
    auto begin = trim();
    auto e = l6();
    while (true) {
        if (match(Token::Equal)) {
            auto r = l6();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::Equal, e, r
            ));
        } else if (match(Token::NotEqual)) {
            auto r = l6();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::NotEqual, e, r
            ));
        } else if (match(Token::LessThan)) {
            auto r = l6();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::LessThan, e, r
            ));
        } else if (match(Token::GreaterThan)) {
            auto r = l6();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position),
                BinaryOperator::GreaterThan,
                e,
                r
            ));
        } else if (match(Token::LessOrEqual)) {
            auto r = l6();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position),
                BinaryOperator::LessOrEqual,
                e,
                r
            ));
        } else if (match(Token::GreaterOrEqual)) {
            auto r = l6();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position),
                BinaryOperator::GreaterOrEqual,
                e,
                r
            ));
        } else {
            return e;
        }
    }
}

ASTExpression* Parser::l8() {
    auto begin = trim();
    auto e = l7();
    while (true) {
        if (match(Token::LogicalAnd)) {
            auto r = l7();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::LogicalAnd, e, r
            ));
        } else {
            return e;
        }
    }
}

ASTExpression* Parser::l9() {
    auto begin = trim();
    auto e = l8();
    while (true) {
        if (match(Token::LogicalOr)) {
            auto r = l8();
            e = ctx.make(ASTBinaryExpression(
                SourceRef(id, begin, position), BinaryOperator::LogicalOr, e, r
            ));
        } else {
            return e;
        }
    }
}

ASTExpression* Parser::l10() {
    auto begin = trim();
    auto e = l9();
    if (match(Token::Assign)) {
        auto r = l10();
        return ctx.make(ASTBinaryExpression(
            SourceRef(id, begin, position), BinaryOperator::Assign, e, r
        ));
    } else {
        return e;
    }
}

ASTExpression* Parser::expression() {
    return l10();
}

ASTStatement* Parser::blockStatement(size_t begin) {
    std::vector<ASTStatement*> statements;
    while (!match(Token::BraceClose) && !match(Token::Eof)) {
        statements.push_back(statement());
    }
    return ctx.make(
        ASTBlockStatement(SourceRef(id, begin, position), statements)
    );
}

ASTStatement* Parser::variableStatement(size_t begin) {
    std::vector<std::pair<std::string, ASTExpression*>> items;
    while (match(Token::Identifier)) {
        auto name = getTokenValue();
        expect(Token::Assign);
        items.push_back(std::pair(name, expression()));
        match(Token::Comma);
    }
    return ctx.make(ASTVariableStatement(SourceRef(id, begin, position), items)
    );
}

ASTStatement* Parser::returnStatement(size_t begin) {
    if (match(Token::NewLine)) {
        return ctx.make(
            ASTReturnStatement(SourceRef(id, begin, position), nullptr)
        );
    } else {
        auto e = expression();
        return ctx.make(ASTReturnStatement(SourceRef(id, begin, position), e));
    }
}

ASTStatement* Parser::whileStatement(size_t begin) {
    expect(Token::ParenOpen);
    auto condition = expression();
    expect(Token::ParenClose);
    auto body = statement();
    return ctx.make(
        ASTWhileStatement(SourceRef(id, begin, position), condition, body)
    );
}

ASTStatement* Parser::ifStatement(size_t begin) {
    expect(Token::ParenOpen);
    auto condition = expression();
    expect(Token::ParenClose);
    auto ifBody = statement();
    if (match(Token::Else)) {
        auto elseBody = statement();
        return ctx.make(ASTIfStatement(
            SourceRef(id, begin, position), condition, ifBody, elseBody
        ));
    } else {
        return ctx.make(ASTIfStatement(
            SourceRef(id, begin, position), condition, ifBody, nullptr
        ));
    }
}

ASTStatement* Parser::statement() {
    auto begin = trim();
    if (match(Token::BraceOpen)) {
        return blockStatement(begin);
    } else if (match(Token::Let)) {
        return variableStatement(begin);
    } else if (match(Token::Return)) {
        return returnStatement(begin);
    } else if (match(Token::While)) {
        return whileStatement(begin);
    } else if (match(Token::If)) {
        return ifStatement(begin);
    } else {
        auto e = expression();
        return ctx.make(
            ASTExpressionStatement(SourceRef(id, begin, position), e)
        );
    }
}

ASTConstraintDeclaration* Parser::constraintDeclaration(size_t begin) {
    expect(Token::Identifier);
    auto constraintName = getTokenValue();
    auto typeParams = typeParamList();
    auto constraints = constraintList();

    expect(Token::BraceOpen);
    std::vector<ASTDeclaration*> declarations;
    while (!match(Token::BraceClose) && !match(Token::Eof)) {
        auto declBegin = trim();
        if (match(Token::Struct)) {
            declarations.push_back(structDeclaration(declBegin));
        } else if (match(Token::Function)) {
            declarations.push_back(functionDeclaration(declBegin));
        } else if (match(Token::Extern)) {
            declarations.push_back(externFunctionDeclaration(declBegin));
        } else {
            logger.error(
                SourceRef(id, position),
                "Expected eiter StructDeclaration, FunctionDeclaration or ExternFunctionDeclaration"
            );
        }
    }

    return ctx.make(ASTConstraintDeclaration(
        SourceRef(id, begin, position),
        constraintName,
        typeParams,
        constraints,
        declarations
    ));
}

ASTStructDeclaration* Parser::structDeclaration(size_t begin) {
    expect(Token::Identifier);
    auto structName = getTokenValue();

    auto typeParams = typeParamList();

    expect(Token::BraceOpen);
    std::vector<std::pair<std::string, ASTType*>> fields;
    while (!match(Token::BraceClose) && !match(Token::Eof)) {
        expect(Token::Identifier);
        auto name = getTokenValue();
        expect(Token::Colon);
        fields.push_back(std::pair(name, typeName()));
        if (!match(Token::Comma)) {
            expect(Token::BraceClose);
            break;
        }
    }

    auto constraints = constraintList();

    return ctx.make(ASTStructDeclaration(
        SourceRef(id, begin, position),
        structName,
        typeParams,
        constraints,
        fields
    ));
}

ASTFunctionDeclaration* Parser::functionDeclaration(size_t begin) {
    expect(Token::Identifier);
    auto name = getTokenValue();

    auto typeParams = typeParamList();

    std::vector<std::pair<std::string, ASTType*>> parameters;
    expect(Token::ParenOpen);
    while (!match(Token::ParenClose) && !match(Token::Eof)) {
        expect(Token::Identifier);
        auto paramName = getTokenValue();
        expect(Token::Colon);
        auto type = typeName();
        parameters.push_back({ paramName, type });
        match(Token::Comma);
    }

    ASTType* result =
        (!match(Token::Colon)
             ? ctx.make(ASTNamedType(SourceRef(id, begin, position), "void", {})
             )
             : typeName());

    auto constraints = constraintList();

    auto bodyBegin = trim();
    expect(Token::BraceOpen);
    auto body = blockStatement(bodyBegin);
    return ctx.make(ASTFunctionDeclaration(
        SourceRef(id, begin, position),
        name,
        typeParams,
        constraints,
        result,
        parameters,
        body
    ));
}

ASTExternFunctionDeclaration* Parser::externFunctionDeclaration(size_t begin) {
    expect(Token::ParenOpen);
    expect(Token::Identifier);
    auto lib = getTokenValue();
    expect(Token::ParenClose);

    expect(Token::Function);
    expect(Token::Identifier);
    auto name = getTokenValue();

    auto typeParams = typeParamList();

    std::vector<std::pair<std::string, ASTType*>> parameters;
    expect(Token::ParenOpen);
    while (!match(Token::ParenClose) && !match(Token::Eof)) {
        expect(Token::Identifier);
        auto paramName = getTokenValue();
        expect(Token::Colon);
        auto type = typeName();
        parameters.push_back({ paramName, type });
        match(Token::Comma);
    }

    ASTType* result =
        (!match(Token::Colon)
             ? ctx.make(ASTNamedType(SourceRef(id, begin, position), "void", {})
             )
             : typeName());

    auto constraints = constraintList();

    return ctx.make(ASTExternFunctionDeclaration(
        SourceRef(id, begin, position),
        lib,
        name,
        typeParams,
        constraints,
        result,
        parameters
    ));
}

ASTSourceFile* Parser::sourceFile() {
    auto begin = trim();

    std::string modulePath;
    expect(Token::Module);
    while (!match(Token::Eof)) {
        expect(Token::Identifier);
        modulePath +=
            (modulePath.empty() ? getTokenValue() : "." + getTokenValue());
        if (!match(Token::Dot))
            break;
        modulePath += ".";
    }

    std::vector<std::string> imports;
    while (match(Token::Import)) {
        std::string importPath;
        while (!match(Token::Eof)) {
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
    while (!match(Token::Eof)) {
        auto declBegin = trim();
        if (match(Token::Constraint)) {
            declarations.push_back(constraintDeclaration(declBegin));
        } else if (match(Token::Struct)) {
            declarations.push_back(structDeclaration(declBegin));
        } else if (match(Token::Function)) {
            declarations.push_back(functionDeclaration(declBegin));
        } else if (match(Token::Extern)) {
            declarations.push_back(externFunctionDeclaration(declBegin));
        } else {
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

ASTType* Parser::typeName() {
    auto begin = trim();
    expect(Token::Identifier);

    auto typeArgs = typeArgList();

    ASTType* type = ctx.make(
        ASTNamedType(SourceRef(id, begin, position), getTokenValue(), typeArgs)
    );
    while (true) {
        if (match(Token::Multiply)) {
            type =
                ctx.make(ASTPointerType(SourceRef(id, begin, position), type));
        } else if (match(Token::BracketOpen) && match(Token::BracketClose)) {
            type = ctx.make(ASTArrayType(SourceRef(id, begin, position), type));
        } else {
            return type;
        }
    }
}

//

std::vector<std::string> Parser::typeParamList() {
    std::vector<std::string> params;
    if (match(Token::LessThan)) {
        while (!match(Token::GreaterThan) && !match(Token::Eof)) {
            expect(Token::Identifier);
            params.push_back(getTokenValue());
            if (!match(Token::Comma)) {
                expect(Token::GreaterThan);
                break;
            }
        }
    }
    return params;
}

std::vector<ASTType*> Parser::typeArgList() {
    std::vector<ASTType*> args;
    if (match(Token::LessThan)) {
        while (!match(Token::GreaterThan) && !match(Token::Eof)) {
            args.push_back(typeName());
            if (!match(Token::Comma)) {
                expect(Token::GreaterThan);
                break;
            }
        }
    }
    return args;
}

std::vector<std::pair<std::string, std::vector<ASTType*>>>
Parser::constraintList() {
    std::vector<std::pair<std::string, std::vector<ASTType*>>> constraints;
    if (match(Token::BracketOpen)) {
        while (!match(Token::BracketClose) && !match(Token::Eof)) {
            expect(Token::Identifier);
            constraints.push_back(std::pair(getTokenValue(), typeArgList()));
            if (!match(Token::Comma)) {
                expect(Token::BracketClose);
                break;
            }
        }
    }
    return constraints;
}