// $Id$
/*
 * MIDIO 128 V3
 *
 * ==========================================================================
 *
 *  Copyright (C) 2011 Thorsten Klose (tk@midibox.org)
 *  Licensed for personal non-commercial use only.
 *  All other rights reserved.
 * 
 * ==========================================================================
 */

/////////////////////////////////////////////////////////////////////////////
// Include files
/////////////////////////////////////////////////////////////////////////////

#include <mios32.h>
#include "app.h"
#include "tasks.h"

#include "midio_sysex.h"
#include "midio_patch.h"
#include "midio_din.h"
#include "midio_dout.h"
#include "midio_matrix.h"
#include "midio_router.h"
#include "midio_port.h"

// include source of the SCS
#include <scs.h>
#include "scs_config.h"

#include "file.h"
#include "midio_file.h"
#include "midio_file_p.h"

#include <seq_bpm.h>
#include <seq_midi_out.h>
#include "seq.h"
#include "mid_file.h"

#include "terminal.h"
#include "midimon.h"

#include "uip_task.h"
#include "osc_client.h"


// define priority level for sequencer
// use same priority as MIOS32 specific tasks
#define PRIORITY_TASK_PERIOD_1mS ( tskIDLE_PRIORITY + 3 )

// local prototype of the task function
static void TASK_Period_1mS(void *pvParameters);

// define priority level for control surface handler
// use lower priority as MIOS32 specific tasks (2), so that slow LCDs don't affect overall performance
#define PRIORITY_TASK_PERIOD_1mS_LP ( tskIDLE_PRIORITY + 2 )

// local prototype of the task function
static void TASK_Period_1mS_LP(void *pvParameters);


/////////////////////////////////////////////////////////////////////////////
// local variables
/////////////////////////////////////////////////////////////////////////////
static u8 din_enabled;

// for mutual exclusive SD Card access between different tasks
// The mutex is handled with MUTEX_SDCARD_TAKE and MUTEX_SDCARD_GIVE
// macros inside the application, which contain a different implementation 
// for emulation
xSemaphoreHandle xSDCardSemaphore;

// Mutex for MIDI IN/OUT handler
xSemaphoreHandle xMIDIINSemaphore;
xSemaphoreHandle xMIDIOUTSemaphore;


static u32 ms_counter;


/////////////////////////////////////////////////////////////////////////////
// Local prototypes
/////////////////////////////////////////////////////////////////////////////
static void APP_Periodic_100uS(void);
static s32 NOTIFY_MIDI_Rx(mios32_midi_port_t port, u8 byte);



/////////////////////////////////////////////////////////////////////////////
// This hook is called after startup to initialize the application
/////////////////////////////////////////////////////////////////////////////
void APP_Init(void)
{
  // initialize all LEDs
  MIOS32_BOARD_LED_Init(0xffffffff);

  // DINs will be enabled once configuration has been loaded from SD Card
  // (resp. no SD Card is available)
  din_enabled = 0;

  // initialize all J10 pins as inputs with internal Pull-Up
  int pin;
  for(pin=0; pin<8; ++pin)
    MIOS32_BOARD_J10_PinInit(pin, MIOS32_BOARD_PIN_MODE_INPUT_PU);


  // create semaphores
  xSDCardSemaphore = xSemaphoreCreateRecursiveMutex();
  xMIDIINSemaphore = xSemaphoreCreateRecursiveMutex();
  xMIDIOUTSemaphore = xSemaphoreCreateRecursiveMutex();

  // install SysEx callback
  MIOS32_MIDI_SysExCallback_Init(APP_SYSEX_Parser);

  // install MIDI Rx callback function
  MIOS32_MIDI_DirectRxCallback_Init(NOTIFY_MIDI_Rx);

  // initialize code modules
  MIDIO_PORT_Init(0);
  MIDIO_SYSEX_Init(0);
  MIDIO_ROUTER_Init(0);
  MIDIO_PATCH_Init(0);
  MIDIO_DIN_Init(0);
  MIDIO_DOUT_Init(0);
  MIDIO_MATRIX_Init(0);
  UIP_TASK_Init(0);
  SCS_Init(0);
  SCS_CONFIG_Init(0);
  TERMINAL_Init(0);
  MIDIMON_Init(0);
  MIDIO_FILE_Init(0);
  SEQ_MIDI_OUT_Init(0);
  SEQ_Init(0);

  // install timer function which is called each 100 uS
  MIOS32_TIMER_Init(1, 100, APP_Periodic_100uS, MIOS32_IRQ_PRIO_MID);

  // start tasks
  xTaskCreate(TASK_Period_1mS, (signed portCHAR *)"1mS", configMINIMAL_STACK_SIZE, NULL, PRIORITY_TASK_PERIOD_1mS, NULL);
  xTaskCreate(TASK_Period_1mS_LP, (signed portCHAR *)"1mS_LP", 2*configMINIMAL_STACK_SIZE, NULL, PRIORITY_TASK_PERIOD_1mS_LP, NULL);

}


