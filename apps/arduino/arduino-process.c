/*
 * Copyright (c) 2014, Ralf Schlatterbeck Open Source Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \addgroup Arduino Process
 *
 * This wraps the Arduino-API entry points `loop` and `setup` in a
 * contiki process.
 *
 * If the normal contiki includes are used and resources initialized in
 * `setup`, Contiki resources can be used in an arduino sketch.
 *
 * @{
 */

/**
 * \file
 *        Wrapper for Arduino sketches
 * \author
 *        Ralf Schlatterbeck <rsc@runtux.com>
 *
 */

#include <stdlib.h>
#include <string.h>
#include "arduino-process.h"
#include "hw_timer.h"
#include "adc.h"
#include "hw-arduino.h"
#include "contiki.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif


extern volatile uint8_t mcusleepcycle;
#if PLATFORM_HAS_BUTTON
#include "rest-engine.h"
#include "dev/button-sensor.h"
extern resource_t res_event, res_separate;
#endif /* PLATFORM_HAS_BUTTON */

volatile uint8_t mcusleepcycleval;

/*-------------- enabled sleep mode ----------------------------------------*/
void
mcu_sleep_init(void)
{
	mcusleepcycleval=mcusleepcycle;
}
void
mcu_sleep_on(void)
{
	mcusleepcycle= mcusleepcycleval;
}
/*--------------- disable sleep mode ---------------------------------------*/
void
mcu_sleep_off(void)
{
	mcusleepcycle=0;
}
/*---------------- set duty cycle value ------------------------------------*/
void
mcu_sleep_set(uint8_t value)
{
	mcusleepcycleval= value;
	mcusleepcycle = mcusleepcycleval;
}

PROCESS(arduino_sketch, "Arduino Sketch Wrapper");

#ifndef LOOP_INTERVAL
#define LOOP_INTERVAL		(1 * CLOCK_SECOND)
#endif

PROCESS_THREAD(arduino_sketch, ev, data)
{
  static struct etimer loop_periodic_timer;

  PROCESS_BEGIN();
  adc_init ();
  mcu_sleep_init ();
  setup ();
  /* Define application-specific events here. */
  etimer_set(&loop_periodic_timer, LOOP_INTERVAL);

  while (1) {
	PROCESS_WAIT_EVENT();
#if PLATFORM_HAS_BUTTON
    if(ev == sensors_event && data == &button_sensor) {
      mcu_sleep_off();
      PRINTF("*******BUTTON*******\n");
      button ();
      mcu_sleep_on();
    }
#endif /* PLATFORM_HAS_BUTTON */

    if(etimer_expired(&loop_periodic_timer)) {
      mcu_sleep_off();
      loop ();
      mcu_sleep_on();
      etimer_reset(&loop_periodic_timer);
    }
  }
  PROCESS_END();
}

/*
 * VI settings, see coding style
 * ex:ts=8:et:sw=2
 */

/** @} */
