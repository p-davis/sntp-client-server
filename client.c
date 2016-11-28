/*  client.c - SNTP client
 *
 */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include "sntp.h"

#define PORT 123         /* server port the client connects to */

int main(int argc, char const *argv[]) {
  int sockfd, numbytes;
  struct hostent *he;            /* server data struct */
  struct sockaddr_in their_addr; /* server addr info */

  // NTP server address
  char serverIP[] = "0.uk.pool.ntp.org";
  //char serverIP[] = "ntp.uwe.ac.uk"
  
  // Initialise NTP packet struct
  ntp_packet packet;

  // zero packet struct
  memset(&packet, 0, sizeof(packet));

  /* set up flag bitfield of struct, so li is 0, vn is 4, mode is 3 */
  // li
  packet.flags = 0;
  packet.flags <<= 3;
  // vn
  packet.flags |= 4;
  packet.flags <<= 3;
  // mode
  packet.flags |= 3;


  /*if(argc != 2) {
    fprintf(stderr, "usage: client serverIP\n");
    exit(1);
  }*/

  /* resolve server host name or IP address */
  if((he = gethostbyname(serverIP)) == NULL) {
    perror("client gethostbyname");
    exit(1);
  }

  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("client socket");
    exit(1);
  }

  printf("Sending..\n");

  gettimeofday(&tv, NULL);
  print_unix_time(&tv);

  memset(&their_addr, 0, sizeof(their_addr));
  their_addr.sin_family = AF_INET;
  their_addr.sin_port = htons(PORT);
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);

  packet.transmitTimestamp = getCurrentTimestamp();

  host_to_network(&packet);

  if((numbytes = sendto(sockfd, &packet, sizeof(ntp_packet), 0,
      (struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
    perror("client sendto");
    exit(1);
  }

  printf("\n\nReceiving..\n");

  socklen_t addr_len = (socklen_t)sizeof(struct sockaddr);

  ntp_packet recvBuf;
  memset(&recvBuf, 0, sizeof(recvBuf));

  if((numbytes = recvfrom(sockfd, &recvBuf, sizeof(ntp_packet), 0,
    (struct sockaddr *) &their_addr, &addr_len)) == -1) {
    perror("listener recv from");
    exit(1);
}

ntp_timestamp destTimestamp = getCurrentTimestamp();

network_to_host(&recvBuf);

printf("\nReceived:\n");

double offset = calculate_offset(&recvBuf, &destTimestamp);
double delay = calculate_delay(&recvBuf, &destTimestamp);
int mode = recvBuf.flags & 0x07;

print_unix_time(&tv);
printf("%+f +/- %f s%d ", offset, delay, recvBuf.stratum);
printf("Mode: %d\n", mode);

char str[INET_ADDRSTRLEN];
inet_ntop(AF_INET, &(their_addr.sin_addr), str, INET_ADDRSTRLEN);
printf("%s %s\n", serverIP, str);

  return 0;
}
