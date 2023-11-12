#include <bits/stdc++.h>
#include "../headers/modules.h"

using namespace std;

/*vector<string> tokenize_old(string input)
{
    vector<string> tokens;

    stringstream check1(input);

    string intermediate;
    string quotedToken;
    bool insideQuotes = false; // we check is inside Quotes
    int quotecounter = 0;

    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == '"')
        {
            quotecounter++;
        }
    }

    if (quotecounter % 2 != 0)
    {
        cout << "Malformed text" << endl;
        return tokens;
    }
    while (getline(check1, intermediate, ' '))
    {

        if (intermediate.front() == '"' && intermediate.back() == '"')
        {
            intermediate = intermediate.substr(1);
            intermediate = intermediate.substr(0, intermediate.size() - 1);
            tokens.push_back(intermediate);
        }
        else if (intermediate.front() == '"' && !insideQuotes)
        {
            insideQuotes = true;
            intermediate = intermediate.substr(1);
            bool push = false;
            for (int i = 0; i < intermediate.size(); i++)
            {
                if (intermediate[i] == '"' && intermediate[i + 1] != ' ')
                {
                    insideQuotes = false;
                    quotedToken = intermediate.substr(0, i) + intermediate.substr(i + 1, intermediate.size());
                    tokens.push_back(quotedToken);
                    push = true;
                    quotedToken = "";
                }
            }
            if (!push)
            {
                quotedToken = intermediate;
            }
        }
        else if (intermediate.back() == '"' && insideQuotes)
        {
            insideQuotes = false;
            quotedToken += " " + intermediate.substr(0, intermediate.size() - 1);
            tokens.push_back(quotedToken);
        }
        else if (insideQuotes)
        {
            bool push = false;
            for (int i = 0; i < intermediate.size(); i++)
            {
                if (intermediate[i] == '"' && intermediate[i + 1] != ' ')
                {
                    insideQuotes = false;
                    quotedToken += " " + intermediate.substr(0, i) + intermediate.substr(i + 1, intermediate.size());
                    tokens.push_back(quotedToken);
                    push = true;
                    quotedToken = "";
                }
            }
            if (!push)
            {
                quotedToken += " " + intermediate;
            }
        }
        else
        {
            tokens.push_back(intermediate);
        }
    }

    return tokens;
}*/

vector<string> tokenize(string input)
{
    vector<string> tokens;
    string token = "";
    bool quoted = false;
    bool intoken = false;

    int quotecounter = 0;

    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == '"')
        {
            quotecounter++;
        }
    }

    if (quotecounter % 2 != 0)
    {
        cout << "Malformed text" << endl;
        return tokens;
    }

    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] != ' ' && input[i] == '"' && !quoted && !intoken)
        {
            quoted = true;
            intoken = true;
            continue;
        }
        else if (input[i] != ' ' && input[i] != '"' && intoken)
        {
            token += input[i];
        }
        else if (input[i] != ' ' && input[i] == '"' && intoken && quoted)
        {  // if(i<input.size()-1 && input[i+1] != ' '){
            //quoted = false;
            //continue;
        //}
            quoted = false;
            intoken = false;
            tokens.push_back(token);
            token = "";
        }
        else if (input[i] == ' ' && intoken && !quoted)
        {
            tokens.push_back(token);
            token = "";
            intoken = false;
        }
        else if (input[i] != ' ' && input[i] != '"' && !intoken)
        {
            token += input[i];
            intoken = true;
        }
        else if (input[i] == ' ' && intoken && quoted)
        {
            token += input[i];
            continue;
        }
        else if (input[i] == ' ' && !intoken)
        {
            continue;
        }
    }

    if (intoken)
    {
        tokens.push_back(token);
        token = "";
    }
    return tokens;
}

string getUserInput()
{
    cout << ">_ ";
    string input;
    getline(cin, input);
    return input;
}

bool checkUserInput(vector<string> input)
{
    // Needs to be moved to a global variable

    unordered_map<string, int> validCommands;

    validCommands.insert({"pwd", 0});
    validCommands.insert({"cd", 1});
    validCommands.insert({"ls", 0});
    validCommands.insert({"cat", 1});
    validCommands.insert({"share", 2});
    validCommands.insert({"mkfile", 2});
    validCommands.insert({"mkdir", 1});
    validCommands.insert({"exit", 0});
    validCommands.insert({"adduser", 1});

    if (input.size() == 0)
        return false;

    if (validCommands.find(input[0]) == validCommands.end())
    {
        cout << "Unrecognized input." << endl;
        return false;
    }

    int expectedArgs = validCommands[input[0]];

    if (input.size() - 1 < expectedArgs)
    {
        // change != to <
        cout << "Command " << input[0] << " expects at least " << expectedArgs << " arguments." << endl;
        return false;
    }

    return true;
}

void getAndExecuteCommand(bool *running)
{
    string input = getUserInput();
    if (input == "")
        return;
    vector<string> tokenizedInput = tokenize(input);

    bool isValidInput = checkUserInput(tokenizedInput);

    if (!isValidInput)
    {
        // cout << "Unrecognized input"<<endl;
        return;
    }

    string command = tokenizedInput[0];

    if (command == "cat")
    {
        // cat(tokenizedInput);
        cat2(tokenizedInput);
    }
    else if (command == "pwd")
    {
        pwd();
    }
    else if (command == "cd")
    {
        cd(tokenizedInput[1]);
    }
    else if (command == "ls")
    {
        ls();
    }
    else if (command == "share")
    {
        share(tokenizedInput);
    }
    else if (command == "mkfile")
    {
        mkfile(tokenizedInput);
    }
    else if (command == "mkdir")
    {
        mkdir(tokenizedInput);
    }
    else if (command == "exit")
    {
        exitmodule(running);
    }
    else if (command == "adduser")
    {
        adduser(tokenizedInput);
    }
}
