INCS=-I soft_i2c -I BMP280_driver
LIBS=-l wiringPi

all: read_bmp280

read_bmp280: read_bmp280.c
	gcc $(INCS) $(LIBS) -o read_bmp280 read_bmp280.c BMP280_driver/bmp280.c soft_i2c/soft_i2c.c

