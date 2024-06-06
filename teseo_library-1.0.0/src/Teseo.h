// Teseo.h
// rev 1 - June 2024 - shabaz - first revision

#ifndef TESEO_H
#define TESEO_H

#include "Arduino.h"
#include <math.h>


// defines
#define MAX_SAT 24

#define SOURCE_INVALID 99
#define SOURCE_UNK 0
#define SOURCE_GPS 1
#define SOURCE_GALILEO 2
#define SOURCE_BEIDOU 3
#define SOURCE_GLONASS 4

#define PRINT_ENABLE 1
#define PRINT_DISABLE 0

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


// class definition
class Teseo
{
  public:
    Teseo();
    void init(long baudrate);
    void flush_buffer(void);
    void get_data(int print_enable = 0);
    void print_data(void);
    void build_checksum(char* line, char* csum_text);
    void send_command(const char* cmd);
    void wait_send_complete(void);
    void send_and_read(const char* cmd, char* response, int max_len, unsigned long max_wait_ms = 100);

    gsv_t gsv[MAX_SAT]; // satellite information
    rmc_t rmc; // date/time/lat/lon/speed
    gga_t gga; // altitude
    uint8_t satnum; // number of satellites

  private:
  
};

#endif // TESEO_H
