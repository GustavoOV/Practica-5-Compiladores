#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

/*
 * Driver SLR(1)
 *
 * Formato de entrada: una sola línea con nombres de tokens separados por espacios
 * Ejemplo: type:int id:x = num:10 ; print ( id:x ) ;
 *
 * Salida: Se indica si la cadena es valida y mostrará el AST
 *
 */

namespace
{

    struct Token
    {
        std::string kind;
        std::string lexeme;
    };

    struct ParseNode
    {
        std::string symbol;
        std::string lexeme;
        int rule = 0; // cero para hojas terminales
        std::vector<std::unique_ptr<ParseNode>> children;
    };

    enum class ActionKind
    {
        Shift,
        Reduce,
        Accept
    };

    struct Action
    {
        ActionKind kind;
        int value = 0; // estado destino para shift
    };

    struct Production
    {
        std::string lhs;
        std::vector<std::string> rhs;
    };

    // ACTION y GOTO guardan solo las celdas definidas de las tablas SLR
    using ActionTable = std::map<int, std::map<std::string, Action>>;
    using GotoTable = std::map<int, std::map<std::string, int>>;

    const std::vector<std::string> kTerminals = {
        "id", "type", "num", "while", "print", "(", ")", "{", "}",
        ";", "=", "==", "<", "+", "-", "*", "/", "$"};

    // Las posiciones 1-22 corresponden a las reducciones r1-r22
    const std::vector<Production> kProductions = {
        {"", {}},
        {"Program", {"Bloque"}},
        {"Bloque", {"Bloque", "Accion"}},
        {"Bloque", {"Accion"}},
        {"Accion", {"Decl"}},
        {"Accion", {"Asign"}},
        {"Accion", {"Loop"}},
        {"Accion", {"Print"}},
        {"Decl", {"type", "id", ";"}},
        {"Asign", {"id", "=", "Expr", ";"}},
        {"Loop", {"while", "(", "Cond", ")", "{", "Bloque", "}"}},
        {"Print", {"print", "(", "Expr", ")", ";"}},
        {"Cond", {"Expr", "==", "Expr"}},
        {"Cond", {"Expr", "<", "Expr"}},
        {"Expr", {"Expr", "+", "Term"}},
        {"Expr", {"Expr", "-", "Term"}},
        {"Expr", {"Term"}},
        {"Term", {"Term", "*", "Val"}},
        {"Term", {"Term", "/", "Val"}},
        {"Term", {"Val"}},
        {"Val", {"(", "Expr", ")"}},
        {"Val", {"id"}},
        {"Val", {"num"}}};

    void addShift(ActionTable &table, int state, const std::string &terminal,
                  int destination)
    {
        table[state][terminal] = {ActionKind::Shift, destination};
    }

    void addReduce(ActionTable &table, int state, int rule,
                   const std::vector<std::string> &lookaheads)
    {
        for (const auto &terminal : lookaheads)
        {
            table[state][terminal] = {ActionKind::Reduce, rule};
        }
    }

