#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cstdlib>
#include <stack>
#include <unordered_map>

std::vector<std::string> splitString(const std::string& input, char delim);

#define DEBUG 0

#define FILE "level3_2.in"

#define INPUT_DIR "inputs/inputs-3"
#define PATH INPUT_DIR "/" FILE

bool skip_else = false;

typedef std::vector<std::string>::iterator vector_it_type;
typedef void (*TokenExecutorType)();

std::vector<std::string> tokens;
std::vector<std::string>::iterator token_it;
int line = 0;

void printTokens() {
    if (DEBUG) {
        for (auto it = tokens.begin(); it != tokens.end(); ++it) {
            std::cout << *it << std::endl;
        }
    }
}

class Function
{
    public:
        typedef std::pair<std::string, bool> pair_t;
        typedef std::unordered_map<std::string, std::string> map_t;
 
        enum State {
            FUNCTION,
            IF,
            ELSE
        };

    public:
        Function()
        : num_end(1) {
            state.push(FUNCTION);
        }
        ~Function() {}

        pair_t getVariable(const std::string& s) {
            auto it = variables.find(s);
            if (it == variables.end()) {
                return pair_t("", false);
            }
            return pair_t(it->second, true);
        }

        bool createVariable(const std::string& name, const std::string& value) {
            auto it = variables.find(name);
            if (it != variables.end()) {
                std::cout << "ERROR" << std::endl;
                clearContent();
                return false;
            }

            variables[name] = value;
            return true;
        }

        bool setVariable(const std::string& name, const std::string& value) {
            auto it = variables.find(name);
            if (it == variables.end()) {
                std::cout << "ERROR" << std::endl;
                clearContent();
                return false;
            }

            variables[name] = value;
            return true;
        }

        void addIf(bool value) {
            skip_elses.push(value);
        }

        bool getSkipElse() {
            bool val = skip_elses.top();
            skip_elses.pop();
            return val;
        }

        void incEnd() {
            ++num_end;
        }

        void decEnd() {
            --num_end;
        }

        /*
        Skip to the final end of a function
        */
        void skipFunction() {
            while (token_it != tokens.end() && *token_it != "start") {
                ++token_it;
            }
            --token_it;
            num_end = 1;
            assert(*token_it == "end");
        }

        void clearContent() {
            output.clear();
        }

        void addOutput(const std::string& s) {
            output.append(s);
        }

        int getNumEnd() const {
            return num_end;
        }

        const std::string& getOutput() const {
            return output;
        }

        State getState() const {
            return state.top();
        }

        void popState() {
            state.pop();
        }

        void addState(State s) {
            state.push(s);
        }

        std::string getStateName() {
            switch (getState()) {
                case FUNCTION:
                    return "FUNCTION";
                case IF:
                    return "IF";
                case ELSE:
                    return "ELSE";
            }
            return "";
        }

    private:
        int num_end;
        std::stack<State> state;
        map_t variables;
        std::string output;
        std::stack<bool> skip_elses;
};

std::stack<Function*> call_stack;
Function* current_function = NULL;

std::string getValue(const std::string& key) {
    auto p = current_function->getVariable(key);
    if (p.second) {
        return p.first;
    }

    return key;
}

void execStart()
{
    if (DEBUG) {
        std::cout << "  START: Creating Function" << std::endl;
    }
    call_stack.push(new Function());
    current_function = call_stack.top();
    ++token_it;
}

void execEnd()
{
    if (DEBUG) {
        std::cout << "  END: State: " << current_function->getStateName() << std::endl;
    }
    ++token_it;
    if (current_function->getNumEnd() == 1) {
        if (DEBUG) {
            std::cout << "  END: Deleting Function" << std::endl;
        }
        call_stack.pop();
        if (current_function->getOutput().size() != 0) {
            std::cout << current_function->getOutput() << std::endl;
        }
        delete current_function;
        if (call_stack.size() != 0) {
            current_function = call_stack.top();
        } else {
            if (token_it != tokens.end() && *token_it != "start") {
                std::cerr << "Deleted function prematurely: " << *token_it << std::endl;
                assert(false);
            }
            current_function = NULL;
        }
        return;
    }

    // If or Else ending
    if (current_function->getState() == Function::IF) {
        skip_else = current_function->getSkipElse();
    }

    current_function->popState();

    if (DEBUG) {
        std::cout << "  END: NEXT: " << *token_it << std::endl;
    }

    current_function->decEnd();
}

