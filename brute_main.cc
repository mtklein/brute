#include <iostream>
    using std::cin;
    using std::cout;
    using std::endl;
#include <string>
    using std::string;
#include <vector>
    using std::vector;

#include <cstring>
#include <cstdlib>

#define LOG(expr) cout << #expr << ": " << "'" << expr << "'" << endl;

static void split(const string& line, const char* delims, vector<string>* splits) {
    for (const char* s = line.c_str(); s != line.c_str() + line.size(); ) {
        const size_t span = strcspn(s, delims);
        splits->push_back(string(s, span));
        s += span;
        s += strspn(s, " ");
    }
}

class Forth {
public:
    Forth() {}

    void update(const string& line) {
        vector<string> tokens;
        split(line, " ", &tokens);

        for (size_t i = 0; i < tokens.size(); i++) {
            eval(tokens[i]);
        }
    }

    size_t stack_size() const { return stack_.size(); }
    double stack(size_t i) const { return stack_[i]; }

private:
    void eval(const string& token) {
        char* end;
        double literal = strtod(token.c_str(), &end);

        if (end != token.c_str()) {
            stack_.push_back(literal);
        } else {
            LOG("word");
        }
    }

    vector<double> stack_;
};


int main(int /*argc*/, char** /*argv*/) {
    Forth forth;
    string line;
    while (true) {
        cout << "bƒ ";
        getline(cin, line);
        forth.update(line);
        for (size_t i = 0; i < forth.stack_size(); i++) {
            cout << forth.stack(i) << " ";
        }
        cout << endl;
    }
}
