#include<vector>
#include<iterator>
#include<string>
#include<unistd.h>
#include<stdlib.h>
#include<iostream>
#include<functional>

class Message {
public:
    int size;
    std::vector<char> buf;
    std::string prefix;
    int args;
    char delim;


    Message(int sz, std::string pr, int arg_, char del) : size(sz), prefix(pr),
                    args(arg_), delim(del) {}

    int parse (std::vector<char> &a) {
        buf.insert(buf.begin(), a.begin() + prefix.size(),
                a.begin() + prefix.size() + size);
        return size + prefix.size();
    }
};
class msgstream {
    int fd;
    size_t size;
    size_t current_size;
    std::vector<char> buffer;
    std::vector<Message> commands;

public:
    void add_command(Message &a) {
        commands.push_back(a);
    }
    msgstream(int fd) : fd(fd), current_size(0), size(4096) {
        buffer.resize(size);
    }

    Message get_nex_msg() {
       while (true) {
            if (current_size == 0) {
                int read_count = read(fd, buffer.data() + current_size,
                        size - current_size);
                if (read_count < 0)
                    _exit(1);
                current_size += read_count;
            }
           for (int i = 0; i < commands.size(); i++) {
               bool ok = true;
               for (int j = 0; commands[i].prefix.size(); j++) {
                  if (commands[i].prefix[j] != buffer[j]) {
                      ok = false;
                      break; 
                  }
                  if (ok) {
                    Message msg = commands[i];
                    int to_move = msg.parse(buffer);
                    std::vector<char>::iterator it = buffer.begin() + to_move;
                    std::vector<char> temp(buffer.size());
                    temp.insert(temp.begin(), it, buffer.end());
                    buffer = std::move(temp);
                    current_size -= to_move;
                    return msg;
                  }
               }
           }
           current_size = 0;
       }
    }
};

int main() {
    Message m(5, "MSG", 6, '\n');
    Message m1(0, "PING", 6, '\n');
    Message m2(0, "PONG", 6, '\n');
    msgstream str(0);
    str.add_command(m);
    str.add_command(m1);
    str.add_command(m2);
    while (true) {
        Message res = str.get_nex_msg();
        write(1, res.buf.data(), res.buf.size());
    }
    return 0;
}
