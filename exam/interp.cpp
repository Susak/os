#include<string>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<vector>
#include<iostream>
#include<ctime>
using namespace std;


vector<string> parse(vector<char>& vec) {
    bool symbol = false;
    string st;
    vector<string> res;
    for (int i = 0; i < vec.size(); i++) {
        if (vec[i] == ' ' && symbol) {
           st += vec[i];
           continue;
        }

        if (vec[i] == '\"') {
            symbol ^= 1;
            continue;
        }

        if (vec[i] == ' ' && !symbol || vec[i] == '\n') {
           res.push_back(st);
           st.clear();
           continue;
        }
        st += vec[i];
    }
    return res;
}

int main() {
    vector<char> buffer;
    while (true) {
        buffer.resize(4096);
        int read_count = read(0, buffer.data(), buffer.size());
        vector<string> command = parse(buffer);
        char* char_command[1000];
        int h = (command[0][0] - '0') * 10 + (command[0][1] - '0');
        int m = (command[0][3] - '0') * 10 + (command[0][4] - '0');
        h  = h * 60 + m;
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        int sec = (h - (tm.tm_hour * 60 + tm.tm_min)) * 60;
        cout << sec <<endl;
        sleep(sec / 20);
        for (int i = 1; i < command.size(); i++) {
            char_command[i - 1] = const_cast<char* > (command[i].data());
        } 
        char_command[command.size()] = NULL;
        int exec_val = execvp(char_command[0], char_command);
        buffer.clear();
        if (exec_val != 0)
            return 1;
    }
    return 0;
}
