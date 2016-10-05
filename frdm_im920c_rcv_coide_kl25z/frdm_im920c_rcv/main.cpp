#include "mbed.h"

#define MYDEBUG

#define COMPLETE 1
#define INCOMPLETE 0
#define END_OF_PACKET '\n'
#define SIZE_OF_SEND_PACKET 16

#define CHECK_CHUTTER_US 500000
#define ALERT_CYCLE_SEC 1
#define ARERT_MAX_CNT 60

#define CHECK_CONNECT_SEC 2

#define BUZZER_ON  1
#define BUZZER_OFF 0
#define SW_LED_ON  1
#define SW_LED_OFF 0
#define ALERT_START  1
#define ALERT_STOP 0

// alert "bit" definition
#define ALERT_1 1
#define ALERT_2 2
#define ALERT_3 4
#define ALERT_NON 0

// connect "bit" definition
#define CONNECT_1 1
#define CONNECT_2 2
#define CONNECT_3 4
#define CONNECT_NON 0
#define ON_SIGNAL_1 1
#define ON_SIGNAL_2 2
#define ON_SIGNAL_3 4
#define ON_SIGNAL_NON 0

DigitalOut myled(LED1);
#ifdef MYDEBUG
Serial pc(USBTX, USBRX);
#endif
Serial uart(PTE0, PTE1);  // tx, rx
DigitalOut buz(PTA13);

InterruptIn sw1(PTD1);
DigitalOut sw_led1(PTB0);
InterruptIn sw2(PTD5);
DigitalOut sw_led2(PTD0);
InterruptIn sw3(PTD3);
DigitalOut sw_led3(PTD2);

void offBuzzer();
void onBuzzer();
void toggleBuzzer();
void stopAlert1();
void stopAlert2();
void stopAlert3();
void startAlert1();
void startAlert2();
void startAlert3();
void offCheckSignal1();
void offCheckSignal2();
void offCheckSignal3();


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
Ticker  CheckConnectTimer;
uint8_t intCnt;
uint8_t intCnt2;
uint8_t intCnt3;
uint8_t buzzer_state=BUZZER_OFF;
uint8_t alert=ALERT_STOP;
uint8_t alert_number=ALERT_NON;
uint8_t alert_cnt=0;

uint8_t check_connect=CONNECT_NON;
uint8_t on_signal=ON_SIGNAL_NON;

void checkConnectCallback() // if go into this func, check connect is failed
{
    if((check_connect & CONNECT_1) != 0) {
        check_connect &= ~(CONNECT_1);
        startAlert1();
    }
    if((check_connect & CONNECT_2) != 0) {
        check_connect &= ~(CONNECT_2);
        startAlert2();
    }
    if((check_connect & CONNECT_3) != 0) {
        check_connect &= ~(CONNECT_3);
        startAlert3();
    }
}
void periodicCallback1() {
    chatTimer.detach();
    intCnt = 0;
}

void push1() {
    if(intCnt == 0) {
        if((on_signal & ON_SIGNAL_1) != 0) {
            on_signal &= ~(ON_SIGNAL_1);
            offCheckSignal1();
        }
        else if(alert_number == ALERT_NON) {
            check_connect &= ~(CONNECT_1);
            uart.putc('t'); //check connection message to device 0001
            uart.putc('x');  
            uart.putc('d');  
            uart.putc('t');  
            uart.putc(' ');  
            uart.putc('0'); 
            uart.putc('0'); 
            uart.putc('1'); 
            uart.putc('0'); 
            uart.putc('0'); 
            uart.putc('2'); 
            uart.putc('0'); 
            uart.putc('1'); 
            uart.putc('0'); 
            uart.putc('1'); 
            uart.putc('\r');  
            uart.putc('\n');  
            CheckConnectTimer.attach(checkConnectCallback, CHECK_CONNECT_SEC);
        }
        else {
            stopAlert1();
        }
        chatTimer.attach_us(periodicCallback1, CHECK_CHUTTER_US); //start detecting chattering
    }
    intCnt++;
}

void periodicCallback2() {
    chatTimer.detach();
    intCnt2 = 0;
}

