#include "bbcar_control.h"
#include "mbed.h"
#include "bbcar.h"
#include "drivers/DigitalOut.h"
#include "erpc_simple_server.h"
#include "erpc_basic_codec.h"
#include "erpc_crc16.h"
#include "UARTTransport.h"
#include "DynamicMessageBufferFactory.h"
#include "bbcar_control_server.h"
#define CONSOLE_FLOWCONTROL_RTS     1
#define CONSOLE_FLOWCONTROL_CTS     2
#define CONSOLE_FLOWCONTROL_RTSCTS  3
#define mbed_console_concat_(x) CONSOLE_FLOWCONTROL_##x
#define mbed_console_concat(x) mbed_console_concat_(x)
#define CONSOLE_FLOWCONTROL mbed_console_concat(MBED_CONF_TARGET_CONSOLE_UART_FLOW_CONTROL)
BufferedSerial pc(USBTX, USBRX);

DigitalOut led1(LED1);
Ticker servo_ticker;
Ticker encoder_ticker;
PwmOut pin5(D10), pin6(D11);
BusInOut qti(D5, D4, D3, D2);
DigitalInOut pin10(D8);
DigitalIn encoder(D9);
volatile int steps;
volatile int last;
float dist = 0, spe = 0, tmp = 0;
BBCar car(pin5, pin6, servo_ticker);
Thread t;

ep::UARTTransport uart_transport(D1, D0, 9600);
ep::DynamicMessageBufferFactory dynamic_mbf;
erpc::BasicCodecFactory basic_cf;
erpc::Crc16 crc16;
erpc::SimpleServer rpc_server;

//BBCar* cars[] = {&car};
void encoder_control() {
   int value = encoder;
   if (!last && value) steps++;
   last = value;
}

void BBCar::selfturn(float speed) {
    servo0.set_speed(-speed);
    servo1.set_speed(-speed);
    ThisThread::sleep_for(2s);
}

void distance() {
    printf("distance is %f\n", steps*6.5*3.14/32);
}
void Speed() {
    steps = 0;
    last = 0; 
    printf("speed is %f\n", steps*6.5*3.14/32);
}
void erpc_start() {
    rpc_server.run();
}

BBCarService_service car_control_service;

int main() {
   // Set up hardware flow control, if needed
 #if CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_RTS
   uart_transport.set_flow_control(mbed::SerialBase::RTS, STDIO_UART_RTS, NC);
 #elif CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_CTS
   uart_transport.set_flow_control(mbed::SerialBase::CTS, NC, STDIO_UART_CTS);
 #elif CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_RTSCTS
   uart_transport.set_flow_control(mbed::SerialBase::RTSCTS, STDIO_UART_RTS, STDIO_UART_CTS);
 #endif
 
   printf("Initializing server.\n");
   rpc_server.setTransport(&uart_transport);
   rpc_server.setCodecFactory(&basic_cf);
   rpc_server.setMessageBufferFactory(&dynamic_mbf);
 
   // Add the led service to the server
   printf("Adding BBCar server.\n");
   rpc_server.addService(&car_control_service);
 
   // Run the server. This should never exit
   printf("Running server.\n");
   //rpc_server.run();
    t.start(erpc_start);
    pc.set_baud(9600);
    encoder_ticker.attach(&encoder_control, 1s);
    uart_transport.setCrc16(&crc16);
 
    int count = 0;
    int turn = 0;
    int stop = 0;
    int right = 2;
    parallax_ping  ping1(pin10);
    car.goStraight(30.0);
    while(1) {
        if ((float)ping1 > 10) led1 = 1;
        else {
            led1 = 0;
            car.stop();
            car.selfturn(20.0);
            right = 3;
        }  
        qti.output();
        qti = 0b1111;
        wait_us(230);
        qti.input();     
        wait_us(230);   
        if (qti == 0b1000) {
            pin5 = 0;
            right = 0;
            car.turn(100.0, 0.00001);
        }
        else if (qti == 0b1100) {
            right = 0;
            car.turn(100.0, 0.001);
        }
        else if (qti == 0b0100) {
            right = 1;
            car.turn(100.0, 0.01);
        }
        else if (qti == 0b0110) {
            car.goStraight(30.0);
        }
        else if (qti == 0b0010) {
            right = 2;
            car.turn(100.0, -0.01);
        }
        else if (qti == 0b0011) {
            right = 2;
            car.turn(100.0, -0.0001);
        }
        else if (qti == 0b0001) {
            right = 2;
            pin6 = 0;
            car.turn(100.0, -0.00001);
        }
        else if (qti == 0b1110) {
            if (turn == 1) {
                count++;
                if (count > 15) {
                    car.turn(200.0, 0.001);
                    ThisThread::sleep_for(700ms);
                    turn = 0;
                    count = 0;
                    right = 2;
                }
            }
            else {
                count++;
                car.goStraight(40.0);
                if (count > 15) {
                    turn = 1;
                    count = 0;
                }
            }
        }
        else if (qti == 0b0111) {
            if (turn == 2) {
                count++;
                if (count > 15) {
                    car.turn(200.0, -0.001);
                    ThisThread::sleep_for(700ms);
                    turn = 0;
                    count = 0;
                    right = 1;
                }
            }
            else {
                count++;
                car.goStraight(30.0);
                if (count > 15) {
                    turn = 2;
                    count = 0;
                }
            }
        }
        else if (qti == 0b1111) {
            if (turn == 2) {
                count++;
                if (count > 15) {
                    car.turn(200.0, -0.001);
                    ThisThread::sleep_for(700ms);
                    turn = 0;
                    count = 0;
                }
            }
            else if (turn == 1) {
                count++;
                if (count > 15) {
                    car.turn(200.0, 0.001);
                    ThisThread::sleep_for(700ms);
                    turn = 0;
                    count = 0;
                }
            }
            else {
                car.goStraight(30.0);
            }
        }
        else if (qti == 0b0000) {
            if (right == 3) {
                car.selfturn(20.0);
            }
            else if (right == 0){
                car.turn(100.0, 0.0001);
                right = 2;
            }
            else if (right == 1) {
                car.turn(100.0, -0.0001);
                right = 2;
            }
            else {
                car.stop();
                break;
            }            
        }
        ThisThread::sleep_for(20ms);
   }
}
