# Caso da Vinheria Agnello - Data Logger - Checkpoint 2 - 1ESPA - Q4/2024

> [!CAUTION]
> página de projeto do cp2 de edge (incluindo um 2do), deem uma olhada lá
> https://github.com/orgs/GrupoMoskitto/projects/3/views/1

## Mestre: 
- Professor Fábio Cabrini
## Membros do Grupo - **Moskitto**:
- [**Ana Luiza**](https://github.com/anarand) - RM561194
- [**Francisco Vargas**](https://github.com/Franciscov25) - RM560322
- [**Gabriel Kato**](https://github.com/kato8088) - RM560000
- [**Gabriel Couto**](https://github.com/rouri404) - RM559579
- [**João Vitor**](https://github.com/joaomatosq) - RM559246

## Como funciona:
O funcionamento das novas medições é bem similar ao cp1, entretanto, dessa vez nós optamos por usar mais funções em prol da quantidade de variáveis. Nosso novo código possui 2 novas funções, além de alternar entre duas telas a cada 5 segundos.
- Função umidTemp(): Realiza as medições da temperatura e umidade usando o DHT22, e converte os valores para caberem no display 16x2.
- Função nivelCritico(): Usando os valores cŕíticos pré-determinados de temp. e umid., ela demonstra na "segunda tela" do display se tais valores estão "ruins' ou "okay".
A cada 10 segundos os valores são armazenados usando a EEPROM, desse modo, o cliente poderá ver no monitor serial os últimos valores críticos que a vinheria atingiu.

