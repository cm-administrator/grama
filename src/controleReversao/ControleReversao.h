#pragma once
#include <Arduino.h>
#include "../motores/DriverDoisMotores.h"

struct DadosControleReversao
{
    int dutyAplicado;
    bool direcaoAplicadaMotor1;
    bool direcaoAplicadaMotor2;
};

class ControleReversao
{
public:
    enum Estado
    {
        RODANDO,
        REV_FREIO_ABS,
        REV_ESPERA_ANTES_DIR,
        REV_ESPERA_DEPOIS_DIR
    };

private:
    DriverDoisMotores &driver;

    const unsigned long deadAntesMs;
    const unsigned long deadDepoisMs;

    const unsigned long absTotalMs;
    const unsigned long absOnMs;
    const unsigned long absOffMs;

    const int passoSubida;
    const int passoDescida;
    const int limiarTrocaDirDuty;
    const int dutyMax;

    // soft-start
    bool softStartAtivo;
    unsigned long softStartInicioMs;
    const unsigned long softStartDuracaoMs;
    const int passoSoftStartSubida;

    // aplicado
    Estado estado;
    int dutyAplicado;
    bool direcaoAplicadaMotor1;
    bool direcaoAplicadaMotor2;

    // timestamps ABS/estados
    unsigned long tsEstadoMs;
    unsigned long tsAbsInicioMs;
    unsigned long tsAbsFaseMs;
    bool absLigado;

public:
    ControleReversao(DriverDoisMotores &driver_,
                     unsigned long deadAntesMs_, unsigned long deadDepoisMs_,
                     unsigned long absTotalMs_, unsigned long absOnMs_, unsigned long absOffMs_,
                     int passoSubida_, int passoDescida_,
                     int limiarTrocaDirDuty_, int dutyMax_,
                     unsigned long softStartDuracaoMs_, int passoSoftStartSubida_)
        : driver(driver_),
          deadAntesMs(deadAntesMs_), deadDepoisMs(deadDepoisMs_),
          absTotalMs(absTotalMs_), absOnMs(absOnMs_), absOffMs(absOffMs_),
          passoSubida(passoSubida_), passoDescida(passoDescida_),
          limiarTrocaDirDuty(limiarTrocaDirDuty_), dutyMax(dutyMax_),
          softStartAtivo(false), softStartInicioMs(0),
          softStartDuracaoMs(softStartDuracaoMs_), passoSoftStartSubida(passoSoftStartSubida_),
          estado(RODANDO), dutyAplicado(0), direcaoAplicadaMotor1(LOW), direcaoAplicadaMotor2(HIGH),
          tsEstadoMs(0), tsAbsInicioMs(0), tsAbsFaseMs(0), absLigado(false) {}

    void reset()
    {
        estado = RODANDO;
        dutyAplicado = 0;
        direcaoAplicadaMotor1 = LOW;
        direcaoAplicadaMotor2 = HIGH;
        softStartAtivo = false;
    }

    void cortarTudo()
    {
        dutyAplicado = 0;
        driver.pararTudo();
        estado = RODANDO;
        softStartAtivo = false;
    }

    void tick(unsigned long agoraMs, bool direcaoAlvoHwMotor1, bool direcaoAlvoHwMotor2, int dutyAlvo)
    {
        switch (estado)
        {
        case RODANDO:
        {
            driver.freioOff();

            // reversão solicitada?
            if (direcaoAlvoHwMotor1 != direcaoAplicadaMotor1 && direcaoAlvoHwMotor2 != direcaoAplicadaMotor2)
            {
                // desce duty
                if (dutyAplicado > 0)
                {
                    dutyAplicado -= passoDescida;
                    if (dutyAplicado < 0)
                        dutyAplicado = 0;
                    driver.setDuty(dutyAplicado);
                }

                // quando estiver baixo, inicia sequência
                if (dutyAplicado <= limiarTrocaDirDuty)
                {
                    dutyAplicado = 0;
                    driver.setDuty(0);

                    driver.disable();

                    estado = REV_FREIO_ABS;
                    tsAbsInicioMs = agoraMs;
                    tsAbsFaseMs = agoraMs;
                    absLigado = true;
                    driver.freioOn();
                }
                return;
            }

            // soft-start
            int passoSubidaEfetivo = passoSubida;
            if (softStartAtivo)
            {
                if (agoraMs - softStartInicioMs >= softStartDuracaoMs)
                {
                    softStartAtivo = false;
                }
                else
                {
                    passoSubidaEfetivo = passoSoftStartSubida;
                }
            }

            // rampa até duty alvo
            if (dutyAplicado < dutyAlvo)
            {
                dutyAplicado += passoSubidaEfetivo;
                if (dutyAplicado > dutyAlvo)
                    dutyAplicado = dutyAlvo;
            }
            else if (dutyAplicado > dutyAlvo)
            {
                dutyAplicado -= passoDescida;
                if (dutyAplicado < dutyAlvo)
                    dutyAplicado = dutyAlvo;
            }

            dutyAplicado = constrain(dutyAplicado, 0, dutyMax);

            driver.setDirecao(direcaoAplicadaMotor1, direcaoAplicadaMotor2);
            driver.setDuty(dutyAplicado);
            break;
        }

        case REV_FREIO_ABS:
        {
            if (agoraMs - tsAbsInicioMs >= absTotalMs)
            {
                driver.freioOff();
                estado = REV_ESPERA_ANTES_DIR;
                tsEstadoMs = agoraMs;
                break;
            }

            // alterna freio ON/OFF
            if (absLigado)
            {
                if (agoraMs - tsAbsFaseMs >= absOnMs)
                {
                    absLigado = false;
                    tsAbsFaseMs = agoraMs;
                    driver.freioOff();
                }
            }
            else
            {
                if (agoraMs - tsAbsFaseMs >= absOffMs)
                {
                    absLigado = true;
                    tsAbsFaseMs = agoraMs;
                    driver.freioOn();
                }
            }
            break;
        }

        case REV_ESPERA_ANTES_DIR:
        {
            if (agoraMs - tsEstadoMs >= deadAntesMs)
            {
                direcaoAplicadaMotor1 = direcaoAlvoHwMotor1;
                direcaoAplicadaMotor2 = direcaoAlvoHwMotor2;
                driver.setDirecao(direcaoAplicadaMotor1, direcaoAplicadaMotor2);

                estado = REV_ESPERA_DEPOIS_DIR;
                tsEstadoMs = agoraMs;
            }
            break;
        }

        case REV_ESPERA_DEPOIS_DIR:
        {
            if (agoraMs - tsEstadoMs >= deadDepoisMs)
            {
                driver.enable();
                softStartAtivo = true;
                softStartInicioMs = agoraMs;
                estado = RODANDO;
            }
            break;
        }
        }
    }

    Estado getEstado() const { return estado; }

    const char *getEstadoNome() const
    {
        switch (estado)
        {
        case RODANDO:
            return "RODANDO";
        case REV_FREIO_ABS:
            return "REV_FREIO_ABS";
        case REV_ESPERA_ANTES_DIR:
            return "REV_ESPERA_ANTES_DIR";
        case REV_ESPERA_DEPOIS_DIR:
            return "REV_ESPERA_DEPOIS_DIR";
        default:
            return "DESCONHECIDO";
        }
    }

    DadosControleReversao getDados() const
    {
        return {dutyAplicado, direcaoAplicadaMotor1, direcaoAplicadaMotor2};
    }
};
