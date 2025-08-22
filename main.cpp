#include <bits/stdc++.h>

using namespace std;

map<pair<string, string>, string> parseTable;
map<string, vector<string>> Grammar;
set<string> nontermianl;
map<pair<string, string>, string> Transitions;
map<string, string> State_Type;
map<string, string> Token;
vector<string> tokens;
string start_state;
vector<string> input;

void printStack(stack<string> s) 
{
    while (!s.empty())
    {
        cout << s.top() << " ";
        s.pop();
    }
    cout << endl;
}
vector<string> split(const string &line, char delimiter)
{
    vector<string> result;
    stringstream ss(line);
    string item;
    while (getline(ss, item, delimiter))
        result.push_back(item);
    return result;
}

void readtransition(const string filename)
{
    ifstream tr("transition.txt");
    string s = "";

    while (getline(tr, s))
    {
        string c, i, n;
        stringstream ss(s);
        getline(ss, c, ',');
        getline(ss, i, ',');
        getline(ss, n, ',');
        Transitions[{c, i}] = n;
    }
    tr.close();
}

string getnextstate(string currstate, string input)
{

    if (Transitions.find({currstate, input}) != Transitions.end())
    {
        return Transitions[{currstate, input}];
    }
    return "error in transition";
}

void readstates(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cout << "error opening automaton file " << endl;
    }
    string line;
    getline(file, line);

    getline(file, line);
    State_Type[line] = "Start";
    start_state = line;

    getline(file, line);

    vector<string> finalstates = split(line, ',');
    for (auto state : finalstates)
    {
        State_Type[state] = "Final";
    }

    getline(file, line);
    State_Type[line] = "Error";
}

void readtokens(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cout << "error opening read token file" << endl;
    }
    string line;
    while (getline(file, line))
    {
        vector<string> tokens = split(line, ',');
        if (tokens.size() == 2)
        {
            Token[tokens[0]] = tokens[1];
        }
    }
}

void readgrammar(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cout << "error opening file" << endl;
    }
    string line;
    while (getline(file, line))
    {
        stringstream ss(line);
        string LHS, RHS;

        LHS = line.substr(0, line.find("->"));
        RHS = line.substr(line.find("->") + 2, line.size());
        Grammar[LHS].push_back(RHS);
        nontermianl.insert(LHS);
    }
}

set<string> readfirst(string non)
{
    set<string> firstset;
    set<string> ss;
    string k;
    for (auto s : Grammar[non])
    {
        bool isepslon = true;

        stringstream ss(s);
        while (ss >> k && isepslon)
        {

            if (nontermianl.count(k) == 1)
            {
                set<string> ss = readfirst(k);
                for (auto l : ss)
                {

                    firstset.insert(l);
                }
                if (firstset.find("e") == firstset.end())
                {
                    isepslon = false;
                }
            }
            else
            {
                isepslon = false;

                firstset.insert(k);
            }
        }
    }

    return firstset;
}

set<string> readfollow(string non)
{
    set<string> follow;
    if (non == Grammar.begin()->first)
    {
        follow.insert("$");
    }

    for (auto &rule : Grammar)
    {
        string lhs = rule.first;

        for (auto &prod : rule.second)
        {
            vector<string> symbols;
            stringstream ss(prod);
            string sym;
            while (ss >> sym)
                symbols.push_back(sym);

            for (size_t i = 0; i < symbols.size(); i++)
            {
                if (symbols[i] == non)
                {
                    if (i + 1 < symbols.size())
                    {
                        string nextSym = symbols[i + 1];
                        if (nontermianl.count(nextSym))
                        {
                            set<string> firstSet = readfirst(nextSym);
                            bool hasEps = false;
                            for (auto &f : firstSet)
                            {
                                if (f == "e")
                                    hasEps = true;
                                else
                                    follow.insert(f);
                            }
                            if (hasEps)
                            {
                                set<string> followLHS = readfollow(lhs);
                                follow.insert(followLHS.begin(), followLHS.end());
                            }
                        }
                        else
                        {
                            follow.insert(nextSym);
                        }
                    }
                    else if (lhs != non)
                    {
                        set<string> followLHS = readfollow(lhs);
                        follow.insert(followLHS.begin(), followLHS.end());
                    }
                }
            }
        }
    }

    return follow;
}

