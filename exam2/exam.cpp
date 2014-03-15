#include<unistd.h>
#include<algorithm>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<string>
#include<vector>
#include<iostream>
using namespace std;

bool back_ground;
vector<string> parse(vector<char> const &vec) {
    bool symbol = false;
    vector<string> res;
    string st;
    for (int i = 0; i < vec.size(); i++) {
        if (vec[i] == ' ' && symbol) {
            st += vec[i];
            continue;
        }
        if (vec[i] == '\"') {
            symbol ^= 1;
            continue;
        }
        if (vec[i] == '&') {
             back_ground = true;
             return res;
        }
        if (vec[i] == ' ' && !symbol) {
             res.push_back(st);
             st.clear();
             continue;
        }
        if (vec[i] == '\n') {
             res.push_back(st);
             return res;
        }
        st += vec[i];
    }
}

int main() {
    vector<char> buf;
    vector<int> child;
    while (true) {
        int read_count = 0;
        int count = 0;
        buf.resize(4096);
        while (true) {
            if (find(buf.begin(), buf.end(), '\n') != buf.end()) {
                break;
            }
            count = read(0, buf.data() + read_count, 4096);
            read_count += count;
        }
        char* command[1000];
        vector<string> res = parse(buf);
        if (res[0] == "wait") {
            int n = atoi(res[1].data());
            waitpid(child[child.size() - n - 1], NULL, 0);
            child.clear();
            cout << "okkk" << endl;
            continue;
        }
        for (int i = 0; i < res.size(); i++) {
            command[i] = const_cast<char*>(res[i].data());
        }
        command[res.size()] = NULL;
        res.clear();
        buf.clear();
        int fork_val = fork();
        if (fork_val) {
            if (back_ground) {
                back_ground = false;
                child.push_back(fork_val);
            } else {
                waitpid(fork_val, NULL, 0);
            }
        } else {
            execvp(command[0], command);
        }
    }
    return 0;
}
