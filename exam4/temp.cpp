#include<vector>

void f(std::vector<char> & buf) 
{
}

int main() {
    std::vector<char> buf;
    f(buf);
}
