//! clang++ --std=c++11 -O3 brute_main.cc -o brute_main \
//!   -Weverything -Werror -Wno-c++98-compat -Wno-padded

#include <iostream>
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

static bool parse_literal(const string& token, double* value) {
    char* end;
    *value = strtod(token.c_str(), &end);
    return end == token.c_str() + token.size();
}

class Forth {
public:
    Forth() : recording_(false) {}

    vector<double> stack() const { return stack_; }

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

    bool recording() const { return recording_; }

    void startRecording() {
        recording_ = true;

        name_.clear();
        body_.clear();
    }

    void stopRecording() {
        recording_ = false;

        string body(body_);
        normal_[name_] = [body](Forth* f) { f->eval(body); };
    }

    void eval(const string& line) {
        const char* s = line.c_str();
        const char* end = s + line.size();
        s += strspn(s, " ");
        while (s != end) {
            size_t span = strcspn(s, " ");
            string token(s, span);
            s += span;
            s += strspn(s, " ");
            this->evalToken(token);
        }
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
            body_ += " ";
            body_ += token;
        }
    }

    void evalToken(const string& token) {
        if (Word w = lookup(token, immediate_)) return w(this);
        if (recording_) return this->record(token);
        double literal;
        if (parse_literal(token, &literal)) this->push(literal);
        if (Word w = lookup(token, normal_)) return w(this);
    }

    vector<double> stack_;

    unordered_map<string, Word> immediate_, normal_;

    bool recording_;
    string name_, body_;
};

int main(int /*argc*/, char** /*argv*/) {
    Forth forth;
    forth.add("+", [](Forth* f){ double r = f->pop(), l = f->pop(); f->push(l+r); });
    forth.add("-", [](Forth* f){ double r = f->pop(), l = f->pop(); f->push(l-r); });
    forth.add("*", [](Forth* f){ double r = f->pop(), l = f->pop(); f->push(l*r); });
    forth.add("/", [](Forth* f){ double r = f->pop(), l = f->pop(); f->push(l/r); });

    forth.add(":", [](Forth* f){ f->startRecording(); });
    forth.addImmediate(";", [](Forth* f){ f->stopRecording(); });

    forth.add("drop", [](Forth* f){ f->pop(); });
    forth.add("dup",  [](Forth* f){ double v = f->pop(); f->push(v); f->push(v); });
    forth.add("swap", [](Forth* f){
        double a = f->pop(), b = f->pop();
        f->push(a); f->push(b);
    });
    forth.add("rot", [](Forth* f){
        double a = f->pop(), b = f->pop(), c = f->pop();
        f->push(b); f->push(a); f->push(c);
    });

    forth.eval(": over swap dup rot swap ;");

    do {
        for (double v : forth.stack()) cout << v << " ";
        cout << endl;
        if(forth.recording()) cout << "*";
        cout << "bƒ ";

        string line;
        getline(cin, line);
        forth.eval(line);
    } while(!line.empty());
}
