//
// Modulo di controllo della lettura del CAN-BUS
//
#ifndef INCLUDE_SPI_CAN 1
  #define INCLUDE_SPI_CAN 1
  #include <SPI.h>
  #include <mcp2515.h>
#endif
//
// https://github.com/autowp/arduino-mcp2515
//
volatile bool _CanbusInitDone = false;
//
// Metriche
//
volatile int _CountMessageSecond = 0;
volatile int _MetricMessageSecond = 0;

volatile int _CountErrorSecond = 0;
volatile int _MetricErrorSecond = 0;
volatile int _LastErrorCode = 0;

volatile int _NextPrintTime = 0;
volatile int _NextMetricsTime = 0;
//
// CONST
//
const int MKRCAN_EFI_CS_PIN  = 3;
const int MKRCAN_EFI_INT_PIN = 7;
//
// Global Var and object
//
MCP2515 _canBus(MKRCAN_EFI_CS_PIN);

// ----------------------------------------------------------------------
// Inizializza il controllo CANBUS
//
void CanBus_Setup()
{
  //
  // Set CS dei moduli CAN to disabled (H)
  // Per evitare che il set del primo, configuri erroneamente un eventuale secondo
  //  
  pinMode(MKRCAN_EFI_CS_PIN, OUTPUT);
  digitalWrite(MKRCAN_EFI_CS_PIN, HIGH);
  //
  // Config SPI
  //
  SPI.begin();
  //
  // Config can-bus
  //
  _canBus.reset();
  _canBus.setBitrate(CAN_1000KBPS);
  _canBus.setNormalMode();
  //
  // Inizializzazione completata
  //
  _CanbusInitDone = true;
}

// ----------------------------------------------------------------------
// Condivido con l'esterno tramite un poiner lo stato EngineMapLaunch
//
void CanBus_Loop()
{
  if (! _CanbusInitDone) exit;
  //
  // Valutazione timer per attività
  //
  int currentTime = millis();
  
  //
  // Pick and process all message in buffer
  //
  bool bMessageReaded = true;
  while(bMessageReaded)
  {
    bMessageReaded = pollingMessage();
    //
    // Contatore messaggi ricevuti
    //
    if (bMessageReaded) _CountMessageSecond++;
  };
    
  // ----------------------------------------------
  // Prescaler PRINT
  //  
  if( DEBUG_PRINT_CANBUS && currentTime > _NextPrintTime)
  {
    //
    // Prossimo print tra 500 ms
    //
    _NextPrintTime = millis() + 500;
    //
    // debug info
    //
    canbusPrintDebug();
  }

  // ----------------------------------------------
  // Prescaler SECOND calcolo Metriche
  //  
  if( currentTime > _NextMetricsTime)
  {
    //
    // Prossimo secondo passato
    //
    _NextMetricsTime = millis() + 1000;
    //
    // Metriche di processo
    //
    _MetricMessageSecond = _CountMessageSecond;
    _CountMessageSecond = 0;
    
    _MetricErrorSecond = _CountErrorSecond;
    _CountErrorSecond = 0;
  }
}

/**************************************************************************************
 * FUNCTION proccess message
 **************************************************************************************/

bool pollingMessage()
{
  //
  // Read and process message
  //
  int readState;
  struct can_frame frame;

  readState = _canBus.readMessage(&frame);
  if (readState == MCP2515::ERROR_OK)
  {
    //
    // frame received
    //
    processaMessage(&frame);

    return true;
  }
  else
  { return false; }
}

//
//Ordine dei byte
//Nel caso di una DWORD (32 bit), il numero esadecimale 0x01234567 verrà immagazzinato come:
//
//           Little endian             Big endian
//       +----+----+----+----+    +----+----+----+----+
//       |0x67|0x45|0x23|0x01|    |0x01|0x23|0x45|0x67|
//       +----+----+----+----+    +----+----+----+----+
// byte:    0    1    2    3         0    1    2    3

void processaMessage(can_frame *pCanMsg)
{
  // Formato Big endian
  // Processa il messaggio leggendo i dati dai vari ID conosciuti
  // Tutti i 300 provengono dalla EFI EURO2
  // Tutti i 200 provengono dalla AIM DASH
  // Scegliere appropriatamente gli ID perche la priorità è basata sul suo valore
  //    (0 più alta, 7FF la più bassa)
  //
  // Tutti i valori letti sono salvati globalmente in var STATIC
  //
  // if( pCanMsg->can_id == 0x300)
  // {
  //   _CanbusRpmValue = pCanMsg->data[1] + (pCanMsg->data[0] * 256);
  //   _CanbusTpsValue = (int)(pCanMsg->data[2] / 2.55);
  // }
 
}

/**************************************************************************************
 * FUNCTION Send message AIM
 **************************************************************************************/

void sendMessagge()
{
  //
  // Preparo messaggi
  //
  //struct can_frame canMsgAnalog;
  //
  // Read analog input EXT AIM chanell
  // Questi valori sono letti solo per essere inviati
  //
  // int iReadAnalog4 = analogRead(PIN_CAN_ANALOG1);
  // int iReadAnalog5 = analogRead(PIN_CAN_ANALOG2);
  // int iReadAnalog6 = analogRead(PIN_CAN_ANALOG3);

  // canMsgAnalog.can_id  = 0x402;
  // canMsgAnalog.can_dlc = 8;
  
  // canMsgAnalog.data[0] = lowByte(iReadAnalog4);
  // canMsgAnalog.data[1] = highByte(iReadAnalog4);
  
  // canMsgAnalog.data[2] = lowByte(iReadAnalog5);
  // canMsgAnalog.data[3] = highByte(iReadAnalog5);
  
  // canMsgAnalog.data[4] = lowByte(iReadAnalog6);
  // canMsgAnalog.data[5] = highByte(iReadAnalog6);

  // canMsgAnalog.data[6] = 0;
  // canMsgAnalog.data[7] = 0;
  
  // int errSent = 0;
  // errSent = _canBus.sendMessage(&canMsgAnalog);
  // if(errSent != 0)
  // {
  //   _LastErrorCode = errSent;
  //   _CountErrorSecond ++;
  // }
   
}

/**************************************************************************************
 * FUNCTION DEBUG
 **************************************************************************************/

void canbusPrintDebug()
{
  Serial.println();
  Serial.println("--- Metric ------------------------------------------------------");
  
  Serial.print("| Message sec=");
  Serial.print(_MetricMessageSecond);
  
  Serial.print("\t| Err sec=");
  Serial.print(_MetricErrorSecond);
  
  Serial.print("\t| Count Err=");
  Serial.print(_CountErrorSecond);
  
  Serial.print("\t| Last Err=");
  Serial.print(_LastErrorCode);

  Serial.println("\t|");

  Serial.println("-----------------------------------------------------------------");
}
