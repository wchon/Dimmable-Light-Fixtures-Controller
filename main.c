#include <cstdint>
#include "error_handler.c"
#include "define.c"

// Global variables
Packet commandPacket;
Packet statusPacket;
bool commandReceived = false;
bool rampingInProgress = false;
uint8_t currentBrightness = 0;  // Current brightness level of the LED
uint8_t targetBrightness = 0;   // Target brightness level of the LED

// Function prototypes
void UartInit(int baudRate, uint8_t wordLength, uint8_t parity, bool stopBit);
void SystemTickInit(int ticks); 
void TIM_PWM_Init(char str[] timer, uint8_t channel, uint8_t lightPin);

// Increase PWM output value to compareVal
void PWM_SET_Compare(char str[] timer, uint8_t channel, uint8_t compareVal);

void InterruptEnable(char str[] interrupt); 
void InterruptDisable(char str[] interrupt); 
uint8_t UartReadByte();
void UartWriteByte(uint8_t data);
void GPIO_SET(uint8_t output);
void GPIO_CLR(uint8_t output);



// Interrupt Service Routine for System Tick
void SystemTickIsr() {
  // Check if ramping is in progress
  if (rampingInProgress) {
    // Calculate the increment 
    
    /*(PWM_MAX_VALUE * 0.1) calculates the fraction of the maximum PWM value that 
    represents a 10% change in brightness. For example, if PWM_MAX_VALUE is 255, 
    then the result of this expression is 25.5.

    (1000 / PWM_FREQUENCY) calculates the time period in milliseconds corresponding 
    to the PWM frequency. For example, if PWM_FREQUENCY is 100 Hz, 
    then the result of this expression is 10.
    So, increment would be 25.5/10 = 2.55
    */
    uint8_t increment = (PWM_MAX_VALUE * 0.1) / (1000 / PWM_FREQUENCY); 
    
    // Pick on selected LED,its Timer and Channel 
    char timer[];
    uint8_t channel;
    if (commandPacket.lightInstance == 1) {
      timer = "TIM1";
      channel = 1;
    }
    else if (commandPacket.lightInstance == 2){
      timer = "TIM2";
      channel = 2;
    }
    else {
      Error_Handler("cmd");
    }

    // Check if ramping up or down
    if (currentBrightness < targetBrightness) {
      // Set PWM value to the current brightness to current controlled LED
      currentBrightness += increment;
      PWM_SET_Compare(timer, channel, currentBrightness);
      if (currentBrightness >= targetBrightness) {
        currentBrightness = targetBrightness;
        PWM_SET_Compare(timer, channel, currentBrightness);
        rampingInProgress = false;
      }
    } else if (currentBrightness > targetBrightness) {
      // Set PWM value to the current brightness to current controlled LED
      currentBrightness -= increment;
      PWM_SET_Compare(timer, channel, currentBrightness);
      if (currentBrightness <= targetBrightness) {
        currentBrightness = targetBrightness;
        PWM_SET_Compare(timer, channel, currentBrightness);
        rampingInProgress = false;
      }
    }

    // Update the status packet
    statusPacket.frameSync = 0xA5;
    statusPacket.packetType = 0xF1;
    statusPacket.lightInstance = commandPacket.lightInstance;
    statusPacket.brightness = currentBrightness;
    statusPacket.checksum = 0xFF + statusPacket.packetType + statusPacket.lightInstance +
                            statusPacket.brightness;
    
    // Send the status packet over UART when reached target brightness
    if (currentBrightness == targetBrightness) {
      // UartWriteByte(statusPacket.frameSync);
      // UartWriteByte(statusPacket.packetType);
      UartWriteByte(statusPacket.lightInstance);
      UartWriteByte(statusPacket.brightness);
    }
  }
}

// Interrupt Service Routine for Uart Receive, called when one byte has been received by the UART interface
void UartRxIsr() {
  uint8_t data = UartReadByte();

  // Check if frame sync is received
  if (data == 0xA5) {
    commandPacket.frameSync = data;
    commandReceived = true;
  } else if (commandReceived) {
    // Update the command packet based on the received data
    commandPacket.packetType = data;
    commandPacket.lightInstance = UartReadByte();
    commandPacket.brightness = UartReadByte();
    commandPacket.checksum = UartReadByte();

    // Validate checksum
    uint16_t checksum = 0xFF + commandPacket.packetType + commandPacket.lightInstance +
                        commandPacket.brightness;
    if ((checksum & 0xFF) == commandPacket.checksum) {
      // Start ramping to the target brightness
      targetBrightness = commandPacket.brightness;
      rampingInProgress = true;
      // Disable Uart Rx Interrupt temporarily to avoid unnecessary bytes or spamming  
      commandReceived = false;
      InterruptDisable(UART_RX_INTERRUPT);
    }
    else {
      Error_Handler("checksum");
    }
  }
}

// Interrupt Service Routine for UART Transmit, called when there is room to write one byte to the UART interface
void UartTxIsr() {
  // Finished dimmering LED process and status report, enable Uart Rx Interrupt for next command packet
  InterruptEnable(UART_RX_INTERRUPT);
}

int main() {
  // Initialize SystemTick, UART, TIM_PWM
  SystemTickInit(1000);
  UartInit(UART_BAUD_RATE, UART_WORD_LENGTH, UART_PARITY, UART_STOP_BIT);
  GPIO_CLR(LIGHTPIN1);
  GPIO_CLR(LIGHTPIN2);
  TIM_PWM_Init(TIM1, CHANNEL_1, 1);
  TIM_PWM_Init(TIM2, CHANNEL_2, 2);

  // Enable system tick interrupt
  InterruptEnable(SYSTEM_TICK_INTERRUPT);

  // Enable UART RX, TX interrupt
  InterruptEnable(UART_RX_INTERRUPT);
  InterruptEnable(UART_TX_INTERRUPT);

  // Start PWM Generation
  TIM_PWM_Start(TIM1, CHANNEL_1);
  TIM_PWM_Start(TIM2, CHANNEL_2);
  
  // Main program loop
  while (1) {
    // Perform any necessary tasks in the main loop
  }

  return 0;
}
