#include <unistd.h>
#include <arpa/inet.h>

#include <cstdlib>
#include <thread>

#include "reutils.h"

static int serverSocket;
static volatile int clientno = 0;

void closeSocket()
{
  close(serverSocket);
}


int main()
{
  newSocket(serverSocket);

  sockaddr_in serverAddress; //information of the server
  setAndBindServerSocket(serverAddress, serverSocket);
  
  atexit(closeSocket);

  std::thread shutserver(closeServer, std::ref(serverSocket));
  manageClient(serverSocket, 0);

  shutserver.join();

  return 0;
}

