#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SENSORES 8
#define MAX_NOME_SENSOR 5
#define TOTAL_LEITURAS 2000

typedef struct
{
    char nome[MAX_NOME_SENSOR];
    char tipo[10]; // "float", "int", "bool", "string"
} Sensor;

const char *strings_exemplo[] = {
    "NORMAL", "ALERTA", "CRITICO", "OK", "ERRO"};

time_t gerar_timestamp_aleatorio(struct tm *inicial, struct tm *final)
{

    time_t timestamp_inicial, timestamp_final;

    timestamp_inicial = mktime(inicial);
    if (timestamp_inicial == -1)
    {
        printf("Data inválida.\n");
        return -1;
    }

    timestamp_final = mktime(final);
    if (timestamp_final == -1)
    {
        printf("Data inválida.\n");
        return -1;
    }

    time_t timestamp_aleatorio = timestamp_inicial + rand() % (timestamp_final - timestamp_inicial + 1);

    return timestamp_aleatorio;
}

void gerar_valor_aleatorio(FILE *f, const char *sensor, const char *tipo, time_t timestamp)
{
    if (strcmp(tipo, "float") == 0)
    {
        float valor = (rand() % 10000) / 100.0f; // 0.00 até 99.99
        fprintf(f, "%ld %s %.2f\n", timestamp, sensor, valor);
    }
    else if (strcmp(tipo, "int") == 0)
    {
        int valor = rand() % 1000; // 0 a 999
        fprintf(f, "%ld %s %d\n", timestamp, sensor, valor);
    }
    else if (strcmp(tipo, "bool") == 0)
    {
        int valor = rand() % 2; // 0 ou 1
        fprintf(f, "%ld %s %s\n", timestamp, sensor, valor ? "true" : "false");
    }
    else if (strcmp(tipo, "string") == 0)
    {
        const char *valor = strings_exemplo[rand() % 5];
        fprintf(f, "%ld %s %s\n", timestamp, sensor, valor);
    }
    else
    {
        printf("Tipo de dado inválido para o sensor %s.\n", sensor);
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 17 || (argc - 9) % 2 != 0)
    {
        printf("Uso: %s DIA_INI MES_INI ANO_INI HORA_INI MIN_INI SEG_INI DIA_FIM MES_FIM ANO_FIM HORA_FIM MIN_FIM SEG_FIM <SENSOR1> <TIPO1> ...\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    struct tm t_ini = {
        .tm_mday = atoi(argv[1]),
        .tm_mon = atoi(argv[2]) - 1,
        .tm_year = atoi(argv[3]) - 1900,
        .tm_hour = atoi(argv[4]),
        .tm_min = atoi(argv[5]),
        .tm_sec = atoi(argv[6])};

    struct tm t_fim = {
        .tm_mday = atoi(argv[7]),
        .tm_mon = atoi(argv[8]) - 1,
        .tm_year = atoi(argv[9]) - 1900,
        .tm_hour = atoi(argv[10]),
        .tm_min = atoi(argv[11]),
        .tm_sec = atoi(argv[12])};

    int total_sensores = (argc - 13) / 2;
    Sensor sensores[MAX_SENSORES];

    for (int i = 0; i < total_sensores; i++)
    {
        strncpy(sensores[i].nome, argv[13 + i * 2], MAX_NOME_SENSOR);
        strncpy(sensores[i].tipo, argv[14 + i * 2], 10);
    }

    FILE *saida = fopen("arquivoTeste.txt", "w");
    if (!saida)
    {
        printf("Erro ao criar o arquivo de saída.\n");
        return 1;
    }

    for (int i = 0; i < total_sensores; i++)
    {
        for (int j = 0; j < TOTAL_LEITURAS; j++)
        {
            time_t ts = gerar_timestamp_aleatorio(&t_ini, &t_fim);
            gerar_valor_aleatorio(saida, sensores[i].nome, sensores[i].tipo, ts);
        }
    }

    fclose(saida);
    printf("Arquivo 'arquivoTeste.txt' gerado com sucesso com %d leituras por sensor.\n", TOTAL_LEITURAS);
    return 0;
}
