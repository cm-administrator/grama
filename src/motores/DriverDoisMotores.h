#pragma once
#include <Arduino.h>

class DriverDoisMotores
{
    int pwm1, dir1, stop1, freio1;
    int pwm2, dir2, stop2, freio2;

public:
    DriverDoisMotores(int pwm1_, int dir1_, int stop1_, int freio1_,
                      int pwm2_, int dir2_, int stop2_, int freio2_)
        : pwm1(pwm1_), dir1(dir1_), stop1(stop1_), freio1(freio1_),
          pwm2(pwm2_), dir2(dir2_), stop2(stop2_), freio2(freio2_) {}

    void begin()
    {
        pinMode(pwm1, OUTPUT);
        pinMode(dir1, OUTPUT);
        pinMode(stop1, OUTPUT);
        pinMode(freio1, OUTPUT);
        pinMode(pwm2, OUTPUT);
        pinMode(dir2, OUTPUT);
        pinMode(stop2, OUTPUT);
        pinMode(freio2, OUTPUT);

        analogWrite(pwm1, 0);
        analogWrite(pwm2, 0);

        digitalWrite(dir1, LOW);
        digitalWrite(dir2, HIGH);

        freioOff();
        disable();
        delay(50);
        enable();
    }

    void enable()
    {
        digitalWrite(stop1, HIGH);
        digitalWrite(stop2, HIGH);
    }

    void disable()
    {
        digitalWrite(stop1, LOW);
        digitalWrite(stop2, LOW);
    }

    void freioOn()
    {
        digitalWrite(freio1, HIGH);
        digitalWrite(freio2, HIGH);
    }

    void freioOff()
    {
        digitalWrite(freio1, LOW);
        digitalWrite(freio2, LOW);
    }

    void setDirecao(bool dir_motor1_, bool dir_motor2_)
    {
        digitalWrite(dir1, dir_motor1_);
        digitalWrite(dir2, dir_motor2_);
    }

    void setDuty(int duty)
    {
        analogWrite(pwm1, duty);
        analogWrite(pwm2, duty);
    }

    void pararTudo()
    {
        setDuty(0);
        freioOff();
        disable();
    }
};
