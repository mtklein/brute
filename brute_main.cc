//! clang++ --std=c++11 -O3 -march=native brute_main.cc -o brute_main \
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
#include <map>
    using std::map;
#include <string>
    using std::string;
#include <utility>
    using std::make_pair;
    using std::pair;
    using std::tie;
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
        dict_[name] = make_pair(word, false);
    }

    void addImmediate(const string& name, Word word) {
        dict_[name] = make_pair(word, true);
    }

    vector<Word> words() const {
        vector<Word> words;
        for (const auto& entry : dict_) {
            Word word;
            bool immediate;
            tie(word, immediate) = entry.second;

            if (!immediate) words.push_back(word);
        }
        return words;
    }

    static Word Concat(vector<Word> words) {
        return [words](){ for (auto word : words) word(); };
    }

    bool recording() const { return recording_; }

    void startRecording() {
        recording_ = true;
        name_.clear();
        body_.clear();
    }

    void stopRecording() {
        recording_ = false;
        dict_[name_] = make_pair(Concat(body_), false);
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

            if (this->recording() && name_ == "") {
                name_ = token;
                continue;
            }

            Word w;
            bool immediate;
            if (!this->evalToken(token, &w, &immediate)) continue;

            if (recording_ && !immediate) {
                body_.push_back(w);
            } else {
                w();
            }
        }
    }

private:
    bool evalToken(const string& token, Word* word, bool* immediate) {
        double literal;
        if (parse_literal(token, &literal)) {
            *word = [=](){ this->push(literal); };
            *immediate = false;
            return true;
        }

        auto it = dict_.find(token);
        if (it != dict_.end()) {
            *word = it->second.first;
            *immediate = it->second.second;
            return true;
        }

        return false;
    }

    vector<double> stack_;

    map<string, pair<Word,bool>> dict_;

    bool recording_;
    string name_;
    vector<Word> body_;
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
    f->clear();
    f->eval(out);
    vector<double> expected = f->stack();

    const vector<Forth::Word> words = f->words();

    deque<vector<Forth::Word>> candidates;
    for (auto word : words) candidates.push_back({word});

    while (!candidates.empty()) {
        const vector<Forth::Word>& candidate = candidates.front();

        f->clear();
        f->eval(in);
        Forth::Word w = Forth::Concat(candidate);
        w();

        if (f->stack() == expected) {
            f->add(name, w);
            f->clear();
            return;
        }

        for (auto word : words) {
            vector<Forth::Word> child(candidate);
            child.push_back(word);
            candidates.push_back(child);
        }
        candidates.pop_front();
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
