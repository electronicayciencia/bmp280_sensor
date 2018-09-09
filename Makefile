INCS=-I soft_i2c -I BMP280_driver
LIBS=-l wiringPi
FLAGS=-O2 -pipe -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard

all: read_bmp280

read_bmp280: read_bmp280.c
	gcc $(FLAGS) $(INCS) $(LIBS) -o read_bmp280 read_bmp280.c BMP280_driver/bmp280.c soft_i2c/soft_i2c.c

