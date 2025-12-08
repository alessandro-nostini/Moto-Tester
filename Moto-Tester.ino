// LIBRERIE
//
// https://github.com/autowp/arduino-mcp2515
// https://github.com/arduino-libraries/Scheduler
//
// Usati nel file CanBus.ino
//
#ifndef INCLUDE_SPI_CAN 1
  #define INCLUDE_SPI_CAN 1
  #include <SPI.h>
  #include <mcp2515.h>
#endif
//
// Multiple tasks.
//
#include <Scheduler.h>
//
// Metriche
//
volatile int _nMain_CanBus_CountLoopSec = 0;
volatile int _nMain_CanBus_MetricLoopSec = 0;
//
// Task
//
volatile int _nMain_Task_CountLoopSec = 0;
volatile int _nMain_Task_MetricLoopSec = 0;
//
// Debug ed Info
//
const bool DEBUG_PRINT_MAIN = false;
const bool DEBUG_PRINT_CANBUS = false;
const bool DEBUG_PRINT_SENSOR = true;

#define Main_Debug(m) ({if (DEBUG_PRINT_MAIN) Serial.print(m);})
#define Main_DebugLine(m) ({if (DEBUG_PRINT_MAIN) Serial.println(m);})

#define CanBus_Debug(m) ({if (DEBUG_PRINT_CANBUS) Serial.print(m);})
#define CanBus_DebugLine(m) ({if (DEBUG_PRINT_CANBUS) Serial.println(m);})

//###########################################################################
// Setup della BOARD, timer e loop
//###########################################################################

void setup()
{
  //
  // Attivo seriale e aspetto la connessione
  //
  Serial.begin(9600);
  //
  // Analogic resolution
  //
  analogReadResolution(10);
  analogWriteResolution(10);
  //
  // Inizializzo i vari servizi
  //
  CanBus_Setup();
   //
  // Add other LOOP to scheduling.
  //
  Scheduler.startLoop(loop_Task);
  //
  // Add DEBUG to scheduling.
  //
  if (DEBUG_PRINT_MAIN)
    Scheduler.startLoop(loop_Metriche);
  //
  // Fine
  //
  Serial.println("Setup completato!");
}


//###########################################################################
// Loop principale, gestisce il CAN-BUS
//###########################################################################
//
// Condivido il controllo dello stato
//
volatile bool _bEngineMapLaunch = false;
volatile bool _bValvolaOpen = false;

//
// the loop routine runs over and over again forever:
//
void loop()
{
  CanBus_Loop();
  //
  // Incrementa contatore per metriche
  //
  _nMain_CanBus_CountLoopSec++;
  //
  // Tempo ad altro
  //
  yield();
}

//###########################################################################
// Loop, tramite SCHEDULER TASK
//###########################################################################

void loop_Task()
{
  //
  // Incrementa contatore per metriche
  //
  _nMain_Task_CountLoopSec++;
  //
  // Pausa e Tempo ad altro
  //
  delay(10);
}


//###########################################################################
// Loop metriche, tramite SCHEDULER TASK
//###########################################################################

void loop_Metriche()
{
  //
  // Metriche di processo al secondo
  //
  int nSamplingMillisec = 1000;

  _nMain_CanBus_MetricLoopSec = _nMain_CanBus_CountLoopSec * (1000 / nSamplingMillisec);
  _nMain_CanBus_CountLoopSec = 0;

  _nMain_Task_MetricLoopSec = _nMain_Task_CountLoopSec * (1000 / nSamplingMillisec);
  _nMain_Task_CountLoopSec = 0;
  //
  // Stampa le metriche
  //
  if (DEBUG_PRINT_MAIN)
    mainPintMetrics();
  //
  // Pausa e Tempo ad altro
  //
  delay(nSamplingMillisec);
}

/**************************************************************************************
 * FUNCTION DEBUG
 **************************************************************************************/

void mainPintMetrics()
{
  Serial.println();
  Serial.println("-----------------------------------------------------------------");
  Serial.println("--- MAIN Metric -------------------------------------------------");

  Serial.print("| CanBus loop/sec=");
  Serial.print(_nMain_CanBus_MetricLoopSec);

  Serial.print("\t| Task loop/sec=");
  Serial.print(_nMain_Task_MetricLoopSec);

  Serial.println("\t|");

  Serial.println("-----------------------------------------------------------------");
}
