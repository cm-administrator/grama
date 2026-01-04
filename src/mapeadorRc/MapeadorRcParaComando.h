#pragma once
#include <Arduino.h>
#include "../Util.h"

struct ComandoAlvo
{
    bool direcaoAlvoHwMotor1;
    bool direcaoAlvoHwMotor2;
    int dutyAlvo;
};

class MapeadorRcParaComando
{
    bool inverterDirecao;
    bool nivelDirFrenteMotor1;
    bool nivelDirFrenteMotor2;
    int limiteDutyFrente;
    int dutyMax;

public:
    MapeadorRcParaComando(bool inverterDirecao_, bool nivelDirFrenteMotor1_, bool nivelDirFrenteMotor2_, int limiteDutyFrente_, int dutyMax_)
        : inverterDirecao(inverterDirecao_),
          nivelDirFrenteMotor1(nivelDirFrenteMotor1_),
          nivelDirFrenteMotor2(nivelDirFrenteMotor2_),
          limiteDutyFrente(limiteDutyFrente_),
          dutyMax(dutyMax_) {}

    ComandoAlvo converter(float pulsoSuavizadoUs, bool direcaoAplicadaAtualMotor1, bool direcaoAplicadaAtualMotor2)
    {
        bool direcaoAlvoLogicaMotor1 = direcaoAplicadaAtualMotor1;
        bool direcaoAlvoLogicaMotor2 = direcaoAplicadaAtualMotor2;
        int dutyAlvo = 0;

        // zona morta (neutro)
        if (pulsoSuavizadoUs > 1460 && pulsoSuavizadoUs < 1540)
        {
            dutyAlvo = 0;
            direcaoAlvoLogicaMotor1 = direcaoAplicadaAtualMotor1;
            direcaoAlvoLogicaMotor2 = direcaoAplicadaAtualMotor2;
        }
        else if (pulsoSuavizadoUs <= 1460)
        {
            direcaoAlvoLogicaMotor1 = LOW;
            direcaoAlvoLogicaMotor2 = HIGH;
            dutyAlvo = mapFloatParaInt(pulsoSuavizadoUs, 1460.0f, 1000.0f, 0, dutyMax);
        }
        else
        {
            direcaoAlvoLogicaMotor1 = HIGH;
            direcaoAlvoLogicaMotor2 = LOW;
            dutyAlvo = mapFloatParaInt(pulsoSuavizadoUs, 1540.0f, 2000.0f, 0, dutyMax);
        }

        dutyAlvo = constrain(dutyAlvo, 0, dutyMax);

        bool direcaoAlvoHwMotor1 = inverterDirecao ? !direcaoAlvoLogicaMotor1 : direcaoAlvoLogicaMotor1;
        bool direcaoAlvoHwMotor2 = inverterDirecao ? !direcaoAlvoLogicaMotor2 : direcaoAlvoLogicaMotor2;

        // limitador só pra frente
        if (direcaoAlvoHwMotor1 == nivelDirFrenteMotor1 && direcaoAlvoHwMotor2 == nivelDirFrenteMotor2)
        {
            dutyAlvo = min(dutyAlvo, limiteDutyFrente);
        }

        return {direcaoAlvoHwMotor1, direcaoAlvoHwMotor2, dutyAlvo};
    }
};
