#include "webinterface.hpp"

GyverPortal ui;
Temperature dataT;

float set_Temp;
int16_t temp[2][NUMBER_OF_TEMP_VALUE];
const char *names[2];
String list;
uint32_t dates[NUMBER_OF_TEMP_VALUE];
uint8_t flag[START_FLAG_INDEX + 1];
uint32_t startTime, setTime;

/**
 * @brief build web interface
 * @details design graph and list, design form for temperature control
 * @author Catalin
 * @date 05/11/2024
 * @return void
 */
void build()
{
  names[0] = "Real_temp";
  set_Temp = 0;
  list = String("real,seted,time");
  dates[NUMBER_OF_TEMP_VALUE - 1] = GPunix(2023, 10, 9, 0, 0, 0);
  names[1] = "Set_temp";

  GP.BUILD_BEGIN(700);
  GP.THEME(GP_LIGHT);

  GP.PAGE_TITLE("Temperature control");

  GP.TITLE("Temperature control");
  GP.HR();

  GP.NAV_TABS_LINKS("/,/Graph,/List", "Home,Graph,List");
  // second WINDOW
  if (ui.uri() == "/Graph")
  {
    GP.PLOT_STOCK_DARK<2, NUMBER_OF_TEMP_VALUE>("table", names, dates, temp);

    M_BLOCK_TAB(
        "View",
        M_BOX(GP.LABEL("Internal temperature: "); GP.LABEL(String(dataT.readKTemp()), "KTemp"););
        M_BOX(GP.LABEL("Output temperature: "); GP.LABEL(String(dataT.readNTCTemp()), "NtcTemp"););
        M_BOX(GP.LABEL("Seted temperature"); GP.NUMBER("set", "Set value", set_Temp););
        M_BOX(GP.LABEL("Time"); GP.LABEL(formatTime(startTime), "time"););
        M_BOX(GP.BUTTON("btn1", "Start"); GP.BUTTON("btn2", "Stop");););
  }
  else if (ui.uri() == "/List")
  {
    // last WINDOW
    GP.BLOCK_BEGIN(GP_TAB, "", "Log", GP_DEFAULT);
    GP.BUTTON("btn3", "Update");
    GP.AREA("list", 20, list);
  }
  else
  {
    // first WINDOW
    // GP_MAKE_GRID(
    M_BLOCK_TAB(
        "View",
        M_BOX(GP.LABEL("Internal temperature: "); GP.LABEL(String(dataT.readKTemp()), "KTemp"););
        M_BOX(GP.LABEL("Output temperature: "); GP.LABEL(String(dataT.readNTCTemp()), "NtcTemp"););
        M_BOX(GP.LABEL("Seted temperature"); GP.NUMBER("set", "Set value", set_Temp););
        M_BOX(GP.LABEL("Seted time, min"); GP.NUMBER("tim", "Set value", setTime););
        M_BOX(GP.LABEL("Time"); GP.LABEL(formatTime(startTime), "time"););
        M_BOX(GP.BUTTON("btn1", "Start"); GP.BUTTON("btn2", "Stop");););

    M_BLOCK_TAB(
        "PID CONFIG",
        M_BOX(GP.LABEL("Kp"); GP.SPINNER("kp", dataT.getKp(), 0, 40, 0.1, 1); GP.LABEL("Recommended 0.1"););
        M_BOX(GP.LABEL("Ki"); GP.SPINNER("ki", dataT.getKi(), 0, 40, 0.05, 1); GP.LABEL("Recommended 0.05"););
        M_BOX(GP.LABEL("Kd"); GP.SPINNER("kd", dataT.getKd(), 0, 40, 0.01, 1); GP.LABEL("Recommended 0.01");););
    // );
  }

  GP.BLOCK_END();
}

/**
 * @brief click response
 * @author Catalin
 * @date 05/11/2024
 * @return void
 */
