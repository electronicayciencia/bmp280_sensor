/* File to read BMP280 using Bosch Sensortec API Drivers
 *
 * Blog Electronica y Ciencia
 * https://electronicayciencia.blogspot.com
 *
 * GitHub
 * https://github.com/electronicayciencia/bmp280_sensor
 *
 * Basic compiling command. See Makefile.
 * gcc -l wiringPi -o read_bmp280 read_bmp280.c bmp280.c sotf_i2c.c
 *
 * Reinoso G.
 * 06/09/2018
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


void print_time_ms (void)
{
	int ms;
	time_t s;
	struct timespec spec;

	clock_gettime(CLOCK_REALTIME, &spec);

	s  = spec.tv_sec;
	ms = (int)((spec.tv_nsec / 1.0e6) + 0.5);
	if (ms > 999) {
		s++;
		ms = 0;
	}

	printf("%ld.%03d ", s, ms);
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
	rslt = bmp280_get_config(&conf, &bmp);

	/* Overwrite the desired settings */
	//conf.filter = BMP280_FILTER_OFF;
	conf.filter = BMP280_FILTER_COEFF_4;
	conf.os_pres = BMP280_OS_16X;
	conf.os_temp = BMP280_OS_2X;
	conf.odr = BMP280_ODR_0_5_MS;
	//conf.odr = BMP280_ODR_62_5_MS;

	rslt = bmp280_set_config(&conf, &bmp);

	/* Always set the power mode after setting the configuration */
	rslt = bmp280_set_power_mode(BMP280_NORMAL_MODE, &bmp);

	if (rslt != BMP280_OK) {
		printf("Failed to configure device.\n");
		exit(1);
	}

	uint8_t meas_dur = bmp280_compute_meas_time(&bmp);
	fprintf(stderr, "Measurement duration: %dms\r\n", meas_dur);

	/* Reading loop */
	for (;;) {
		struct bmp280_uncomp_data ucomp_data;
		bmp.delay_ms(meas_dur);

		rslt = bmp280_get_uncomp_data(&ucomp_data, &bmp);
		
		/* Check for apparently invalid values */
		if (rslt != BMP280_OK) {
			fprintf(stderr, "Warning: read error\n");
			continue;
		}

		if (  ucomp_data.uncomp_temp == 0xfffff || ucomp_data.uncomp_press == 0xfffff
		   || ucomp_data.uncomp_temp == 0x0 || ucomp_data.uncomp_press == 0x0) {
			fprintf(stderr, "Warning: read error\n");
			continue;
		}

		/* Convert */
		double temp = bmp280_comp_temp_double(ucomp_data.uncomp_temp, &bmp);
		double pres = bmp280_comp_pres_double(ucomp_data.uncomp_press, &bmp);

		if (temp < 50 && temp > 0 && pres > 93000 && pres < 96000) {
			print_time_ms();
			printf("\t%f\t%f\n", temp, pres);
		}
		else {
			fprintf(stderr, "Odd values: T:%f P:%f (T:%x P:%x)\n", 
					temp, pres, ucomp_data.uncomp_temp, ucomp_data.uncomp_press);
		}
	
		//bmp.delay_ms(10);
	}
}
