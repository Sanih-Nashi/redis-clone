#pragma once

#include <arpa/inet.h>

#include <unordered_map>
#include <string>
#include <array>

#define DATA_FILE_NAME "data.raotf"
#define CLIENT_MAX 5

// #define DEV_AND_DEBUG


void newSocket(int &server);

void setAndBindServerSocket(sockaddr_in &server, int &file_descriptor_server);

bool connectClientSocket(sockaddr_in &client, int &file_descriptor_client, const int &file_descriptor_server);

void Communicate(int &clientSocket);

void manageClient(const int &serverSocket,const int clientno);

void closeServer(int &serverSocket);

inline std::unordered_map<std::string, std::string> Data;
inline std::array<int, CLIENT_MAX> clientFile;
