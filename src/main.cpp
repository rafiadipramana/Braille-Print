#include <Arduino.h>
#include <WiFi.h>
#include <Update.h>
#include <FirebaseClient.h>
#include <FirebaseJson.h>

#define API_KEY "AIzaSyDakBC5QOUNzxOUijRgAI7sHlmgK7NBE1g"
#define FIREBASE_HOST "https://ebraille-printer-default-rtdb.firebaseio.com/"
#define USER_EMAIL "muhammad.rifqi.fathurrohman@polines.ac.id"
#define USER_PASSWORD "Polines#2023"

#define WIFI_SSID "RifqiFathur"
#define WIFI_PASSWORD "mrifqi456"

void asyncCb(AsyncResult &aResult);
void printResult(AsyncResult &aResult);

DefaultNetwork network;
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);

FirebaseApp app;
FirebaseJson json;
FirebaseJsonData jsonData;

#include <WiFiClientSecure.h>
WiFiClientSecure sslClient;

using AsyncClient = AsyncClientClass;
AsyncClient aClient(sslClient, getNetwork(network));

RealtimeDatabase Database;
bool taskComplete = false;

void setup()
{
  Serial.begin(115200);
  Serial.println("Menghubungkan...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.println("Terkoneksi");
  Serial.print("Teknoneksi dengan IP: ");
  Serial.println(WiFi.localIP());

  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  sslClient.setInsecure();

  initializeApp(aClient, app, getAuth(user_auth), asyncCb, "authTask");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(FIREBASE_HOST);
}

void prosesKata(const char *kata);

void loop()
{
  unsigned long currentMillis = millis();
  static unsigned long lastFirebaseUpdate = 0;

  if (app.ready() && currentMillis - lastFirebaseUpdate > 2500)
  {
    lastFirebaseUpdate = currentMillis;
    String jsonKata = Database.get<String>(aClient, "/EBRAILLLE PRINTER");
    Serial.println(jsonKata);
    json.setJsonData(jsonKata);

    if (json.get<String>(jsonData, "/Teks yang di Print"))
    {
      String rawJsonData = jsonData.to<String>();
      rawJsonData.replace("\\\"", "\"");
      prosesKata(rawJsonData.c_str());
    }
    else
    {
      Serial.println("Failed to get the value from JSON");
    }
  }
}

int *konversiHurufKeKodeBraile(char huruf)
{
  static int kodeBraille[6];

  switch (huruf)
  {
  case 'R':
    kodeBraille[0] = 1;
    kodeBraille[1] = 0;
    kodeBraille[2] = 1;
    kodeBraille[3] = 1;
    kodeBraille[4] = 1;
    kodeBraille[5] = 0;
    break;
  case 'A':
    kodeBraille[0] = 1;
    kodeBraille[1] = 0;
    kodeBraille[2] = 0;
    kodeBraille[3] = 0;
    kodeBraille[4] = 0;
    kodeBraille[5] = 0;
    break;
  case 'F':
    kodeBraille[0] = 1;
    kodeBraille[1] = 1;
    kodeBraille[2] = 1;
    kodeBraille[3] = 0;
    kodeBraille[4] = 0;
    kodeBraille[5] = 0;
    break;
  case 'I':
    kodeBraille[0] = 0;
    kodeBraille[1] = 1;
    kodeBraille[2] = 1;
    kodeBraille[3] = 0;
    kodeBraille[4] = 0;
    kodeBraille[5] = 0;
    break;
  default:
    kodeBraille[0] = 0;
    kodeBraille[1] = 0;
    kodeBraille[2] = 0;
    kodeBraille[3] = 0;
    kodeBraille[4] = 0;
    kodeBraille[5] = 0;
    break;
  }
  return kodeBraille;
}

void tambahkanKodeBraille(char *outputBaris1, char *outputBaris2, char *outputBaris3, char huruf)
{
  if (huruf == ' ')
  {
    strcat(outputBaris1, "  ");
    strcat(outputBaris2, "  ");
    strcat(outputBaris3, "  ");
  }
  else
  {
    int *kodeBrailleUtuh = konversiHurufKeKodeBraile(huruf);
    char buffer[3];
    snprintf(buffer, sizeof(buffer), "%d%d ", kodeBrailleUtuh[0], kodeBrailleUtuh[1]);
    strcat(outputBaris1, buffer);
    snprintf(buffer, sizeof(buffer), "%d%d ", kodeBrailleUtuh[2], kodeBrailleUtuh[3]);
    strcat(outputBaris2, buffer);
    snprintf(buffer, sizeof(buffer), "%d%d ", kodeBrailleUtuh[4], kodeBrailleUtuh[5]);
    strcat(outputBaris3, buffer);
  }
}

void prosesKata(const char *kata)
{
  char outputBaris1[100] = "";
  char outputBaris2[100] = "";
  char outputBaris3[100] = "";

  for (int i = 0; i < strlen(kata); i++)
  {
    tambahkanKodeBraille(outputBaris1, outputBaris2, outputBaris3, kata[i]);
  }

  Serial.println(outputBaris1);
  Serial.println(outputBaris2);
  Serial.println(outputBaris3);
}

// ! DO NOT MODIFY THE CODE BELOW
void asyncCb(AsyncResult &aResult)
{
  printResult(aResult);
}

void printResult(AsyncResult &aResult)
{
  if (aResult.isEvent())
  {
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
  }

  if (aResult.isDebug())
  {
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
  }

  if (aResult.isError())
  {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
  }

  if (aResult.available())
  {
    Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
  }
}