    ActionTable buildActionTable()
    {
        ActionTable action;
        auto s = [&](int state, const char *terminal, int destination)
        {
            addShift(action, state, terminal, destination);
        };
        auto r = [&](int state, int rule,
                     const std::vector<std::string> &lookaheads)
        {
            addReduce(action, state, rule, lookaheads);
        };
        auto blockFollow = [&](int state, int rule)
        {
            r(state, rule, {"id", "type", "while", "print", "}", "$"});
        };
        const std::vector<std::string> exprFollow = {
            ")", ";", "=", "==", "<", "+", "-"};
        const std::vector<std::string> termFollow = {
            ")", ";", "=", "==", "<", "+", "-", "*", "/"};

        // Carga las acciones de la tabla (desplazamientos, reducciones y accept)
        s(0, "id", 9);
        s(0, "type", 8);
        s(0, "while", 10);
        s(0, "print", 11);
        action[1]["$"] = {ActionKind::Accept, 0};

        s(2, "id", 9);
        s(2, "type", 8);
        s(2, "while", 10);
        s(2, "print", 11);
        r(2, 1, {"$"});
        blockFollow(3, 3);
        blockFollow(4, 4);
        blockFollow(5, 5);
        blockFollow(6, 6);
        blockFollow(7, 7);
        s(8, "id", 13);
        s(9, "=", 15);
        s(10, "(", 27);
        s(11, "(", 34);
        blockFollow(12, 2);
        s(13, ";", 14);
        blockFollow(14, 8);
        s(15, "id", 20);
        s(15, "num", 21);
        s(15, "(", 19);
        s(16, ";", 22);
        s(16, "+", 23);
        s(16, "-", 24);
        r(17, 16, exprFollow);
        s(17, "*", 25);
        s(17, "/", 26);
        r(18, 19, termFollow);
        s(19, "id", 20);
        s(19, "num", 21);
        s(19, "(", 19);
        r(20, 21, termFollow);
        r(21, 22, termFollow);
        blockFollow(22, 9);
        s(23, "id", 20);
        s(23, "num", 21);
        s(23, "(", 19);
        s(24, "id", 20);
        s(24, "num", 21);
        s(24, "(", 19);
        s(25, "id", 20);
        s(25, "num", 21);
        s(25, "(", 19);
        s(26, "id", 20);
        s(26, "num", 21);
        s(26, "(", 19);
        s(27, "id", 20);
        s(27, "num", 21);
        s(27, "(", 19);
        s(28, ")", 30);
        s(29, "==", 44);
        s(29, "<", 45);
        s(29, "+", 23);
        s(29, "-", 24);
        s(30, "{", 31);
        s(31, "id", 9);
        s(31, "type", 8);
        s(31, "while", 10);
        s(31, "print", 11);
        s(32, "id", 9);
        s(32, "type", 8);
        s(32, "while", 10);
        s(32, "print", 11);
        s(32, "}", 33);
        blockFollow(33, 10);
        s(34, "id", 20);
        s(34, "num", 21);
        s(34, "(", 19);
        s(35, ")", 36);
        s(35, "+", 23);
        s(35, "-", 24);
        s(36, ";", 37);
        blockFollow(37, 11);
        r(38, 14, exprFollow);
        s(38, "*", 25);
        s(38, "/", 26);
        r(39, 15, exprFollow);
        s(39, "*", 25);
        s(39, "/", 26);
        r(40, 17, termFollow);
        r(41, 18, termFollow);
        s(42, ")", 43);
        s(42, "+", 23);
        s(42, "-", 24);
        r(43, 20, termFollow);
        s(44, "id", 20);
        s(44, "num", 21);
        s(44, "(", 19);
        s(45, "id", 20);
        s(45, "num", 21);
        s(45, "(", 19);
        s(46, "+", 23);
        s(46, "-", 24);
        r(46, 12, {")"});
        s(47, "+", 23);
        s(47, "-", 24);
        r(47, 13, {")"});

        return action;
    }

    GotoTable buildGotoTable()
    {
        // Indica el estado destino después de reducir un no terminal
        return {
            {0, {{"Program", 1}, {"Bloque", 2}, {"Accion", 3}, {"Decl", 4}, {"Asign", 5}, {"Loop", 6}, {"Print", 7}}},
            {2, {{"Accion", 12}, {"Decl", 4}, {"Asign", 5}, {"Loop", 6}, {"Print", 7}}},
            {15, {{"Expr", 16}, {"Term", 17}, {"Val", 18}}},
            {19, {{"Expr", 42}, {"Term", 17}, {"Val", 18}}},
            {23, {{"Term", 38}, {"Val", 18}}},
            {24, {{"Term", 39}, {"Val", 18}}},
            {25, {{"Val", 40}}},
            {26, {{"Val", 41}}},
            {27, {{"Cond", 28}, {"Expr", 29}, {"Term", 17}, {"Val", 18}}},
            {31, {{"Bloque", 32}, {"Accion", 3}, {"Decl", 4}, {"Asign", 5}, {"Loop", 6}, {"Print", 7}}},
            {32, {{"Accion", 12}, {"Decl", 4}, {"Asign", 5}, {"Loop", 6}, {"Print", 7}}},
            {34, {{"Expr", 35}, {"Term", 17}, {"Val", 18}}},
            {44, {{"Expr", 46}, {"Term", 17}, {"Val", 18}}},
            {45, {{"Expr", 47}, {"Term", 17}, {"Val", 18}}}};
    }

