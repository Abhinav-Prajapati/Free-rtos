#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <IBusBM.h>

IBusBM ibusRc;
QueueHandle_t integerQueue;
QueueHandle_t integerQueue_2;

HardwareSerial &ibusRcSerial = Serial1;
HardwareSerial &debugSerial = Serial;

int readChannel(byte channelInput, int minLimit, int maxLimit, int defaultValue)
{
  uint16_t ch = ibusRc.readChannel(channelInput);
  if (ch < 100)
    return defaultValue;
  return map(ch, 1219, 2000, minLimit, maxLimit);
}

bool redSwitch(byte channelInput, bool defaultValue)
{
  int intDefaultValue = (defaultValue) ? 100 : 0;
  int ch = readChannel(channelInput, 0, 100, intDefaultValue);
  return (ch > 50);
}
//Tack for fsi6s to recive signal
void ReadContorller(void *pvParameters)
{
  (void)pvParameters;

  ibusRc.begin(ibusRcSerial);

  int ch_2;
  int ch;
  while (1)
  {

    if (ch_2 != redSwitch(8, 1))
    {
      ch_2 = redSwitch(8, 1);
    }
    if (ch != redSwitch(9, 1))
    {
      ch = redSwitch(9, 1);
    }

    xQueueSend(integerQueue, &ch, portMAX_DELAY);
    xQueueSend(integerQueue_2, &ch_2, portMAX_DELAY);

    vTaskDelay(1);
  }
}
//task to print the value in serial moniter
void TaskSerial(void *pvParameters)
{
  (void)pvParameters;

  while (!Serial)
  {
    vTaskDelay(1);
  }

  int valueFromQueue = 0;

  while (1)
  {
    if (xQueueReceive(integerQueue, &valueFromQueue, portMAX_DELAY) == pdPASS)
    {
      // debugSerial.println(valueFromQueue);
    }
  }
}
// Stand alone led blinkg @ 1 second interval
void TaskBlink(void *pvParameters)
{
  (void)pvParameters;

  pinMode(LED_BUILTIN, OUTPUT);

  for (;;)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
// Led blingking task for blingking led for 5 second 
// on reciving cmd from controller task
void Led_Task_5(void *pvParameters)
{
  (void)pvParameters;
  int valueFromQueue = 0;

  pinMode(8, OUTPUT);

  while (1)
  {
    if (xQueueReceive(integerQueue, &valueFromQueue, portMAX_DELAY) == pdPASS)
    {
      if (valueFromQueue == 1)
      {

        digitalWrite(8, HIGH);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        digitalWrite(8, LOW);
      }
    }
  }
}

// Led blingking task for blingking led fro 2 second 
// on reciving cmd from controller task
void Led_Task_2(void *pvParameters)
{
  (void)pvParameters;
  int valueFromQueue = 0;

  pinMode(9, OUTPUT);

  while (1)
  {
    if (xQueueReceive(integerQueue_2, &valueFromQueue, portMAX_DELAY) == pdPASS)
    {
      if (valueFromQueue == 1)
      {

        digitalWrite(9, HIGH);
        vTaskDelay(600 / portTICK_PERIOD_MS); // Not exectly 2 second but got this value by hit and trial
        digitalWrite(9, LOW);
      }
    }
  }
}
void setup()
{

  /**
   * Create a queue.
   * https://www.freertos.org/a00116.html
   */

  debugSerial.begin(9600);
  integerQueue = xQueueCreate(10, sizeof(int));
  integerQueue_2 = xQueueCreate(1, sizeof(int));

  if (integerQueue != NULL) // Make different if condition for function that consume same queue message
  {

    // Create task that consumes the queue if it was created.
    xTaskCreate(TaskSerial, // Task function
                "Serial",   // A name just for humans
                500,        // This stack size can be checked & adjusted by reading the Stack Highwater
                NULL,
                1, // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
                NULL);
    xTaskCreate(Led_Task_5,             // Task function
                "led blink task 5 sec", // A name just for humans
                500,                    // This stack size can be checked & adjusted by reading the Stack Highwater
                NULL,
                1, // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
                NULL);

    // Create task that publish data in the queue if it was created.
    xTaskCreate(ReadContorller, // Task function
                "AnalogRead",   // Task name
                500,            // Stack size
                NULL,
                1, // Priority
                NULL);
  }
  if (integerQueue_2 != NULL)
  {

    xTaskCreate(Led_Task_2,             // Task function
                "led blink task 2 sec", // A name just for humans
                500,                    // This stack size can be checked & adjusted by reading the Stack Highwater
                NULL,
                2, // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
                NULL);
  }

  xTaskCreate(TaskBlink, // Task function
              "Blink",   // Task name
              500,       // Stack size
              NULL,
              1, // Priority
              NULL);
}

void loop() {}