
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include "gbk.h"
#include "display.h"

WiFiUDP udp;
extern "C"
{
  // Includes the content of the file "wifissid.txt" in the project root.
  // Make sure this file doesn't end with an empty line.
  extern const char WIFI_SSID[] asm("_binary_wifissid_txt_start");
  // Includes the content of the file "wifipass.txt" in the project root.
  // Make sure this file doesn't end with an empty line.
  extern const char WIFI_PASS[] asm("_binary_wifipass_txt_start");
}
bool hasMatch[2] = {false, false};
// 需要筛选的 ID
const int TARGET_IDs[2] = {
    10726, // 北京
    11072  // 北控
};
char lineBuf[LINE_SIZE];

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 8 * 3600, 60000);

unsigned long lastRequestTime = 0;
const long interval = 30000; // 30秒
bool isSleeping = false;

String getTeamNameById(long teamId)
{
  if (teamMap.count(teamId))
  {
    return teamMap[teamId];
  }
  else
  {
    return "..";
  }
}

void sleepawhile()
{
  int hour = timeClient.getHours();
  if (hour >= 22 or hour < 7)
  {
    isSleeping = true;
    Display::cls();
    delay(1000 * 60 * 60);
    return;
  }
  isSleeping = false;
  delay(1000 - millis() % 1000);
}

void setup()
{
  Display::setup();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    Display::display_waiting_connect();
    delay(500);
  }
  Display::display_connected();
  timeClient.begin();
}

void fetchData()
{
  if (!timeClient.update())
    return;
  if (isSleeping)
    return;

  // 1. 获取格式化日期 2025-12-26 (基于 2025 年当前时间)
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime(&epochTime);
  sprintf(lineBuf, "%04d-%02d-%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);

  // 2. 构造 URL
  WiFiClient client;
  HTTPClient http;
  String url = "http://api.sports.163.com/api/cba/v2/schedule/getByMatchDay?firstDay=" + String(lineBuf) + "&lastDay=" + String(lineBuf);

  if (http.begin(client, url))
  {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();

      // 3. 解析 JSON
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error)
      {
        hasMatch[0] = hasMatch[1] = false;
        JsonArray items = doc["data"]["items"].as<JsonArray>();

        for (JsonObject item : items)
        {
          long homeId = item["homeId"];
          long awayId = item["awayId"];

          for (int i = 0; i <= 1; i++)
          {
            // 通过 ID 进行逻辑判断
            if (homeId == TARGET_IDs[i] || awayId == TARGET_IDs[i])
            {
              hasMatch[i] = true;
              static String homeName = getTeamNameById(homeId);
              static String awayName = getTeamNameById(awayId);
              static int homeScore = item["homeScore"];
              static int awayScore = item["awayScore"];

              // 4 & 5. 格式化拼接并串口输出
              // 格式说明: %s 字符串, %3d 占位3位的数字(靠右对齐)
              snprintf(lineBuf, sizeof(lineBuf), "%s %3d : %-3d %s",
                       homeName.c_str(), homeScore, awayScore, awayName.c_str());
              Display::display_replace_line(i, lineBuf);
            }
          }
        }

        for (int i = 0; i <= 1; i++)
        {
          if (!hasMatch[i])
          {
            Display::display_replace_line(i, "                   ");
          }
        }
      }
    }
    http.end();
  }
}

void loop()
{
  // 每30秒执行一次请求
  if (millis() - lastRequestTime >= interval)
  {
    fetchData();
    lastRequestTime = millis();
  }
  sleepawhile();
}