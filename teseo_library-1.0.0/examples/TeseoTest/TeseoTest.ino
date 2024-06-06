// TeseoTest.ino
// rev 1 - June 2024 - shabaz
// This code demonstrates the Teseo library

// ******** includes ********
#include <Teseo.h>

// ******** defines ********
// defines
#define FOREVER 1
#define DELAY_MS delay

// ********global variables ********
Teseo gnss;

// ******** setup() function ********
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // console debug
  gnss.init(9600);
}

// ******** loop() function ********
void loop() {
  gnss.flush_buffer(); // clear out the buffer
  gnss.get_data(PRINT_ENABLE); // get GNSS data and print the raw sentences
  gnss.print_data(); // print the data to Serial console
  DELAY_MS(1000);
}
