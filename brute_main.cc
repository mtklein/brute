//! clang++ --std=c++11 -O3 brute_main.cc -o brute_main \
//!   -Weverything -Werror -Wno-c++98-compat -Wno-padded

#include <deque>
    using std::deque;
#include <functional>
    using std::function;
#include <iostream>
    using std::cerr;
    using std::cin;
    using std::cout;
    using std::endl;
#include <unordered_map>
    using std::unordered_map;
#include <string>
    using std::string;
#include <vector>
    using std::vector;

#include <cassert>
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

    void clear() {
        stack_.clear();
    }

    typedef function<void()> Word;

    void add(const string& name, Word word) {
        normal_[name] = word;
    }

    void addImmediate(const string& name, Word word) {
        immediate_[name] = word;
    }

    vector<string> words() const {
        vector<string> words;
        for (const auto& entry : normal_) {
            words.push_back(entry.first);
        }
        return words;
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
        normal_[name_] = [this, body]() { this->eval(body); };
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
        if (Word w = lookup(token, immediate_)) return w();
        if (recording_) return this->record(token);
        double literal;
        if (parse_literal(token, &literal)) this->push(literal);
        if (Word w = lookup(token, normal_)) return w();
    }

    vector<double> stack_;
    unordered_map<string, Word> immediate_, normal_;
    bool recording_;
    string name_, body_;
};

static void test(Forth* f, const string& line, vector<double> expectedStack) {
    f->eval(line);
    if (expectedStack != f->stack()) {
        cerr << "Expected";
        for (double v : expectedStack) cerr << " " << v;
        cerr << ", actual";
        for (double v : f->stack()) cerr << " " << v;
        cerr << endl;
        assert(false);
    }
}

static void brute(Forth* f, const string& name, const string& in, const string& out) {
    const vector<string> words = f->words();

    deque<string> candidates(words.begin(), words.end());

    f->clear();
    f->eval(out);
    vector<double> expected = f->stack();

    while (!candidates.empty()) {
        string candidate = candidates.front();

        f->clear();
        f->eval(in);
        f->eval(candidate);
        if (f->stack() == expected) {
            string def = ": " + name + " " + candidate + " ;";
            cout << def << endl;
            f->eval(def);
            f->clear();
            return;
        }

        candidates.pop_front();
        for (const string& word : words) {
            candidates.push_back(candidate + " " + word);
        }
    }

    assert(false);
}

int main(int /*argc*/, char** /*argv*/) {
    Forth f;

    f.addImmediate(":", [&](){ f.startRecording(); });
    f.addImmediate(";", [&](){ f.stopRecording(); });

    f.add("+", [&](){ double r = f.pop(), l = f.pop(); f.push(l+r); });
    f.add("-", [&](){ double r = f.pop(), l = f.pop(); f.push(l-r); });
    f.add("*", [&](){ double r = f.pop(), l = f.pop(); f.push(l*r); });
    f.add("/", [&](){ double r = f.pop(), l = f.pop(); f.push(l/r); });

    f.add("drop",  [&](){ f.pop(); });
    f.add("clear", [&](){ f.clear(); });

    f.add("dup",  [&](){ double v = f.pop(); f.push(v); f.push(v); });
    f.add("swap", [&](){ double a = f.pop(), b = f.pop(); f.push(a); f.push(b); });
    f.add("rot",  [&](){
        double a = f.pop(), b = f.pop(), c = f.pop();
        f.push(b); f.push(a); f.push(c);
    });

    brute(&f, "over", "3 7", "3 7 3");
    brute(&f, "tuck", "3 7", "7 3 7");
    brute(&f, "nip",  "3 7", "7");

    brute(&f, "square", "7", "49");
    brute(&f, "sum-squares", "3 7", "58");

    brute(&f, "-rot", "3 7 13", "13 3 7");

    test(&f, "2 square", {4});

    f.clear();
    string line;
    do {
        for (double v : f.stack()) cout << v << " ";
        cout << endl;
        if(f.recording()) cout << "*";
        cout << "bƒ ";

        getline(cin, line);
        f.eval(line);
    } while(!line.empty());
}
