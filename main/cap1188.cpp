#include "cap1188.h"
#include <cstdio>

static const uint8_t spiRxPin = 0;
static const uint8_t spiCsPin = 1;
static const uint8_t spiClkPin = 2;
static const uint8_t spiTxPin = 3;
static const uint8_t spiResetPin = 4;

static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    asm volatile("nop \n nop \n nop");
}

bool CAP1188::init(void) {
  
  //spi_dev = new Adafruit_SPIDevice(cspin, clkpin, misopin, mosipin, 2000000);
  
  spi_init(spi0, 2000000);
  
  gpio_set_function(spiRxPin, GPIO_FUNC_SPI);
  gpio_set_function(spiClkPin, GPIO_FUNC_SPI);
  gpio_set_function(spiTxPin, GPIO_FUNC_SPI);
  
  gpio_init(spiCsPin);
  gpio_set_dir(spiCsPin, GPIO_OUT);
  cs_deselect();

  gpio_init(spiResetPin);
  gpio_set_dir(spiResetPin, GPIO_OUT);
  gpio_put(spiResetPin, 0);
  sleep_ms(100);
  gpio_put(spiResetPin, 1);
  sleep_ms(100);
  gpio_put(spiResetPin, 0);
  sleep_ms(100);

  readRegister(CAP1188_PRODID);

  // Useful debugging info

  printf("Product ID: 0x%x\r\n", readRegister(CAP1188_PRODID));
  printf("Manuf. ID: 0x%x\r\n", readRegister(CAP1188_MANUID));
  printf("Revision: 0x%x\r\n", readRegister(CAP1188_REV));

  if ((readRegister(CAP1188_PRODID) != 0x50) ||
      (readRegister(CAP1188_MANUID) != 0x5D) ||
      (readRegister(CAP1188_REV) != 0x83)) {
    return false;
  }
  // allow multiple touches
  writeRegister(CAP1188_MTBLK, 0);
  // Have LEDs follow touches
  writeRegister(CAP1188_LEDLINK, 0xFF);
  // speed up a bit
  writeRegister(CAP1188_STANDBYCFG, 0x30);
  return true;
}

uint8_t CAP1188::touched(void) {
  uint8_t t = readRegister(CAP1188_SENINPUTSTATUS);
  if (t) {
    writeRegister(CAP1188_MAIN, readRegister(CAP1188_MAIN) & ~CAP1188_MAIN_INT);
  }
  return t;
}

void CAP1188::LEDpolarity(uint8_t inverted) {
  writeRegister(CAP1188_LEDPOL, inverted);
}

uint8_t CAP1188::readRegister(uint8_t reg) {
  uint8_t buffer[3] = {reg, 0, 0};
  buffer[0] = 0x7D;
  buffer[1] = reg;
  buffer[2] = 0x7F;
  
  //spi_dev->write_then_read(buffer, 3, buffer, 1);
  
  cs_select();
  spi_write_blocking(spi_default, buffer, 1);
  //sleep_ms(10); -- hope this isn't important!
  spi_read_blocking(spi_default, 0, buffer, 1);
  cs_deselect();
  //sleep_ms(10); -- hope this isn't important!
  
  return buffer[0];
}

void CAP1188::writeRegister(uint8_t reg, uint8_t value) {
  uint8_t buffer[4] = {reg, value, 0, 0};
  buffer[0] = 0x7D;
  buffer[1] = reg;
  buffer[2] = 0x7E;
  buffer[3] = value;
  //spi_dev->write(buffer, 4);
  spi_write_blocking(spi_default, buffer, 4);
}