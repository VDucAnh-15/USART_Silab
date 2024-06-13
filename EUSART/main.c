#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_eusart.h"
#include "em_gpio.h"
#include "stdio.h"
#include "stddef.h"

int state = 0;
char receivedChar;
int count = 0;
char received_data[100];
char ONLED[] = " ON_LED\n";
char OFFLED[] = " OFF_LED\n";
char ON[] = "ON";
char OFF[] = "OFF";

void EUSART1_init(void);
void EUSART_RX_IRQHandler(void);
void EUSART_SendChar(char c);
void EUSART_SendString(char *string);

void delay(volatile uint32_t count);



int main(void)
{
  // Initialize chip
  CHIP_Init();

  // Initialize EUSART1
  EUSART1_init();

  // Infinite loop
  while (1)
  {
      delay(1000000);
//      EUSART_SendString("Loi Tran BaVi_____________");
//      EUSART_SendChar('\n');
      if (state != 0)
      {
          EUSART_SendString(received_data);
          EUSART_SendChar('\n');
          state = 0;
      }
      delay(1000000);
  }
}

void EUSART1_init(void)
{
  CMU_ClockEnable(cmuClock_EUSART1, true);
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Default asynchronous initializer (8N1, no flow control)
  EUSART_UartInit_TypeDef init = EUSART_UART_INIT_DEFAULT_HF;
  init.enable = eusartDisable;
  init.databits = eusartDataBits8;
  init.stopbits = eusartStopbits1;
  init.parity = eusartNoParity;
  init.baudrate = 115200;
  EUSART_UartInitHf(EUSART1, &init);

  // Configure TX and RX pins
  GPIO_PinModeSet(gpioPortC, 1, gpioModePushPull, 1); // TX
  GPIO_PinModeSet(gpioPortC, 2, gpioModeInput, 0);  // RX

  // Route TX and RX pins to EUSART1 module
  GPIO->EUSARTROUTE[1].TXROUTE = (gpioPortC << _GPIO_EUSART_TXROUTE_PORT_SHIFT) | (1 << _GPIO_EUSART_TXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[1].RXROUTE = (gpioPortC << _GPIO_EUSART_RXROUTE_PORT_SHIFT) | (2 << _GPIO_EUSART_RXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[1].ROUTEEN = GPIO_EUSART_ROUTEEN_TXPEN | GPIO_EUSART_ROUTEEN_RXPEN;

  // Enable RX interrupts
  EUSART_IntEnable(EUSART1, EUSART_IEN_RXFL);
  NVIC_ClearPendingIRQ(EUSART1_RX_IRQn);
  NVIC_EnableIRQ(EUSART1_RX_IRQn);

  // Enable USART1
  EUSART_Enable(EUSART1, eusartEnable);
}

void EUSART1_RX_IRQHandler(void)
{
    // Read the received data
    receivedChar = EUSART_Rx(EUSART1);
//    EUSART_SendChar(receivedChar);

    if (receivedChar == '\n') // Enter key
    {
        received_data[count] = '\0';
        count = 0;
        state = 1;
    }
    else if (receivedChar == '\b') // Backspace key
    {
        if (count > 0)
        {
            count--;
            received_data[count] = '\0';
        }
    }
    else
    {
        received_data[count] = receivedChar;
        count++;
    }

    // Clear the interrupt flag
    EUSART_IntClear(EUSART1, EUSART_IF_RXFL);
}


void EUSART_SendChar(char c)
{
  while (!(EUSART1->STATUS & EUSART_STATUS_TXFL))
    ;              // Wait until TX buffer is empty
  EUSART1->TXDATA = (uint32_t)c; // Send character
//  EUSART_Tx(EUSART1, c);
}

void EUSART_SendString(char *string)
{
  int i;
  for (i = 0; i < strlen(string); i++)
  {
    EUSART_SendChar(string[i]);
    delay(1000);
  }
}

void delay(volatile uint32_t count)
{
    while (count--)
    {
        // Do nothing, just decrement the counter
    }
}
