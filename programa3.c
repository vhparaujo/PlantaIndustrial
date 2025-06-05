#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_SENSORES 8
#define MAX_NOME_SENSOR 6
#define TOTAL_ESCRITAS 2000
#define MAX_TIPO 10

typedef struct
{
    char nome[MAX_NOME_SENSOR];
    char tipo[MAX_TIPO]; // "float", "int", "bool", "string"
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

void to_lowercase(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = tolower(str[i]);
    }
}

void to_uppercase(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = toupper(str[i]);
    }
}

void gerar_valor_aleatorio(FILE *f, const char *sensor, const char *tipo, time_t timestamp)
{
    char tipo_lower[MAX_TIPO];
    strncpy(tipo_lower, tipo, MAX_TIPO);
    to_lowercase(tipo_lower);

    char sensor_upper[MAX_NOME_SENSOR];
    strncpy(sensor_upper, sensor, MAX_NOME_SENSOR);
    to_uppercase(sensor_upper);

    if (strcmp(tipo_lower, "float") == 0)
    {
        float valor = (rand() % 10000) / 100.0f; // 0.00 até 99.99
        fprintf(f, "%ld %s %.2f\n", timestamp, sensor_upper, valor);
    }
    else if (strcmp(tipo_lower, "int") == 0)
    {
        int valor = rand() % 1000; // 0 a 999
        fprintf(f, "%ld %s %d\n", timestamp, sensor_upper, valor);
    }
    else if (strcmp(tipo_lower, "bool") == 0)
    {
        int valor = rand() % 2; // 0 ou 1
        fprintf(f, "%ld %s %s\n", timestamp, sensor_upper, valor ? "true" : "false");
    }
    else if (strcmp(tipo_lower, "string") == 0)
    {
        const char *valor = strings_exemplo[rand() % 5];
        fprintf(f, "%ld %s %s\n", timestamp, sensor_upper, valor);
    }
    else
    {
        printf("Tipo de dado inválido para o sensor %s.\n", sensor);
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 15 || (argc - 9) % 2 != 0)
    {
        printf("Uso: %s DIA_INI MES_INI ANO_INI HORA_INI MIN_INI SEG_INI DIA_FIM MES_FIM ANO_FIM HORA_FIM MIN_FIM SEG_FIM <SENSOR1> <TIPO1> ...\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    int diaInicial = atoi(argv[1]);
    int mesInicial = atoi(argv[2]);
    int anoInicial = atoi(argv[3]);
    int horaInicial = atoi(argv[4]);
    int minutoInicial = atoi(argv[5]);
    int segundoInicial = atoi(argv[6]);

    int diaFinal = atoi(argv[7]);
    int mesFinal = atoi(argv[8]);
    int anoFinal = atoi(argv[9]);
    int horaFinal = atoi(argv[10]);
    int minutoFinal = atoi(argv[11]);
    int segundoFinal = atoi(argv[12]);

    if (diaInicial < 1 || diaFinal < 1 || mesInicial < 1 || mesFinal < 1 || anoInicial < 0 || anoFinal < 0 || horaInicial < 0 || horaFinal < 0 || minutoInicial < 0 || minutoFinal < 0 || segundoInicial < 0 || segundoFinal < 0)
    {
        printf("Erro: Dia e mês devem ser maiores que 0, já ano, hora, minuto e segundo devem ser maiores ou iguais a 0.\n");
        return 1;
    }

    if (diaInicial > 31 || diaFinal > 31 || mesInicial > 12 || mesFinal > 12 || horaInicial > 23 || horaFinal > 23 || minutoInicial > 59 || minutoFinal > 59 || segundoInicial > 59 || segundoFinal > 59)
    {
        printf("Erro: Dia deve ser no máximo 31; mês até 12; hora até 23; minuto e segundo até 59.\n");
        return 1;
    }

    if ((mesInicial == 4 || mesInicial == 6 || mesInicial == 9 || mesInicial == 11) && diaInicial > 30 ||
        (mesFinal == 4 || mesFinal == 6 || mesFinal == 9 || mesFinal == 11) && diaFinal > 30)
    {
        printf("Erro: Dia não pode ser maior que 30 para meses com 30 dias.\n");
        return 1;
    }

    if ((mesInicial == 2 && diaInicial > 29) || (mesFinal == 2 && diaFinal > 29))
    {
        printf("Erro: Dia não pode ser maior que 29 para fevereiro.\n");
        return 1;
    }

    int ehBissextoIni = (anoInicial % 4 == 0 && (anoInicial % 100 != 0 || anoInicial % 400 == 0));
    int ehBissextoFim = (anoFinal % 4 == 0 && (anoFinal % 100 != 0 || anoFinal % 400 == 0));

    if ((mesInicial == 2 && diaInicial > 28 && !ehBissextoIni) ||
        (mesFinal == 2 && diaFinal > 28 && !ehBissextoFim))
    {
        printf("Erro: Dia não pode ser maior que 28 para fevereiro em anos não bissextos.\n");
        return 1;
    }
    struct tm t_ini = {
        .tm_year = anoInicial - 1900,
        .tm_mon = mesInicial - 1,
        .tm_mday = diaInicial,
        .tm_hour = horaInicial,
        .tm_min = minutoInicial,
        .tm_sec = segundoInicial,
        .tm_isdst = -1};

    struct tm t_fim = {
        .tm_year = anoFinal - 1900,
        .tm_mon = mesFinal - 1,
        .tm_mday = diaFinal,
        .tm_hour = horaFinal,
        .tm_min = minutoFinal,
        .tm_sec = segundoFinal,
        .tm_isdst = -1};

    int total_sensores = (argc - 13) / 2;

    Sensor sensores[MAX_SENSORES];

    Sensor supported_sensors[] = {
        {"TEMP", "float"},
        {"PRES", "float"},
        {"VIBR", "string"},
        {"UMID", "float"},
        {"VOLT", "float"},
        {"CORR", "float"},
        {"NIVEL", "int"},
        {"FLOW", "bool"}};

    printf("Sensores suportados:\n");
    for (int i = 0; i < (sizeof(supported_sensors) / sizeof(Sensor)); i++)
    {
        printf(" %s - %s\n", supported_sensors[i].nome, supported_sensors[i].tipo);
    }
    printf("\n");

    for (int i = 0; i < total_sensores; i++)
    {

        char nomeSensorUpper[MAX_NOME_SENSOR];
        strncpy(nomeSensorUpper, argv[13 + i * 2], MAX_NOME_SENSOR - 1);
        nomeSensorUpper[MAX_NOME_SENSOR - 1] = '\0';
        to_uppercase(nomeSensorUpper);

        char tipoSensorLower[MAX_TIPO];
        strncpy(tipoSensorLower, argv[14 + i * 2], MAX_TIPO - 1);
        tipoSensorLower[MAX_TIPO - 1] = '\0';
        to_lowercase(tipoSensorLower);

        int sensorValido = 0;

        for (int j = 0; j < sizeof(supported_sensors) / sizeof(Sensor); j++)
        {
            if (strcmp(nomeSensorUpper, supported_sensors[j].nome) == 0 &&
                strcmp(tipoSensorLower, supported_sensors[j].tipo) == 0)
            {
                sensorValido = 1;
                break;
            }
        }

        if (!sensorValido)
        {
            printf("Sensor %s com tipo %s não é suportado.\n", argv[13 + i * 2], argv[14 + i * 2]);
            return 1;
        }

        // Verificar duplicação (com os já validados)
        for (int j = 0; j < i; j++)
        {
            if (strcmp(nomeSensorUpper, sensores[j].nome) == 0)
            {
                printf("Erro: o sensor %s foi informado mais de uma vez.\n", nomeSensorUpper);
                return 1;
            }
        }

        strncpy(sensores[i].nome, argv[13 + i * 2], MAX_NOME_SENSOR - 1);
        sensores[i].nome[MAX_NOME_SENSOR - 1] = '\0';
        strncpy(sensores[i].tipo, argv[14 + i * 2], MAX_TIPO - 1);
        sensores[i].tipo[MAX_TIPO - 1] = '\0';
    }

    char *nomeArquivo;
    nomeArquivo = "entrada.txt";

    FILE *saida = fopen(nomeArquivo, "w");

    if (!saida)
    {
        printf("Erro ao criar o arquivo de saída.\n");
        return 1;
    }

    for (int i = 0; i < total_sensores; i++)
    {
        for (int j = 0; j < TOTAL_ESCRITAS; j++)
        {
            time_t ts = gerar_timestamp_aleatorio(&t_ini, &t_fim);
            gerar_valor_aleatorio(saida, sensores[i].nome, sensores[i].tipo, ts);
        }
    }

    fclose(saida);
    printf("Arquivo '%s' gerado com sucesso com %d leituras por sensor.\n", nomeArquivo, TOTAL_ESCRITAS);
    return 0;
}
