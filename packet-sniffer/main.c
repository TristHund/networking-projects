#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "display.h"

volatile sig_atomic_t stop_sniffer = 0;

void handle_signal(int signal) { stop_sniffer = 1; }

void process_packet(unsigned char *buffer, int size) {
  struct ethhdr *eth = (struct ethhdr *)buffer;

  print_ethernet_header(buffer);

  if (ntohs(eth->h_proto) == ETH_P_IP) {
    print_ip_header(buffer);
    struct iphdr *iph = (struct iphdr *)(buffer + sizeof(struct ethhdr));

    // Filter out multicast traffic
    if ((ntohl(iph->daddr) & 0xF0000000) == 0xE0000000) {
      // This is a multicast address
      return;
    }

    switch (iph->protocol) {
    case IPPROTO_TCP:
      print_tcp_header(buffer);
      break;
    case IPPROTO_UDP:
      print_udp_header(buffer);
      break;
    default:
      printf("Unknown Protocol\n");
      break;
    }
  } else {
    printf("Not an IP Packet\n");
  }
}

int main() {
  int raw_sock;
  struct sockaddr saddr;
  unsigned char *buffer = (unsigned char *)malloc(65536);
  int saddr_len = sizeof(saddr);

  // Register the signal handler for SIGINT
  signal(SIGINT, handle_signal);

  // Create a raw socket
  raw_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (raw_sock < 0) {
    perror("Socket Error");
    return 1;
  }

  // Set the interface to promiscuous mode
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, "wlan0",
          IFNAMSIZ - 1); // Replace "wlan0" with your interface name <- maybe
                         // there is a way to get this with code
  if (ioctl(raw_sock, SIOCGIFFLAGS, &ifr) == -1) {
    perror("Error getting interface flags");
    close(raw_sock);
    return 1;
  }
  ifr.ifr_flags |= IFF_PROMISC;
  if (ioctl(raw_sock, SIOCSIFFLAGS, &ifr) == -1) {
    perror("Error setting interface flags");
    close(raw_sock);
    return 1;
  }

  while (!stop_sniffer) {
    // Receive a packet
    int data_size =
        recvfrom(raw_sock, buffer, 65536, 0, &saddr, (socklen_t *)&saddr_len);
    if (data_size < 0) {
      perror("Recvfrom Error");
      break;
    }

    // Process the packet
    process_packet(buffer, data_size);
  }

  printf("Exiting...\n");

  // Disable promiscuous mode
  ifr.ifr_flags &= ~IFF_PROMISC;
  if (ioctl(raw_sock, SIOCSIFFLAGS, &ifr) == -1) {
    perror("Error unsetting promiscuous mode");
  }

  close(raw_sock);
  free(buffer);
  return 0;
}
