#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

  
using namespace std;

int main() {

  cout << "Server Has Started";

//create a socket
  
    int listening = socket (AF_INET, SOCK_STREAM, 0);
    if (listening == -1){
      cerr << "Error creating socket" << endl;
      return -1;
    }

  
  
//Bind the socket to an IP address

sockaddr_in hint;
hint.sin_family = AF_INET;
hint.sin_port = htons(54000);
inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

  if (bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1){
    cerr << "Cant bind to port" << endl;
    return -2;
  }
  
// Marks the socket as a listening socket
  
if (listen(listening, SOMAXCONN) == -1){
  cerr << "Error listening" << endl;
  return -3;
}
// Accepts incoming connections

sockaddr_in client;
socklen_t clientSize = sizeof(client);


int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

if (clientSocket == -1){
  cerr << "Error Accepting Connection" << endl;
  return -4;
}

//Close Listening Socket

close (listening);

char host[NI_MAXHOST];
char service[NI_MAXSERV];
  
memset(host, 0, NI_MAXHOST);
memset(service, 0, NI_MAXSERV);

int result = getnameinfo((sockaddr*)&client, sizeof(client),host,NI_MAXHOST,service, NI_MAXSERV, 0);

if (result){
  cout << host << "connected on " << service << endl;
}  

else {
  inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
  cout << host << "connected on " << ntohs(client.sin_port) << endl;
}
  
// Receive & Send data to/from the client

char buffer [4096];
/*while (true){
  //Clear the buffer
  memset(buffer, 0, 4096);
  // Wait for a message
  int bytesReceived = recv(clientSocket, buffer, 4096, 0);
  if (bytesReceived == -1){
    cerr << "Error receiving data" << endl;
    break;
  }
  // Display the message
  if (bytesReceived == 0){
    cout << "Client disconnected" << endl;
    break;
  }
  cout << string(buffer, 0, bytesReceived) << endl;
  //Resend the message
  send(clientSocket, buffer, bytesReceived + 1, 0);
  
}
  */
  while (true) {
      // Clear the buffer
      memset(buffer, 0, 4096);
      // Wait for a message
      int bytesReceived = recv(clientSocket, buffer, 4096, 0);
      if (bytesReceived == -1) {
          cerr << "Error receiving data" << endl;
          break;
      }
      // Display the message
      if (bytesReceived == 0) {
          cout << "Client disconnected" << endl;
          break;
      }

      // Parse the received string into two numbers
      int num1, num2;
      sscanf(buffer, "%d %d", &num1, &num2);

      // Add the numbers
      int sum = num1 + num2;

      // Convert the sum to a string
      string sumString = to_string(sum);

      // Send the sum back to the client
      send(clientSocket, sumString.c_str(), sumString.size() + 1, 0);
  }
  

// Close the socket
close(clientSocket);
  
}