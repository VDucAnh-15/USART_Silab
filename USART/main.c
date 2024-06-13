#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "string.h"

int state = 0;
char receivedChar;
int count = 0;
char received_data[100];
char ONLED[] = " ON_LED\n";
char OFFLED[] = " OFF_LED\n";
char ON[] = "ON";
char OFF[] = "OFF";

void initUSART(void)
{
    // Enable clock for USART0 and GPIO
    CMU_ClockEnable(cmuClock_USART0, true);
    CMU_ClockEnable(cmuClock_GPIO, true);

    // Configure GPIO pins for USART0
    // TX = PA08, RX = PA09
    GPIO_PinModeSet(gpioPortA, 8, gpioModePushPull, 1); // TX
    GPIO_PinModeSet(gpioPortA, 9, gpioModeInput, 0);    // RX

    // Initialize USART0
    USART_InitAsync_TypeDef usartInit = USART_INITASYNC_DEFAULT;
    usartInit.enable = usartDisable; // Don't enable yet

    // Modify default init struct if needed
    usartInit.baudrate = 115200;       // Set desired baud rate
    usartInit.databits = usartDatabits8; // 8 data bits
    usartInit.stopbits = usartStopbits1; // 1 stop bit
    usartInit.parity = usartNoParity;    // No parity

    // Initialize USART with the given settings
    USART_InitAsync(USART0, &usartInit);

    // Enable TX and RX pins for USART0 at location 1 (adjust if needed)
    GPIO->USARTROUTE[0].TXROUTE = (gpioPortA << _GPIO_USART_TXROUTE_PORT_SHIFT)
                                | (8 << _GPIO_USART_TXROUTE_PIN_SHIFT);
    GPIO->USARTROUTE[0].RXROUTE = (gpioPortA << _GPIO_USART_RXROUTE_PORT_SHIFT)
                                | (9 << _GPIO_USART_RXROUTE_PIN_SHIFT);
    GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_TXPEN | GPIO_USART_ROUTEEN_RXPEN;

    // Enable RX interrupts
    USART_IntEnable(USART0, USART_IEN_RXDATAV);
    NVIC_ClearPendingIRQ(USART0_RX_IRQn);
    NVIC_EnableIRQ(USART0_RX_IRQn);

    // Enable USART0
    USART_Enable(USART0, usartEnable);
}

void USART_SendChar(char c);
void USART_SendString(char *string);
char USART_ReceiveChar(void);
void USART0_RX_IRQHandler(void);
void delay(volatile uint32_t count);
uint8_t USART_Compare(char *chuoinhan, char *chuoisosanh);

int main(void)
{
    CHIP_Init();
    initUSART();

    while (1)
    {

        delay(1000000);
        if (state != 0)
        {
            USART_SendString(received_data);
            USART_SendChar('\n');
            state = 0;
        }
        delay(1000000);
    }
}

void USART_SendChar(char c)
{
    // Wait for the transmit buffer to be empty
    while (!(USART0->STATUS & USART_STATUS_TXBL));

    // Send the character
    USART_Tx(USART0, c);
}

void USART_SendString(char *string)
{
  int i;
  for (i = 0; i < strlen(string); i++){
      USART_SendChar(string[i]);
      delay(1000);
  }
}

char USART_ReceiveChar(void)
{
    // Read the character from the receive buffer
    char received = USART_Rx(USART0);
    // Update state after receiving a character
    return received;
}

void USART0_RX_IRQHandler(void)
{
    // Check for RXDATAV interrupt flag
    if (USART0->IF & USART_IF_RXDATAV)
    {
        // Read the received data
        receivedChar = USART_Rx(USART0);

        if(receivedChar == '\n')    //Ki tu enter
        {
            received_data[count] = '\0';
            count = 0;
            state = 1;
        }
        else if(receivedChar == '\b')   //Ki tu xoa
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
    }
    // Clear the interrupt flag
    USART_IntClear(USART0, USART_IF_RXDATAV);
}

uint8_t USART_Compare(char *chuoinhan, char *chuoisosanh) // BIEN CHUOI NHAN VAO CHUOI SO SANH
{
    int i;
    for (i = 0; chuoisosanh[i] != '\0' && chuoinhan[i] != '\0'; i++)
    {
        if (chuoisosanh[i] != chuoinhan[i])
        {
            return 0;
        }
    }
    if (chuoisosanh[i] != '\0' || chuoinhan[i] != '\0')
    {
        return 0;
    }
    return 1;
}

void delay(volatile uint32_t count)
{
    while (count--)
    {
        // Do nothing, just decrement the counter
    }
}
