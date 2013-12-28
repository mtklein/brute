//! clang++ --std=c++11 -g -O3 -march=native brute_main.cc -o brute \
//!   -Werror -Weverything -Wno-c++98-compat -Wno-padded
#include <deque>
    using std::deque;
#include <functional>
    using std::bind;
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

typedef function<void()> Word;

class Forth {
public:
    Forth() : state_(Normal) {}

    const vector<double>& stack() const { return stack_; }

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


    void add(const string& name, Word word) {
        dict_[name] = make_pair(word, false);
    }

    void addImmediate(const string& name, Word word) {
        dict_[name] = make_pair(word, true);
    }

    vector<pair<string, Word>> dict() const {
        vector<pair<string, Word>> out;
        for (const auto& entry : dict_) {
            Word word;
            bool immediate;
            tie(word, immediate) = entry.second;

            if (!immediate) out.push_back(make_pair(entry.first, word));
        }
        return out;
    }

    void startRecording() {
        name_.clear();
        body_.clear();
        state_ = Record;
    }

    void startBrute() {
        name_.clear();
        state_ = Brute;
    }

    void startBruteOutput() {
        input_.clear();
        input_.swap(stack_);
    }

    void done() {
        Word w = nullptr;
        if (state_ == Record) {
            w = Concat(body_);
        } else if (state_ == Brute) {
            vector<double> output;
            output.swap(stack_);
            w = this->brute(input_, output);
        }
        dict_[name_] = make_pair(w, false);

        state_ = Normal;
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

            if (state_ != Normal && name_.empty()) {
                name_.swap(token);
                continue;
            }

            Word w;
            bool immediate;
            if (!this->evalToken(token, &w, &immediate)) continue;

            if (state_ == Record && !immediate) {
                body_.push_back(w);
            } else {
                w();
            }
        }
    }


private:
    static Word Concat(vector<Word> words) {
        return [=](){ for (auto word : words) word(); };
    }

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

    Word brute(const vector<double>& input, const vector<double>& output) {
        struct Cons {
            Cons(const pair<string, Word>& e, const Cons* t) : entry(e), tail(t) {}

            const pair<string, Word>& entry;
            const Cons* tail;

            const string& name() const { return entry.first; }
            const Word& word()   const { return entry.second; }
            void operator()() const { word()(); if (tail) (*tail)(); }
        };

        const auto dict = this->dict();
        deque<Cons> candidates, retired;
        for (const auto& entry : dict) candidates.emplace_back(entry, nullptr);

        while (!candidates.empty()) {
            retired.push_back(candidates.front());
            candidates.pop_front();
            const auto& candidate = retired.back();

            this->clear();
            for (double v : input) this->push(v);
            candidate();

            if (this->stack() == output) {
                this->clear();
                vector<Word> flattened;
                for (const Cons* c = &candidate; c; c = c->tail) {
                    cout << c->name() << " ";
                    flattened.push_back(c->word());
                }
                cout << endl;
                return Concat(flattened);
            }
            for (const auto& entry : dict) candidates.emplace_back(entry, &candidate);
        }
        return nullptr;
    }

    enum { Normal, Record, Brute } state_;
    vector<double> stack_;

    map<string, pair<Word,bool>> dict_;

    string name_;
    vector<Word> body_;

    vector<double> input_;
};


int main(int /*argc*/, char** /*argv*/) {
    Forth f;

    f.addImmediate(":",  bind(&Forth::startRecording, &f));
    f.addImmediate("::", bind(&Forth::startBrute, &f));
    f.addImmediate("->", bind(&Forth::startBruteOutput, &f));
    f.addImmediate(";",  bind(&Forth::done, &f));

    f.add("+", [&](){ double r = f.pop(), l = f.pop(); f.push(l+r); });
    f.add("-", [&](){ double r = f.pop(), l = f.pop(); f.push(l-r); });
    f.add("*", [&](){ double r = f.pop(), l = f.pop(); f.push(l*r); });
    f.add("/", [&](){ double r = f.pop(), l = f.pop(); f.push(l/r); });

    f.add("clear", bind(&Forth::clear, &f));
    f.add("drop",  [&](){ f.pop(); });

    f.add("dup",  [&](){ double v = f.pop(); f.push(v); f.push(v); });
    f.add("swap", [&](){ double a = f.pop(), b = f.pop(); f.push(a); f.push(b); });
    f.add("rot",  [&](){
        double a = f.pop(), b = f.pop(), c = f.pop();
        f.push(b); f.push(a); f.push(c);
    });

    f.eval(":: over 3 7 -> 3 7 3 ;");
    f.eval(":: tuck 3 7 -> 7 3 7 ;");
    f.eval(":: nip 3 7 -> 7 ;");
    f.eval(":: square 7 -> 49 ; ");
    f.eval(":: sum-squares 3 7 -> 58 ;");
    f.eval(":: -rot 3 7 13 -> 13 3 7 ;");
    f.eval(":: neg 1 7 3 -> 1 7 -3 ;");

    f.clear();
    string line;
    do {
        for (double v : f.stack()) cout << v << " ";
        cout << endl << "bƒ ";

        getline(cin, line);
        f.eval(line);
    } while(!line.empty());
}
