#include<unistd.h>
#include<string.h>
#include<vector>
#include<algorithm>

std::vector<std::vector<char> > parse_buffer(std::vector<char> a,
        char delim) {
    auto it = std::find(a.begin(), a.end(), delim);
    std::vector<std::vector<char> > res;
    while (it != a.end()) {
        it++;
        res.push_back(std::vector<char>(a.begin(), it));
        a.erase(a.begin(), it);
        it = std::find(a.begin(), a.end(), delim);
    }
    return res;
}

int main() {
    std::vector<char> a(1000);
    std::vector<std::vector<char> > res;
    while (true) {
        read(0, a.data(), 1000);
        res = parse_buffer(a, '\n');
        for (int i = 0; i < res.size(); i++) {
            write(1, res[i].data(), res[i].size());
        }
    }
    return 0;
}
