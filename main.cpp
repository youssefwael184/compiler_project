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

void printStack(stack<string> s) // pass by value (copy)
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

    // Rule 1: if it's the start symbol
    if (non == Grammar.begin()->first)
    {
        follow.insert("$");
    }

    for (auto &rule : Grammar)
    {
        string lhs = rule.first;

        for (auto &prod : rule.second)
        {
            // Split RHS into symbols
            vector<string> symbols;
            stringstream ss(prod);
            string sym;
            while (ss >> sym)
                symbols.push_back(sym);

            for (size_t i = 0; i < symbols.size(); i++)
            {
                if (symbols[i] == non)
                {
                    // Case 1: Something follows
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
                    // Case 2: At the end
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
    // Temporarily add it to a fake nonterminal
    string temp = "TMP";
    auto backup = Grammar[temp];
    Grammar[temp] = {prod};

    set<string> first = readfirst(temp);

    Grammar[temp] = backup; // restore
    return first;
}

string findProd(const string &nonterm, const string &lookahead)
{
    for (auto &prod : Grammar[nonterm])
    {
        set<string> firstSet = firstOfProduction(prod);

        // If lookahead is in FIRST(prod), this is the correct production
        if (firstSet.count(lookahead))
            return prod;

        // Epsilon handling: if epsilon in FIRST, check FOLLOW
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
            bool recognized = true; // Track if this token is valid

            if (isdigit(input[0]))
            {
                string buffer = "";
                current = start_state; // Always reset before reading a new token
                for (char c : input)
                {
                    string s(1, c);
                    current = getnextstate(current, s);

                    if (current.empty()) // No valid transition
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

void parser() {
    // ----- تجهيز input tokens -----
    vector<string> in = tokens;
    in.push_back("$");
    size_t ip = 0;

    auto tokName = [](const string& tok)->string {
        size_t p = tok.find(':');
        return (p == string::npos) ? tok : tok.substr(0, p);
    };
    auto tokValue = [](const string& tok)->double {
        size_t p = tok.find(':');
        return (p == string::npos) ? 0.0 : stod(tok.substr(p + 1));
    };

    // ----- الـ stacks -----
    stack<string> pstack;   // parsing symbols + markers
    vector<double> vstack;  // values of finished (num / nonterm)

    string startSymbol = Grammar.begin()->first;
    pstack.push("$");
    pstack.push(startSymbol);

    auto reduce = [&](const string& marker) {
        // marker شكلها:  "#LHS->RHS"
        // نفصل LHS و RHS
        size_t arrow = marker.find("->");
        string LHS = marker.substr(1, arrow - 1);      // skip '#'
        string RHS = marker.substr(arrow + 2);
        LHS = trim(LHS);
        RHS = trim(RHS);

        // اجمع الرموز اللي ليها قيمة من RHS (nonterm + num)
        vector<string> rhs = split_ws(RHS);
        vector<int> take; // indices to take values for
        for (int i = 0; i < (int)rhs.size(); ++i) {
            string s = rhs[i];
            if (s == "e" || s == "ε") continue;
            if (s == "num" || isNonterminalSym(s)) take.push_back(i);
        }

        // اسحب القيم من value stack بالترتيب الصحيح (RHS order)
        vector<double> vals;
        vals.resize(take.size());
        for (int i = (int)take.size() - 1; i >= 0; --i) {
            if (vstack.empty()) throw runtime_error("Value stack underflow");
            vals[i] = vstack.back();
            vstack.pop_back();
        }

        auto rhsIs = [&](const string& s) { return RHS == s; };

        // ====== قواعد السيميتيك حسب الجرامر ======
        double res = 0.0;

        if (LHS == "Expr" && rhsIs("Term Expr`")) {
            // vals = [Term, Expr`]
            res = vals[0] + vals[1];
        }
        else if (LHS == "Expr`" && rhsIs("plus Term Expr`")) {
            res = vals[0] + vals[1]; // Term + Expr`
        }
        else if (LHS == "Expr`" && rhsIs("minus Term Expr`")) {
            res = -vals[0] + vals[1]; // (-Term) + Expr`
        }
        else if (LHS == "Expr`" && (rhsIs("e") || rhsIs("ε") || RHS.empty())) {
            res = 0.0; // محايد الجمع
        }
        else if (LHS == "Term" && rhsIs("Power Term`")) {
            res = vals[0] * vals[1]; // Power * Term`
        }
        else if (LHS == "Term`" && rhsIs("mult Power Term`")) {
            res = vals[0] * vals[1]; // Power * Term`
        }
        else if (LHS == "Term`" && rhsIs("div Power Term`")) {
            // (1 / Power) * Term`
            res = (1.0 / vals[0]) * vals[1];
        }
        else if (LHS == "Term`" && (rhsIs("e") || rhsIs("ε") || RHS.empty())) {
            res = 1.0; // محايد الضرب
        }
        else if (LHS == "Power" && rhsIs("Unary Power`")) {
            // right-assoc: value = pow(Unary, Power`)
            res = pow(vals[0], vals[1]);
        }
        else if (LHS == "Power`" && rhsIs("pow Unary Power`")) {
            // value = pow(Unary, Power`)  (تركيب الأس يميني)
            res = pow(vals[0], vals[1]);
        }
        else if (LHS == "Power`" && (rhsIs("e") || rhsIs("ε") || RHS.empty())) {
            res = 1.0; // أساس الأس (a^1 = a)
        }
        else if (LHS == "Unary" && rhsIs("minus Unary")) {
            res = -vals[0];
        }
        else if (LHS == "Unary" && rhsIs("Primary")) {
            res = vals[0];
        }
        else if (LHS == "Primary" && rhsIs("Func")) {
            res = vals[0];
        }
        else if (LHS == "Primary" && rhsIs("leftp Expr rightp")) {
            res = vals[0]; // قيمة Expr
        }
        else if (LHS == "Primary" && rhsIs("num")) {
            res = vals[0]; // قيمة الرقم
        }
        else if (LHS == "Func" && rhsIs("sin leftp Expr rightp")) {
            res = sin(vals[0]);
        }
        else if (LHS == "Func" && rhsIs("cos leftp Expr rightp")) {
            res = cos(vals[0]);
        }
        else if (LHS == "Func" && rhsIs("tan leftp Expr rightp")) {
            res = tan(vals[0]);
        }
        else if (LHS == "Func" && rhsIs("log leftp Expr rightp")) {
            res = log10(vals[0]);
        }
        else if (LHS == "Func" && rhsIs("ln leftp Expr rightp")) {
            res = log(vals[0]);
        }
        else if (LHS == "Func" && rhsIs("sqrt leftp Expr rightp")) {
            res = sqrt(vals[0]);
        }
        else {
            // لو فيه production غير متغطي — رجّع 0 بشكل افتراضي
            // ممكن ترمي استثناء أو تعمل assert هنا لو عايز تشدد.
            res = (vals.empty() ? 0.0 : vals[0]);
        }

        // ادفع نتيجة الـ LHS على value stack
        vstack.push_back(res);
    };

    while (!pstack.empty()) {
        string X = pstack.top();

        if (X == "$") {
            if (tokName(in[ip]) == "$") {
                // المفروض يبقى على vstack قيمة واحدة هي نتيجة الـ Expr
                double ans = vstack.empty() ? 0.0 : vstack.back();
                cout << "Parsing successful ✅\nResult = " << ans << endl;
                return;
            } else {
                cerr << "Parse error: extra input at end: " << in[ip] << endl;
                return;
            }
        }

        // هل X ماركر تقليص؟
        if (!X.empty() && X[0] == '#') {
            pstack.pop();
            reduce(X);
            continue;
        }

        string a = tokName(in[ip]);

        // Terminal?
        if (!isNonterminalSym(X)) {
            // match?
            if (X == a) {
                // لو num، خزّن قيمته
                if (a == "num") vstack.push_back(tokValue(in[ip]));
                pstack.pop();
                ip++;
            } else if (X == "e" || X == "ε") {
                // ماينفعش نوصل هنا لأننا عمرنا ما بنرمي e على الstack
                pstack.pop(); // احتياط
            } else {
                cerr << "Parse error: expected '" << X << "' but found '" << in[ip] << "'\n";
                return;
            }
        }
        // Nonterminal: استخدم الـ LL table
        else {
            auto key = make_pair(X, a);
            if (!parseTable.count(key)) {
                cerr << "Parse error at token '" << in[ip] << "' with nonterminal '" << X << "'\n";
                return;
            }
            string RHS = trim(parseTable[key]);   // ملاحظة: الـ table بيرجع RHS فقط
            pstack.pop();

            // ε-Production؟ حط ماركر بس
            if (RHS == "e" || RHS == "ε" || RHS.empty()) {
                pstack.push("#" + X + "->" + RHS);
                continue;
            }

            // ادفع RHS بالعكس، وقبلهم ماركر التقليص
            vector<string> syms = split_ws(RHS);
            pstack.push("#" + X + "->" + RHS);
            for (int i = (int)syms.size() - 1; i >= 0; --i) {
                if (syms[i] == "e" || syms[i] == "ε" || syms[i].empty()) continue;
                pstack.push(syms[i]);
            }
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
    readinput("input.txt"); // <-- tokenize file into global 'tokens'
    parser();               // <-- parse using 'tokens'
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
