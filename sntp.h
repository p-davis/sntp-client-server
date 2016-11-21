#include <time.h>

#define UNIX_EPOCH 0x83AA7E80;  // the seconds from Jan 1, 1900 to Jan 1, 1970

typedef struct {
  uint32_t second;
  uint32_t fraction;
} ntp_timestamp;

struct timeval tv;
ntp_timestamp ntp_t;

ntp_timestamp getCurrentTimestamp()
{
  ntp_timestamp current;
  struct timeval tv;
  gettimeofday(&tv, NULL);

  convert_unix_to_ntp(&tv, &current);

  return current;
}

typedef struct {
  unsigned li     : 2;        /* Two-bit Leap indicator */
  unsigned vn     : 3;        /* Three-bit version number indicator */
  unsigned mode   : 3;        /* Three-bit protocol number */
  uint8_t stratum;            /* Eight-bit stratum indicator */
  uint8_t pollInterval;       /* Eight-bit max poll interval */
  uint8_t precision;           /* Eight-bit precision (in seconds) */

  /* Fixed-point indicating total roundtrip delay to primary reference source */
  uint32_t rootDelay;

  /* Fixed-point indicating max error due to clock freq tolerance in seconds*/
  uint32_t rootDispersion;

  /* 32-bit bitstring identifying the ref source */
  uint32_t refIdentifier;

  /* time system clock was last set or corrected, in 64-bit timestamp format*/
  ntp_timestamp refTimestamp;

  /* time request departed the client for server, in 64-bit timestamp format*/
  ntp_timestamp orgTimestamp;

  /* time the request arrived at server or the reply arrived at client */
  ntp_timestamp recvTimestamp;

  /* time the request departed the client or the reply departed the server*/
  ntp_timestamp transmitTimestamp;

} ntp_packet;

void print_unix_time(struct timeval tv) {
    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64];

    nowtime = tv.tv_sec;
    nowtm = localtime(&nowtime);
    strftime(tmbuf, sizeof (tmbuf), "%Y-%m-%d %H:%M:%S", nowtm);
    printf("%s.%06d\n", tmbuf, (int)tv.tv_usec);
}

void convert_ntp_to_unix(ntp_timestamp *ntp, struct timeval *unix_time)
{
    unix_time->tv_sec = ntp->second - UNIX_EPOCH;
    unix_time->tv_usec = (uint32_t)( (double)ntp->fraction * 1.0e6 / (double)(1LL<<32) );
}

void convert_unix_to_ntp(struct timeval *unix_time, ntp_timestamp *ntp)
{
    ntp->second = unix_time->tv_sec + UNIX_EPOCH;
    ntp->fraction = (uint32_t)( (double)(unix_time->tv_usec+1) * (double)(1LL<<32) * 1.0e-6 );
}
