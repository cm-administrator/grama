#pragma once
#include <Arduino.h>

class FiltroExponencial
{
    float alpha;
    float valor;

public:
    FiltroExponencial(float alpha_, float valorInicial)
        : alpha(alpha_), valor(valorInicial) {}

    float atualizar(float novoValor)
    {
        valor = (novoValor * alpha) + (valor * (1.0f - alpha));
        return valor;
    }

    float obter() const { return valor; }
};
