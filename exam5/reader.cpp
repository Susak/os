#include<stdio.h>
#include<string.h>
#include<iostream>
#include<vector>
#include<stdlib.h>

using namespace std;

void panic(const char * msg)
{
    cout << msg;
    exit(EXIT_FAILURE);
}


int main() {
    vector<float> res(50);
    while (true) {
        sleep(5);

        int read_count = read(0, res.data(), 50 * 4);

        if (read_count == -1)
            panic("read");
        cout << res.data();
    }
}