void execPrint()
{
    ++token_it;
    if (DEBUG) {
        std::cout << "  PARAM: " << getValue(*token_it) << std::endl;
    }
    if (token_it == tokens.end()) {
        std::cerr << "No parameter after print" << std::endl;
        exit(EXIT_FAILURE);
    }
    current_function->addOutput(getValue(*token_it));
    ++token_it;
}

void execReturn()
{
    current_function->skipFunction();
}

void execIf()
{
    current_function->incEnd();
    ++token_it;
    current_function->addState(Function::IF);
    std::string b = getValue(*token_it);
    if (b != "true" && b != "false") {
        if (DEBUG) {
            std::cout << "B equals: " << b << std::endl;
        }
        std::cout << "ERROR" << std::endl;
        current_function->skipFunction();
        return;
    }
    if (b == "true")
    {
        if (DEBUG) {
            std::cout << "  IF: TRUE" << std::endl;
        }
        ++token_it;
        current_function->addIf(true);
    }
    else
    {
        if (DEBUG) {
            std::cout << "  IF: FALSE" << std::endl;
        }
        int num_end = 1;
        while (token_it != tokens.end() && num_end > 0)
        {
            if (*token_it == "if" || *token_it == "start" || *token_it == "else") {
                ++num_end;
            }
            if (*token_it == "end") {
                num_end--;
            }
            ++token_it;
        }
        current_function->addIf(false);
    }
    if (DEBUG) {
        std::cout << "  IF NEXT: " << *token_it << std::endl;
    }
}

void execElse()
{
    current_function->addState(Function::ELSE);
    if (skip_else) {
        while (*token_it != "end" && token_it != tokens.end())
        {
            ++token_it;
        }
    } else {
        ++token_it;
    }
    current_function->incEnd();
}

void execVar()
{
    ++token_it;
    std::string key = *token_it;
    ++token_it;
    std::string value = getValue(*token_it);
    ++token_it;
    if (current_function->createVariable(key, value) == false) {
        current_function->skipFunction();
        current_function->clearContent();
    }
}

void execSet()
{
    ++token_it;
    std::string key = *token_it;
    ++token_it;
    std::string value = getValue(*token_it);
    ++token_it;
    if (current_function->setVariable(key, value) == false) {
        current_function->skipFunction();
        current_function->clearContent();
    }
}

void execToken()
{
    std::map<std::string, TokenExecutorType> token_map = {
        {"start", execStart},
        {"end", execEnd},
        {"print", execPrint},
        {"return", execReturn},
        {"if", execIf},
        {"else", execElse},
        {"var", execVar},
        {"set", execSet}
    };


    auto it = token_map.find(*token_it);
    if (it == token_map.end()) {
        std::cerr << std::endl << "Token not recognized: [" << *token_it << "]" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (DEBUG) {
        std::cout << it->first << std::endl;
    }
    it->second();
}

void removeChars(std::string& str, char x)
{
    str.erase(std::remove(str.begin(), str.end(), x), str.end());
}

int main() {

    std::ifstream ifs(PATH);

    if (!ifs.is_open()) {
        std::cerr << PATH << " cannot be opened" << std::endl;
    }

    int n;
    ifs >> n;

    // std::cout << "Lines: " << n << std::endl;
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string content;
    for (int i = 0; i < n; ++i) {
        std::string line;
        std::getline(ifs, line);
        content.append(line);
        content.push_back(' ');
    }

    removeChars(content, '\r');

    tokens = splitString(content, ' ');

    token_it = tokens.begin();
    while (token_it != tokens.end())
    {
        execToken();
    }

    // std::cout << content << std::endl;
    ifs.close();
    return 0;
}