void push2() {
    if(intCnt2 == 0) {
        if((on_signal & ON_SIGNAL_2) != 0) {
            on_signal &= ~(ON_SIGNAL_2);
            offCheckSignal2();
        }
        else if(alert_number == ALERT_NON) {
            check_connect &= ~(CONNECT_2);
            uart.putc('t'); //check connection message to device 0002
            uart.putc('x');  
            uart.putc('d');  
            uart.putc('t');  
            uart.putc(' ');  
            uart.putc('0'); 
            uart.putc('0'); 
            uart.putc('1'); 
            uart.putc('0'); 
            uart.putc('0'); 
            uart.putc('2'); 
            uart.putc('0'); 
            uart.putc('1'); 
            uart.putc('0'); 
            uart.putc('2'); 
            uart.putc('\r');  
            uart.putc('\n');  
            CheckConnectTimer.attach(checkConnectCallback, CHECK_CONNECT_SEC);
        }
        else {
            stopAlert2();
        }
        chatTimer.attach_us(periodicCallback2, CHECK_CHUTTER_US); //start detecting chattering
    }
    intCnt2++;
}

void periodicCallback3() {
    chatTimer.detach();
    intCnt3 = 0;
}

void push3() {
    if(intCnt3 == 0) {
        if((on_signal & ON_SIGNAL_3) != 0) {
            on_signal &= ~(ON_SIGNAL_3);
            offCheckSignal3();
        }
        else if(alert_number == ALERT_NON) {
            check_connect &= ~(CONNECT_3);
            uart.putc('t'); //check connection message to device 0003
            uart.putc('x');  
            uart.putc('d');  
            uart.putc('t');  
            uart.putc(' ');  
            uart.putc('0'); 
            uart.putc('0'); 
            uart.putc('1'); 
            uart.putc('0'); 
            uart.putc('0'); 
            uart.putc('2'); 
            uart.putc('0'); 
            uart.putc('1'); 
            uart.putc('0'); 
            uart.putc('3'); 
            uart.putc('\r');  
            uart.putc('\n');  
            CheckConnectTimer.attach(checkConnectCallback, CHECK_CONNECT_SEC);
        }
        else {
            stopAlert3();
        }
        chatTimer.attach_us(periodicCallback3, CHECK_CHUTTER_US); //start detecting chattering
    }
    intCnt3++;
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
    if(alert_cnt >= ARERT_MAX_CNT) {
        offBuzzer();
    }
    else {
        toggleBuzzer();
        alert_cnt++;
    }
    if((alert_number & ALERT_1) != 0) {
        sw_led1 = !sw_led1;
    }
    if((alert_number & ALERT_2) != 0) {
        sw_led2 = !sw_led2;
    }
    if((alert_number & ALERT_3) != 0) {
        sw_led3 = !sw_led3;
    }
}

void onCheckSignal1()
{
    on_signal|=ON_SIGNAL_1;
    onBuzzer();
    sw_led1 = SW_LED_ON;
}

void onCheckSignal2()
{
    on_signal|=ON_SIGNAL_2;
    onBuzzer();
    sw_led2 = SW_LED_ON;
}

void onCheckSignal3()
{
    on_signal|=ON_SIGNAL_3;
    onBuzzer();
    sw_led3 = SW_LED_ON;
}

void offCheckSignal1()
{
    on_signal &= ~(ON_SIGNAL_1);
    offBuzzer();
    sw_led1 = SW_LED_OFF;
}

void offCheckSignal2()
{
    on_signal &= ~(ON_SIGNAL_2);
    offBuzzer();
    sw_led1 = SW_LED_OFF;
}

void offCheckSignal3()
{
    on_signal &= ~(ON_SIGNAL_3);
    offBuzzer();
    sw_led1 = SW_LED_OFF;
}

