/***************************************************************
* PHT LAB: Photonics Technology Team, NECTEC, NSTDA, THAILAND
* Version: 1.03
* Updated: 2019-01-12
* By: Armote Somboonkaew
* Contact: armote@nectec.or.th
****************************************************************/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "esp_system.h"
#include "kidbright32.h"
#include "chemkid.h"

#define F_CPU (CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ * 1000000U)
#define CYCLES_800_T0H  (F_CPU / 2500000) //=64 // 0.4us        ///160*1000000/2500000=64
#define CYCLES_800_T1H  (F_CPU / 1250000) //=1066.6667 // 0.8us ///160*1000000/150000=1066.667
#define CYCLES_800      (F_CPU /  800000) // 1.25us per bit     ///160*1000000/800000=200
#define BLUE1 128

static uint32_t _getCycleCount(void) __attribute__((always_inline));
static void led_control(uint8_t r1, uint8_t g1, uint8_t b1) __attribute__((always_inline));
int count=0;
bool first_start = 1;

static inline uint32_t _getCycleCount(void)
{
  uint32_t ccount;
  __asm__ __volatile__("rsr %0,ccount":"=a" (ccount));
  return ccount;
}

static inline void led_control(uint8_t r1, uint8_t g1, uint8_t b1){
	uint8_t *p, *end1, pix, mask;
	uint32_t t, time0, time1, period, c, startTime, pinMask;
  uint8_t parr[3]={0,0,0};//={0,0,0,0,0};//G R B
  parr[0]=g1;//g
  parr[1]=r1;//r
  parr[2]=b1;//b
  p=parr;
  ///pinMask   = _BV(4);///pin=4 ///not use for me in ESP32
  end1      =  p+3;///numBytes=3 for our led
  pix       = *p++;
  mask      = 0x80;
  startTime = 0;
  time0  = CYCLES_800_T0H;
  time1  = CYCLES_800_T1H;
  period = CYCLES_800;

  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << CHAIN_SDA_GPIO);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_set_level(CHAIN_SDA_GPIO, 0); // active low
  gpio_config(&io_conf);
  gpio_set_level(CHAIN_SDA_GPIO, 0); // active low

  for(t = time0;; t = time0)
  {//for: 1
    if(pix & mask)
				t = time1;
		                       // Bit high duration
    while(((c = _getCycleCount()) - startTime) < period); // Wait for bit start

		gpio_set_level(CHAIN_SDA_GPIO, 1);

		startTime = c;
		                             // Save start time
    while(((c = _getCycleCount()) - startTime) < t);      // Wait high duration

		gpio_set_level(CHAIN_SDA_GPIO, 0);
    if(!(mask >>= 1))
    {//if: 1                                  // Next bit/byte
      if(p >= end1)
					break;
      pix  = *p++;
      mask = 0x80;
    }//if: 1
  }//for: 1
  while((_getCycleCount() - startTime) < period); // Wait for last bit

}//led_coltrol main

void CHEMKID::init(void) {// init
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << CHAIN_SDA_GPIO);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_set_level(CHAIN_SDA_GPIO, 0);
  gpio_config(&io_conf);
  gpio_set_level(CHAIN_SDA_GPIO, 0);
  initialized = true;
	first_read = true;
	illuminance = 111;
	state = s_led_on;
	first_start=1;
}// init

