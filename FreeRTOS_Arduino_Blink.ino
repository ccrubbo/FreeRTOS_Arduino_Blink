#include <Arduino_FreeRTOS.h>
#include <FspTimer.h>
#include <limits.h>


// Task globals
TaskHandle_t loop_task, blink_task;
void loop_task_func(void *pvParameters);  /* does nothing */
void blink_task_func(void *pvParameters); /* waits for notification from timer interrupt */

// Initialize and start both the `loop` and `blink` tasks
bool tasks_init(void) {
  auto const ret1 = xTaskCreate(loop_task_func, "Loop Thread", 128, nullptr, tskIDLE_PRIORITY+1, &loop_task);
  auto const ret2 = xTaskCreate(blink_task_func, "Blink Thread", 128, nullptr, tskIDLE_PRIORITY+1, &blink_task);
  return (ret1 && ret2);
}

// To configure a pin that can be used to evaluate timing via logic analyzer
#define DEBUG_PIN             PIN_D0
#define DEBUG_ENABLED         1

// To configure the first LED behavior
#define LED1_PIN              PIN_D1
#define LED1_TOGGLE_CYCLES    65000   /* timer cycles before toggling */

// To configure the second LED behavior
#define LED2_PIN              PIN_D2
#define LED2_BLINK_PERIOD_MS  3000    /* time between blink sequences */
#define LED2_BLINK_COUNT      3       /* number of blinks in sequence */
#define LED2_BLINK_DELAY_MS   250     /* time between blinks in sequence */

// To configure the timer
#define TIMER_MODE      TIMER_MODE_PERIODIC
#define TIMER_TYPE      AGT_TIMER  /* could instead use GPT_TIMER */
#define TIMER_FREQ_KHZ  10
#define TIMER_COUNT_TO_MS(x) (x/TIMER_FREQ_KHZ)
FspTimer timer;
uint16_t timer_count = 0;
uint32_t c_start = 0;
uint32_t c_stop = 0;

// Timer callback used to control the blink behavior of both LEDs
void timer_cb(timer_callback_args_t __attribute((unused)) *p_args) {
  
  // Toggle the first LED after a certain number of cycles
  if (++timer_count == LED1_TOGGLE_CYCLES) {
    digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
  }

  // Start the second LED blink sequence after a certain amount of milliseconds
  c_stop = timer_count;
  uint32_t c_elapsed = (c_stop < c_start) ? (UINT16_MAX - c_start) + c_stop : c_stop - c_start; // overflow protection
  if (TIMER_COUNT_TO_MS(c_elapsed) >= LED2_BLINK_PERIOD_MS) {
    c_start = c_stop;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(blink_task, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken ); 
  }
}

// Initialize timer and if successful, start timer
bool timer_init(void) {
  
  // Get the first available timer 
  uint8_t timer_type = TIMER_TYPE;
  int8_t timer_index = FspTimer::get_available_timer(timer_type, true);
  if (timer_index < 0) return false;

  // Configure and start the timer
  FspTimer::force_use_of_pwm_reserved_timer();
  if (!timer.begin(TIMER_MODE_PERIODIC, timer_type, timer_index, TIMER_FREQ_KHZ * 1000, 0.0f, timer_cb)) return false;
  if (!timer.setup_overflow_irq()) return false;
  if (!timer.open()) return false;
  if (!timer.start()) return false;
  return true;
}

// Initialize a pin and set to LOW
void pin_init(pin_size_t pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}


void setup()
{
  Serial.begin(115200);
  while (!Serial) { }

  // Initialize and start the tasks
  if (!tasks_init()) {
    Serial.println("Tasks initialization failed!");
    return;
  }

  // Initialize the LED and debug pins 
  pin_init(DEBUG_PIN);
  pin_init(LED1_PIN);
  pin_init(LED2_PIN);

  // Initialize timer and if successful, start timer
  if (!timer_init()) {
    Serial.println("Timer initialization failed!");
    return;
  }

  Serial.println("Starting scheduler ...");
  
  // Start the scheduler
  vTaskStartScheduler();
  
  // We'll never get here
  while(1);
}


void loop()
{
  // Do nothing
}

void loop_task_func(void *pvParameters)
{
  while(1)
  {
    loop();
    taskYIELD();
  }
}

void blink_task_func(void *pvParameters)
{
  while(1)
  {
    // Wait for a notification from timer interrupt
    ulTaskNotifyTakeIndexed( 0,pdTRUE, portMAX_DELAY);

    // For debugging, toggle the debug pin (makes it easier to use logic analyzer)
    digitalWrite(DEBUG_PIN, !digitalRead(DEBUG_PIN));

    // Blink the second LED according to configuration
    for (int i = 0; i < LED2_BLINK_COUNT * 2; i++) {
      digitalWrite(LED2_PIN, !digitalRead(LED2_PIN));
      vTaskDelay(pdMS_TO_TICKS(LED2_BLINK_DELAY_MS));
    }

  }
}