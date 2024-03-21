#include <stdio.h>

// Error Handler
void Error_Handler(char str[] errorType) {
  switch (errorType) {
    case "cmd":
      printf("Something went wrong in command packet. Please try again.\n");
    break;
    case "status":
      printf("Something went wrong in status packet. Please try again.\n");
    break;
    case "checksum":
      printf("Something went wrong in checksum byte. Please try again.\n");
    default:
}
};
