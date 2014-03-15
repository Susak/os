#include<vector>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<iostream>
#include "optional.h"
class lazyStream {

    public:
    int fd;
    size_t size;
    size_t current_size;
    char delim;
    std::vector<char> buffer;

    int parse_buf(std::vector<char> &buf) {
        for (int i = 0; i < buf.size(); i++) {
           if (delim == buf[i]) {
               std::cout<<"is i " << i << std::endl;
               return i;
           }
        }
        return -1;
    }
public:
    lazyStream(int fd_, size_t size_, char delim_) : fd(fd_), size(size_),
                delim(delim_), current_size(size_) {
        buffer.resize(size);
    };

    optional<std::vector<char> > readnext() {
        int read_counter = 1;
        std::vector<char> temp(size);
        while(read_counter > 0) {
            int size_parse;
            if (current_size == size) {
                read_counter = read(fd, buffer.data() + size - current_size,
                    current_size);
                size_parse = parse_buf(buffer);
                current_size -= read_counter;
            } else {
                size_parse = parse_buf(buffer);
                current_size -= read_counter;
                if (size_parse >= 0) {
                    int size_to_move = size - current_size;
                    auto it =  buffer.begin() + size_to_move;
                    temp.insert(temp.begin(), it, buffer.end());
                    buffer.resize(size_parse);
                    std::vector<char> res = 
                        optional<std::vector<char> >(std::move(buffer));
                    buffer = std::move(temp);
                    current_size += size_parse;
                    return std::move(res);
                } 
                read_counter = read(fd, buffer.data() + size - current_size,
                    current_size);
                }
        return optional<std::vector<char>();
        }
    }

    void writenext(std::vector<char> & a) {
        write(fd, a.data(), a.size());
    }

};

int main() {
    lazyStream stream(0, 100, 'a');
    std::vector<char> res = stream.readnext();
    stream.writenext(res);
    return 0;
}
