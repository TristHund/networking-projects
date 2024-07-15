
### **Lets get this party started**

To improve my understanding of web clients work and other network applications I decided to start building them while also learning how to code in C. I will walk through a Web Client program I made and explain every piece at a fundamental level. Before reading this or coding along, I think it would be beneficial to have some basic understanding of networking, C and how to compile and run a C program on your system. From there, I will try to explain everything in a way that is easy to understand.

#### **Sprint planning**

Lets break everything down by the parts that we need to include in the Web Socket:

1. A way to accept the host name we are connecting to as an argument and validating this is a proper hostname.
2. A socket created to receive the data from the web host
3. A connection to the host server
4. A way to make a request to the host server
5. Last a way to receive and print the data out on the console 

### **Lets get cooking**

First we need to define some of the key variables that will be used. We want to connect to a destination website, which will be the main (and for now), only parameter we accept into our main function:

`const char *hostname = argv[1];`

When connecting to the host we will want to get server details. To do this we will use the [`hostent` struct defined in `<netdb.h>`](https://pubs.opengroup.org/onlinepubs/7908799/xns/netdb.h.html):

`struct hostent *server;`

Keep in mind for the above, since we are dealing with fairly dynamic data we are using pointers to avoid issues in value assignment and efficiently allocate memory.

The next variable defined is going to be where store server address information. To do this we will be using the [sockaddr_in]((https://web.stanford.edu/class/archive/cs/cs110/cs110.1182/autumn-2017/lectures/15-networking-socket-hierarchy.html#(1)) type, a struct that is dedicated to IPv4 address/port pairs.

`struct sockaddr_in server_addr;`

Then we will declare the socket file descriptor, the common naming convention for this is **sockfd**(socket file descriptor):

`int sockfd;`

Then the buffer is declared to store the data being received from the server, where the BUFFER_SIZE (here being the standard 4kb, which is the typical size of a memory page on many systems:

`char buffer[BUFFER_SIZE];`

### **Main Function**
And now for our first bit of code in the main function, here we will want to validate command line arguments being passed through and make sure we are getting exactly 1 otherwise an error should be returned and the program should not try to execute.

```
int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Error: Only one hostname argument is accepted for '%s <hostname>'\n", argv[0]);
    return 1;
  }
```

#### Resolve the hostname
The next thing we will want to do is create code to handle resolving the hostname and provide an error if the host does not exist, to do this we will use the function [gethostbyname](https://www.man7.org/linux/man-pages/man3/gethostbyname.3.html):

```
  if ((server = gethostbyname(hostname)) == NULL) {
    fprintf(stderr, "Error: This host does not exist\n");
    return 1;
  }
```

### Create Socket

We now will need to create a socket. Real quick in case you are a newb like me and need a quick filler in on sockets, a socket is basically just some endpoint for sending and receiving data across a network. In C the library we will be using `<sys/socket.h>` (If you are on windows, winsock.h ) and function [socket](https://pubs.opengroup.org/onlinepubs/009604599/functions/socket.html) will be used to streamline our socket creation.

The socket function will be assigned to `sockfd` and it accepts 3 parameters, first for the domain, second for the type and third to indicate the protocol. For our domain we will be using the value [AF_INET](https://stackoverflow.com/a/1594039) which is used to specify the address family of IPv4 addresses. For the socket type we are using [SOCK_STREAM](https://stackoverflow.com/a/10810040) which is specifying that this is a TCP socket. And last for the protocol we are using 0 which tells the system that it can choose the default protocol for our socket type which as mentioned just a sentence ago it will be TCP. 

Last we include some error handling incase the socket could not be created.

```
  // Create Socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error: Could not create socket");
    return 1;
  }
```

#### Prepare the server Address
Before we make the connection we first need to access the server IP address. This is needed so the socket knows where to connect. Lets start by setting all bytes in the server address, server_addr, to 0 this accomplishes the initialization of the structure to a known state. To accomplish this we will use the [memset](https://www.geeksforgeeks.org/memset-c-example/) function. The memset function accepts 3 variables:

1. A [void pointer](https://www.geeksforgeeks.org/void-pointer-c-cpp/) to the starting address of memory that needs to be filled
2. The value that will be filled into the starting memory address
3. The number of bytes to be filled 

`memset(&server_addr, 0, sizeof(server_addr));`

After that we are going to define the address family and port number. The address family will be set to AF_INET as we earlier discussed this means that the socket will use IPv4 addresses. For the port number we are going to use a PORT variable set to 80 which is the standardized port for HTTP. We will use a function htons (host to network short)  to format the port number into the network byte order ([big-endian](https://www.freecodecamp.org/news/what-is-endianness-big-endian-vs-little-endian/)) 

```
 server_addr.sin_family = AF_INET;
 server_addr.sin_port = htons(PORT);
```

Now to copy into the server_addr the IP address of the server using the memcpy function. This function will need a destination provided (Pointer to the memory location we will copy into), the source (pointer to the memory location we are copying from) and the count which indicates the number of bytes to copy.

In the function we will be designating the IPv4 IP address location we will be copying into using &server_addr.sin_addr.s_addr, here the sin_addr is the IP address in the socket and s_addr is the variable that is holding the information about the address we are agreeing to accept.. So all together `&server.sin_addr.s_addr` is the address in memory of the server IP address variable we want to connect to.

`server->h_addr` is saying lets get the first address associated with the server (this is what we will store in our server_addr variable) and last we will get the length of that address with `server`->h_length. h_addr and h_length are defined as parts of the hostent struct

`memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);`

#### Connecting to the Server
After this we have our Server address and continue on to connect to the server. To do this we use the [connect](https://www.man7.org/linux/man-pages/man2/connect.2.html) function. The connect function takes in 3 parameters, the socket identified in `sockfd` to initiate the connection, the server address to determine the target IP address and port and then the address length (`addrlen`) to provide the size of the server address data.

Since we are using a TCP connection the function will initiate a threeway handshake with the server. The client will send a syn packet to the server, the server will respond with a SYN-ACK packet and then the client will send an ACK packet back to the server to complete the handshake.

The arguments input are fairly straightforward aside from the target server address, the expected type to be passed in is `const struct sockaddr *` the server_addr is of type `struct sockaddr_in`. To get in the compatible type we will have to cast into the `struct sockaddr *`. 

Of course we will add in error handling and close the socket on failure.

```
  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Error: Could not connect to server please check hostname");
    close(sockfd);
    return 1;
  }
```

#### HTTP GET Request
Next we are going to send a GET request to the server. We will construct a simple GET HTTP request and format using `snprintf` and  store the the string in the buffer variable then use the [send]() function because we are already connected to another socket. We will use the `sockfd`, the buffer value and the buffer length to input in to the function and send to the server for a response. 

```
 snprintf(buffer, sizeof(buffer),
           "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", hostname);

  if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
    perror("Error: Could not send request");
    close(sockfd);
    return 1;
  }
```

#### Receive and Print Response
For the final part we will be receiving a response from the server and printing out the data. We define out the variable `received` which will store the number of bytes received. Then we set a while loop so as to continuously receive data from the server until there is no data left to receive. 

The [recv](https://www.man7.org/linux/man-pages/man2/recv.2.html) function is used for the process of receiving the data from the socket. The `buffer[received]` will fill up with data and then will be printed out to the console.

```
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

```


### Putting it all Together

```
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

    fprintf(stderr, "Error: Only one hostname argument is accepted for '%s <hostname>'\n", argv[0]);
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

```


### Final

I hope you got something out of this tutorial, again I am still learning and wrote this as a way of compiling research and explaining and understanding this theme. If you have any thoughts on something that can be done better in the code or clarified in more detail please leave a comment or send me a message.