set<string> firstOfProduction(const string &prod)
{
    string temp = "TMP";
    auto backup = Grammar[temp];
    Grammar[temp] = {prod};

    set<string> first = readfirst(temp);

    Grammar[temp] = backup;
    return first;
}

string findProd(const string &nonterm, const string &lookahead)
{
    for (auto &prod : Grammar[nonterm])
    {
        set<string> firstSet = firstOfProduction(prod);
        if (firstSet.count(lookahead))
            return prod;
        if (firstSet.count("e"))
        {
            set<string> followSet = readfollow(nonterm);
            if (followSet.count(lookahead))
                return prod;
        }
    }
    return "error: no matching production";
}

void generateTable()
{
    for (auto nonterm : nontermianl)
    {
        set<string> storeFirst = readfirst(nonterm);
        for (auto terminal : storeFirst)
        {
            if (terminal == "e")
            {
                set<string> storefollow = readfollow(nonterm);
                for (auto i : storefollow)
                {
                    parseTable[{nonterm, i}] = findProd(nonterm, i);
                }
            }
            else
            {
                parseTable[{nonterm, terminal}] = findProd(nonterm, terminal);
            }
        }
    }
}

void readinput(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cout << "error opening input.txt " << endl;
        return;
    }

    string line;
    string current = start_state;

    while (getline(file, line))
    {
        stringstream ss(line);
        string input;

        while (ss >> input)
        {
            bool recognized = true;

            if (isdigit(input[0]))
            {
                string buffer = "";
                current = start_state;
                for (char c : input)
                {
                    string s(1, c);
                    current = getnextstate(current, s);
                    if (current.empty())
                    {
                        recognized = false;
                        break;
                    }

                    buffer += c;
                }

                if (recognized && State_Type[current] == "Final")
                {
                    tokens.push_back(Token[current] + ":" + buffer);
                }
                else
                {
                    cout << "Invalid token: " << buffer << endl;
                    tokens.push_back("INVALID");
                }
                current = start_state;
            }
            else
            {
                current = start_state;
                current = getnextstate(current, input);

                if (!current.empty() && State_Type[current] == "Final")
                {
                    tokens.push_back(Token[current] + ":" + input);
                }
                else
                {
                    cout << "Invalid token: " << input << endl;
                    tokens.push_back("INVALID");
                }
                current = start_state;
            }
        }
    }
}


// void parser()
// {
//     // Build input stream from the global 'tokens' vector:
//     vector<string> input = tokens;   // copy
//     input.push_back("$");            // end marker

//     // Initialize parser stack with start symbol
//     stack<string> st;
//     string startSymbol = Grammar.begin()->first; // your code already relies on this
//     st.push("$");
//     st.push(startSymbol);

//     // Cursor into input
//     size_t ip = 0;

//     auto normalizeTokenName = [](const string& tok) -> string {
//         // If later you store "num:123", strip to "num"
//         size_t p = tok.find(':');
//         if (p != string::npos) return tok.substr(0, p);
//         return tok;
//     };

//     while (!st.empty())
//     {
//         string top = st.top();
//         string lookahead = normalizeTokenName(input[ip]);

//         // Accept?
//         if (top == "$" && lookahead == "$") {
//             cout << "Parsing successful" << endl;
//             return;
//         }

