#include<unistd.h>
#include<functional>
#include<stdlib.h>
#include<time.h>
#include<string>
#include<vector>
#include<fcntl.h>
#include<iostream>
#include<iterator>
#include<algorithm>
#include<sstream>

#define my_function std::function<int(std::vector<int>&)>
const int files_size = 2;
const int fan_size = 2;
std::string fan[files_size] = {"/sys/class/hwmon/hwmon0/device/fan1_input",
    "/sys/class/hwmon/hwmon0/device/fan2_input" };
std::string files[fan_size] = {
    "/sys/class/hwmon/hwmon0/temp1_input",
    "/sys/class/hwmon/hwmon0/temp2_input"};

int fun1(std::vector<int> &a) {
    int temperature = 0;
    for (int i = 0; i < a.size(); i++)
        temperature += a[i];
    return temperature / a.size() * 2;
}

int fun2(std::vector<int> &a) {
    int temperature = 0;
    for (int i = 0; i < a.size(); i++)
        temperature += a[i];
    return temperature / a.size();
}

const my_function functions[fan_size] = {fun1, fun2};
int return_number(std::string &a) {
    return atoi(a.data());
}
std::string int_to_str(int num) {
    std::ostringstream ss;
    ss << num;
    return ss.str();
}

int main() {
    if (fork() == 0) {
        setsid();
        int size = 4096;
        std::vector<int> information(2);
        while (true) {
            for (int i = 0; i < files_size; i++) {
                int fd = open(files[i].data(), O_RDONLY);
                if (fd < 0) {
                    _exit(1);
                }
                std::vector<char> buf(size);
                int read_couter = 1;
                while (read_couter > 0) {
                    read_couter = read(fd, buf.data(), size);
                    if (read_couter < 0) {
                        _exit(1);
                    }
                }
                information[i] = atoi(buf.data());
                close(fd);
            }
            for (int i = 0; i < fan_size; i++) {
                int fd = open(fan[i].data(), O_WRONLY, 0644);
                if (fd < 0)
                    _exit(2);
                std::string res = int_to_str(functions[i](information));
                write(fd, res.data(), res.size());
                close(fd);
            }
            sleep(5);
        }
        return 0;
    }
    return 0;
}