void startAlert1()
{
    alert_cnt=0;
    offBuzzer();
    alert_number |= ALERT_1;
    sw_led1 = SW_LED_OFF;
    sw_led2 = SW_LED_OFF;
    sw_led3 = SW_LED_OFF;
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
    alert_cnt=0;
    offBuzzer();
    alert_number |= ALERT_2;
    sw_led1 = SW_LED_OFF;
    sw_led2 = SW_LED_OFF;
    sw_led3 = SW_LED_OFF;
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

void startAlert3()
{
    alert_cnt=0;
    offBuzzer();
    alert_number |= ALERT_3;
    sw_led1 = SW_LED_OFF;
    sw_led2 = SW_LED_OFF;
    sw_led3 = SW_LED_OFF;
    if(alert_number == ALERT_3) {
        AlertTimer.attach(alertCallback, ALERT_CYCLE_SEC);
    }
}

void stopAlert3()
{
    sw_led3 = SW_LED_OFF;
    alert_number &= ~(ALERT_3);
    if(alert_number == ALERT_NON) {
        offBuzzer();
        AlertTimer.detach();
    }
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
    alert_cnt=0;
    check_connect=CONNECT_NON;
    on_signal=ON_SIGNAL_NON;

    intCnt = 0;
    intCnt2 = 0;
    intCnt3 = 0;
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
            pc.putc('R'); 
            pc.putc('V'); 
            pc.putc(':'); 
            for(rBuffIndex=0; rBuffIndex<receiveSize; ++rBuffIndex) {
                pc.putc(receiveBuff[rBuffIndex]);       // 送信
            }
#endif
            if(receiveSize > 7) {
                if(
                        (receiveBuff[3] == '5') &&
                        (receiveBuff[4] == '1') &&
                        (receiveBuff[5] == '7') &&
                        (receiveBuff[6] == '4')
                ) {
                    startAlert1();
                }
                else if( 
                        (receiveBuff[11] == '0') && // data:0001010010 :normal alert
                        (receiveBuff[12] == '0') &&
                        (receiveBuff[14] == '0') &&
                        (receiveBuff[15] == '1') &&
                        (receiveBuff[17] == '0') &&
                        (receiveBuff[18] == '1') &&
                        (receiveBuff[20] == '0') &&
                        (receiveBuff[21] == '0') &&
                        (receiveBuff[23] == '1') &&
                        (receiveBuff[24] == '0')
                ) {
                    startAlert1();
                }
                else if( 
                        (receiveBuff[11] == '0') && // data:0101010010 :check connection success
                        (receiveBuff[12] == '1') &&
                        (receiveBuff[14] == '0') &&
                        (receiveBuff[15] == '1') &&
                        (receiveBuff[17] == '0') &&
                        (receiveBuff[18] == '2') &&
                        (receiveBuff[20] == '0') &&
                        (receiveBuff[21] == '0') &&
                        (receiveBuff[23] == '1') &&
                        (receiveBuff[24] == '0')
                ) {
                    check_connect &= ~(CONNECT_1);
                    CheckConnectTimer.detach();
                    onCheckSignal1();
                }
                else if( 
                        (receiveBuff[11] == '0') && // data:0102010010 :check connection success
                        (receiveBuff[12] == '1') &&
                        (receiveBuff[14] == '0') &&
                        (receiveBuff[15] == '2') &&
                        (receiveBuff[17] == '0') &&
                        (receiveBuff[18] == '2') &&
                        (receiveBuff[20] == '0') &&
                        (receiveBuff[21] == '0') &&
                        (receiveBuff[23] == '1') &&
                        (receiveBuff[24] == '0')
                ) {
                    check_connect &= ~(CONNECT_2);
                    CheckConnectTimer.detach();
                    onCheckSignal2();
                }
                else if( 
                        (receiveBuff[11] == '0') && // data:0103010010 :check connection success
                        (receiveBuff[12] == '1') &&
                        (receiveBuff[14] == '0') &&
                        (receiveBuff[15] == '3') &&
                        (receiveBuff[17] == '0') &&
                        (receiveBuff[18] == '2') &&
                        (receiveBuff[20] == '0') &&
                        (receiveBuff[21] == '0') &&
                        (receiveBuff[23] == '1') &&
                        (receiveBuff[24] == '0')
                ) {
                    check_connect &= ~(CONNECT_3);
                    CheckConnectTimer.detach();
                    onCheckSignal3();
                }
                else if( 
                        (receiveBuff[11] == '0') && // data:0010030101
                        (receiveBuff[12] == '0') &&
                        (receiveBuff[14] == '1') &&
                        (receiveBuff[15] == '0') &&
                        (receiveBuff[17] == '0') &&
                        (receiveBuff[18] == '3') &&
                        (receiveBuff[20] == '0') &&
                        (receiveBuff[21] == '1') &&
                        (receiveBuff[23] == '0') &&
                        (receiveBuff[24] == '1')
                ) {
                    check_connect|=CONNECT_1;
                }
                else if( 
                        (receiveBuff[11] == '0') && // data:0010030102
                        (receiveBuff[12] == '0') &&
                        (receiveBuff[14] == '1') &&
                        (receiveBuff[15] == '0') &&
                        (receiveBuff[17] == '0') &&
                        (receiveBuff[18] == '3') &&
                        (receiveBuff[20] == '0') &&
                        (receiveBuff[21] == '1') &&
                        (receiveBuff[23] == '0') &&
                        (receiveBuff[24] == '2')
                ) {
                    check_connect|=CONNECT_2;
                }
                else if( 
                        (receiveBuff[11] == '0') && // data:0010030103
                        (receiveBuff[12] == '0') &&
                        (receiveBuff[14] == '1') &&
                        (receiveBuff[15] == '0') &&
                        (receiveBuff[17] == '0') &&
                        (receiveBuff[18] == '3') &&
                        (receiveBuff[20] == '0') &&
                        (receiveBuff[21] == '1') &&
                        (receiveBuff[23] == '0') &&
                        (receiveBuff[24] == '3')
                ) {
                    check_connect|=CONNECT_3;
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
