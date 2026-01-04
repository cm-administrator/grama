#pragma once
#include <Arduino.h>

class LeitorPulsoFailsafe
{
    int pino;
    unsigned long timeoutUs;

    long ultimoValidoUs;
    int ruinsSeguidas;
    int limiteRuins;

public:
    LeitorPulsoFailsafe(int pino_, unsigned long timeoutUs_, long inicialUs, int limiteRuins_)
        : pino(pino_),
          timeoutUs(timeoutUs_),
          ultimoValidoUs(inicialUs),
          ruinsSeguidas(0),
          limiteRuins(limiteRuins_) {}

    void begin() { pinMode(pino, INPUT); }

    // Retorna false se deve cortar tudo (falhou por muito tempo)
    bool ler(long &pulsoUs)
    {
        long bruto = pulseIn(pino, HIGH, timeoutUs);
        bool valido = (bruto >= 900 && bruto <= 2100);

        if (valido)
        {
            ultimoValidoUs = bruto;
            ruinsSeguidas = 0;
            pulsoUs = bruto;
            return true;
        }

        // leitura ruim: usa o último válido por enquanto
        ruinsSeguidas++;
        pulsoUs = ultimoValidoUs;

        // se passou do limite: manda cortar
        return (ruinsSeguidas < limiteRuins);
    }

    long ultimoValido() const { return ultimoValidoUs; }
};
