#include "reutils.h"
#include "parser.h"

#include <iostream>
#include <cstring>
#include <fstream>
#include <string_view>

#include <unistd.h>

#define toString(x) (x + '0')

void new_socket(int &server)
{

  // creates a new socket
  server = socket(AF_INET, SOCK_STREAM, 0);
  if (server == -1)
  {
    std::cerr << "Failed to create socket!" << "\n";
    exit(1);
  }
}

void SetAndBindServerSocket(sockaddr_in &server, int &file_descriptor_server)
{
  server.sin_family = AF_INET;
  server.sin_port = htons(8080);       // Port number
  server.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces

  if (bind(file_descriptor_server, (sockaddr *)&server, sizeof(server)) == -1)
  {
    std::cerr << "Failed to bind socket!" << "\b";
    close(file_descriptor_server);
    exit(1);
  }

  // Listen for incoming connections
  if (listen(file_descriptor_server, 5) == -1)
  { // Backlog of 5
    std::cerr << "Failed to listen on socket!" << "\n";
    close(file_descriptor_server);
    exit(1);
  }

  std::cout << "Server is listening on port 8080..." << "\n";
}

void ConnectClientSocket(sockaddr_in &client, int &file_descriptor_client, int &file_descriptor_server)
{

  // accepts the request by the client

  socklen_t clientAddressSize = sizeof(client);
  file_descriptor_client = accept(file_descriptor_server, (sockaddr *)&client, &clientAddressSize);
  if (file_descriptor_client == -1)
  {
    std::cerr << "Failed to accept connection!" << "\n";
    close(file_descriptor_server);
    exit(1);
  }

  std::cout << "Client connected!" << "\n";
}

void readDatafile();

void Communicate(int &clientSocket)
{
  // reads the file "data.raotf" aka DATA_FILE_NAME
  readDatafile();

  // creates an ostream obj to append the commands that will be executed by the client
  std::ofstream aot(DATA_FILE_NAME, std::ios_base::app);

  char buffer[1024];
  while (true)
  {
    // Clear the buffer
    memset(buffer, 0, sizeof(buffer));

    // Receive data from the client
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesReceived == -1)
    {
      std::cerr << "Error receiving data!" << "\n";
      break;
    }
    else if (bytesReceived == 0)
    {
      std::cout << "Client disconnected." << "\n";
      break;
    }

    Parser parser;
    std::vector<std::string> v = parser.ParseCommand(buffer);

    // Print the received message
    for (std::string a : v)
    {
      std::cout << a << "\n";
    }

    // processes the commands
    if (v[0] == "GET")
    {
      if (v.size() > 1 && data.count(v[1]) > 0)
      {
        // makes the element in the redis protocol
        std::string response = "+" + data[v[1]] + "\r\n";

        // sends the msg
        send(clientSocket, response.c_str(), response.size(), 0);
      }
      else
      {
        const char *response = "+[nil]\r\n";

        send(clientSocket, response, strlen(response), 0);
      }
    }
    else if (v[0] == "SET")
    {
      // inserts the data

      // checks if the command has valid no:of parameters
      if (v.size() < 3)
      {
        const char *response = "-invalid use of SET\r\n";
        send(clientSocket, response, strlen(response), 0);
        continue;
      }

      // if the key already exists, modifies its contents to the curent
      if (data.contains(v[1]))
        data[v[1]] = v[2];

      else
        data.insert(std::make_pair(v[1], v[2]));

      // prints ok
      const char *response = "+Ok\r\n";
      send(clientSocket, response, strlen(response), 0);

      // stores it in a file
      for (std::string &str : v)
        aot << str << " ";

      // adds a null termination after command
      aot << "\n";
    }
    else if (v[0] == "DEL")
    {
      std::string response;

      // if it exists then will return 1, else 0
      if (v.size() >= 2)
      {
        data.erase(v[1]);
        response = "+(integer)1\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
      }
      else
      {
        response = "+(integer)0\r\n";
        send(clientSocket, response.c_str(), response.size(), 0);
      }

      for (std::string &str : v)
        aot << str << " ";

      // adds a null termination after command
      aot << "\n";
    }
    // just returns the final value
    else if (v[0] == "PING")
    {
      std::string response;
      response = "+" + *(v.end()) + "\r\n";

      send(clientSocket, response.c_str(), response.size(), 0);
    }
    else if (v[0] == "CLOSESERVER")
    {
      exit(0);
    }
    else
    { // will expand it furthur
      const char *response = "-Not a valid operator yet\r\n";
      send(clientSocket, response, strlen(response), 0);
    }
  }
}

void readDatafile()
{
  // reads the file
  std::ifstream in(DATA_FILE_NAME);

  if (!in.is_open())
  {
    std::cerr << "file \"" << DATA_FILE_NAME << "\" is not open"
              << "\n exited with code 1";
    exit(1);
  }

  std::string buffer;
  Parser parser;

  while (std::getline(in, buffer))
  {

    std::vector<std::string> v = parser.ParseData(buffer);

    if (v[0] == "SET")
    {
      // inserts the data

      // checks whether it is valid or not
      if (v.size() < 3)
      {
        continue;
      }

      // if the key exists, modify it to the current one
      if (data.contains(v[1]))
        data[v[1]] = v[2];

      else
        data.insert(std::make_pair(v[1], v[2]));
    }
    else if (v[0] == "DEL")
    {
      // if it is valid then deletes
      if (v.size() >= 2)
      {
        data.erase(v[1]);
      }
    }
  }
}