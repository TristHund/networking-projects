#include <arpa/inet.h>
#include <ctype.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <stdio.h>

#include "display.h"

void print_ethernet_header(const unsigned char *buffer) {
  struct ethhdr *eth = (struct ethhdr *)buffer;

  printf("\nEthernet Header\n");
  printf("\t|-Destination Address : %02x:%02x:%02x:%02x:%02x:%02x\n",
         eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3],
         eth->h_dest[4], eth->h_dest[5]);
  printf("\t|-Source Address      : %02x:%02x:%02x:%02x:%02x:%02x\n",
         eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3],
         eth->h_source[4], eth->h_source[5]);
  printf("\t|-Protocol            : %u\n", (unsigned short)eth->h_proto);
}

void print_ip_header(const unsigned char *buffer) {
  struct iphdr *iph = (struct iphdr *)(buffer + sizeof(struct ethhdr));
  struct sockaddr_in source, dest;
  memset(&source, 0, sizeof(source));
  source.sin_addr.s_addr = iph->saddr;
  memset(&dest, 0, sizeof(dest));
  dest.sin_addr.s_addr = iph->daddr;

  printf("\nIP Header\n");
  printf("\t|-IP Version        : %d\n", (unsigned int)iph->version);
  printf("\t|-IP Header Length  : %d DWORDS or %d Bytes\n",
         (unsigned int)iph->ihl, ((unsigned int)(iph->ihl)) * 4);
  printf("\t|-Type Of Service   : %d\n", (unsigned int)iph->tos);
  printf("\t|-IP Total Length   : %d Bytes (Size of Packet)\n",
         ntohs(iph->tot_len));
  printf("\t|-Identification    : %d\n", ntohs(iph->id));
  printf("\t|-TTL               : %d\n", (unsigned int)iph->ttl);
  printf("\t|-Protocol          : %d\n", (unsigned int)iph->protocol);
  printf("\t|-Checksum          : %d\n", ntohs(iph->check));
  printf("\t|-Source IP         : %s\n", inet_ntoa(source.sin_addr));
  printf("\t|-Destination IP    : %s\n", inet_ntoa(dest.sin_addr));
}

void print_tcp_header(const unsigned char *buffer) {
  struct iphdr *iph = (struct iphdr *)(buffer + sizeof(struct ethhdr));
  struct tcphdr *tcph =
      (struct tcphdr *)(buffer + iph->ihl * 4 + sizeof(struct ethhdr));

  printf("\nTCP Header\n");
  printf("\t|-Source Port      : %u\n", ntohs(tcph->source));
  printf("\t|-Destination Port : %u\n", ntohs(tcph->dest));
  printf("\t|-Sequence Number    : %u\n", ntohl(tcph->seq));
  printf("\t|-Acknowledge Number : %u\n", ntohl(tcph->ack_seq));
  printf("\t|-Header Length      : %d DWORDS or %d BYTES\n",
         (unsigned int)tcph->doff, (unsigned int)tcph->doff * 4);
  printf("\t|----------Flags-----------\n");
  printf("\t\t|-Urgent Flag          : %d\n", (unsigned int)tcph->urg);
  printf("\t\t|-Acknowledgement Flag : %d\n", (unsigned int)tcph->ack);
  printf("\t\t|-Push Flag            : %d\n", (unsigned int)tcph->psh);
  printf("\t\t|-Reset Flag           : %d\n", (unsigned int)tcph->rst);
  printf("\t\t|-Synchronise Flag     : %d\n", (unsigned int)tcph->syn);
  printf("\t\t|-Finish Flag          : %d\n", (unsigned int)tcph->fin);
  printf("\t|-Window         : %d\n", ntohs(tcph->window));
  printf("\t|-Checksum       : %d\n", ntohs(tcph->check));
  printf("\t|-Urgent Pointer : %d\n", tcph->urg_ptr);
}

void print_udp_header(const unsigned char *buffer) {
  struct iphdr *iph = (struct iphdr *)(buffer + sizeof(struct ethhdr));
  struct udphdr *udph =
      (struct udphdr *)(buffer + iph->ihl * 4 + sizeof(struct ethhdr));

  printf("\nUDP Header\n");
  printf("\t|-Source Port      : %d\n", ntohs(udph->source));
  printf("\t|-Destination Port : %d\n", ntohs(udph->dest));
  printf("\t|-UDP Length       : %d\n", ntohs(udph->len));
  printf("\t|-UDP Checksum     : %d\n", ntohs(udph->check));
}

void print_payload(const unsigned char *payload, int size) {
  int i, j;
  printf("\nPayload (%d bytes):\n", size);
  for (i = 0; i < size; i++) {
    if (i % 16 == 0)
      printf("   ");
    printf("%02X ", payload[i]);
    if (i % 16 == 15 || i == size - 1) {
      for (j = 0; j < 15 - i % 16; j++)
        printf("   ");
      printf("   ");
      for (j = i - i % 16; j <= i; j++) {
        printf("%c", isprint(payload[j]) ? payload[j] : '.');
      }
      printf("\n");
    }
  }
}
