/* File to read BMP280 using Bosch Sensortec API Drivers
 *
 * Blog Electronica y Ciencia
 * https://electronicayciencia.blogspot.com
 *
 * Compiling command:
 * gcc -l wiringPi -o read_bmp280 read_bmp280.c bmp280.c sotf_i2c.c
 *
 * Reinoso G.
 * 06/09/2018
 */

#include <stdio.h>
#include <stdlib.h>
#include "soft_i2c.h"
#include "bmp280.h"
//#include "bmp280_defs.h"

#define SCL_PIN 4
#define SDA_PIN 5

i2c_t i2c; // global, i2c bus


/* Interface for write:
 *  Example: rslt = dev->write(dev->dev_id, reg_addr[0], temp_buff, temp_len);
 */
int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
	//puts("call to write");
	i2c_start(i2c);

	if (i2c_send_byte(i2c, dev_id << 1 | I2C_WRITE) != I2C_ACK) {
		fprintf(stderr, "No device found at address %02x.\n", dev_id);
	}

	i2c_send_byte(i2c, reg_addr);

	int i;
	for (i = 0; i < len; i++) {
		if (i2c_send_byte(i2c, data[i]) != I2C_ACK) {
			fprintf(stderr, "Failed to write I2C bus.\n");
		}
	}
	
	i2c_stop(i2c);

	return BMP280_OK;
}


/* Interface for read:
 * Example: rslt = dev->read(dev->dev_id, reg_addr, reg_data, len);
 */
int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
	//puts("call to read");
	i2c_start(i2c);

	if (i2c_send_byte(i2c, dev_id << 1 | I2C_WRITE) != I2C_ACK) {
		fprintf(stderr, "No device found at address %02x.\n", dev_id);
	}

	i2c_send_byte(i2c, reg_addr);

	i2c_start(i2c);
	i2c_send_byte(i2c, dev_id << 1 | I2C_READ);

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


/* Interface for delay_ms:
 * Example: dev->delay_ms(2);
 */
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
		/* Sensor chip ID will be printed if initialization was successful */
		printf("Device found with chip id 0x%x\n", bmp.chip_id);
	}

	rslt = bmp280_get_config(&conf, &bmp);

	/* Overwrite the desired settings */
	conf.filter = BMP280_FILTER_OFF;
	conf.os_pres = BMP280_OS_8X;
	conf.os_temp = BMP280_OS_2X;
	conf.odr = BMP280_ODR_0_5_MS;

	rslt = bmp280_set_config(&conf, &bmp);

	/* Always set the power mode after setting the configuration */
	rslt = bmp280_set_power_mode(BMP280_NORMAL_MODE, &bmp);

	if (rslt != BMP280_OK) {
		printf("Failed to configure device.\n");
		exit(1);
	}

	uint8_t meas_dur = bmp280_compute_meas_time(&bmp);
	printf("Measurement duration: %dms\r\n", meas_dur);

	for (;;) {
		struct bmp280_uncomp_data ucomp_data;
	    bmp.delay_ms(meas_dur);

    	rslt = bmp280_get_uncomp_data(&ucomp_data, &bmp);
		
		if (rslt != BMP280_OK) {
			printf("Failed to read from device.\n");
			exit(1);
		}

	    double temp = bmp280_comp_temp_double(ucomp_data.uncomp_temp, &bmp);
	    double pres = bmp280_comp_pres_double(ucomp_data.uncomp_press, &bmp);

	    if (temp < 50 && temp > 0 && pres > 0)
			printf("T: %f, P: %f\n", temp, pres);
	
	    //bmp.delay_ms(10);  
	}
}