void action()
{
  float KP, KI, KD;

  if (ui.click())
  {
    if (ui.clickInt("set", set_Temp))
    {
      dataT.setSetpoint(set_Temp);
      Serial.print("Temperature: ");
      Serial.println(set_Temp);
    }

    if (ui.clickInt("tim", setTime))
    {
      Serial.print("Time: ");
      Serial.println(setTime);
    }

    if (ui.click("btn1"))
    { // start
      flag[START_FLAG_INDEX] = true;
      Serial.println("Start");
    }

    if (ui.click("btn2"))
    {
      flag[START_FLAG_INDEX] = false;
      Serial.println("Stop");
    }

    if (ui.click("btn3"))
    {
      ui.updateString("list", list);
    }

    if (ui.copyFloat("kp", KP))
      dataT.setKp(KP);
    if (ui.copyFloat("ki", KI))
      dataT.setKi(KI);
    if (ui.copyFloat("kd", KD))
      dataT.setKd(KD);
  }
}

/**
 * @brief init wifi
 * @date 05/11/2024
 * @author Catalin
 * @return void
 */
void initWifi()
{
  Screen screen;

  screen.init();

  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASS);

  screen.clearDisplay();
  screen.printText(0, 0, 1, "Connecting ...");
  screen.display();

  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(500);
  }

  String ipAddress = WiFi.localIP().toString();
  screen.clearDisplay();
  screen.printText(0, 0, 1, "Connected!");
  screen.printText(0, 20, 1, "IP: " + ipAddress);
  screen.display();

  vTaskDelay(2000);

  delete &screen;
}

/**
 * @brief start access point
 * @date 07/11/2024
 * @author Catalin
 * @return void
 */
void startAccessPoint()
{

  Screen screen;
  // Configurați ESP32 ca Access Point
  WiFi.softAP(WF_SSID, WF_PASS);

  screen.clearDisplay();
  screen.printText(0, 0, 1, "AP Started!");
  screen.printText(0, 20, 1, "Connect to:");
  screen.printText(0, 40, 1, WF_SSID);
  screen.display();

  // Configurați serverul web pentru a permite introducerea SSID-ului și parolei
  ui.attachBuild(wF_build); // Folosiți build existent
  ui.attach(wF_action);     // Folosiți action existent
  ui.start();

  delete &screen;
}

/**
 * @brief build response
 * @author Catalin
 * @date 07/11/2024
 * @return void
 * @details Design form for SSID and password
 */
void wF_action()
{
Screen screen;
  // Logica pentru a salva SSID-ul și parola introduse de utilizator
  String ssid = ui.getString("ssid");         // Obține SSID-ul introdus
  String password = ui.getString("password"); // Obține parola introdusă

  if (ssid.length() > 0 && password.length() > 0)
  {
    WiFi.begin(ssid.c_str(), password.c_str()); // Încercați să vă conectați la noua rețea

    screen.clearDisplay();
    screen.printText(0, 0, 1, "Connecting to:");
    screen.printText(0, 20, 1, ssid);
    screen.display();

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
      vTaskDelay(500 / portTICK_PERIOD_MS);
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      String ipAddress = WiFi.localIP().toString();
      screen.clearDisplay();
      screen.printText(0, 0, 1, "Connected!");
      screen.printText(0, 20, 1, "IP: " + ipAddress);
      screen.display();
      vTaskDelay(2000);
    }
    else
    {
      // Afișați un mesaj de eroare dacă nu s-a putut conecta
      screen.clearDisplay();
      screen.printText(0, 0, 1, "Failed to connect");
      screen.display();
      vTaskDelay(2000);
      startAccessPoint(); // Reporniți AP pentru a permite reconfigurarea
    }
  }
  delete &screen;
}

/**
 * @brief build response
 * @author Catalin
 * @date 07/11/2024
 * @return void
 * @details Design form for SSID and password
 */
void wF_build()
{
  GP.BUILD_BEGIN(700);
  GP.THEME(GP_LIGHT);

  GP.TITLE("WiFi Configuration");

  M_BOX(GP.LABEL("Enter SSID:"));
  GP.TEXT("ssid", "", "SSID");

  M_BOX(GP.LABEL("Enter Password:"));
  GP.PASS("password", "", "Password");

  GP.BUTTON("save", "Save");
}
/**
 * @brief Convert time to string
 *
 * @param milliseconds
 * @return String
 */
String formatTime(unsigned long milliseconds)
{
  unsigned long totalSeconds = milliseconds / 1000;
  unsigned long hours = totalSeconds / 3600;
  unsigned long minutes = (totalSeconds % 3600) / 60;

  return String(hours) + ":" + String(minutes);
}
