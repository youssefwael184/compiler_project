#include <bits/stdc++.h>

using namespace std;
map<pair<string,string>,string>Transitions;

string getnextstate(string currstate,string input)
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

    if(Transitions.find({currstate,input})!=Transitions.end())
        {
            return Transitions[{currstate,input}];
        }
}

int main()
{
    cout<<getnextstate("0","=")<<endl;


    return 0;
}
