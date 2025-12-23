//
// Modulo di controllo della lettura del CAN-BUS
//
#ifndef INCLUDE_DIP_RGB 1
  #define INCLUDE_DIP_RGB 1
  #include <ArduinoGraphics.h>
  #include <Arduino_MKRRGB.h>
#endif

volatile bool _sensorInitDone = false;
//
// Metriche
//
volatile unsigned long _sensorPrintTime = 0;
volatile unsigned long _sensorTestTime = 0;

volatile unsigned long _displyTime = 0;
volatile int _displyColumn = 0;

const int DISPLAY_TRACK_TIME  = 1000;
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
// in Interrupt
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

volatile unsigned long _sensorCicleRefTimeMicrosec = 0;
volatile unsigned long _sensorCicleRefValueMicrosec = 0;

volatile unsigned long _sensorOffCycleIgnOrrCount = 0;
volatile unsigned long _sensorOffCycleIgnVerCount = 0;
volatile unsigned long _sensorOffCycleInjOrrCount = 0;
volatile unsigned long _sensorOffCycleInjVerCount = 0;
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
  pinMode(MKRCAN_IN_IGN_ORR, INPUT_PULLUP);
  pinMode(MKRCAN_IN_IGN_VER, INPUT_PULLUP);
  pinMode(MKRCAN_IN_INJ_ORR, INPUT_PULLUP);
  pinMode(MKRCAN_IN_INJ_VER, INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); 
  //
  // Interupt
  //
  attachInterrupt(digitalPinToInterrupt(MKRCAN_IN_IGN_ORR), Interrupt_IgnOrr, FALLING);
  attachInterrupt(digitalPinToInterrupt(MKRCAN_IN_IGN_VER), Interrupt_IgnVer, FALLING);
  attachInterrupt(digitalPinToInterrupt(MKRCAN_IN_INJ_ORR), Interrupt_InjOrr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(MKRCAN_IN_INJ_VER), Interrupt_InjVer, CHANGE);

  //
  // Display RGB
  //
  MATRIX.begin();
  MATRIX.brightness(10);
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
    //
    // Calcolo riferimento e ritorna se in ciclo.
    //
    bool onCicle = CalcoloCicloRiferimento(_sensorCicleIgnOrrMicrosec);

    if (!onCicle)
      _sensorOffCycleIgnOrrCount++;
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
    //
    // Calcolo riferimento e ritorna se in ciclo.
    //
    bool onCicle = CalcoloCicloRiferimento(_sensorCicleIgnVerMicrosec);

    if (!onCicle)
      _sensorOffCycleIgnVerCount++;
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
  if (status == LOW)
  {
    _sensorOnInjOrrMicrosec = now;
  }
  else
  {
    //
    // HIGH status
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
      //
      // Calcolo riferimento e ritorna se in ciclo.
      //
      bool onCicle = CalcoloCicloRiferimento(_sensorCicleInjOrrMicrosec);

      if (!onCicle)
        _sensorOffCycleInjOrrCount++;
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
  if (status == LOW)
  {
    _sensorOnInjVerMicrosec = now;
  }
  else
  {
    //
    // HIGH status
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
      //
      // Calcolo riferimento e ritorna se in ciclo.
      //
      bool onCicle = CalcoloCicloRiferimento(_sensorCicleInjVerMicrosec);

      if (!onCicle)
        _sensorOffCycleInjVerCount++;     
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
  // Test
  //
  if (DEBUG_TEST_SIM && (millis() - _sensorTestTime > 300))
  {
    _sensorTestTime = millis();

    _sensorCicleRefValueMicrosec = 1000;
    _sensorCicleIgnOrrMicrosec = random(750, 1550);
    _sensorCicleIgnVerMicrosec = random(750, 1550);
    _sensorCicleInjOrrMicrosec = random(750, 1550);
    _sensorCicleInjVerMicrosec = random(750, 1550);

    if (_sensorCicleIgnOrrMicrosec > _sensorCicleRefValueMicrosec * 1.5 )
      _sensorOffCycleIgnOrrCount++;

    if (_sensorCicleIgnVerMicrosec > _sensorCicleRefValueMicrosec * 1.5 )
      _sensorOffCycleIgnVerCount++;

    if (_sensorCicleInjOrrMicrosec > _sensorCicleRefValueMicrosec * 1.5 )
      _sensorOffCycleInjOrrCount++;

    if (_sensorCicleInjVerMicrosec > _sensorCicleRefValueMicrosec * 1.5 )
      _sensorOffCycleInjVerCount++;
  }
  
  //
  // Valori mantenuti tra loop diversi
  // Valori letti al loop precedente
  //
  static int stOffCycleIgnOrr = 0;
  static int stOffCycleIgnVer = 0;
  static int stOffCycleInjOrr = 0;
  static int stOffCycleInjVer = 0;
  //
  // Esco se non inizializzato
  //
  if (! _sensorInitDone) exit;
  //
  // Valutazione timer per attività
  //
  unsigned long now = millis();
  //
  // Gestisco display
  //
  if( now - _displyTime > DISPLAY_TRACK_TIME)
  {
    _displyTime = now;
    //
    // Calcolo se nell'ultimo secondo è andato fuori ciclo
    //
    bool offCycleIgnOrr = (_sensorOffCycleIgnOrrCount > stOffCycleIgnOrr);
    stOffCycleIgnOrr = _sensorOffCycleIgnOrrCount;

    bool offCycleIgnVer = (_sensorOffCycleIgnVerCount > stOffCycleIgnVer);
    stOffCycleIgnVer = _sensorOffCycleIgnVerCount;

    bool offCycleInjOrr = (_sensorOffCycleInjOrrCount > stOffCycleInjOrr);
    stOffCycleInjOrr = _sensorOffCycleInjOrrCount;

    bool offCycleInjVer = (_sensorOffCycleInjVerCount > stOffCycleInjVer);
    stOffCycleInjVer = _sensorOffCycleInjVerCount;

    //
    // Draw display
    //
    MATRIX.beginDraw();
    //
    // Tracce completamente disegnata BLU con pallino ROSSO
    //
    for (int x = 0; x < 12; x++)
    {
      MATRIX.set(x, 0, x == _displyColumn ? 255 : 0,
                       x == _displyColumn ? 0 : 0,
                       x == _displyColumn ? 0 : 255);

    }
    //
    // Tracce incrementali pallino VERDE, errore ROSSO
    //
    int prevPoint = _displyColumn == 0 ? 11 : _displyColumn - 1;
    uint8_t prevColorR = 0;
    uint8_t prevColorG = 0;
    uint8_t prevColorB = 0;
    //
    // Canale IgnOrr
    //
    prevColorR = offCycleIgnOrr ? 255 : 0;
    MATRIX.set(prevPoint, 3, prevColorR, prevColorG, prevColorB);
    MATRIX.set(_displyColumn, 3, 0, 255, 0);
    //
    // Canale IgnVer
    //
    prevColorR = offCycleIgnVer ? 255 : 0;
    MATRIX.set(prevPoint, 4, prevColorR, prevColorG, prevColorB);
    MATRIX.set(_displyColumn, 4, 0, 255, 0);
    //
    // Canale InjOrr
    //
    prevColorR = offCycleInjOrr ? 255 : 0;
    MATRIX.set(prevPoint, 5, prevColorR, prevColorG, prevColorB);
    MATRIX.set(_displyColumn, 5, 0, 255, 0);
    //
    // Canale InjVer
    //
    prevColorR = offCycleInjVer ? 255 : 0;
    MATRIX.set(prevPoint, 6, prevColorR, prevColorG, prevColorB);
    MATRIX.set(_displyColumn, 6, 0, 255, 0);

    MATRIX.endDraw();
    //
    // Prossima colonna
    //
    if (_displyColumn < 11)
      _displyColumn++;
    else
      _displyColumn = 0;
  }

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


bool CalcoloCicloRiferimento(unsigned long valueMillisec)
{
  //
  // Microsecondi corrente
  //
  unsigned long now = micros();
  //
  // Calcolo riferimento. Reset ogni ciclo e mezzo
  //
  if (_sensorCicleRefTimeMicrosec > (now - _sensorCicleRefValueMicrosec * 1.5))
  {
    _sensorCicleRefTimeMicrosec = now;
    _sensorCicleRefValueMicrosec = valueMillisec;
  }
  else
  {
    _sensorCicleRefValueMicrosec = min(_sensorCicleRefValueMicrosec, valueMillisec);
  }
  //
  // Ritorna true se in ciclo
  //
  return (valueMillisec < _sensorCicleRefValueMicrosec * 1.5);
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

 Serial.println("--- Off Cycle -------------------------------------------------------------------");
 
  Serial.print("| IgnOrr count=");
  Serial.print(_sensorOffCycleIgnOrrCount);

  Serial.print("\t| IgnVer count=");
  Serial.print(_sensorOffCycleIgnVerCount);

  Serial.print("\t| InjOrr count=");
  Serial.print(_sensorOffCycleInjOrrCount);

  Serial.print("\t| InjVer count=");
  Serial.print(_sensorOffCycleInjVerCount);

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
