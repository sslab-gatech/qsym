#include "common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  int a, b;
  int socket_fds[2];
  int res = socketpair(AF_UNIX, SOCK_STREAM, 0, socket_fds);
  read(0, &a, sizeof(a));
  send(socket_fds[1], &a, sizeof(a), 0);
  recv(socket_fds[0], &b, sizeof(b), 0);

  if (b == 0xdeadbeef)
    good();
  else
    bad();
}
