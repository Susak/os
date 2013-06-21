#include<vector>
#include<string>
#include<unistd.h>
#include<stdlib.h>

class stream {
    int fd;
    size_t size;
    size_t current_size;
    std::string buffer;
};
