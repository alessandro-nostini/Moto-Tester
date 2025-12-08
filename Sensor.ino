volatile bool _sensorInitDone = false;
//
// Metriche
//
volatile unsigned long _sensorPrintTime = 0;
volatile unsigned long _sensorLedTime = 0;
//
// CONST
//
// Accensione
// 
const int MKRCAN_IN_IGN_ORR  = 6;
const int MKRCAN_IN_IGN_VER = 7;
//
// Iniezione
//
const int MKRCAN_IN_INJ_ORR  = 4;
const int MKRCAN_IN_INJ_VER = 5;
//
// Rele
//
const int MKRCAN_RELE_1  = 1;
const int MKRCAN_RELE_2 = 2;
//
// Private
//
volatile unsigned long _sensorStartIgnOrrMicrosec = 0;
volatile unsigned long _sensorCicleIgnOrrMicrosec = 0;
volatile long _sensorDeltaIgnOrrMicrosec = 0;

volatile unsigned long _sensorStartIgnVerMicrosec = 0;
volatile unsigned long _sensorCicleIgnVerMicrosec = 0;
volatile long _sensorDeltaIgnVerMicrosec = 0;

volatile unsigned long _sensorStartInjOrrMicrosec = 0;
volatile unsigned long _sensorCicleInjOrrMicrosec = 0;

volatile unsigned long _sensorOnInjOrrMicrosec = 0;
volatile unsigned long _sensorOffInjOrrMicrosec = 0;
volatile unsigned long _sensorOpenInjOrrMicrosec = 0;

volatile unsigned long _sensorStartInjVerMicrosec = 0;
volatile unsigned long _sensorCicleInjVerMicrosec = 0;

volatile unsigned long _sensorOnInjVerMicrosec = 0;
volatile unsigned long _sensorOffInjVerMicrosec = 0;
volatile unsigned long _sensorOpenInjVerMicrosec = 0;

