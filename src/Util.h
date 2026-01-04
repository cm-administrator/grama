#pragma once
#include <Arduino.h>

// Mapa float -> int (equivalente ao seu mapFloatToInt original)
inline int mapFloatParaInt(float x, float in_min, float in_max, int out_min, int out_max)
{
    if (in_max == in_min)
        return out_min;

    float t = (x - in_min) / (in_max - in_min);
    float y = out_min + t * (out_max - out_min);

    if (y < out_min)
        y = out_min;
    if (y > out_max)
        y = out_max;

    return (int)(y + (y >= 0 ? 0.5f : -0.5f));
}

// Log “bonito”
template <typename EstadoEnum>
inline void logBonito(long pulsoBruto, float pulsoSuave, bool dirM1, bool dirM2, int duty, EstadoEnum st)
{
    Serial.print("PulsoBruto(us): ");
    Serial.print(pulsoBruto);
    Serial.print(" | PulsoSuave(us): ");
    Serial.print(pulsoSuave, 1);
    Serial.print(" | DirecaoAplicada Motor 1: ");
    Serial.print(dirM1 ? "HIGH" : "LOW");
    Serial.print(" | DirecaoAplicada Motor 2: ");
    Serial.print(dirM2 ? "HIGH" : "LOW");
    Serial.print(" | DutyAplicado: ");
    Serial.print(duty);
    Serial.print(" | Estado: ");
    Serial.println((int)st); // (int) para evitar dependência de nome string aqui
}
