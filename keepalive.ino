/*
SUPERMINI
----------
Flash: 4MB
PSRAM: QSPI
Partition: Default 4MB with SPIFFS

N16R8
------
Flash: 16MB
PSRAM: OPI
Partition: 16MB Flash (3MB APP/9.9MB FATFS)
*/

void setup() {
  
  delay(3000);
  
  Serial.begin(115200);
  Serial.println("Hello ESP32S3");

  Serial.printf("PSRAM size: %u\n", ESP.getPsramSize());
  Serial.printf("PSRAM free: %u\n", ESP.getFreePsram());

}

void loop() {
  
  digitalWrite(RGB_BUILTIN, HIGH);
  delay(500);

  digitalWrite(RGB_BUILTIN, LOW);
  delay(500);

}