volatile unsigned long _sensorIrqIgnOrrCount = 0;
volatile unsigned long _sensorIrqIgnVerCount = 0;
volatile unsigned long _sensorIrqInjOrrCount = 0;
volatile unsigned long _sensorIrqInjVerCount = 0;
//
// Setup
//
void Sensor_Setup()
{
  //
  // Pin setup
  //
  // INPUT_PULLUP
  //
  pinMode(MKRCAN_IN_IGN_ORR, INPUT);
  pinMode(MKRCAN_IN_IGN_VER, INPUT);
  pinMode(MKRCAN_IN_INJ_ORR, INPUT);
  pinMode(MKRCAN_IN_INJ_VER, INPUT);

  pinMode(MKRCAN_RELE_1, OUTPUT);
  pinMode(MKRCAN_RELE_2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(MKRCAN_RELE_1, LOW);
  digitalWrite(MKRCAN_RELE_2, LOW);
  digitalWrite(LED_BUILTIN, LOW); 
  //
  // Interupt
  //
  attachInterrupt(digitalPinToInterrupt(MKRCAN_IN_IGN_ORR), Interrupt_IgnOrr, RISING);
  attachInterrupt(digitalPinToInterrupt(MKRCAN_IN_IGN_VER), Interrupt_IgnVer, RISING);
  attachInterrupt(digitalPinToInterrupt(MKRCAN_IN_INJ_ORR), Interrupt_InjOrr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(MKRCAN_IN_INJ_VER), Interrupt_InjVer, CHANGE);
  //
  // Setup finito
  //
  _sensorInitDone = true;
}

//
// Interrupt
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
  // Per accensione misuro il ciclo
  //
  // Controllo overflow(ogni 70 minuti)
  // Calcolo ciclo e set start
  //
  if (now > _sensorStartIgnOrrMicrosec)
  {
    _sensorCicleIgnOrrMicrosec = now - _sensorStartIgnOrrMicrosec;
  }
  
  _sensorStartIgnOrrMicrosec = now;
  //
  // Calcolo delta Ign e fine Inj
  //
  _sensorDeltaIgnOrrMicrosec = now - _sensorOffInjOrrMicrosec;
  //
  // Inc status
  //
  _sensorIrqIgnOrrCount++;
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
  // Per accensione misuro il ciclo
  //
  // Controllo overflow (ogni 70 minuti)
  // Calcolo ciclo e set start
  //
  if (now > _sensorStartIgnVerMicrosec)
  {
    _sensorCicleIgnVerMicrosec = now - _sensorStartIgnVerMicrosec;
  }

  _sensorStartIgnVerMicrosec = now;
  //
  // Calcolo delta Ign e fine Inj
  //
  _sensorDeltaIgnVerMicrosec = now - _sensorOffInjVerMicrosec;
  //
  // Inc status
  //
  _sensorIrqIgnVerCount++;
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
  int status = digitalRead(MKRCAN_IN_INJ_ORR);
  //
  // Per iniezzione misuro i tempo di apertura
  //
  if (status == HIGH)
  {
    _sensorOnInjOrrMicrosec = now;
  }
  else
  {
    //
    // LOW status
    //
    _sensorOffInjOrrMicrosec = now; 
    //
    // Controllo overflow (ogni 70 minuti)
    // Calcolo tempo apertura
    //
    if (_sensorOffInjOrrMicrosec > _sensorOnInjOrrMicrosec) 
    {
      _sensorOpenInjOrrMicrosec = _sensorOffInjOrrMicrosec - _sensorOnInjOrrMicrosec;
    }
    //
    // Controllo overflow (ogni 70 minuti)
    // Calcolo ciclo e set start
    //
    if (now > _sensorStartInjOrrMicrosec)
    {
      _sensorCicleInjOrrMicrosec = now - _sensorStartInjOrrMicrosec;
    }

    _sensorStartInjOrrMicrosec = now;    
  }
  //
  // Inc counter
  //
  _sensorIrqInjOrrCount++;
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
  int status = digitalRead(MKRCAN_IN_INJ_VER);
  //
  // Per iniezzione misuro i tempo di apertura
  //
  if (status == HIGH)
  {
    _sensorOnInjVerMicrosec = now;
  }
  else
  {
    //
    // LOW status
    //
    _sensorOffInjVerMicrosec = now; 
    //
    // Controllo overflow (ogni 70 minuti)
    // Calcolo tempo apertura
    //
    if (_sensorOffInjVerMicrosec > _sensorOnInjVerMicrosec) 
    {
      _sensorOpenInjVerMicrosec = _sensorOffInjVerMicrosec - _sensorOnInjVerMicrosec;
    }
    //
    // Controllo overflow(ogni 70 minuti)
    // Calcolo ciclo e set start
    //
    if (now > _sensorStartInjVerMicrosec)
    {
      _sensorCicleInjVerMicrosec = now - _sensorStartInjVerMicrosec;
    }
  
    _sensorStartInjVerMicrosec = now;
  }
  //
  // Inc counter
  //
  _sensorIrqInjVerCount++;
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

  // if( now - _sensorLedTime > 1)
  // {
  //   _sensorLedTime = now;
  //   //
  //   // Segnale LED
  //   //
  //   PinStatus statusIgnOrr = digitalRead(MKRCAN_IN_IGN_ORR);
  //   digitalWrite(LED_BUILTIN, statusIgnOrr);
  // }

  // ----------------------------------------------
  // Prescaler PRINT ogni 500 millisec
  // Con la sottrazione, mai overflow
  //  
  if( DEBUG_PRINT_SENSOR && (now - _sensorPrintTime > 500) )
  {
    _sensorPrintTime = now;
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
  Serial.println("--- Cicle ------------------------------------------------------------------------");
  
  Serial.print("| IgnOrr usec=");
  Serial.print(_sensorCicleIgnOrrMicrosec);

  Serial.print("\t| IgnVer usec=");
  Serial.print(_sensorCicleIgnVerMicrosec);

  Serial.print("\t| InjOrr usec=");
  Serial.print(_sensorCicleInjOrrMicrosec);

  Serial.print("\t| InjVer usec=");
  Serial.print(_sensorCicleInjVerMicrosec);

  Serial.println("\t|");

  Serial.println("--- Open -------------------------------------------------------------------------");
  
  Serial.print("| InjOrr usec=");
  Serial.print(_sensorOpenInjOrrMicrosec);

  Serial.print("\t| InjVer usec=");
  Serial.print(_sensorOpenInjVerMicrosec);

  Serial.println("\t|");

  Serial.println("--- Delta Ign->Inj-----------------------------------------------------------------");
  
  Serial.print("| IgnInjOrr usec=");
  Serial.print(_sensorDeltaIgnOrrMicrosec);

  Serial.print("\t| IgnInjVer usec=");
  Serial.print(_sensorDeltaIgnVerMicrosec);

  Serial.println("\t|");

  Serial.println("--- Interrupt -------------------------------------------------------------------");
 
  Serial.print("| IgnOrr count=");
  Serial.print(_sensorIrqIgnOrrCount);

  Serial.print("\t| IgnVer count=");
  Serial.print(_sensorIrqIgnVerCount);

  Serial.print("\t| InjOrr count=");
  Serial.print(_sensorIrqInjOrrCount);

  Serial.print("\t| InjVer count=");
  Serial.print(_sensorIrqInjVerCount);

  Serial.println("\t|");

  Serial.println("-----------------------------------------------------------------");
}