void CHEMKID::process(Driver *drv) {//process: ...
  I2CDev *i2c = (I2CDev *)drv;
	uint8_t byte, data[8];
  double rr, gg, bb, cc;
  if(first_start){
    led_control(ck_redled, ck_greenled, ck_blueled);
    if((count++)>10)
      first_start=0;
  }

  switch (state) {//switch: 1
		case s_led_on:
			state = s_detect;
		break;

		case s_detect://s_detect: 1
			polling_tickcnt = get_tickcnt();
			if (i2c->detect(channel, address) == ESP_OK) {//detect1
				data[0] = 0x41; // BH1745NUC_REG_MODE_CONTROL_1: 0x41
				data[1] = ck_exposure_time;// 0x02; // Exposure: 160msset0x00, 320msset0x01, 640msset0x02, 1280msSet0x03, 2560msSet0x04, 5120msSet0x05
				if (i2c->write(channel, address, data, 2) == ESP_OK) {//write: 1
					data[0] = 0x42; // BH1745NUC_REG_MODE_CONTROL_2: 0X42
					data[1] = ck_gain;// 0x12; // (RGBC_ON=0x10, OFF=0x00) OR with (Gain: G1Set0x00, G2Set0x01, G16Set0x02)
					if (i2c->write(channel, address, data, 2) == ESP_OK) {//write: 2
            data[0] = 0x43;
            data[1] = 0x02;//control reg mode 3
            if (i2c->write(channel, address, data, 2) == ESP_OK) {//write: 3
							error = false;
							tickcnt = get_tickcnt();
							state = s_get_lux;
            }//write 3
            else{
              state = s_error;
            }
					}//write: 2
					else {
						state = s_error;
					}
				}//write: 1
				else {
					state = s_error;
				}
			}//detect: 1
			else {
				state = s_error;
			}
			break;//s_detect: 1

		case s_get_lux://s_getlux: 1
  		if (is_tickcnt_elapsed(tickcnt, 500) || (!first_read)) {//tickcnt and !first_read
  			first_read = false;
  			byte = 0x50; //Read Color: redLSB0x50-MSB0x51, greenLSB0x52-MSB0x53, blueLSB0x54-MSB0x55, clearLSB0x56-MSB0x57
  			if (i2c->read(channel, address, &byte, 1, data, 2) == ESP_OK) {//read: 1 red
          byte = 0x52;
          if (i2c->read(channel, address, &byte, 1, &data[2], 2) == ESP_OK) {//read: 2 green
      				byte = 0x54; //Read Color: redLSB0x50-MSB0x51, greenLSB0x52-MSB0x53, blueLSB0x54-MSB0x55, clearLSB0x56-MSB0x57
      				if (i2c->read(channel, address, &byte, 1, &data[4], 2) == ESP_OK) {//read: 3 blue
                byte = 0x56;
                if (i2c->read(channel, address, &byte, 1, &data[6], 2) == ESP_OK) {//read: 4 clear
                  rr = ((uint16_t)data[0] | ((uint16_t)data[1] << 8));
                  gg = ((uint16_t)data[2] | ((uint16_t)data[3] << 8));
                  bb = ((uint16_t)data[4] | ((uint16_t)data[5] << 8));
                  cc = ((uint16_t)data[6] | ((uint16_t)data[7] << 8));
                  switch(ck_output1){
                    case 0:illuminance = rr;break;
                    case 1:illuminance = gg;break;
                    case 2:illuminance = bb;break;
                    case 3:illuminance = cc;
                  }
                  fprintf(stdout, "%0.2f, %0.2f, %0.2f, %0.2f\n", bb, rr, gg, cc);
        					initialized = true;
        					tickcnt = polling_tickcnt;
        					state = s_wait;
                }//read 4
                else{ //read 4
                  state = s_error;
                }
      				}//read: 3
              else{
                state = s_error;
              }
            }//read 2
  				else {
  					state = s_error;
  				}
  			}//read: 1
  			else {
  				state = s_error;
  			}
  		}//tickcnt and !first_read
			break;//s_getlux: 1

		case s_error://s_error:
			error = true;
			initialized = false;
			first_read = true;
			tickcnt = get_tickcnt();
			state = s_wait;
			break;//s_error: 1

		case s_wait://s_wait: 1
			// wait polling_ms timeout
			if (is_tickcnt_elapsed(tickcnt, polling_ms)) {
				state = s_led_on;//s_detect;
			}
			break;//s_wait: 1
	}//switch: 1

}//process: ...

CHEMKID::CHEMKID(int det_redled, int det_greenled, int det_blueled, int det_gain, int det_exposure_time, int det_output1) {
  ///ck_ledpin = det_ledpin;// chemkid led_pin
  ck_redled = det_redled;// chemkid redled
  ck_greenled = det_greenled;// chemkid greenled
  ck_blueled = det_blueled;// chemkid blueled
  channel = 0;/////bus_ch;
	address = 0x38;/////dev_addr;
  ck_gain = det_gain;// chemkid gain
  ck_exposure_time = det_exposure_time;// chemkid exposure
  ////ck_response_time = det_response_time;// chemkid exposure
  ck_output1 = det_output1;
	polling_ms = CHEMKID_POLLING_MS;
}
int CHEMKID::prop_count(void) {
	return 1;
}
bool CHEMKID::prop_name(int index, char *name) {
	if (index == 0) {
		snprintf(name, DEVICE_PROP_NAME_LEN_MAX, "%s", "illuminance");
		return true;
	}
	// not supported
	return false;
}
bool CHEMKID::prop_unit(int index, char *unit) {
	if (index == 0) {
		snprintf(unit, DEVICE_PROP_UNIT_LEN_MAX, "%s", "lux");
		return true;
	}
	// not supported
	return false;
}

bool CHEMKID::prop_attr(int index, char *attr) {
	if (index == 0) {
		get_attr_str(attr, PROP_ATTR_READ_FLAG | PROP_ATTR_TYPE_NUMBER_FLAG); // read only, number
		return true;
	}
	// not supported
	return false;
}

bool CHEMKID::prop_read(int index, char *value) {
	if (index == 0) {
		snprintf(value, DEVICE_PROP_VALUE_LEN_MAX, "%f", illuminance);
		return true;
	}
	// not supported
	return false;
}

bool CHEMKID::prop_write(int index, char *value) {
	// not supported
	return false;
}

double CHEMKID::get_illuminance(void) {
	return illuminance;
}