//         // Terminal on stack
//         if (nontermianl.count(top) == 0)
//         {
//             if (top == lookahead) {
//                 st.pop();
//                 ip++;                 // consume input
//             } else {
//                 cout << "Parse error: expected '" << top << "' but found '" << input[ip] << "'" << endl;
//                 return;
//             }
//         }
//         // Nonterminal on stack
//         else
//         {
//             // Safe lookup: your parseTable keys must use the SAME terminal strings as grammar
//             auto key = make_pair(top, lookahead);
//             if (!parseTable.count(key)) {
//                 // Try a couple of useful fallbacks when lookahead is number-like
//                 if (lookahead == "Number") lookahead = "num";
//                 key = make_pair(top, lookahead);
//             }
//             if (!parseTable.count(key)) {
//                 cout << "Parse error at token '" << input[ip] << "' with nonterminal '" << top << "'" << endl;
//                 return;
//             }

//             string prod = parseTable[key];
//             st.pop();

//             // epsilon?
//             if (prod == "e" || prod == "ε" || prod.empty()) {
//                 // do nothing (epsilon)
//             } else {
//                 // push RHS in reverse order
//                 vector<string> rhs = split(prod, ' ');
//                 for (int i = (int)rhs.size() - 1; i >= 0; --i) {
//                     if (rhs[i].empty() || rhs[i] == "e" || rhs[i] == "ε") continue;
//                     st.push(rhs[i]);
//                 }
//             }
//         }
//     }
//     cout << "Parse error: unexpected end of input" << endl;
// }
struct Symbol {
    string name;
    double value;
    bool hasValue;
};
static inline string trim(const string& s) {
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == string::npos) return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

static vector<string> split_ws(const string& s) {
    vector<string> out;
    string t;
    stringstream ss(s);
    while (ss >> t) out.push_back(t);
    return out;
}

static inline bool isNonterminalSym(const string& s) {
    return nontermianl.count(s) > 0;
}

string tokName(const string& tok) {
    size_t p = tok.find(':');
    return (p == string::npos) ? tok : tok.substr(0, p);
}

double tokValue(const string& tok) {
    size_t p = tok.find(':');
    return (p == string::npos) ? 0.0 : stod(tok.substr(p + 1));
}

bool isEpsilon(const string& s) {
    return s == "e" || s == "ε" || s.empty();
}

