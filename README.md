# _Queue Example_

Code demonstrates use of a Queue, a way of passing data between tasks that seamlessly integrates with I2C and Semaphores. Queues in FreeRTOS are preset to sync with semaphores.

## How to use example
2 LCDs, HCSR04 ultrasound sensor

2 LCDs:
    - SDA (GPIO 4)

    - SCD (GPIO 3)

_LCDs *MUST* be different addresses. Address of LCD called can be edited at the top of main.c_

HCSR04:
    - Trig (GPIO 14)
    
    - Echo (GPIO 15)

## Contents

This project contains two components:

BlakeLCD:

    -LCD code operator edited by Blake Hanneford

    -hcsr04 code operator (Credits in README.md & LICENSE.txt)

main.c includes 3 tasks:

    hcsr04 task: Repeatedly measures and verifies measurment. Measurement is pushed to Queue

    LCD_task1: Displays queued values on LCD display

    LCD_task2: Random task that counts up, used to test mutex with queue
