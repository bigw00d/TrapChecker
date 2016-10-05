#include "mbed.h"

#define MYDEBUG

#define COMPLETE 1
#define INCOMPLETE 0
#define END_OF_PACKET '\n'
#define SIZE_OF_SEND_PACKET 16

#define CHECK_CHUTTER_US 500000
#define ALERT_CYCLE_SEC 1

#define BUZZER_ON  1
#define BUZZER_OFF 0
#define SW_LED_ON  1
#define SW_LED_OFF 0
#define ALERT_START  1
#define ALERT_STOP 0

#define ALERT_1 1
#define ALERT_2 2
#define ALERT_NON 0

DigitalOut myled(LED1);
#ifdef MYDEBUG
Serial pc(USBTX, USBRX);
#endif
Serial uart(PTE0, PTE1);  // tx, rx
DigitalOut buz(PTA13);
InterruptIn sw1(PTD5);
DigitalOut sw_led1(PTD0);
InterruptIn sw2(PTD3);
DigitalOut sw_led2(PTD2);

void offBuzzer();
void onBuzzer();
void toggleBuzzer();
void stopAlert1();
void stopAlert2();

uint8_t  receiveSize=0;
uint8_t  sendSize=0;
uint8_t  receiveComplete = INCOMPLETE;
uint8_t  sendComplete = INCOMPLETE;
uint8_t  rBuffIndex=0;
uint8_t  sBuffIndex=0;
char receiveBuff[50];
char sendBuff[50];
Ticker  chatTimer;
Ticker  AlertTimer;
uint8_t intCnt;
uint8_t buzzer_state=BUZZER_OFF;
uint8_t alert=ALERT_STOP;
uint8_t alert_number=ALERT_NON;

void periodicCallback1() {
    chatTimer.detach();
    intCnt = 0;
}

void push1() {
    if(intCnt == 0) {
        stopAlert1();
        chatTimer.attach_us(periodicCallback1, CHECK_CHUTTER_US); //start detecting chattering
    }
    intCnt++;
}

void periodicCallback2() {
    chatTimer.detach();
    intCnt = 0;
}

