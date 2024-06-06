// Teseo.cpp
// rev 1 - June 2024 - shabaz - first version

#include "Teseo.h"
#include <stdio.h>
#include <string.h>

// defines

// *************************************************
// ********  Teseo class implementation  ********
// *************************************************

Teseo::Teseo()
{
	// Teseo object constructor
}

void Teseo::init(long baudrate) {
	Serial1.begin(baudrate);
}

void Teseo::flush_buffer(unsigned long wait_ms)
{
	unsigned long start = millis();
	while (millis() - start < 100) {
		while (Serial1.available()) {
			Serial1.read();
			start = millis();
		}
	}
}

void Teseo::get_data(int print_enable)
{
	char linebuf[128];
    int idx = 0;
    char source;
    // clear out the satellite data
	satnum = 0;
    for (int i = 0; i < MAX_SAT; i++) {
        gsv[i].prn = SOURCE_INVALID;
    }
    // wait for serial data to arrive, then parse it all until
    //the serial stream has been clear for 100 msec
    while (!Serial1.available());
    // ok, data is available, start reading it
    // read the data, one line at a time (0x0a terminated)
    unsigned long start = millis();
    while (millis() - start < 100) {
        while (Serial1.available()) {
            char c = Serial1.read();
            if (c == 0x0a) {
                linebuf[idx] = 0;
				if (print_enable) {
                	Serial.println(linebuf);
				}
                // parse the line
                if (linebuf[0] == '$') {
                    // this is a NMEA sentence
                    // **** test for GSV ***
                    if (strncmp(&linebuf[3], "GSV", 3)==0) {
                        // get satellite source (GPS, Galileo, etc)
                        if (linebuf[2] == 'P') {
                            source = SOURCE_GPS;
                        } else if (linebuf[2] == 'L') {
                            source = SOURCE_GLONASS;
                        } else if (linebuf[2] == 'B') {
                            source = SOURCE_BEIDOU;
                        } else if (linebuf[2] == 'A') {
                            source = SOURCE_GALILEO;
                        } else {
                            source = SOURCE_UNK;
                        }
                        char * p = &linebuf[7];
                        // first three fields are ignored (AMOUNT, NUM, TOT)
                        p = strchr(p, ',') + 1;
                        p = strchr(p, ',') + 1;
                        p = strchr(p, ',') + 1;
                        //next fields are groups of PRN, ELEV, AZIM, SNR
                        // stop when we reach * or end of line or if PRN is blank
                        while (*p != '*' && *p != 0 && satnum < MAX_SAT) {
                            if (*p == ',') {
                                // PRN field is blank, ignore the rest of the line
                                break;
                            }
                            gsv[satnum].prn = atoi(p);
                            p = strchr(p, ',') + 1;
                            gsv[satnum].elev = atoi(p);
                            p = strchr(p, ',') + 1;
                            gsv[satnum].azim = atoi(p);
                            p = strchr(p, ',') + 1;
                            if (*p == '*') { // no SNR and at end of line
                                gsv[satnum].snr = 0;
                            } else {
                                gsv[satnum].snr = atoi(p);
                                if (strlen(p) > 10) { // more satellites to come
                                    p = strchr(p, ',') + 1;
                                } else { // last satellite, set pointer so that we exit the loop
                                    p = strchr(p, '*');
                                }
                            }
                            gsv[satnum].source = source;
                            satnum++;
                        } 
                    }
                    // **** test for RMC ***
                    if (strncmp(&linebuf[3], "RMC", 3)==0) {
                        // format is HHMMSS.sss, status, lat, latdir, lon, londir, speed, trackgood, DDMMYY, ...
                        char * p = &linebuf[7];
                        // parse first two characters as hour
                        rmc.hour = (p[0] - '0') * 10 + (p[1] - '0');
                        rmc.min = (p[2] - '0') * 10 + (p[3] - '0');
                        rmc.sec = (p[4] - '0') * 10 + (p[5] - '0');
                        p = strchr(p, ',') + 1;
                        rmc.status = *p;
                        p = strchr(p, ',') + 1;
                        // parse first two characters as degrees latitude
                        float deg = (p[0] - '0') * 10 + (p[1] - '0');
                        // parse the rest as minutes
                        rmc.lat = deg + atof(&p[2]) / 60.0;
                        p = strchr(p, ',') + 1;
                        if (*p == 'S') {
                            rmc.lat = -rmc.lat;
                        }
                        p = strchr(p, ',') + 1;
                        // parse first three characters as degrees longitude
                        deg = (p[0] - '0') * 100 + (p[1] - '0') * 10 + (p[2] - '0');
                        // parse the rest as minutes
                        rmc.lon = deg + atof(&p[3]) / 60.0;
                        p = strchr(p, ',') + 1;
                        if (*p == 'W') {
                            rmc.lon = -rmc.lon;
                        }
                        p = strchr(p, ',') + 1;
                        rmc.speed = atof(p);
                        p = strchr(p, ',') + 1;
                        // ignore trackgood
                        p = strchr(p, ',') + 1;
                        // parse first two characters as day (1-31)
                        rmc.date = (p[0] - '0') * 10 + (p[1] - '0');
                        // parse next two characters as month (1-12)
                        rmc.month = (p[2] - '0') * 10 + (p[3] - '0');
                        // parse last two characters as year (0-99)
                        rmc.year = (p[4] - '0') * 10 + (p[5] - '0');
                        // add 2000 to year
                        rmc.year += 2000;
                    }
                    // *** test for GGA ***
                    if (strncmp(&linebuf[3], "GGA", 3)==0) {
                        //format is timestamp, lat, latdir, lon, londir, fix, numsats, hdop, alt, altunit, geosep, geounit
                        // we are only interested in alt, altunit, geosep, geounit
                        char * p = &linebuf[7];
                        // ignore the first 8 fields
                        for (int i = 0; i < 8; i++) {
                            p = strchr(p, ',') + 1;
                        }
                        gga.alt = atof(p); // altitude
                        p = strchr(p, ',') + 1;
                        gga.altunit = *p; // altitude unit
                        p = strchr(p, ',') + 1;
                        gga.geosep = atof(p); // geoid separation
                        p = strchr(p, ',') + 1;
                        gga.geounit = *p; // geoid separation unit
                    }
                    // ****** add more NMEA sentences here ********
                }
                idx = 0;
            } else {
                linebuf[idx++] = c;
                if (idx == 127) {
                    // line too long, we reset, and ignore the line
                    idx = 0;
                }
            }
            start = millis();
        }
        // loop until the serial stream has been clear for 100 msec
    }
}

