#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 80
#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {

  const char *hostname = argv[1];
  struct hostent *server;
  struct sockaddr_in server_addr;
  int sockfd;
  char buffer[BUFFER_SIZE];

  if (argc != 2) {

    fprintf(stderr, "Error: Only one hostname argument is accepted for refferent '%s <hostname>'\n", argv[0]);
    return 1;
  }

  // Resolve Hostname
  if ((server = gethostbyname(hostname)) == NULL) {
    fprintf(stderr, "Error: This host does not exist\n");
    return 1;
  }

  // Create Socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error: Could not create socket");
    return 1;
  }

  // Prepare Server Address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

  // Connect to server
  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Error: Could not connect to server");
    close(sockfd);
    return 1;
  }

  // Send HTTP GET request
  snprintf(buffer, sizeof(buffer),
           "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", hostname);

  if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
    perror("Error: Could not send request");
    close(sockfd);
    return 1;
  }

  // Recieve and Print Response
  int received;
  while ((received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
    buffer[received] = '\0';
    printf("%s", buffer);
  }

  if (received < 0) {
    perror("Error: Could not receive response");
  }

  // Close Socket
  if (close(sockfd) < 0) {
    perror("Error: Could not close socket");
    return 1;
  }

  return 0;
}
