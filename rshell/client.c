#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pty.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <termios.h>

#define BUF_SIZE 500
int signal_size_changed = 1;

void my_exit (char ** buffer, int fd, int code)
{

    close(fd);
    free(buffer[0]);
    free(buffer[1]);
    free(buffer);
    exit(code);
}

void signal_window(int signum)
{
    signal_size_changed = 1;
}

int main (int argc, char ** argv)
{
    signal(SIGWINCH, signal_window);
    struct addrinfo hints;
    struct addrinfo * result;
    int sfd;


    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;

    int s = getaddrinfo(NULL, "8822", &hints, &result);

    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    sfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    connect(sfd, result->ai_addr, result->ai_addrlen);

    char buffer[2][4096];
    int buffer_size[2];
    buffer_size[0] = 0;
    buffer_size[1] = 0;
    struct pollfd fds[3];
    int ret;
    int end[2];
    end[0] = 0;
    end[1] = 0;

    fds[0].fd = 0;
    fds[0].events = POLLIN | POLLERR | POLLHUP | POLLNVAL;
    fds[1].fd = sfd;
    fds[1].events = POLLIN | POLLERR | POLLHUP | POLLNVAL;
    fds[2].fd = 1;
    fds[2].events = POLLERR | POLLHUP | POLLNVAL;

    while (!end[0] || !end[1] || buffer_size[0] || buffer_size[1])
    {
        ret = poll(fds, 3, -1);

        if (ret > 0)
        {
            int i;

            for (i = 0; i < 2; i++)
            {
                int j = i == 0 ? 1 : 0;

                if (fds[i].revents & POLLIN)
                {
                    int read_result = read(fds[i].fd, buffer[i] + buffer_size[i], 4095 - buffer_size[i]);

                    if (read_result == 0)
                    {
                        close(fds[i].fd);
                        end[i] = 1;
                    }
                    else if (read_result < 0)
                        if (errno != EWOULDBLOCK && errno != EAGAIN)
                        {
                            fprintf(stderr, "read exit");
                            my_exit(buffer, fds[i].fd, 1);
                        }

                    if (read_result > 0)
                    {
                        buffer_size[i] += read_result;
                    }
                }

                j = i + 1;

                if (fds[j].revents & POLLOUT)
                {
                    int write_result = write(fds[j].fd, buffer[i], buffer_size[i]);

                    if (write_result < 0)
                        if (errno != EWOULDBLOCK && errno != EAGAIN)
                        {
                            fprintf(stderr, "write exit");
                            my_exit(buffer, fds[j].fd, 1);
                        }

                    memmove(buffer[i], buffer[i] + write_result, buffer_size[i] - write_result);

                    buffer_size[i] -= write_result;
                }

                //    if (buffer_size[1] > 0)
                //    {
                //        fds[2].events = fds[2].events | POLLOUT;
                //    }
                //    else
                //    {
                //        if (fds[2].events & POLLOUT)
                //        {
                //            fds[2].events = fds[2].events ^ POLLOUT;

                //            if (end)
                //            {
                //                fprintf(stderr, "end exit");
                //                my_exit(buffer, fds[j].fd, 1);
                //            }
                //        }
                //    }


                if (buffer_size[i] > 0)
                {
                    fds[j].events = fds[j].events | POLLOUT;
                }
                else
                {
                    if (fds[j].events & POLLOUT)
                    {
                        if (!end[i])
                        {
                            fds[j].events = fds[j].events ^ POLLOUT;
                        }
                        else
                        {
                            fprintf(stderr, "end exit");
                            my_exit(buffer, fds[j].fd, 1);
                        }
                    }
                }

                int k = 0;

                for (k = 0; k < 2; k++)
                {
                    if (end[k])
                    {
                        if (fds[k].events & POLLOUT)
                        {
                            fds[k].events = fds[k].events ^ POLLOUT;
                        }
                    }
                }

            }

            if (signal_size_changed)
            {
                //	fprintf(stderr, "window");
                //    const int ttyfd = open("/dev/tty", O_RDONLY);
                //    struct winsize ws;

                //    if (ioctl(ttyfd, TIOCGWINSZ, &ws) != 0)
                //    {
                //        fprintf(stderr, "ioctl: error");
                //        exit(EXIT_FAILURE);
                //    }

                //    if (buffer_size[0] + 6 < 4096)
                //    {
                //        buffer[0][buffer_size[0]] = 's';
                //        buffer_size[0]++;
                //        *(short *)(buffer[0] + buffer_size[0]) = ws.ws_row;
                //        buffer_size[0] += 2;
                //        *(short *)(buffer[0] + buffer_size[0]) = ws.ws_col;
                //        buffer_size[0] += 2;
                //        buffer[0][buffer_size[0]] = '\0';
                //        buffer_size[0]++;
                //        signal_size_changed = 0;
                //    }
            }

            for (i = 0; i < 3; i++)
            {
                int const else_polls = POLLERR | POLLHUP | POLLNVAL;

                if (fds[i].revents & else_polls)
                {
                    fprintf(stderr, "error exit");
                    my_exit(buffer, fds[i].fd, 1);
                }
            }
        }
        else
        {
            fprintf(stderr, "nornal exit");
            break;
        }
    }

    free(buffer[0]);
    free(buffer[1]);
    free(buffer);
    fprintf(stderr, "nornal exit");
    exit(0);

    return 0;
}