double applyRule(const string& LHS, const string& RHS, const vector<double>& vals) {
    auto rhsIs = [&](const string& s)
    { 
        return trim(RHS) == s ;
    };
    if (LHS == "Expr"  && rhsIs("Term Expr`"))           
        return vals[0] + vals[1];
    if (LHS == "Expr`" && rhsIs("plus Term Expr`"))      
        return vals[0] + vals[1];
    if (LHS == "Expr`" && rhsIs("minus Term Expr`"))     
        return -vals[0] + vals[1];
    if (LHS == "Expr`" && isEpsilon(RHS))                
        return 0.0;
    if (LHS == "Term"  && rhsIs("Power Term`"))          
        return vals[0] * vals[1];
    if (LHS == "Term`" && rhsIs("mult Power Term`"))     
        return vals[0] * vals[1];
    if (LHS == "Term`" && rhsIs("div Power Term`"))      
        return (1.0 / vals[0]) * vals[1];
    if (LHS == "Term`" && isEpsilon(RHS))               
        return 1.0;
    if (LHS == "Power" && rhsIs("Unary Power`"))        
        return pow(vals[0], vals[1]);
    if (LHS == "Power`" && rhsIs("pow Unary Power`"))    
        return pow(vals[0], vals[1]);
    if (LHS == "Power`" && isEpsilon(RHS))               
        return 1.0;
    if (LHS == "Unary" && rhsIs("minus Unary"))          
        return -vals[0];
    if (LHS == "Unary" && rhsIs("Primary"))              
        return vals[0];
    if (LHS == "Primary" && rhsIs("Func"))               
        return vals[0];
    if (LHS == "Primary" && rhsIs("leftp Expr rightp"))  
        return vals[0];
    if (LHS == "Primary" && rhsIs("num"))                
        return vals[0];
    if (LHS == "Func" && rhsIs("sin leftp Expr rightp")) 
        return sin(vals[0]);
    if (LHS == "Func" && rhsIs("cos leftp Expr rightp")) 
        return cos(vals[0]);
    if (LHS == "Func" && rhsIs("tan leftp Expr rightp")) 
        return tan(vals[0]);
    if (LHS == "Func" && rhsIs("log leftp Expr rightp"))
        return log10(vals[0]);
    if (LHS == "Func" && rhsIs("ln leftp Expr rightp"))  
        return log(vals[0]);
    if (LHS == "Func" && rhsIs("sqrt leftp Expr rightp"))
        return sqrt(vals[0]);
    return vals.empty() ? 0.0 : vals[0];
}
void parser() {
    vector<string> in = tokens;
    in.push_back("$");
    size_t ip = 0;
    stack<string> pstack;
    vector<double> vstack;
    string start = Grammar.begin()->first;
    pstack.push("$");
    pstack.push(start);
    while (!pstack.empty()) {
        string X = pstack.top();
        string a = tokName(in[ip]);
        if (X == "$") {
            if (a == "$") {
                cout <<"parsing successful and result is: "<<"\nResult = " << (vstack.empty() ? 0.0 : vstack.back()) << endl;
                return;
            }
            cerr << "Parse error: extra input '" << in[ip] << "'\n";
            return;
        }
        if (!X.empty() && X[0] == '#') {
            pstack.pop();
            size_t arrow = X.find("->");
            string LHS = trim(X.substr(1, arrow - 1));
            string RHS = trim(X.substr(arrow + 2));
            vector<string> rhs = split_ws(RHS);
            vector<double> vals;
            for (int i = (int)rhs.size() - 1; i >= 0; --i) {
                if (rhs[i] == "num" || isNonterminalSym(rhs[i])) {
                    vals.insert(vals.begin(), vstack.back());
                    vstack.pop_back();
                }
            }
            vstack.push_back(applyRule(LHS, RHS, vals));
            continue;
        }
        if (!isNonterminalSym(X)) {
            if (X == a) {
                if (a == "num") vstack.push_back(tokValue(in[ip]));
                pstack.pop();
                ip++;
            } else {
                cerr << "Parse error: expected '" << X << "' but found '" << in[ip] << "'\n";
                return;
            }
            continue;
        }
        auto key = make_pair(X, a);
        if (!parseTable.count(key)) {
            cerr << "Parse error at token '" << in[ip] << "' with nonterminal '" << X << "'\n";
            return;
        }

        string RHS = trim(parseTable[key]);
        pstack.pop();
        pstack.push("#" + X + "->" + RHS);
        if (!isEpsilon(RHS)) {
            vector<string> syms = split_ws(RHS);
            for (int i = (int)syms.size() - 1; i >= 0; --i)
                if (!isEpsilon(syms[i])) pstack.push(syms[i]);
        }
    }

    cerr << "Parse error: unexpected end of input\n";
}

int main()
{

    readstates("automaton.txt");
    readtransition("transition.txt");
    readtokens("tokens.txt");
    readgrammar("grammar.txt");
    generateTable();
    readinput("input.txt");
    parser();               
    // for (auto s : State_Type)
    // {
    //     cout << s.first << " :" << s.second << endl;
    // }

    // cout << getnextstate("q0", "2") << endl;
    // for (auto s : Transitions)
    // {

    //     cout << s.first.first <<":"<< s.first.second <<":"<< s.second << endl;
    // }
    // cout << "tokens" << endl;
    // for (auto s : Token)
    // {
    //     cout << s.first << ":" << s.second << endl;
    // }
    // for(auto s: Grammar){
    //     cout<< s.first<<endl;
    //     for(auto ss : s.second){
    //         cout<<":"<<ss<<endl;
    //     }
    // }
    // for (auto i : readfirst("Term"))
    // {
    //     cout << i << endl;
    // }
    // for (auto i : readfollow("Func"))
    // {
    //     cout << i << endl;
    // }
    // cout << findProd("Expr`", "+");
    // for (auto i : parseTable)
    // {
    //     cout << i.first.first << ", " << i.first.second << " => " << i.second << endl;
    // }
    // printStack(input);
    for (auto i : tokens)
    {
        cout << i << endl;
    }

    return 0;
}