void push2() {
    if(intCnt == 0) {
        stopAlert2();
        chatTimer.attach_us(periodicCallback2, CHECK_CHUTTER_US); //start detecting chattering
    }
    intCnt++;
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

void offBuzzer()
{
    buzzer_state = BUZZER_OFF;
    buz = BUZZER_OFF;
}

void onBuzzer()
{
    buzzer_state = BUZZER_ON;
    buz = BUZZER_ON;
}

void toggleBuzzer()
{
    if(buzzer_state == BUZZER_OFF) {
        onBuzzer();
    }
    else {
        offBuzzer();
    }
}

void alertCallback()
{
    toggleBuzzer();
    if((alert_number & ALERT_1) != 0) {
        sw_led1 = !sw_led1;
    }
    if((alert_number & ALERT_2) != 0) {
        sw_led2 = !sw_led2;
    }
}

void startAlert1()
{
    offBuzzer();
    alert_number |= ALERT_1;
    sw_led1 = SW_LED_OFF;
    sw_led2 = SW_LED_OFF;
    if(alert_number == ALERT_1) {
        AlertTimer.attach(alertCallback, ALERT_CYCLE_SEC);
    }
}

void stopAlert1()
{
    sw_led1 = SW_LED_OFF;
    alert_number &= ~(ALERT_1);
    if(alert_number == ALERT_NON) {
        offBuzzer();
        AlertTimer.detach();
    }
}

void startAlert2()
{
    offBuzzer();
    alert_number |= ALERT_2;
    sw_led1 = SW_LED_OFF;
    sw_led2 = SW_LED_OFF;
    if(alert_number == ALERT_2) {
        AlertTimer.attach(alertCallback, ALERT_CYCLE_SEC);
    }
}

void stopAlert2()
{
    sw_led2 = SW_LED_OFF;
    alert_number &= ~(ALERT_2);
    if(alert_number == ALERT_NON) {
        offBuzzer();
        AlertTimer.detach();
    }
}

Ticker autoSendTimer;
#define AUTO_SEND_SEC (3.0) //3s
void AutoSend() { 
    autoSendTimer.detach();

    //send alert
    uart.putc('t');
    uart.putc('x');  
    uart.putc('d');  
    uart.putc('t');  
    uart.putc(' ');  
    uart.putc('0');  //src :0E44
    uart.putc('E'); 
    uart.putc('4'); 
    uart.putc('4'); 
    uart.putc('9');  //type :99
    uart.putc('9'); 
    uart.putc('5');  //dst :5176
    uart.putc('1'); 
    uart.putc('7'); 
    uart.putc('6'); 
    uart.putc('\r');  
    uart.putc('\n');

    autoSendTimer.attach(AutoSend, AUTO_SEND_SEC);
}

int main()
{
    receiveSize=0;
    sendSize=0;
    receiveComplete = INCOMPLETE;
    sendComplete = INCOMPLETE;
    rBuffIndex=0;
    sBuffIndex=0;
    myled = 1;
    sw_led1 = SW_LED_OFF;
    sw_led2 = SW_LED_OFF;
    offBuzzer();
    alert_number=ALERT_NON;

    intCnt = 0;
    sw1.mode(PullUp);
    sw1.fall(&push1); 
    sw2.mode(PullUp);
    sw2.fall(&push2); 

    uart.baud (19200);
#ifdef MYDEBUG
    pc.baud (9600);
#endif
    uart.attach(uartCB , Serial::RxIrq);
#ifdef MYDEBUG
    pc.attach(pcCB , Serial::RxIrq);
#endif

    // autoSendTimer.attach(AutoSend, AUTO_SEND_SEC);

    while (true) {
        sleep(); //wait for interrupt

        if(alert == ALERT_START) {
            alert = ALERT_STOP;
            toggleBuzzer();
            if((alert_number & ALERT_1) != 0) {
                sw_led1 = !sw_led1;
            }
            if((alert_number & ALERT_2) != 0) {
                sw_led2 = !sw_led2;
            }
        }
        if(receiveComplete == COMPLETE) {
#ifdef MYDEBUG
            pc.putc('r');
            pc.putc('V'); 
            pc.putc(':'); 
            for(rBuffIndex=0; rBuffIndex<receiveSize; ++rBuffIndex) {
                pc.putc(receiveBuff[rBuffIndex]);       // 送信
            }
#endif
            if(receiveSize > 7) {
                if(  //case:normal transmission
                        (receiveBuff[17] == '0') &&
                        (receiveBuff[18] == '0')
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
                    uart.putc('1');  
                    uart.putc(receiveBuff[20]); 
                    uart.putc(receiveBuff[21]); 
                    uart.putc(receiveBuff[23]); 
                    uart.putc(receiveBuff[24]); 
                    uart.putc('\r');  
                    uart.putc('\n');  
                }
                else if(  //case:check conection
                        (receiveBuff[17] == '0') && // ****03010* ex.0010030101
                        (receiveBuff[18] == '3') &&
                        (receiveBuff[20] == '0') &&
                        (receiveBuff[21] == '1') &&
                        (receiveBuff[23] == '0')
                ) {
                    uart.putc('t'); //send reverse src & dst data
                    uart.putc('x');  
                    uart.putc('d');  
                    uart.putc('t');  
                    uart.putc(' ');  
                    uart.putc(receiveBuff[20]); 
                    uart.putc(receiveBuff[21]); 
                    uart.putc(receiveBuff[23]); 
                    uart.putc(receiveBuff[24]); 
                    uart.putc('0'); //no change type
                    uart.putc('3');
                    uart.putc(receiveBuff[11]); 
                    uart.putc(receiveBuff[12]); 
                    uart.putc(receiveBuff[14]); 
                    uart.putc(receiveBuff[15]); 
                    uart.putc('\r');  
                    uart.putc('\n');  
                }
                else if( 
                        (receiveBuff[3] == '0') &&
                        (receiveBuff[4] == 'D') &&
                        (receiveBuff[5] == '8') &&
                        (receiveBuff[6] == '5')
                ) {
                    if(
                            (receiveBuff[11] == '5') &&
                            (receiveBuff[12] == '1') &&
                            (receiveBuff[14] == '7') &&
                            (receiveBuff[15] == '6') &&
                            (receiveBuff[17] == '0') &&
                            (receiveBuff[18] == '2')
                    ) {
                        uart.putc('t');
                        uart.putc('x');  
                        uart.putc('d');  
                        uart.putc('t');  
                        uart.putc(' ');  
                        uart.putc('0'); 
                        uart.putc('D'); 
                        uart.putc('8'); 
                        uart.putc('5'); 
                        uart.putc('0'); 
                        uart.putc('2'); 
                        uart.putc('\r');  
                        uart.putc('\n');  
                    }
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