void Teseo::print_data(void)
{
	Serial.print("Time: ");
	if (rmc.hour < 10)
		Serial.print("0");
    Serial.print(rmc.hour);
    Serial.print(":");
	if (rmc.min < 10)
		Serial.print("0");
    Serial.print(rmc.min);
    Serial.print(":");
	if (rmc.sec < 10)
		Serial.print("0");
    Serial.print(rmc.sec);
    Serial.print("\r\n");
    Serial.print("Status: ");
    if (rmc.status == 'A') {
        Serial.println("A : Active (valid)");
    } else {
        Serial.print(rmc.status);
        Serial.println(" : Void");
    }
    Serial.print("Lat(N)/Lon(W): ");
    Serial.print(rmc.lat);
    Serial.print(" ");
    Serial.print(rmc.lon);
    Serial.print("\r\n");
    Serial.print("Speed (knots): ");
    Serial.print(rmc.speed);
    Serial.print("\r\n");
    Serial.print("Date/Month/Year: ");
	if (rmc.date < 10)
		Serial.print("0");
    Serial.print(rmc.date);
    Serial.print("/");
	if (rmc.date < 10)
		Serial.print("0");
    Serial.print(rmc.month);
    Serial.print("/");
    Serial.println(rmc.year);
        Serial.print("Altitude: ");
    Serial.print(gga.alt);
    if (gga.altunit == 'M') {
        Serial.print(" meters");
    } else {
        Serial.print(" units");
    }
    Serial.print(", Geoid separation: ");
    Serial.print(gga.geosep);
    if (gga.geounit == 'M') {
        Serial.println(" meters");
    } else {
        Serial.println(" units");
    }
    Serial.println("Satellites:");
    for (int i = 0; i < MAX_SAT; i++) {
        if (gsv[i].prn != SOURCE_INVALID) {
            Serial.print("Source: ");
            Serial.print((uint8_t)gsv[i].source);
            Serial.print(", PRN:");
            Serial.print((uint8_t)gsv[i].prn);
            Serial.print(", Elev:");
            Serial.print(gsv[i].elev);
            Serial.print(", Azim:");
            Serial.print(gsv[i].azim);
            Serial.print(", SNR:");
            Serial.println((uint8_t)gsv[i].snr);
        }
    }
}

void Teseo::build_checksum(char* line, char* csum_text)
{
	// get 8-bit checksum of the data but not including the first character ($) or the * character
    char checksum = 0;
    for (int i = 1; line[i] != '*'; i++) {
        checksum ^= line[i];
    }
    // convert checksum to two ASCII characters
    csum_text[0] = (checksum >> 4) + '0';
    if (csum_text[0] > '9') {
        csum_text[0] += 7;
    }
    csum_text[1] = (checksum & 0x0f) + '0';
    if (csum_text[1] > '9') {
        csum_text[1] += 7;
    }
    csum_text[2] = 0;
}

void Teseo::send_command(const char * cmd) {
    // append asterisk
    char line[128];
    strcpy(line, cmd);
    strcat(line, "*");
    // calculate checksum
    char csum_text[3];
    build_checksum(line, csum_text);
    strcat(line, csum_text);
    // send the command
    Serial1.print(line);
    Serial1.print("\r\n");
}

void Teseo::wait_send_complete(void) {
    Serial1.flush(); // this command waits until the send buffer is empty

}

void Teseo::send_and_read(const char* cmd, char* response, int max_len, unsigned long max_wait_ms) {
    send_command(cmd);
    wait_send_complete();
    // read the response
    int idx = 0;
    response[0]=0; // assume no response initially
    unsigned long start = millis();
    while (millis() - start < max_wait_ms) {
        while (Serial1.available()) {
            char c = Serial1.read();
            if (c == 0x0a) {
                response[idx] = 0;
                return;
            } else {
                response[idx++] = c;
                if (idx == max_len - 1) {
                    response[idx] = 0;
                    return;
                }
            }
        }
    }
}

// *************************************************
// ****   End of Teseo class implementation  ****
// *************************************************

