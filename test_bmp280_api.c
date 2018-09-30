/* File to read BMP280 using Bosch Sensortec API Drivers
 *
 * Blog Electronica y Ciencia
 * https://electronicayciencia.blogspot.com
 *
 * GitHub
 * https://github.com/electronicayciencia/bmp280_sensor
 *
 * Basic compiling command. See Makefile.
 * gcc -l wiringPi -o test_bmp280_api test_bmp280_api.c bmp280.c sotf_i2c.c
 *
 * Reinoso G.
 * 30/09/2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "soft_i2c.h"
#include "bmp280.h"

/* Tried pins 3,4: not reccomended */
#define SCL_PIN 10
#define SDA_PIN 11

i2c_t i2c; // global, i2c bus


/* Custom I2C write function */
int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
	i2c_start(i2c);

	if (i2c_send_byte(i2c, dev_id << 1 | I2C_WRITE) != I2C_ACK) {
		fprintf(stderr, "No device found at address %02x.\n", dev_id);
		i2c_stop(i2c);
		return BMP280_E_COMM_FAIL;
	}

	if (i2c_send_byte(i2c, reg_addr) != I2C_ACK) {
		i2c_stop(i2c);
		return BMP280_E_COMM_FAIL;
	}

	int i;
	for (i = 0; i < len; i++) {
		if (i2c_send_byte(i2c, data[i]) != I2C_ACK) {
			fprintf(stderr, "Failed to write I2C bus.\n");
			i2c_stop(i2c);
			return BMP280_E_COMM_FAIL;
		}
	}
	
	i2c_stop(i2c);

	return BMP280_OK;
}


/* Custom I2C read function */
int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
	i2c_start(i2c);

	if (i2c_send_byte(i2c, dev_id << 1 | I2C_WRITE) != I2C_ACK) {
		fprintf(stderr, "No device found at address %02x.\n", dev_id);
		i2c_stop(i2c);
		return BMP280_E_COMM_FAIL;
	}

	if (i2c_send_byte(i2c, reg_addr) != I2C_ACK) {
		i2c_stop(i2c);
		return BMP280_E_COMM_FAIL;
	}

	i2c_start(i2c);

	if (i2c_send_byte(i2c, dev_id << 1 | I2C_READ) != I2C_ACK) {
		i2c_stop(i2c);
		return BMP280_E_COMM_FAIL;
	}

	int i = 0;
	data[i] = i2c_read_byte(i2c);

	for (i = 1; i < len; i++) {
		i2c_send_bit(i2c, I2C_ACK);
		data[i] = i2c_read_byte(i2c);
	}
	
	i2c_send_bit(i2c, I2C_NACK);
	i2c_stop(i2c);

	return BMP280_OK;
}


/* Custom delay function */
void user_delay_ms(uint32_t period) {
	usleep(period * 1000);
}


int main(int argc, char ** argv) {
	int8_t rslt;
	struct bmp280_dev bmp;
	struct bmp280_config conf;

	/* Init i2c bus */
	if (wiringPiSetup () == -1)
		return 1;

	i2c = i2c_init(SCL_PIN, SDA_PIN);

	/* Sensor interface over I2C with primary slave address  */
	bmp.dev_id = BMP280_I2C_ADDR_PRIM;
	bmp.intf = BMP280_I2C_INTF;
	bmp.read = user_i2c_read;
	bmp.write = user_i2c_write;
	bmp.delay_ms = user_delay_ms;

	rslt = bmp280_init(&bmp);

	if (rslt == BMP280_OK) {
		// Sensor chip ID will be printed if initialization was successful
		fprintf(stderr, "Device found with chip id 0x%x\n", bmp.chip_id);
	}
	else {
		fprintf(stderr, "Sorry, device not found.\n");
	}
}

