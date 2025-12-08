volatile bool _sensorInitDone = false;
//
// Metriche
//
//volatile int _sensorCountSecond = 0;
//volatile int _sensorMetricSecond = 0;

volatile unsigned long _sensorPrintTime = 0;
//
// CONST
//
// Accensione
// 
const int MKRCAN_IN_IGN_ORR  = 0;
const int MKRCAN_IN_IGN_VER = 1;
//
// Iniezione
//
const int MKRCAN_IN_INJ_ORR  = 4;
const int MKRCAN_IN_INJ_VER = 5;
//
// Private
//
volatile unsigned long _sensorStartIgnOrrMicrosec = 0;
volatile unsigned long _sensorCicleIgnOrrMicrosec = 0;

volatile unsigned long _sensorStartIgnVerMicrosec = 0;
volatile unsigned long _sensorCicleIgnVerMicrosec = 0;

volatile unsigned long _sensorStartInjOrrMicrosec = 0;
volatile unsigned long _sensorCicleInjOrrMicrosec = 0;

volatile unsigned long _sensorStartInjVerMicrosec = 0;
volatile unsigned long _sensorCicleInjVerMicrosec = 0;
//
// Setup
//
void Sensor_Setup()
{
  //
  // Pin setup
  //
  pinMode(MKRCAN_IN_IGN_ORR, INPUT_PULLUP);
  pinMode(MKRCAN_IN_IGN_VER, INPUT_PULLUP);
  pinMode(MKRCAN_IN_INJ_ORR, INPUT_PULLUP);
  pinMode(MKRCAN_IN_INJ_VER, INPUT_PULLUP);
  //
  // Interupt
  //
  attachInterrupt(digitalPinToInterrupt(MKRCAN_IN_IGN_ORR), Interrupt_IgnOrr, FALLING);
  attachInterrupt(digitalPinToInterrupt(MKRCAN_IN_IGN_VER), Interrupt_IgnVer, FALLING);
  attachInterrupt(digitalPinToInterrupt(MKRCAN_IN_INJ_ORR), Interrupt_InjOrr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(MKRCAN_IN_INJ_VER), Interrupt_InjVer, CHANGE);

  _sensorInitDone = true;
}

//
// Inturrupt
//
void Interrupt_IgnOrr()
{
  //
  // Esco se non inizializzato
  //
  if (! _sensorInitDone) exit;
  //
  // Microsecondi corrente
  //
  unsigned long now = micros();
  //
  // Controllo overflow(ogni 70 minuti)
  //
  if (now > _sensorStartIgnOrrMicrosec)
    _sensorCicleIgnOrrMicrosec = now - _sensorStartIgnOrrMicrosec;

  _sensorStartIgnOrrMicrosec = now;
}

void Interrupt_IgnVer()
{
  //
  // Esco se non inizializzato
  //
  if (! _sensorInitDone) exit;
  //
  // Microsecondi corrente
  //
  unsigned long now = micros();
  //
  // Controllo overflow(ogni 70 minuti)
  //
  if (now > _sensorStartIgnVerMicrosec)
    _sensorCicleIgnVerMicrosec = now - _sensorStartIgnVerMicrosec;

  _sensorStartIgnVerMicrosec = now;
}

void Interrupt_InjOrr()
{
  //
  // Esco se non inizializzato
  //
  if (! _sensorInitDone) exit;
  //
  // Microsecondi corrente
  //
  unsigned long now = micros();
  //
  // Controllo overflow(ogni 70 minuti)
  //
  if (now > _sensorStartInjOrrMicrosec)
    _sensorCicleInjOrrMicrosec = now - _sensorStartInjOrrMicrosec;

  _sensorStartInjOrrMicrosec = now;
}

void Interrupt_InjVer()
{
  //
  // Esco se non inizializzato
  //
  if (! _sensorInitDone) exit;
  //
  // Microsecondi corrente
  //
  unsigned long now = micros();
  //
  // Controllo overflow(ogni 70 minuti)
  //
  if (now > _sensorStartInjVerMicrosec)
    _sensorCicleInjVerMicrosec = now - _sensorStartInjVerMicrosec;

  _sensorStartInjVerMicrosec = now;
}

//
// Loop task
//
void Sensor_Loop()
{
  //
  // Esco se non inizializzato
  //
  if (! _sensorInitDone) exit;
  //
  // Valutazione timer per attività
  //
  unsigned long now = millis();
  //
  // Calcola fase
  //




  // ----------------------------------------------
  // Prescaler PRINT ogni 500 millisec
  // Con la sottrazione, mai overflow
  //  
  if( DEBUG_PRINT_SENSOR && (now - _sensorPrintTime > 500) )
  {
    _sensorPrintTime = now + 500;
    //
    // debug info
    //
    sensorPrintDebug();
  }

}


/**************************************************************************************
 * FUNCTION DEBUG
 **************************************************************************************/

void sensorPrintDebug()
{
  Serial.println();
  Serial.println("--- Sensor -----------------------------------------------------");
  
  Serial.print("| IgnOrr usec=");
  Serial.print(_sensorCicleIgnOrrMicrosec);

  Serial.print("\t| IgnVer usec=");
  Serial.print(_sensorCicleIgnVerMicrosec);

  Serial.print("| InjOrr usec=");
  Serial.print(_sensorCicleInjOrrMicrosec);

  Serial.print("\t| InjVer usec=");
  Serial.print(_sensorCicleInjVerMicrosec);

  Serial.println("\t|");

  Serial.println("-----------------------------------------------------------------");
}