    bool isKnownTerminal(const std::string &symbol)
    {
        return std::find(kTerminals.begin(), kTerminals.end(), symbol) !=
               kTerminals.end();
    }

    std::vector<Token> tokenizeInput(const std::string &line)
    {
        // Separa la entrada en tokens y conserva los lexemas proporcionados
        std::vector<Token> tokens;
        std::istringstream input(line);
        std::string part;
        while (input >> part)
        {
            const auto colon = part.find(':');
            Token token;
            if (colon == std::string::npos)
            {
                token = {part, part};
            }
            else
            {
                token = {part.substr(0, colon), part.substr(colon + 1)};
                if (token.lexeme.empty())
                    token.lexeme = token.kind;
            }
            tokens.push_back(std::move(token));
        }
        return tokens;
    }

    std::string productionText(int rule)
    {
        const auto &production = kProductions.at(static_cast<std::size_t>(rule));
        std::ostringstream out;
        out << production.lhs << " ->";
        for (const auto &symbol : production.rhs)
            out << ' ' << symbol;
        return out.str();
    }

    void printTree(const ParseNode &node, int depth)
    {
        // La indentacion representa la jerarquia de las reducciones
        std::cout << std::string(static_cast<std::size_t>(depth) * 2, ' ')
                  << node.symbol;
        if (node.rule != 0)
        {
            std::cout << " [r" << node.rule << ": " << productionText(node.rule)
                      << ']';
        }
        else if (!node.lexeme.empty() && node.lexeme != node.symbol)
        {
            std::cout << " (" << node.lexeme << ')';
        }
        std::cout << '\n';
        for (const auto &child : node.children)
            printTree(*child, depth + 1);
    }

    std::vector<std::string> expectedAt(const ActionTable &action, int state)
    {
        std::vector<std::string> expected;
        const auto row = action.find(state);
        if (row == action.end())
            return expected;
        for (const auto &terminal : kTerminals)
        {
            if (row->second.find(terminal) != row->second.end())
            {
                expected.push_back(terminal);
            }
        }
        return expected;
    }

    void printSyntaxError(const ActionTable &action, int state,
                          const Token &token, std::size_t tokenIndex)
    {
        std::cout << "Cadena invalida\n"
                  << "Error en el token " << (tokenIndex + 1) << " ("
                  << token.kind;
        if (token.lexeme != token.kind)
            std::cout << ":" << token.lexeme;
        std::cout << "), estado " << state << ".\n";
        const auto expected = expectedAt(action, state);
        std::cout << "Se esperaba: ";
        if (expected.empty())
        {
            std::cout << "ningun token definido en ACTION";
        }
        else
        {
            for (std::size_t i = 0; i < expected.size(); ++i)
            {
                if (i != 0)
                    std::cout << ", ";
                std::cout << expected[i];
            }
        }
        std::cout << '\n';
    }

