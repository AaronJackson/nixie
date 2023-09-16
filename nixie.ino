// Pico W Nixie clock

#include <NTPClient.h>
#include <WiFi.h>

const char *ssid     = "Rhwyd_2.4GHz";
const char *password = "";

#define SHIFT_SER 3
#define SHIFT_CLK 4
#define SHIFT_LATCH 5

#define DEBUG 1

WiFiUDP ntpUDP;
 NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000*60*2);

void setup() {
  pinMode(SHIFT_SER, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay (500);
    Serial.print ( "." );
  }

  timeClient.begin();
  timeClient.update();

  Serial.begin(9600);
}

// First bit pushed out becomes the last tube's (minor seconds) least significant bit.
// Each tube uses four bits.
// 123456 (HHMMSS) should become the bit pattern:
// 0001 0010 0011 0100 0101 0110
// which we can then push from the right.
// so during debug, should look like:
// 0110 1010 0010 1100 0100 1000

int getBcdArray() {
  char time[7];

  sprintf(time, "%2d%2d%2d", timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds());

  int bcd = 0;

  for (int i=5; i >= 0; i--) {
    int t = time[i] - '0';
    bcd = (bcd >> 4) | (t << 24);
  }

  return bcd >> 4;
}

void bangBcdArray(int bcd) {
  for (int i=0; i < 24; i++) {
    byte b = bcd & 0x01;
    bcd >>= 1;

    digitalWrite(SHIFT_SER, b);
    digitalWrite(SHIFT_CLK, HIGH);
    digitalWrite(SHIFT_CLK, LOW);

    #if DEBUG
    Serial.printf("%c", b + '0');
    if ((i % 4) == 3) Serial.print(' ');
    #endif
  }

  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

void loop() {
  int bcd = getBcdArray();
  bangBcdArray(bcd);  
  Serial.write("\n");
  delay(500);
}
