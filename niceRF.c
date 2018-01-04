/*
  ******************************************************************************
  * @file    niceRF.c
  * @author  Nice RF
  * @version V1.1
  * @date    04-January-2018
  * @brief   This file contains all the functions for the niceRF firmware 
  ******************************************************************************
  * @attention
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "niceRF.h"

byte SPIreadRegister(byte addr) 
{

 byte result;

 digitalWrite(SS, LOW); // Select LoRa module

 SPDR = addr; // Send address & Start transmission. In READ mode bit 7 of address is always 0! for sx1276
 while (!(SPSR & (1<<SPIF))) // Wait for transmission to finish
 {
 };
 result = SPDR; // Discard first reading

 SPDR = 0x0; // Sending dummy byte to get the result
 while (!(SPSR & (1<<SPIF))) // Wait for transmission to finish
 {
   };
 result = SPDR; // Reading register value

 digitalWrite(SS, HIGH); // Deselect LoRa module

 return (result);

}

byte SPIwriteRegister(byte addr,byte value)
{

 byte result;

 digitalWrite(SS, LOW); // Select LoRa module

 SPDR = addr | 0x80; // Send address & Start transmission. In WRITE mode bit 7 of address is always 1! for sx1276
 while (!(SPSR & (1<<SPIF))) // Wait for transmission to finish
 {
 };
 result = SPDR; // Discard first reading


 SPDR = value; // Sending byte
 while (!(SPSR & (1<<SPIF))) // Wait for transmission to finish
 {
 };
 result = SPDR; // Discard second reading

 digitalWrite(SS, HIGH); // Deselect LoRa module

}

void SPIwriteBurst(unsigned char addr, unsigned char *ptr, unsigned char len)
{
  unsigned char i;
  unsigned char result;

  digitalWrite(SS, LOW); // Select LoRa module

  SPDR = addr | 0x80; // Send address & Start transmission. In WRITE mode bit 7 of address is always 1! for sx1276
  while (!(SPSR & (1<<SPIF))) // Wait for transmission to finish
  {
  };
  result = SPDR; // Discard first reading

  for (i=0; i <= len; i++){

  SPDR = *ptr; // Sending bytes
  while (!(SPSR & (1<<SPIF))) // Wait for transmission to finish
  {
  };
  result = SPDR; // Discard second reading

  //DEBUG DEBUG DEBUG
  Serial.print(*ptr, HEX);
  //DEBUG DEBUG DEBUG
  ptr++;
  }

 //DEBUG DEBUG DEBUG
 Serial.print("\n");
 // DEBUG DEBUG DEBUG

 digitalWrite(SS, HIGH); // Deselect LoRa module
}

void SPIreadBurst(unsigned char addr, unsigned char *ptr, unsigned char len)
{
  unsigned char i;

  unsigned char result;

   digitalWrite(SS, LOW); // Select LoRa module

   SPDR = addr; // Send address & Start transmission. In READ mode bit 7 of address is always 0! for sx1276
   while (!(SPSR & (1<<SPIF))) // Wait for transmission to finish
   {
   };
   result = SPDR; // Discard first reading

   for (i=0; i <= len; i++){

   SPDR = 0; // Sending dummy byte to get the result
   while (!(SPSR & (1<<SPIF))) // Wait for transmission to finish
   {
   };
   *ptr = SPDR; // move pointer


   ptr++;
   }

   //DEBUG DEBUG DEBUG
   Serial.print("\n");
   // DEBUG DEBUG DEBUG

   digitalWrite(SS, HIGH); // Deselect LoRa module
}

void reset_sx1276(void)
{
   digitalWrite(TXEN, LOW);
   digitalWrite(RXEN, LOW);
   digitalWrite(NRESET, LOW);
   delay(10);
   digitalWrite(NRESET, HIGH);
   delay(20);
}


void Config_SX1276(void)
{
  // put in sleep mode to configure
  SPIwriteRegister(LR_RegOpMode,0x00); // sleep mode, high frequency
  delay(10);
  SPIwriteRegister(REG_LR_TCXO,0x09); // external crystal
  SPIwriteRegister(LR_RegOpMode,0x80); // LoRa mode, high frequency
  SPIwriteRegister(LR_RegFrMsb,0xE4);
  SPIwriteRegister(LR_RegFrMid,0xC0);
  SPIwriteRegister(LR_RegFrLsb,0x00); // frequency：915 MHz
  SPIwriteRegister(LR_RegPaConfig,0xFF); // max output power PA_BOOST enabled
  SPIwriteRegister(LR_RegOcp,0x0B); // close over current protection (ocp)
  SPIwriteRegister(LR_RegLna,0x23); // Enable LNA
  SPIwriteRegister(LR_RegModemConfig1,0x72); // signal bandwidth：125kHz,error coding= 4/5, explicit header mode

  SPIwriteRegister(LR_RegModemConfig2,0xC7); // spreading factor：12
  SPIwriteRegister(LR_RegModemConfig3,0x08); // LNA? optimized for low data rate

  SPIwriteRegister(LR_RegSymbTimeoutLsb,0xFF); // max receiving timeout

  SPIwriteRegister(LR_RegPreambleMsb,0x00);
  SPIwriteRegister(LR_RegPreambleLsb,16); // preamble 16 bytes
  SPIwriteRegister(REG_LR_PADAC,0x87); // transmission power 20dBm
  SPIwriteRegister(LR_RegHopPeriod,0x00); // no frequency hoping
  SPIwriteRegister(REG_LR_DIOMAPPING2,0x01); // DIO5=ModeReady,DIO4=CadDetected
  SPIwriteRegister(LR_RegOpMode,0x01); // standby mode, high frequency
}


void mode_tx(void)
{
  unsigned char addr,temp;
  digitalWrite(TXEN,HIGH); // open tx antenna switch
  digitalWrite(RXEN,LOW);

  SPIwriteRegister(REG_LR_DIOMAPPING1,0x41); // DIO0=TxDone,DIO1=RxTimeout,DIO3=ValidHeader
  SPIwriteRegister(LR_RegIrqFlags,0xff); // clearing interupt
  SPIwriteRegister(LR_RegIrqFlagsMask,0xf7); // enabling txdone
  SPIwriteRegister(LR_RegPayloadLength,payload_length); // payload length
  addr = SPIreadRegister(LR_RegFifoTxBaseAddr); // read TxBaseAddr
  SPIwriteRegister(LR_RegFifoAddrPtr,addr); // TxBaseAddr->FifoAddrPtr
  SPIwriteBurst(0x00,txbuf,payload_length); // write data in fifo
  SPIwriteRegister(LR_RegOpMode,0x03); // mode tx, high frequency

  digitalWrite(LED1, !digitalRead(LED1));

  temp=SPIreadRegister(LR_RegIrqFlags); // read interput flag
  while(!(temp&0x08)) // wait for txdone flag
  {
  temp=SPIreadRegister(LR_RegIrqFlags);

  }
  digitalWrite(TXEN,LOW); // close tx antenna switch
   digitalWrite(RXEN,LOW);

  SPIwriteRegister(LR_RegIrqFlags,0xff); // clearing interupt
  SPIwriteRegister(LR_RegOpMode,0x01); // standby mode, high frequency
}

void init_rx(void)
{
  unsigned char addr;
   digitalWrite(TXEN,LOW); // open rx antenna switch
   digitalWrite(RXEN,HIGH);

  SPIwriteRegister(REG_LR_DIOMAPPING1,0x01); //DIO0=00, DIO1=00, DIO2=00, DIO3=01 DIO0=00--RXDONE

  SPIwriteRegister(LR_RegIrqFlagsMask,0x3f); // enable rxdone and rxtimeout
  SPIwriteRegister(LR_RegIrqFlags,0xff); // clearing interupt
  addr = SPIreadRegister(LR_RegFifoRxBaseAddr); // read RxBaseAddr
  SPIwriteRegister(LR_RegFifoAddrPtr,addr); // RxBaseAddr->FifoAddrPtr
  SPIwriteRegister(LR_RegOpMode,0x05); // rx mode continuous high frequency
}

 
