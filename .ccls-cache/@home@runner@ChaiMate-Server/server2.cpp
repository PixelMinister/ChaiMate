#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <vector>
#include <set>
#include <sys/select.h>
#include <sys/time.h>
#include <map>

#include "user.h"

using namespace std;

int main() {
    cout << "ChaiMate Server has started" << endl;

  set<int> pendingRequests;
  map<int, string> requesters;
  map<int, vector<int>> acceptedRequests;

    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1) {
        cerr << "Error creating socket" << endl;
        return -1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

    if (bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1) {
        cerr << "Can't bind to port" << endl;
        return -2;
    }

    if (listen(listening, SOMAXCONN) == -1) {
        cerr << "Error listening" << endl;
        return -3;
    }

    fd_set master;
    FD_ZERO(&master);
    FD_SET(listening, &master);
    int maxSocket = listening;

    set<int> clients;
    vector<User> loggedInUsers;  // Maintain a vector of loggedInUsers, one for each client

    while (true) {
        fd_set copy = master;
        int activity = select(maxSocket + 1, &copy, nullptr, nullptr, nullptr);

        if (activity == -1) {
            cerr << "Select error" << endl;
            return -4;
        }

        // New connection
        if (FD_ISSET(listening, &copy)) {
            int clientSocket = accept(listening, nullptr, nullptr);

            if (clientSocket == -1) {
                cerr << "Error Accepting Connection" << endl;
            } else {
                clients.insert(clientSocket);

                char host[NI_MAXHOST];
                char service[NI_MAXSERV];

                memset(host, 0, NI_MAXHOST);
                memset(service, 0, NI_MAXSERV);

                sockaddr_in client;
                socklen_t clientSize = sizeof(client);
                (void)clientSize;

                int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0);

                if (result) {
                    cout << host << " connected on " << service << endl;
                } else {
                    inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
                    cout << host << " connected on " << ntohs(client.sin_port) << endl;
                }

                FD_SET(clientSocket, &master);
                if (clientSocket > maxSocket) {
                    maxSocket = clientSocket;
                }

                loggedInUsers.push_back(User("", "", "", "", {}, 0));  // Add a new loggedInUser for the new client
            }
        }

        // Handle data from clients
        for (auto it = clients.begin(); it != clients.end(); ) {
            int clientSocket = *it;

            if (FD_ISSET(clientSocket, &copy)) {
                char buffer[4096];
                memset(buffer, 0, 4096);
                int bytesReceived = recv(clientSocket, buffer, 4096, 0);

                if (bytesReceived <= 0) {
                    cout << "Client disconnected" << endl;
                    close(clientSocket);
                    it = clients.erase(it);
                    loggedInUsers.erase(loggedInUsers.begin() + distance(clients.begin(), it));  // Remove corresponding loggedInUser
                    continue;  // Skip the rest of the loop iteration
                }

                User& loggedInUser = loggedInUsers[distance(clients.begin(), it)];  // Get the loggedInUser for this client

                string command(buffer);
                if (command.substr(0, 4) == "EXIT") {
                    cout << "Received EXIT command. Closing connection." << endl;
                    close(clientSocket);
                    it = clients.erase(it);
                    loggedInUsers.erase(loggedInUsers.begin() + distance(clients.begin(), it));  // Remove corresponding loggedInUser
                    continue;  // Skip the rest of the loop iteration

                }
              else if (command.substr(0, 8) == "REGISTER") {
                      memset(buffer, 0, 4096);
                      bytesReceived = recv(clientSocket, buffer, 4096, 0);
                      if (bytesReceived == -1) {
                          cerr << "Error receiving registration data" << endl;
                          close(clientSocket);
                          continue;
                      }

                      stringstream ss(buffer);
                      string name, password, department, city, interest1, interest2, interest3;
                      int degree;
                      ss >> name >> password >> department >> city >> interest1 >> interest2 >> interest3 >> degree;

                      User newUser(name, password, department, city, {interest1, interest2, interest3}, degree);
                      if (newUser.registerUser()) {
                          cout << "Registration successful. User logged in." << endl;
                          newUser.loginUser(); // Automatically log in the registered user
                          loggedInUser = newUser;
                          send(clientSocket, "REGISTRATION_SUCCESS", 20, 0);
                      } else {
                          cout << "Registration failed" << endl;
                      }
              }
                  else if (command.substr(0, 5) == "LOGIN") {
                  // If the command is a "LOGIN" command, receive username and password and call login function
                  memset(buffer, 0, 4096);
                  bytesReceived = recv(clientSocket, buffer, 4096, 0);


                  if (bytesReceived == -1) {
                      cerr << "Error receiving login data" << endl;
                      close(clientSocket);
                      return -7;
                  }

                  // Parse the received data into username and password
                  stringstream ss(buffer);
                  string username, password;
                  ss >> username >> password;

                  // Example usage of User class for login
                  User loginUser(username, password, "", "", {}, 0);
                  if (loginUser.loginUser()) {
                      cout << "Login successful" << endl;
                      loggedInUser = loginUser;
                      send(clientSocket, "LOGIN_SUCCESS", 15, 0);
                  } else {
                      cout << "Login failed" << endl;
                      send(clientSocket, "LOGIN_FAILURE", 15, 0);
                    continue;
                  }
              }

                                else if (command.substr(0, 7) == "REQUEST") {
                                    if (loggedInUser.getName() != "") {
                                        // Notify all other clients about the request
                                        string requestMessage = loggedInUser.getName() + " is looking for a ChaiMate, would you like to accept: Yes/No";
                                        for (int otherSocket : clients) {
                                            if (otherSocket != clientSocket) {
                                                send(otherSocket, requestMessage.c_str(), requestMessage.size() + 1, 0);
                                            }
                                        }

                                        // Receive responses from other clients
                                        vector<int> acceptedClients;
                                        for (int otherSocket : clients) {
                                            if (otherSocket != clientSocket) {
                                                memset(buffer, 0, 4096);
                                                bytesReceived = recv(otherSocket, buffer, 4096, 0);
                                                if (bytesReceived > 0) {
                                                    string response(buffer);
                                                    if (response.find("Yes") == 0) {  // Check if the response starts with "Yes"
                                                        // Add the client to the list of accepted clients
                                                        acceptedClients.push_back(otherSocket);
                                                    }
                                                }
                                            }
                                        }

                                        // Inform the original requester about the accepted requests
                                        if (!acceptedClients.empty()) {
                                            send(clientSocket, "Your request has been accepted.", 30, 0);

                                            // Provide a sample location and time to all accepted clients
                                            string locationMessage = "Tasty Spot at 4:30 PM";
                                            for (int acceptedSocket : acceptedClients) {
                                                send(acceptedSocket, locationMessage.c_str(), locationMessage.size() + 1, 0);
                                            }
                                        } else {
                                            // If no requests were accepted, inform the original requester
                                            // Comment out the line below to prevent notifying the requester about denial
                                            // send(clientSocket, "Your request was not accepted.", 30, 0);
                                        }
                                    }
                                }
                                
                  else {
                  cerr << "Invalid command from client." << endl;
                    continue;
              }
                // Rest of your existing code for command processing goes here...
             

              

            }

            ++it;
        }
    }

    close(listening);

    return 0;
}
