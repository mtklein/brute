#include <iostream>
    using std::cin;
    using std::cout;
    using std::endl;
#include <map>
    using std::map;
#include <string>
    using std::string;
#include <vector>
    using std::vector;

#include <cstring>
#include <cstdlib>

static void split(const string& line, const char* delims, vector<string>* splits) {
    for (const char* s = line.c_str(); s != line.c_str() + line.size(); ) {
        const size_t span = strcspn(s, delims);
        splits->push_back(string(s, span));
        s += span;
        s += strspn(s, " ");
    }
}

static bool parse_literal(const string& token, double* value) {
    char* end;
    *value = strtod(token.c_str(), &end);
    return end == token.c_str() + token.size();
}

struct Forth {
    typedef void(*Word)(Forth*);

    void push(double d) {
        stack_.push_back(d);
    }

    double pop() {
        double v = 0;
        if (!stack_.empty()) {
            v = stack_.back();
            stack_.pop_back();
        }
        return v;
    }

    void add(const string& name, Word word) {
        dict_[name] = word;
    }

    void eval(const string& token) {
        double literal;
        if (parse_literal(token, &literal)) return this->push(literal);

        map<string, Word>::const_iterator it = dict_.find(token);
        if (it != dict_.end()) {
            (it->second)(this);
        }
    }

private:
    vector<double> stack_;
    map<string, Word> dict_;
};

static void add(Forth* f) {
    double r = f->pop();
    double l = f->pop();
    f->push(l + r);
}
static void subtract(Forth* f) {
    double r = f->pop();
    double l = f->pop();
    f->push(l - r);
}
static void multiply(Forth* f) {
    double r = f->pop();
    double l = f->pop();
    f->push(l * r);
}
static void divide(Forth* f) {
    double r = f->pop();
    double l = f->pop();
    f->push(l / r);
}

static void pop(Forth* f) {
    cout << f->pop() << endl;
}

int main(int /*argc*/, char** /*argv*/) {
    Forth forth;
    forth.add("+", add);
    forth.add("-", subtract);
    forth.add("*", multiply);
    forth.add("/", divide);
    forth.add(".", pop);

    string line;
    vector<string> tokens;

    do {
        cout << "bƒ ";
        getline(cin, line);
        tokens.clear();
        split(line, " ", &tokens);
        for (size_t i = 0; i < tokens.size(); i++) {
            forth.eval(tokens[i]);
        }
    } while(!line.empty());
}
