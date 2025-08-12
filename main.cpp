#include <bits/stdc++.h>

using namespace std;
map<pair<string, string>, string> Transitions;
map<string, string> State_Type;

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
    string s="";

    while(getline(tr,s))
    {
        string c,i,n;
        stringstream ss(s);
        getline(ss,c,',');
        getline(ss,i,',');
        getline(ss,n,',');
        Transitions[{c,i}]=n;
    }
    tr.close();
}

string getnextstate(string currstate,string input)
{

    if(Transitions.find({currstate,input})!=Transitions.end())
        {
            return Transitions[{currstate,input}];
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

    getline(file, line);

    vector<string> finalstates = split(line, ',');
    for (auto state : finalstates)
    {
        State_Type[state] = "Final";
    }

    getline(file, line);
    State_Type[line] = "Error";
}



int main()
{
    

    readstates("automaton.txt");
    readtransition("transition.txt");

    for (auto s : State_Type)
    {
        cout << s.first << " :" << s.second << endl;
    }

    cout<<getnextstate("1","4")<<endl;
    for(auto s : Transitions){
        
            cout<<s.first.first <<s.first.second<<s.second<<endl;
        }
    
    return 0;
}


