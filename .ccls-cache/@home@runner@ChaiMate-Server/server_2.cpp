#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <set> // Add this include for the set container

#include "user.h"

using namespace std;

int main() {
    cout << "ChaiMate Server has started" << endl;

    // Create a socket
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1) {
        cerr << "Error creating socket" << endl;
        return -1;
    }

    // Bind the socket to an IP address
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

    if (bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1) {
        cerr << "Can't bind to port" << endl;
        return -2;
    }

    // Marks the socket as a listening socket
    if (listen(listening, SOMAXCONN) == -1) {
        cerr << "Error listening" << endl;
        return -3;
    }

    // Set of connected client sockets
    set<int> clients;

    while (true) {
        fd_set master;
        FD_ZERO(&master);

        // Add the listening socket to the set
        FD_SET(listening, &master);

        // Add connected client sockets to the set
        for (int clientSocket : clients) {
            FD_SET(clientSocket, &master);
        }

        // Wait for activity on any of the sockets
        int activity = select(FD_SETSIZE, &master, nullptr, nullptr, nullptr);

        if (activity == -1) {
            cerr << "Error in select" << endl;
            return -4;
        }

        // New connection
        if (FD_ISSET(listening, &master)) {
            sockaddr_in client;
            socklen_t clientSize = sizeof(client);

            int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

            if (clientSocket == -1) {
                cerr << "Error Accepting Connection" << endl;
            } else {
                clients.insert(clientSocket);

                char host[NI_MAXHOST];
                char service[NI_MAXSERV];

                memset(host, 0, NI_MAXHOST);
                memset(service, 0, NI_MAXSERV);

                int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0);

                if (result) {
                    cout << host << " connected on " << service << endl;
                } else {
                    inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
                    cout << host << " connected on " << ntohs(client.sin_port) << endl;
                }
            }
        }

        // Handle data from clients
        for (auto it = clients.begin(); it != clients.end(); ) {
            int clientSocket = *it;

            if (FD_ISSET(clientSocket, &master)) {
                char buffer[4096];
                memset(buffer, 0, 4096);

                int bytesReceived = recv(clientSocket, buffer, 4096, 0);
                if (bytesReceived <= 0) {
                    // Client disconnected
                    cout << "Client disconnected. Closing socket." << endl;
                    close(clientSocket);
                    it = clients.erase(it);
                } else {
                    // Handle the received data

                  //Accept client connections and set up the socket ...

                    User newUser("", "", "", "", {}, 0); // Declare a dummy user object
                    User loggedInUser("", "", "", "", {}, 0);  // Initialize with empty values


                  while (true){
                    char buffer[4096];
                    memset(buffer, 0, 4096);
                    int bytesReceived = recv(clientSocket, buffer, 4096, 0);
                    if (bytesReceived == -1) {
                        cerr << "Error receiving command" << endl;
                        close(clientSocket);
                        return -5;
                    }

                    if (bytesReceived <= 0) {
                        cout << "Client disconnected. Waiting for another connection..." << endl;
                        break;

                      // Close the current client socket
                          close(clientSocket);

                          // Accept a new connection
                          clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
                          if (clientSocket == -1) {
                              cerr << "Error Accepting Connection" << endl;
                              break;  // Exit the server loop if accepting a new connection fails
                          }

                          continue;  // Continue to the next iteration of the main loop
                      }

                    // Check if the received command is "REGISTER" or "LOGIN"

                      string command(buffer);
                    if (command.substr(0, 4) == "EXIT") {
                        cout << "Received EXIT command. Closing server." << endl;
                        break;  // Exit the server loop
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

                      else if (command.substr(0, 4) == "SHOW") {
                              if (loggedInUser.getName() != "") {
                                  std::string userDetails = loggedInUser.getUserDetails();
                                  send(clientSocket, userDetails.c_str(), userDetails.size() + 1, 0);
                              } else {
                                  cerr << "Error: User not logged in." << endl;
                              }
                          }

                        else {
                        cerr << "Invalid command from client." << endl;
                          continue;
                    }


                  }

                    // Example: Echo the received data back to the client
                    send(clientSocket, buffer, bytesReceived, 0);
                    ++it;
                }
            } else {
                ++it;
            }
        }
    }

    // Close the listening socket
    close(listening);

    return 0;
}
