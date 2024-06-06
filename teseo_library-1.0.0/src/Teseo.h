// Teseo.h
// rev 1 - June 2024 - shabaz - first revision

#ifndef TESEO_H
#define TESEO_H

// ************ includes *************
#include "Arduino.h"
#include <math.h>

// ************ defines **************
// MAX_SAT should really be higher than 24, but to conserve a bit of memory
// it has been set to this value.
#define MAX_SAT 24

// Satellite source enumerations
#define SOURCE_INVALID 99
#define SOURCE_UNK 0
#define SOURCE_GPS 1
#define SOURCE_GALILEO 2
#define SOURCE_BEIDOU 3
#define SOURCE_GLONASS 4

// used for enabling/disabling raw GNSS sentence output to the 
// Arduino Serial Monitor, when using the get_data function.
#define PRINT_ENABLE 1
#define PRINT_DISABLE 0

// these structures are used for decoded information from NMEA sentences
typedef struct gsv_s {
  char source;
  char snr;
  char prn;
  int elev;
  int azim;
} gsv_t;

typedef struct rmc_s {
  char status;
  float lat;
  float lon;
  float speed;
  int sec;
  int min;
  int hour;
  int date;
  int month;
  int year;
} rmc_t;

typedef struct gga_s {
  float alt;
  float geosep;
  char altunit;
  char geounit;
} gga_t;


// *********** class definition **************
class Teseo
{
  public:
    Teseo();
    // call init to set the UART baud rate on the Arduino
    // the GNSS module ordinarily expects 9600 baud.
    void init(long baudrate);
    // flush_buffer() is used to wait until the module is no longer sending any 
    // NMEA sentences. It works by waiting until no UART data is received, and then
    // waits 100 msec more to be completely sure. The 100 msec value can possibly
    // be reduced by the user.
    void flush_buffer(unsigned long wait_ms = 100);
    // get_data is used immediately after flush_buffer. It will wait to fetch new
    // NMEA sentences from the GNSS module, and parse them.
    // The results are stored in the structures such as gsv[], rmc, and gga.
    // This function may take up to 1 second to execute, 
    // if the NMEA sentences repeat every 1 second.
    // To view the raw GNSS sentences on the Arduino Serial Monitor, use
    // get_data(PRINT_ENABLE), otherwise use get_data(PRINT_DISABLE)
    void get_data(int print_enable = 0);
    // print_data sends the decoded content in user-friendly text, to the
    // Arduino Serial Monitor
    void print_data(void);
    // build_checksum expects a string in the format $xxxxxxx* (i.e. beginning
    // with an $ sign and ending with an asterisk) and will return a 2-character string,
    // (null-terminated as any string is), containing a hexadecimal checksum in ASCII.
    void build_checksum(char* line, char* csum_text);
    // send_command expects a string in the format $xxxxxxx (i.e. beginning with 
    // a $ sign, and ending without an asterisk). The function will automatically
    // append an asterisk and checksum and '\r\n', and send that all to the GNSS module.
    void send_command(const char* cmd);
    // The wait_send_complete function simply waits until the serial send operation
    // has completed, i.e. the send buffer is empty.
    void wait_send_complete(void);
    // send_and_read expects a string in the format $xxxxxx (i.e. beginning with
    // a $ sign, and ending without an asterisk), and will send the command, wait for 
    // the send operation to complete, and will fill the response char array with 
    // whatever is returned. The caller needs to provide the response buffer and its
    // length (max_length parameter). Some operations such as NVRAM writes may 
    // need a longer time to wait for a response, and the max_wait_ms parameter
    // can be used to set the maximum wait delay.
    void send_and_read(const char* cmd, char* response, int max_len, unsigned long max_wait_ms = 100);

    // these structures hold the decoded information from the NMEA sentences.
    // they are automatically populated by the get_data function.
    gsv_t gsv[MAX_SAT]; // satellite information
    rmc_t rmc; // date/time/lat/lon/speed
    gga_t gga; // altitude
    uint8_t satnum; // number of satellites

  private:
  
};

#endif // TESEO_H
