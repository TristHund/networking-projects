#ifndef DISPLAY_H
#define DISPLAY_H

void print_ethernet_header(const unsigned char *buffer);
void print_ip_header(const unsigned char *buffer);
void print_tcp_header(const unsigned char *buffer);
void print_udp_header(const unsigned char *buffer);
void print_payload(const unsigned char *payload, int size);

#endif // DISPLAY_H
