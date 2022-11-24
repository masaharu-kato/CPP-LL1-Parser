#include <iostream>
#include <string>
#include <stack>
#include <array>
using namespace std;

string number(const string& s, int& str_i) {
    std::string res = {s[str_i++]};
    while(isdigit(s[str_i])) res.push_back(s[str_i++]);
    return res;
}

struct Symbol {
    const int lv;
    const char* ops;
    const Symbol* nexts[16];
};

constexpr const Symbol SYMBOL_FACTOR = Symbol{0, ""   , {nullptr}};
constexpr const Symbol SYMBOL_MULDIV = Symbol{1, "*/%", {&SYMBOL_FACTOR, nullptr}};
constexpr const Symbol SYMBOL_ADDSUB = Symbol{2, "+-" , {&SYMBOL_MULDIV, &SYMBOL_FACTOR, nullptr}};
constexpr const Symbol SYMBOL_COMMA  = Symbol{9, ","  , {&SYMBOL_ADDSUB, &SYMBOL_MULDIV, &SYMBOL_FACTOR, nullptr}};

constexpr const Symbol* ALL_SYMBOLS[] = {&SYMBOL_COMMA, &SYMBOL_ADDSUB, &SYMBOL_MULDIV, &SYMBOL_FACTOR};

class Parser {
    const string& s;
    int str_i = 0;
    stack<const Symbol*> st;
    stack<string> st_val;
    stack<char> st_op;

public:

    Parser(const string& s) : s(s) {}

    string parse() {
        for(const Symbol* s: ALL_SYMBOLS) st.push(s);

        st_op.push('$'); // dummy

        do {
            Symbol csymbol = *st.top();
            //std::cout << "csymbol:" << csymbol << "\n";

            if(csymbol.lv) {

            //  Symbols

                char op = st_op.top();
                for(const char* t_op=csymbol.ops; *t_op; ++t_op) {
                    if (op == *t_op) {
                        st_op.pop();
                        string v1 = st_val.top(); st_val.pop();
                        string v2 = st_val.top(); st_val.pop();
                        st_val.push("("+v2+op+v1+")");
                        break;
                    }
                }

                bool found = false;
                for(const char* t_op=csymbol.ops; *t_op; ++t_op) {
                    if (s[str_i] == *t_op) {
                        st_op.push(s[str_i++]);
                        for(const Symbol* const* s = csymbol.nexts; *s; ++s) {
                            st.push(*s);
                        }
                        found = true;
                        break;
                    }
                }
                if(!found) {
                    st.pop();
                }

            } else {

            //  Other values

                if (isdigit(s[str_i])) {
                    st_val.push(number(s, str_i));
                    st.pop();
                } else if (s[str_i] == '(') {
                    str_i++;
                    for(const Symbol* s: ALL_SYMBOLS) st.push(s);
                    st_op.push('(');
                } else if (s[str_i] == ')') {
                    str_i++;
                    st.pop();
                    st_op.pop();
                } else {
                    return "ERROR";
                }

            }
        } while(!st.empty());

        return st_val.top();
    }
};


int main() {
    //string str = "1+2-41+23=421|1324&123>95*6/(10-7)";
    string str = "5*1-5+(120+3)/10-5,32*6,32+1*(10+7)";
    //string str = "1+2+3+4";
    Parser parser = {str};
    cout << str << " = " << parser.parse() << endl;
    return 0;
}
