/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#include "aos/init.h"
#include "board.h"
#include <aos/errno.h>
#include <aos/kernel.h>
#include <k_api.h>
#include <stdio.h>
#include <stdlib.h>
#include "led.h"
#include "hal_timer.h"
#include "hal_gpadc.h"

#define LED_CNT 5
#define FREQ_PRINT_INTERVAL 20
#define ADC_PRINT_INTERVAL 20

static void led_all(led_e onoff)
{
    int id;

    for (id = 1; id <= LED_CNT; id++) {
        led_switch(id, onoff);
    }
}

static void print_cpu_freq(void)
{
    uint32_t freq = hal_sys_timer_calc_cpu_freq(5, 0);

    printf("arch=%s cpu=%s, current CPU frequency = %u Hz (%u.%u MHz)\r\n",
           __AOS_ARCH__, __AOS_CPU__,
           freq, freq / 1000000, (freq % 1000000) / 100000);
}

static void adc_read_chan(enum HAL_GPADC_CHAN_T chan, const char *name)
{
    HAL_GPADC_MV_T volt = HAL_GPADC_BAD_VALUE;

    hal_gpadc_open(chan, HAL_GPADC_ATP_ONESHOT, NULL);
    aos_msleep(2);
    if (hal_gpadc_get_volt(chan, &volt)) {
        printf("  %-9s = %u mV\r\n", name, volt);
    } else {
        printf("  %-9s = <read failed>\r\n", name);
    }
    hal_gpadc_close(chan);
}

static void print_adc_channels(void)
{
    printf("internal ADC (GPADC) channels:\r\n");
    adc_read_chan(HAL_GPADC_CHAN_0, "chan0");
    adc_read_chan(HAL_GPADC_CHAN_BATTERY, "battery");
    adc_read_chan(HAL_GPADC_CHAN_2, "chan2");
    adc_read_chan(HAL_GPADC_CHAN_3, "chan3");
    adc_read_chan(HAL_GPADC_CHAN_4, "chan4");
    adc_read_chan(HAL_GPADC_CHAN_5, "chan5");
    adc_read_chan(HAL_GPADC_CHAN_6, "chan6");
    adc_read_chan(HAL_GPADC_CHAN_ADCKEY, "adckey");
}

int application_start(int argc, char *argv[])
{
    int count = 0;
    int id;

    printf("nano entry here!\r\n");
    print_cpu_freq();
    print_adc_channels();

    while (1) {
        printf("hello world! count %d \r\n", count++);

        if ((count % FREQ_PRINT_INTERVAL) == 0) {
            print_cpu_freq();
            print_adc_channels();
        }

        /* knight-rider LED sweep across LED1..LED5 (GPIO40,41,36,35,34) */
        for (id = 1; id <= LED_CNT; id++) {
            led_switch(id, LED_ON);
            aos_msleep(200);
            led_switch(id, LED_OFF);
        }

        /* light all LEDs briefly, then turn all off */
        led_all(LED_ON);
        aos_msleep(500);
        led_all(LED_OFF);

        aos_msleep(500);
    }

    return 0;
}
