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
                    tokens.push_back(Token[current]);
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
                    tokens.push_back(Token[current]);
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


void parser()
{
    // Build input stream from the global 'tokens' vector:
    vector<string> input = tokens;   // copy
    input.push_back("$");            // end marker

    // Initialize parser stack with start symbol
    stack<string> st;
    string startSymbol = Grammar.begin()->first; // your code already relies on this
    st.push("$");
    st.push(startSymbol);

    // Cursor into input
    size_t ip = 0;

    auto normalizeTokenName = [](const string& tok) -> string {
        // If later you store "num:123", strip to "num"
        size_t p = tok.find(':');
        if (p != string::npos) return tok.substr(0, p);
        return tok;
    };

    while (!st.empty())
    {
        string top = st.top();
        string lookahead = normalizeTokenName(input[ip]);

        // Accept?
        if (top == "$" && lookahead == "$") {
            cout << "Parsing successful" << endl;
            return;
        }

        // Terminal on stack
        if (nontermianl.count(top) == 0)
        {
            if (top == lookahead) {
                st.pop();
                ip++;                 // consume input
            } else {
                cout << "Parse error: expected '" << top << "' but found '" << input[ip] << "'" << endl;
                return;
            }
        }
        // Nonterminal on stack
        else
        {
            // Safe lookup: your parseTable keys must use the SAME terminal strings as grammar
            auto key = make_pair(top, lookahead);
            if (!parseTable.count(key)) {
                // Try a couple of useful fallbacks when lookahead is number-like
                if (lookahead == "Number") lookahead = "num";
                key = make_pair(top, lookahead);
            }
            if (!parseTable.count(key)) {
                cout << "Parse error at token '" << input[ip] << "' with nonterminal '" << top << "'" << endl;
                return;
            }

            string prod = parseTable[key];
            st.pop();

            // epsilon?
            if (prod == "e" || prod == "ε" || prod.empty()) {
                // do nothing (epsilon)
            } else {
                // push RHS in reverse order
                vector<string> rhs = split(prod, ' ');
                for (int i = (int)rhs.size() - 1; i >= 0; --i) {
                    if (rhs[i].empty() || rhs[i] == "e" || rhs[i] == "ε") continue;
                    st.push(rhs[i]);
                }
            }
        }
    }

    // If we exit the loop without accepting:
    cout << "Parse error: unexpected end of input" << endl;
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
