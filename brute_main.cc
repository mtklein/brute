#include <iostream>
    using std::cerr;
    using std::cin;
    using std::cout;
    using std::endl;
#include <functional>
    using std::function;
#include <unordered_map>
    using std::unordered_map;
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

class Forth {
public:
    Forth() : recording_(false) {}

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

    typedef function<void(Forth*)> Word;

    void add(const string& name, Word word) {
        normal_[name] = word;
    }

    void addImmediate(const string& name, Word word) {
        immediate_[name] = word;
    }

    void startRecording() {
        recording_ = true;

        name_.clear();
        body_.clear();
    }

    void stopRecording() {
        recording_ = false;

        vector<string> body;
        body.swap(body_);
        normal_[name_] = [body](Forth* f) {
            for (const auto& token : body) f->eval(token);
        };
    }

    void eval(const string& token) {
        if (Word w = lookup(token, immediate_)) return w(this);

        if (recording_) return this->record(token);

        double literal;
        if (parse_literal(token, &literal)) return this->push(literal);

        if (Word w = lookup(token, normal_)) return w(this);
    }

private:
    Word lookup(const string& name, const unordered_map<string, Word>& dict) const {
        auto it = dict.find(name);
        if (it == dict.end()) return NULL;
        return it->second;
    }

    void record(const string& token) {
        if (name_ == "") {
            name_ = token;
        } else {
            body_.push_back(token);
        }
    }

    vector<double> stack_;

    unordered_map<string, Word> immediate_, normal_;

    bool recording_;
    string name_;
    vector<string> body_;
};

int main(int /*argc*/, char** /*argv*/) {
    Forth forth;
    forth.add("+", [](Forth* f){ double r = f->pop(), l = f->pop(); f->push(l+r); });
    forth.add("-", [](Forth* f){ double r = f->pop(), l = f->pop(); f->push(l-r); });
    forth.add("*", [](Forth* f){ double r = f->pop(), l = f->pop(); f->push(l*r); });
    forth.add("/", [](Forth* f){ double r = f->pop(), l = f->pop(); f->push(l/r); });
    forth.add(".", [](Forth* f){ cout << f->pop() << endl; });
    forth.add(":", [](Forth* f){ f->startRecording(); });
    forth.addImmediate(";", [](Forth* f){ f->stopRecording(); });

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
