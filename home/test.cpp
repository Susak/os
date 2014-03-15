#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <time.h>
#include <chrono>
#include <thread>
#include <iostream>
size_t buf_size = 1024;
std::vector<char> buf;
size_t len;
 
int main(int argc, char * argv[]) {
   if (argc < 2) {
       return 1;
   }
   int n = atoi(argv[1]);
   std::queue<std::chrono::high_resolution_clock::time_point> times;
 
   buf.resize(buf_size);
   len = 0;
   int count, eof = 0;
   while (!eof) {
       std::chrono::high_resolution_clock::time_point time =
           std::chrono::high_resolution_clock::now();
       while(!times.empty() && std::chrono::duration_cast<std::chrono::milliseconds>(time
                   - times.front()).count() > 1000) {
           times.pop();
       }
       if (times.size() >= n) {
           std::this_thread::sleep_for(times.back() - times.front());
       } else {
           int pos;
           while (1) {
               pos = -1;
               int i = 0;
               for (; i < len; i++) {
                   if (buf[i] == '\n') {
                       pos = i;
                       break;
                   }
               }
               if (pos != -1) {
                   break;
               }
               int count = write(1, buf.data(), len);
               len -= count;
               int r = read(0, buf.data() + len, buf_size - len);
               if (r == 0) {
                   break;
               }
               len = r;
           }
           int count = write(1, buf.data(), pos + 1);
           len -= count;
           times.push(time);
       }
   }
}
