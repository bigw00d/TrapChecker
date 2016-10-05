#include "mbed.h"

#define MYDEBUG

#define DEV_NO '1'

#define COMPLETE 1
#define INCOMPLETE 0
#define END_OF_PACKET '\n'
#define SIZE_OF_SEND_PACKET 16
    
#define CHECK_CHUTTER_US 100000 //100ms
#define FIRST_AUTO_SEND_SEC (0.1) //0.1s
#define AUTO_SEND_SEC (60.0) //60s
#define AUTO_SEND_TIMING (60) // AUTO_SEND_SEC x AUTO_SEND_TIMING sec
#define SW_CONNECT 1
#define SW_DISCONNECT 0

DigitalOut myled(LED1);
#ifdef MYDEBUG
Serial pc(USBTX, USBRX);
#endif
Serial uart(PTE0, PTE1);  // tx, rx
InterruptIn sw(PTA13);
DigitalIn level_sw(PTA13);


uint8_t  receiveSize=0;
uint8_t  sendSize=0;
uint8_t  receiveComplete = INCOMPLETE;
uint8_t  sendComplete = INCOMPLETE;
uint8_t  rBuffIndex=0;
uint8_t  sBuffIndex=0;
char receiveBuff[50];
char sendBuff[50];
Timeout chatTimer;
uint8_t intCnt;
uint8_t checkCnt;
uint8_t disconCnt;
uint8_t autoSendCnt=0;
uint8_t autoSendTiming=0;

Ticker autoSendTimer;

void AutoSend() { 
    autoSendTimer.detach();
    autoSendCnt++;

    if(autoSendCnt >= autoSendTiming) {
        autoSendCnt = 0;
        autoSendTiming = AUTO_SEND_TIMING;

        //send alert
        uart.putc('t');
        uart.putc('x');  
        uart.putc('d');  
        uart.putc('t');  
        uart.putc(' ');  
        uart.putc('0');  //src :000* *:device number
        uart.putc('0'); 
        uart.putc('0'); 
        uart.putc(DEV_NO); 
        uart.putc('0');  //type :00 
        uart.putc('0'); 
        uart.putc('0');  //dst :0010
        uart.putc('0'); 
        uart.putc('1'); 
        uart.putc('0'); 
        uart.putc('\r');  
        uart.putc('\n');
        
    }

    autoSendTimer.attach(AutoSend, AUTO_SEND_SEC);
}

void periodicCallback() {
    chatTimer.detach();
    checkCnt++;
    if(level_sw == SW_DISCONNECT) {
        disconCnt++;
        if(disconCnt >= 5) {
            
            //send alert
            disconCnt = 0;

            uart.putc('t');
            uart.putc('x');  
            uart.putc('d');  
            uart.putc('t');  
            uart.putc(' ');  
            uart.putc('0');  //src :000* *:device number
            uart.putc('0'); 
            uart.putc('0'); 
            uart.putc(DEV_NO); 
            uart.putc('0');  //type :00 
            uart.putc('0'); 
            uart.putc('0');  //dst :0010
            uart.putc('0'); 
            uart.putc('1'); 
            uart.putc('0'); 
            uart.putc('\r');  
            uart.putc('\n');

            //end check
            checkCnt = 0;
            intCnt = 0;
        }
        else {
            //recheck
            chatTimer.attach_us(periodicCallback, CHECK_CHUTTER_US);
        }
    }
    else {
        disconCnt = 0;
        if(checkCnt >= 20) {
            //end check
            checkCnt = 0;
            intCnt = 0;
        }
        else {
            //retry
            chatTimer.attach_us(periodicCallback, CHECK_CHUTTER_US);
        }
    }
}

void flip() {
    if(intCnt == 0) {
        intCnt++;
        checkCnt = 0;
        disconCnt = 0;
        chatTimer.attach_us(periodicCallback, CHECK_CHUTTER_US);
    }
    else {
        intCnt++;
    }
}

void uartCB(void)
{   
    char ch;

    while(uart.readable())    
    {
        ch = uart.getc();    
        if(receiveComplete == INCOMPLETE) {
            receiveBuff[receiveSize] = ch;
            receiveSize++;
            if(ch == END_OF_PACKET) {
                receiveComplete = COMPLETE;
            }
        }
    }
}

