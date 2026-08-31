#include "args.hpp"
#include "ast.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    Arguments arguments;

    try {
        arguments = parse_arguments(argc, argv);
    } catch (const std::invalid_argument&) {
        return 2;
    } catch (const std::exception&) {
        return 1;
    }

    for (const std::string& filepath : arguments.filepaths) {
        std::ifstream file(filepath);

        if (!file) {
            std::cerr << "Could not open " << filepath << '\n';
            return 1;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        const std::string source = buffer.str();

        Lexer lexer(source, filepath);
        const std::vector<Token> tokens = lexer.tokenize();

        if (csam_debug) {
            for (const Token& token : tokens) {
                std::cout
                    << token.filepath << ":"
                    << token.line << ":"
                    << token.column << " "
                    << token_type_name(token.type);

                if (!token.value.empty()) {
                    std::cout << " \"" << token.value << "\"";
                }

                std::cout << '\n';
            }
        }

        Parser parser(tokens);
        std::unique_ptr<RootNode> ast = parser.parse();

        if (csam_debug) {
            print_ast(*ast);
        }
    }

    return 0;
}
