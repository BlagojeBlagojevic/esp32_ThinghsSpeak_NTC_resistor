#include <WiFi.h>
  const char* host = "api.thingspeak.com"; // 
  const int httpPort = 80; // 
  const String channelID   = "UNIJETI SVOJ"; 
  const String writeApiKey = "UNIJETI SVOJ"; //INPUT YOUR OUN KEYS FROM A thinks network
  const String readApiKey = "Unijeti svoj ";
  int field1 = 0;
  const float BETA = 3950;   //BETA faktor za ntc otpornik 1 / (log(1 / (4096. / analogValue - 1)) / BETA + 1.0 / 298.15) - 273.15 //Iz dataSheata
  int numberOfResults = 3; 
  int fieldNumber = 1; 
  float celsius=20000.1;   //Celzius kao globalni ova vrijednost ako postoji greska
static void setup_wifi()   //podesavanje WIFI
{
  const char *password="", *ssid="";   //Password , ime wifi 
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED)  //Ako nije conected sacekaj dok nebude
  {
    Serial.println("Spaja se!!!");
    //Serial.println(WL_CONNECTED);  //Obicni int moze SWITCH CASE
    delay(1000);
  }
  Serial.println("Connected!!!");
  Serial.println("Ip_adresa: ");
  Serial.print(WiFi.localIP());
}
void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  setup_wifi();
  analogReadResolution(12);   //Rezolucija 12 to jest 12 bit za predstavljanje promjena od jednog 3.3/4096.
  //setup_cloud();
}
static void send_to_cloud(WiFiClient *client)   //Slanje na cloud
{
  unsigned long proteklo_t = millis();

  while(client->available() == 0)   //Provjera 
  {
    if(millis() - proteklo_t >= 5000)   //5s delay kick sa servera
    {
      Serial.println("Time out!!!!!");
      client->stop();
      return;
    }
  }
  while(client->available()) //Slanje
  {
    String za_ispis=client->readStringUntil('\r');
    Serial.println(za_ispis);
  }
}
void loop() 
{
  WiFiClient client; //Client objekat za slanje
  //Citane sa ntc R
  int analogValue=analogRead(34); //uz esp-idf docs 34 ANALOg
  delay(10);//delay za adc        //rez 12 bit
  celsius = 1 / (log(1 / (4096. / analogValue - 1)) / BETA + 1.0 / 298.15) - 273.15;    //MJERENJE 
  Serial.print("Temperature: ");
  Serial.print(celsius);
  //Serial.print(analogValue);
  Serial.println(" ℃");

      //STA SE MORA SLATI UZ PODATAK request itd
  String footer = String(" HTTP/1.1\r\n") + "Host: " + String(host) + "\r\n" + "Connection: close\r\n\r\n";

  if (!client.connect(host, httpPort)) //Skip ako nije
  {
    return;
  }
  //ISPIS STA SE SALJE TIP requesta na sta se salje
  client.print("GET /update?api_key=" + writeApiKey + "&field1=" + celsius + footer);
  send_to_cloud(&client);

  String readRequest = "GET /channels/" + channelID + "/fields/" + fieldNumber + ".json?results=" + numberOfResults + " HTTP/1.1\r\n" +
                       "Host: " + host + "\r\n" +
                       "Connection: close\r\n\r\n";

  if (!client.connect(host, httpPort)) //Skip ako nije spojen
  {
    return;
  }
  //Provjera sta je stiglo sa servera to jest zadnje moze i bez ovoga
  client.print(readRequest);
  send_to_cloud(&client);
  delay(60000);    //Svaki minut salje MANJE VISE
}
