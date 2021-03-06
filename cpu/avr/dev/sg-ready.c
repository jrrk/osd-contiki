/*
 *  Copyright (c) 2015 Bernhard Trinnes
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file
 *
 * \brief
 *      Smart Grid Ready Module - guhRF
 *
 * \author
 *      Bernhard Trinnes bernhard.trinnes@guh.guru
	guh Gmbh
 *
 */

#include "sg-ready.h"
#include <util/delay.h>


#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif



/**
 * \addtogroup relay
 * \{
*/
/*---------------------------------------------------------------------------*/
/**
 * \brief  init RELAY PINS - direction & pull-ups
*/
void
sg_relay_init(uint8_t state)
{
   /*2 latching dual coal relay
   RELAY 1 Coil 1   PB4 - ON
           Coil 2   PB5 - OFF
           Feedback PD5
   RELAY 2 Coil 1   PB6 - ON
           Coil 2   PB7 - OFF
           Feedback PD7

   */

   PRINTF("SET DDRB\n");
   DDRB |= (RELAY1_ON | RELAY1_OFF | RELAY2_ON | RELAY2_OFF);
   PRINTF("SET PORTB\n");
   PORTB &= ~(RELAY1_ON | RELAY1_OFF | RELAY2_ON | RELAY2_OFF);

   PRINTF("SET DDRD\n");
   DDRD &= ~(RELAY1_FB | RELAY2_FB);
   PRINTF("SET PORTD\n");
   PORTD |= (RELAY1_FB | RELAY2_FB);

   PRINTF("SET STATE\n");
   sg_set_state(state); // set default state - heat pump normal operation
}

/**
 * \brief 
*/

void
sg_set_state(uint8_t state)
{
   /* State 1 - Relays 1:0
      State 2 - Relays 0:0  
      State 3 - Relays 0:1
      State 4 - Relays 1:1
   uint8_t i = 0;
   */
   uint8_t current_state;

   PRINTF("GET STATE\n");
   current_state = sg_get_state();

   PRINTF("SET DIFFERENCE: %u %u\n", current_state, state);
   sg_switch_difference(current_state, state);

   

/*   while ( i<=3 || (current_state != state)){
      i++;
      sg_switch_difference(current_state, state);
      current_state = sg_get_state();  
  }
*/ 
}
/**
 * \brief 
*/


uint8_t
sg_get_state()
{
   uint8_t state;
   /* Pull up -> Pin high = Relay Open */
   if ((~PIND & RELAY1_FB) && (~PIND & RELAY2_FB)){ 
      state = 4;
   }else if  (~PIND & RELAY1_FB){ 
      state = 1; 
   } else if (~PIND & RELAY2_FB){
      state = 3;
   } else {
      state = 2;
   }
   return state;
}
/**
 * \brief 
*/

void 
sg_switch_difference(uint8_t old_state, uint8_t new_state)
{
   switch(old_state) {
      case 1: 
          switch(new_state) {
              case 2: 
                 PORTB |=   RELAY1_OFF;
                 PORTB &= ~(RELAY1_ON | RELAY2_ON | RELAY2_OFF);
                 _delay_us(LATCH_TIME);
                 PORTB &= ~(RELAY1_OFF); 
              break;
              case 3: 
                 PORTB |=  (RELAY1_OFF | RELAY2_ON );
                 PORTB &= ~(RELAY1_ON  | RELAY2_OFF);
                 _delay_us(LATCH_TIME);
                 PORTB &= ~(RELAY1_OFF | RELAY2_ON);   
              break;
              case 4: 
                 PORTB |=   RELAY2_ON;
                 PORTB &= ~(RELAY1_ON | RELAY1_OFF | RELAY2_OFF);
                 _delay_us(LATCH_TIME);
                 PORTB &= ~(RELAY2_ON); 
              break;
          }
      break;
      case 2: 
          switch(new_state) {
              case 1: 
                 PORTB |=   RELAY1_ON;
                 PORTB &= ~(RELAY1_OFF | RELAY2_ON | RELAY2_OFF);
                 _delay_us(LATCH_TIME);
                 PORTB &= ~(RELAY1_ON); 
              break;
              case 3: 
                 PORTB |=  (RELAY2_ON);
                 PORTB &= ~(RELAY1_ON | RELAY1_OFF | RELAY2_OFF);
                 _delay_us(LATCH_TIME);
                 PORTB &= ~(RELAY2_ON); 
              break;
              case 4: 
                 PORTB |=  (RELAY1_ON  | RELAY2_ON);
                 PORTB &= ~(RELAY1_OFF | RELAY2_OFF);
                 _delay_us(LATCH_TIME);
                 PORTB &= ~(RELAY1_ON  | RELAY2_ON); 
              break;
          }
  
      break;
      case 3:
          switch(new_state) {
              case 1: 
                 PORTB |=  (RELAY1_ON  | RELAY2_OFF);
                 PORTB &= ~(RELAY1_OFF | RELAY2_ON);
                 _delay_us(LATCH_TIME);
                 PORTB &= ~(RELAY1_ON  | RELAY2_OFF); 
              break;
              case 2: 
                 PORTB |=   RELAY2_OFF;
                 PORTB &= ~(RELAY1_ON  | RELAY1_OFF | RELAY2_ON);
                 _delay_us(LATCH_TIME);
                 PORTB &= ~(RELAY2_OFF);   
              break;
              case 4: 
                 PORTB |=   RELAY1_ON;
                 PORTB &= ~(RELAY1_OFF | RELAY2_ON | RELAY2_OFF);
                 _delay_us(LATCH_TIME);
                 PORTB &= ~(RELAY1_ON); 
              break;
          }
      break;
      case 4: 
          switch(new_state) {
              case 1: 
                 PORTB |=   RELAY2_OFF;
                 PORTB &= ~(RELAY1_ON | RELAY1_OFF | RELAY2_ON);
                 _delay_us(LATCH_TIME);
                 PORTB &= ~(RELAY2_OFF); 
              break;
              case 2: 
                 PORTB |=  (RELAY1_OFF | RELAY2_OFF);
                 PORTB &= ~(RELAY1_ON  | RELAY2_ON );
                 _delay_us(LATCH_TIME);
                 PORTB &= ~(RELAY1_OFF | RELAY2_OFF);  
              break;
              case 3: 
                 PORTB |=   RELAY1_OFF;
                 PORTB &= ~(RELAY1_ON | RELAY2_ON | RELAY2_OFF);
                 _delay_us(LATCH_TIME);
                 PORTB &= ~(RELAY1_OFF); 
              break;
          }
      break;
   }
}

