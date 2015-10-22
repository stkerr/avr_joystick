#include "twi.h"

void TWI_Init(void)
{
	//set SCL to 400kHz
    TWSR = 0x00;
    TWBR = 0x0C;

    //enable TWI
    TWCR = (1<<TWEN);

    PORTD |= 1;
    PORTD |= 2;
}

void TWI_Start(void)
{
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

void TWI_Stop(void)
{
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

void TWI_Write(uint8_t u8data)
{
    TWDR = u8data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
}

uint8_t TWI_ReadACK(void)
{
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
}

uint8_t TWI_ReadNACK(void)
{
    TWCR = (1<<TWINT)|(1<<TWEN);
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
}

uint8_t TWI_GetStatus(void)
{
    uint8_t status;
    //mask status
    status = TWSR & 0xF8;
    return status;
}

void TWI_SetState(uint8_t chip_address, uint8_t data)
{
    TWI_Start();

    // chip address
    TWI_Write((0xc<<4) | ((chip_address & 0xF) << 1));

    // subaddress command - store & write
    TWI_Write(0x44);

    // data bits
    TWI_Write(data);

    TWI_Stop();
}
