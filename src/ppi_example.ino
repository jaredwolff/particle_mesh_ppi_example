/*
 * Project ppi_example
 * Description: PPI Example for Generating 88µS pulse for DMX over RS-485
 * Author: Jared Wolff
 * Date: 10/14/2019
 */

#undef CHG

#include "nrfx_timer.h"
#include "nrfx_ppi.h"
#include "nrfx_gpiote.h"

const uint8_t tx_pin = 6;
nrf_ppi_channel_t compare0,compare1;
nrfx_timer_t timer4 = NRFX_TIMER_INSTANCE(4);

void timerEventHandler(nrf_timer_event_t event_type, void * p_context);

// High speed timer event handler!
void timerEventHandler(nrf_timer_event_t event_type, void * p_context) {

  // Check the event type
  if( event_type == NRF_TIMER_EVENT_COMPARE0) {
    pinResetFast(D9);
  } else if ( event_type == NRF_TIMER_EVENT_COMPARE1 ) {
    pinSetFast(D9);
  }

}

// setup() runs once, when the device is first turned on.
void setup() {
  // Setup for timer control
  attachInterruptDirect(TIMER4_IRQn, nrfx_timer_4_irq_handler);

  // Timer configuration
  nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG;

  // Set priority as high as possible.
  timer_config.interrupt_priority = 3;

  // Init the timer
  uint32_t err_code = nrfx_timer_init(&timer4,&timer_config,timerEventHandler);
  if( err_code != NRF_SUCCESS ) Log.error("nrfx_timer_error");

  // Disable and clear the timer.
  nrfx_timer_disable(&timer4);
  nrfx_timer_clear(&timer4);

  // Arbitrary offset to start at
  uint32_t offset = 1000;

  // Calculate the ticks for 88 uS
  uint32_t ticks = nrfx_timer_us_to_ticks(&timer4,88);

  // Set the compare for the start and the end
  nrfx_timer_compare(&timer4, NRF_TIMER_CC_CHANNEL0, offset, false);
  nrfx_timer_compare(&timer4, NRF_TIMER_CC_CHANNEL1, ticks+offset, false);

  // Set pinmode for interrupt
  // pinMode(D9,OUTPUT);

  // Setup GPIOTE
  nrfx_gpiote_init();
  nrfx_gpiote_out_config_t gpiote_cfg;
  gpiote_cfg.init_state = NRF_GPIOTE_INITIAL_VALUE_HIGH;
  gpiote_cfg.task_pin = true;
  gpiote_cfg.action = NRF_GPIOTE_POLARITY_TOGGLE;
  err_code = nrfx_gpiote_out_init(tx_pin, &gpiote_cfg);

  // Allocate PPI channels
  err_code = nrfx_ppi_channel_alloc(&compare0);
  err_code = nrfx_ppi_channel_alloc(&compare1);

  // Assign events to the PPI channels
  err_code = nrfx_ppi_channel_assign(compare0,
                        nrfx_timer_event_address_get(&timer4,nrf_timer_compare_event_get(NRF_TIMER_CC_CHANNEL0)),
                        nrfx_gpiote_clr_task_addr_get(tx_pin));
  err_code = nrfx_ppi_channel_assign(compare1,
                        nrfx_timer_event_address_get(&timer4,nrf_timer_compare_event_get(NRF_TIMER_CC_CHANNEL1)),
                        nrfx_gpiote_set_task_addr_get(tx_pin));

  // Enable the PPI channels
  nrfx_ppi_channel_enable(compare0);
  nrfx_ppi_channel_enable(compare1);

  // Enable the task
  nrfx_gpiote_out_task_enable(tx_pin);

  // Enable the timer
  nrfx_timer_enable(&timer4);

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.

}