/////////////////////////////////////////////////////////////////////////////
// This task is running endless in background
/////////////////////////////////////////////////////////////////////////////
void APP_Background(void)
{
  // endless loop
  while( 1 ) {
    // toggle the state of all LEDs (allows to measure the execution speed with a scope)
    MIOS32_BOARD_LED_Set(0xffffffff, ~MIOS32_BOARD_LED_Get());
  }
}


/////////////////////////////////////////////////////////////////////////////
// This hook is called when a MIDI package has been received
/////////////////////////////////////////////////////////////////////////////
void APP_MIDI_NotifyPackage(mios32_midi_port_t port, mios32_midi_package_t midi_package)
{
  // -> DOUT
  MIDIO_DOUT_MIDI_NotifyPackage(port, midi_package);

  // -> MIDI Router
  MIDIO_ROUTER_Receive(port, midi_package);

  // forward to MIDI Monitor
  // SysEx messages have to be filtered for USB0 and UART0 to avoid data corruption
  // (the SysEx stream would interfere with monitor messages)
  u8 filter_sysex_message = (port == USB0) || (port == UART0);
  MIDIMON_Receive(port, midi_package, ms_counter, filter_sysex_message);
}

/////////////////////////////////////////////////////////////////////////////
// This function parses an incoming sysex stream for MIOS32 commands
/////////////////////////////////////////////////////////////////////////////
s32 APP_SYSEX_Parser(mios32_midi_port_t port, u8 midi_in)
{
  // -> MIDIO
  MIDIO_SYSEX_Parser(port, midi_in);

  // -> MIDI Router
  MIDIO_ROUTER_ReceiveSysEx(port, midi_in);

  return 0; // no error
}

/////////////////////////////////////////////////////////////////////////////
// This hook is called before the shift register chain is scanned
/////////////////////////////////////////////////////////////////////////////
void APP_SRIO_ServicePrepare(void)
{
  // pass current pin state of J10
  // only available for LPC17xx, all others (like STM32) default to SRIO
  SCS_AllPinsSet(0xff00 | MIOS32_BOARD_J10_Get());

  // update encoders/buttons of SCS
  SCS_EncButtonUpdate_Tick();

  // Matrix handler
  MIDIO_MATRIX_PrepareCol();
}


/////////////////////////////////////////////////////////////////////////////
// This hook is called after the shift register chain has been scanned
/////////////////////////////////////////////////////////////////////////////
void APP_SRIO_ServiceFinish(void)
{
  // Matrix handler
  MIDIO_MATRIX_GetRow();
}


/////////////////////////////////////////////////////////////////////////////
// This hook is called when a button has been toggled
// pin_value is 1 when button released, and 0 when button pressed
/////////////////////////////////////////////////////////////////////////////
void APP_DIN_NotifyToggle(u32 pin, u32 pin_value)
{
  // -> MIDIO_DIN once enabled
  if( din_enabled )
    MIDIO_DIN_NotifyToggle(pin, pin_value);
}


/////////////////////////////////////////////////////////////////////////////
// This hook is called when an encoder has been moved
// incrementer is positive when encoder has been turned clockwise, else
// it is negative
/////////////////////////////////////////////////////////////////////////////
void APP_ENC_NotifyChange(u32 encoder, s32 incrementer)
{
  // pass encoder event to SCS handler
  if( encoder == SCS_ENC_MENU_ID )
    SCS_ENC_MENU_NotifyChange(incrementer);
}


/////////////////////////////////////////////////////////////////////////////
// This hook is called when a pot has been moved
/////////////////////////////////////////////////////////////////////////////
void APP_AIN_NotifyChange(u32 pin, u32 pin_value)
{
}