    bool parse(const std::vector<Token> &tokens)
    {
        const ActionTable action = buildActionTable();
        const GotoTable goTo = buildGotoTable();
        // Las tres pilas mantienen estados, simbolos reconocidos y nodos del arbol
        std::vector<int> stateStack{0};
        std::vector<std::string> symbolStack;
        std::vector<std::unique_ptr<ParseNode>> nodeStack;
        std::size_t inputIndex = 0;

        // Consulta ACTION con el estado actual y el siguiente token de entrada
        while (inputIndex < tokens.size())
        {
            const int state = stateStack.back();
            const Token &lookahead = tokens[inputIndex];
            const auto stateRow = action.find(state);
            if (stateRow == action.end() ||
                stateRow->second.find(lookahead.kind) == stateRow->second.end())
            {
                printSyntaxError(action, state, lookahead, inputIndex);
                return false;
            }

            const Action current = stateRow->second.at(lookahead.kind);
            if (current.kind == ActionKind::Shift)
            {
                // Shift consume el token y apila su simbolo, nodo y estado destino
                auto leaf = std::make_unique<ParseNode>();
                leaf->symbol = lookahead.kind;
                leaf->lexeme = lookahead.lexeme;
                symbolStack.push_back(lookahead.kind);
                nodeStack.push_back(std::move(leaf));
                stateStack.push_back(current.value);
                ++inputIndex;
                continue;
            }

            if (current.kind == ActionKind::Reduce)
            {
                // Reduce reemplaza el lado derecho por su no terminal y crea un nodo
                const Production &production =
                    kProductions.at(static_cast<std::size_t>(current.value));
                const std::size_t rhsSize = production.rhs.size();
                if (rhsSize > symbolStack.size() || rhsSize >= stateStack.size() ||
                    rhsSize > nodeStack.size())
                {
                    std::cerr << "Error interno: pila inconsistente al reducir r"
                              << current.value << ".\n";
                    return false;
                }

                auto parent = std::make_unique<ParseNode>();
                parent->symbol = production.lhs;
                parent->rule = current.value;
                for (std::size_t i = 0; i < rhsSize; ++i)
                {
                    symbolStack.pop_back();
                    stateStack.pop_back();
                    parent->children.push_back(std::move(nodeStack.back()));
                    nodeStack.pop_back();
                }
                std::reverse(parent->children.begin(), parent->children.end());

                const int previousState = stateStack.back();
                const auto gotoRow = goTo.find(previousState);
                if (gotoRow == goTo.end() ||
                    gotoRow->second.find(production.lhs) == gotoRow->second.end())
                {
                    std::cerr << "Error interno: no existe GOTO[" << previousState
                              << "][" << production.lhs << "] después de r"
                              << current.value << ".\n";
                    return false;
                }

                symbolStack.push_back(production.lhs);
                nodeStack.push_back(std::move(parent));
                stateStack.push_back(gotoRow->second.at(production.lhs));
                continue;
            }

            // Accept confirma la cadena y muestra el arbol construido durante reduce
            std::cout << "Cadena valida.\n";
            std::cout << "Arbol sintactico generado por las reducciones:\n";
            if (!nodeStack.empty())
                printTree(*nodeStack.back(), 0);
            return true;
        }

        std::cout << "Cadena invalida: termino la entrada antes de llegar a $/Accept\n";
        return false;
    }

}

int main()
{
    std::cout << "Tokens Mini C separados por espacios (ejemplo: type:int id:x = num:1 ;):\n> ";
    std::string line;
    if (!std::getline(std::cin, line))
    {
        std::cerr << "No se recibio una linea de entrada\n";
        return 1;
    }

    // Prepara los tokens, valida su formato y agrega $ cuando hace falta
    std::vector<Token> tokens = tokenizeInput(line);
    if (tokens.empty())
    {
        std::cout << "Cadena invalida: la lista de tokens esta vacia\n";
        return 1;
    }

    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
        if (!isKnownTerminal(tokens[i].kind))
        {
            std::cout << "Token desconocido: " << tokens[i].kind << "\n";
            return 1;
        }
        if (tokens[i].kind == "$" && i + 1 != tokens.size())
        {
            std::cout << "El marcador $ solo puede aparecer al final\n";
            return 1;
        }
    }
    if (tokens.back().kind != "$")
        tokens.push_back({"$", "$"});

    return parse(tokens) ? 0 : 1;
}