#ifdef MYDEBUG
void pcCB(void)
{   
    char ch;

    while(pc.readable())    
    {
        ch = pc.getc();    
        pc.putc('[');       // 送信
        pc.putc(ch);       // 送信
        sendBuff[sendSize] = ch;
        sendSize++;
        if(ch == END_OF_PACKET) {
            sendComplete = COMPLETE;
        }
    }
}
#endif

int main()
{
    receiveSize=0;
    sendSize=0;
    receiveComplete = INCOMPLETE;
    sendComplete = INCOMPLETE;
    rBuffIndex=0;
    sBuffIndex=0;
    myled = 1;

    intCnt = 0;
    checkCnt = 0;
    disconCnt = 0;
    sw.mode(PullUp);
    sw.fall(&flip); 
    
    uart.baud (19200);
#ifdef MYDEBUG
    pc.baud (9600);
#endif
    uart.attach(uartCB , Serial::RxIrq);
#ifdef MYDEBUG
    pc.attach(pcCB , Serial::RxIrq);
#endif

    autoSendCnt = 0;
    autoSendTiming = 1;
    autoSendTimer.attach(AutoSend, FIRST_AUTO_SEND_SEC); //test

    while (true) {
        sleep(); //wait for interrupt

        if(receiveComplete == COMPLETE) {
#ifdef MYDEBUG
            pc.putc('R'); 
            pc.putc('v');
            pc.putc(':'); 
            for(rBuffIndex=0; rBuffIndex<receiveSize; ++rBuffIndex) {
                pc.putc(receiveBuff[rBuffIndex]);       // 送信
            }
#endif
            if(receiveSize > 7) {
                if(  //case:check connection data
                        (receiveBuff[17] == '0') &&
                        (receiveBuff[18] == '2')
                ) {
                    uart.putc('t'); //send same src & dst data
                    uart.putc('x');  
                    uart.putc('d');  
                    uart.putc('t');  
                    uart.putc(' ');  
                    uart.putc(receiveBuff[11]); 
                    uart.putc(receiveBuff[12]); 
                    uart.putc(receiveBuff[14]); 
                    uart.putc(receiveBuff[15]); 
                    uart.putc('0'); //change type
                    uart.putc('3');  
                    uart.putc(receiveBuff[20]); 
                    uart.putc(receiveBuff[21]); 
                    uart.putc(receiveBuff[23]); 
                    uart.putc(receiveBuff[24]); 
                    uart.putc('\r');  
                    uart.putc('\n');  
                }
                else if(  //case:check connection data
                        (receiveBuff[17] == '0') &&
                        (receiveBuff[18] == '3')
                ) {
                    uart.putc('t'); //send same src & dst data
                    uart.putc('x');  
                    uart.putc('d');  
                    uart.putc('t');  
                    uart.putc(' ');  
                    uart.putc(receiveBuff[11]); 
                    uart.putc(receiveBuff[12]); 
                    uart.putc(receiveBuff[14]); 
                    uart.putc(receiveBuff[15]); 
                    uart.putc('0'); //change type
                    uart.putc('2');  
                    uart.putc(receiveBuff[20]); 
                    uart.putc(receiveBuff[21]); 
                    uart.putc(receiveBuff[23]); 
                    uart.putc(receiveBuff[24]); 
                    uart.putc('\r');  
                    uart.putc('\n');  
                }
            }
            receiveComplete = INCOMPLETE;
            receiveSize = 0;
        }
#ifdef MYDEBUG
        if(sendComplete == COMPLETE) {
            pc.putc('s'); 
            pc.putc('d'); 
            pc.putc(':'); 
            for(sBuffIndex=0; sBuffIndex<sendSize; ++sBuffIndex) {
                pc.putc(sendBuff[sBuffIndex]);       // 送信 
            }
            for(sBuffIndex=0; sBuffIndex<sendSize; ++sBuffIndex) {
                uart.putc(sendBuff[sBuffIndex]);       // 送信 
            }
            sendComplete = INCOMPLETE;
            sendSize = 0;
        }
#endif
    }
}
