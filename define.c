// Define Constants

// TIMER, PWM
#define PWM_MAX_VALUE 255 // assume for 8-bit PWM resolution
#define PWM_FREQUENCY 100
#define TIM1 "TIM1"
#define TIM2 "TIM2"
#define CHANNEL_1 1
#define CHANNEL_2 2
// PINS for light instances
#define LIGHTPIN1 1
#define LIGHTPIN2 2

// UART
#define UART_BAUD_RATE 115200
#define UART_WORD_LENGTH 255 
#define UART_PARITY 0
#define UART_STOP_BIT 1 

// Define packet structure
typedef struct {
  uint8_t frameSync;
  uint8_t packetType;
  uint8_t lightInstance;
  uint8_t brightness;
  uint8_t checksum;
} Packet;
