#include <Arduino.h>

#include "Util.h"
#include "filtro/FiltroExponencial.h"
#include "leitorPulso/LeitorPulsoFailsafe.h"
#include "motores/DriverDoisMotores.h"
#include "mapeadorRc/MapeadorRcParaComando.h"
#include "controleReversao/ControleReversao.h"

// =======================
// CONFIGURAÇÃO DE PINOS
// =======================
const int PINO_CANAL_CH1 = 3;

// Motor 1
const int PINO_PWM_M1 = 11;
const int PINO_DIR_M1 = 10;
const int PINO_STOP_M1 = 8;
const int PINO_FREIO_M1 = 9; // BRAKE ativo em HIGH

// Motor 2
const int PINO_PWM_M2 = 5;
const int PINO_DIR_M2 = 6;
const int PINO_STOP_M2 = 4;
const int PINO_FREIO_M2 = 7; // BRAKE ativo em HIGH

// =======================
// PARÂMETROS (do seu código original)
// =======================
const bool INVERTER_DIRECAO = false;
const int DUTY_MAXIMO = 255;

const int LIMITE_DUTY_FRENTE = 120;
const bool NIVEL_DIR_FRENTE_MOTOR1 = HIGH;
const bool NIVEL_DIR_FRENTE_MOTOR2 = LOW;

const unsigned long DEADTIME_ANTES_DIR_MS = 350;
const unsigned long DEADTIME_DEPOIS_DIR_MS = 200;

const unsigned long ABS_TOTAL_MS = 600;
const unsigned long ABS_ON_MS = 60;
const unsigned long ABS_OFF_MS = 40;

const int PASSO_RAMPA_SUBIDA = 3;
const int PASSO_RAMPA_DESCIDA = 15;
const int LIMIAR_TROCA_DIR_DUTY = 15;

const unsigned long SOFT_START_MS = 300;
const int PASSO_SOFT_START_SUBIDA = 1;

const float ALFA_FILTRO = 0.15f;

// =======================
// INSTÂNCIAS (objetos)
// =======================
DriverDoisMotores driverMotores(
    PINO_PWM_M1, PINO_DIR_M1, PINO_STOP_M1, PINO_FREIO_M1,
    PINO_PWM_M2, PINO_DIR_M2, PINO_STOP_M2, PINO_FREIO_M2);

LeitorPulsoFailsafe leitorPulso(
    PINO_CANAL_CH1,
    30000, // timeout us
    1500,  // pulso inicial
    3      // limite leituras ruins
);

FiltroExponencial filtroPulso(ALFA_FILTRO, 1500.0f);

MapeadorRcParaComando mapeador(
    INVERTER_DIRECAO,
    NIVEL_DIR_FRENTE_MOTOR1,
    NIVEL_DIR_FRENTE_MOTOR2,
    LIMITE_DUTY_FRENTE,
    DUTY_MAXIMO);

ControleReversao controle(
    driverMotores,
    DEADTIME_ANTES_DIR_MS, DEADTIME_DEPOIS_DIR_MS,
    ABS_TOTAL_MS, ABS_ON_MS, ABS_OFF_MS,
    PASSO_RAMPA_SUBIDA, PASSO_RAMPA_DESCIDA,
    LIMIAR_TROCA_DIR_DUTY, DUTY_MAXIMO,
    SOFT_START_MS, PASSO_SOFT_START_SUBIDA);

void setup()
{
  Serial.begin(9600);

  leitorPulso.begin();
  driverMotores.begin();

  controle.reset();
}

void loop()
{
  unsigned long agoraMs = millis();

  long pulsoUs = 0;
  bool okParaDirigir = leitorPulso.ler(pulsoUs);

  if (!okParaDirigir)
  {
    // Fail-safe: corta tudo se falhar demais
    controle.cortarTudo();
    return;
  }

  // Se está “ok”, garante driver ligado
  driverMotores.enable();

  // Suaviza o pulso
  float pulsoSuaveUs = filtroPulso.atualizar((float)pulsoUs);

  // Converte em comando alvo
  ComandoAlvo cmd = mapeador.converter(pulsoSuaveUs, controle.getDados().direcaoAplicadaMotor1, controle.getDados().direcaoAplicadaMotor2);

  // Aplica (rampas + reversão segura)
  controle.tick(agoraMs, cmd.direcaoAlvoHwMotor1, cmd.direcaoAlvoHwMotor2, cmd.dutyAlvo);

  // Log
  // long pulsoBruto, float pulsoSuave, bool dir, int duty, EstadoEnum st
  logBonito(leitorPulso.ultimoValido(), pulsoSuaveUs,
            controle.getDados().direcaoAplicadaMotor1, controle.getDados().direcaoAplicadaMotor2, controle.getDados().dutyAplicado, controle.getEstado());
}