/////////////////////////////////////////////////////////////////////////////
// This task handles the control surface
/////////////////////////////////////////////////////////////////////////////
static void TASK_Period_1mS_LP(void *pvParameters)
{
  u16 sdcard_check_ctr = 0;

  MIOS32_LCD_Clear();

  while( 1 ) {
    vTaskDelay(1 / portTICK_RATE_MS);

    // call SCS handler
    SCS_Tick();

    // each second: check if SD Card (still) available
    if( ++sdcard_check_ctr >= 1000 ) {
      sdcard_check_ctr = 0;

      MUTEX_SDCARD_TAKE;
      s32 status = FILE_CheckSDCard();

      din_enabled = 1; // enable the DINs after first read...

      if( status == 1 ) {
	DEBUG_MSG("SD Card connected: %s\n", FILE_VolumeLabel());
	// load all file infos
	MIDIO_FILE_LoadAllFiles(1); // including HW info
      } else if( status == 2 ) {
	DEBUG_MSG("SD Card disconnected\n");
	// invalidate all file infos
	MIDIO_FILE_UnloadAllFiles();

	// stop sequencer
	SEQ_BPM_Stop();

	// change filename
	sprintf(MID_FILE_UI_NameGet(), "No SD Card");
      } else if( status == 3 ) {
	if( !FILE_SDCardAvailable() ) {
	  DEBUG_MSG("SD Card not found\n");
	  // change filename
	  sprintf(MID_FILE_UI_NameGet(), "No SD Card");
	} else if( !FILE_VolumeAvailable() ) {
	  DEBUG_MSG("ERROR: SD Card contains invalid FAT!\n");
	  // change filename
	  sprintf(MID_FILE_UI_NameGet(), "No FAT");
	} else {
	  // check if patch file exists
	  if( !MIDIO_FILE_P_Valid() ) {
	    // create new one
	    DEBUG_MSG("Creating initial DEFAULT.MIO file\n");

	    if( (status=MIDIO_FILE_P_Write("DEFAULT")) < 0 ) {
	      DEBUG_MSG("Failed to create file! (status: %d)\n", status);
	    }
	  }

	  // change filename
	  sprintf(MID_FILE_UI_NameGet(), "SDCard found");
	}

	// reset sequencer
	SEQ_Reset(0);
      }

      MUTEX_SDCARD_GIVE;
    }
  }

}


/////////////////////////////////////////////////////////////////////////////
// This task is called periodically each mS to handle sequencer requests
/////////////////////////////////////////////////////////////////////////////
static void TASK_Period_1mS(void *pvParameters)
{
  portTickType xLastExecutionTime;

  // Initialise the xLastExecutionTime variable on task entry
  xLastExecutionTime = xTaskGetTickCount();

  while( 1 ) {
    vTaskDelayUntil(&xLastExecutionTime, 1 / portTICK_RATE_MS);

    // execute sequencer handler
    MUTEX_SDCARD_TAKE;
    SEQ_Handler();
    MUTEX_SDCARD_GIVE;

    // send timestamped MIDI events
    MUTEX_MIDIOUT_TAKE;
    SEQ_MIDI_OUT_Handler();
    MUTEX_MIDIOUT_GIVE;

    // Scan Matrix button handler
    MIDIO_MATRIX_ButtonHandler();
  }
}


/////////////////////////////////////////////////////////////////////////////
// This timer function is periodically called each 100 uS
/////////////////////////////////////////////////////////////////////////////
static void APP_Periodic_100uS(void)
{
  static u8 pre_ctr = 0;

  // increment the microsecond counter each 10th tick
  if( ++pre_ctr >= 10 ) {
    pre_ctr = 0;
    ++ms_counter;
  }

  // here we could do some additional high-prio jobs
  // (e.g. PWM LEDs)
}

/////////////////////////////////////////////////////////////////////////////
// Installed via MIOS32_MIDI_DirectRxCallback_Init
/////////////////////////////////////////////////////////////////////////////
static s32 NOTIFY_MIDI_Rx(mios32_midi_port_t port, u8 midi_byte)
{
  // here we could filter a certain port
  // The BPM generator will deliver inaccurate results if MIDI clock 
  // is received from multiple ports
  SEQ_BPM_NotifyMIDIRx(midi_byte);

  return 0; // no error, no filtering
}