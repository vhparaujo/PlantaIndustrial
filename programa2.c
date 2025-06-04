#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_LINHA 100
#define MAX_MEDIDAS 10000
#define MAX_SENSOR_ID 32
#define TAMANHO_MAX_VALOR 64

typedef struct
{
    time_t timestamp;
    float valor;
} Leitura;

time_t converter_para_timestamp(int dia, int mes, int ano, int hora, int min, int seg)
{
    struct tm t;
    t.tm_year = ano - 1900;
    t.tm_mon = mes - 1;
    t.tm_mday = dia;
    t.tm_hour = hora;
    t.tm_min = min;
    t.tm_sec = seg;
    t.tm_isdst = -1;

    return mktime(&t);
}

// Função para encontrar a leitura mais próxima usando busca binária
int busca_binaria_mais_proxima(Leitura leituras[], int n, time_t alvo)
{
    int esquerda = 0, direita = n - 1;
    int melhor_indice = -1;
    long menor_diferenca = __LONG_MAX__;

    while (esquerda <= direita)
    {
        int meio = (esquerda + direita) / 2;
        long diferenca = labs(leituras[meio].timestamp - alvo);

        if (diferenca < menor_diferenca)
        {
            menor_diferenca = diferenca;
            melhor_indice = meio;
        }

        if (leituras[meio].timestamp < alvo)
        {
            esquerda = meio + 1;
        }
        else if (leituras[meio].timestamp > alvo)
        {
            direita = meio - 1;
        }
        else
        {
            return meio;
        }
    }

    return melhor_indice;
}

int main(int argc, char *argv[])
{
    if (argc != 8)
    {
        printf("Uso: %s SENSOR DIA MES ANO HORA MINUTO SEGUNDO\n", argv[0]);
        return 1;
    }

    char sensor[10];
    strcpy(sensor, argv[1]);

    int dia = atoi(argv[2]);
    int mes = atoi(argv[3]);
    int ano = atoi(argv[4]);
    int hora = atoi(argv[5]);
    int minuto = atoi(argv[6]);
    int segundo = atoi(argv[7]);

    if (dia < 1 || mes < 1 || ano < 0 || hora < 0 || minuto < 0 || segundo < 0)
    {
        printf("Erro: Dia e mês devem ser maiores que 0, já ano, hora, minuto e segundo devem ser maiores ou iguais a 0.\n");
        return 1;
    }

    if (dia > 31 || mes > 12 || hora > 23 || minuto > 59 || segundo > 59)
    {
        printf("Erro: Dia, mês, ano, hora, minuto e segundo devem ser menores que 31, 12, 23, 59, 59 respectivamente.\n");
        return 1;
    }

    if (dia > 30 && (mes == 4 || mes == 6 || mes == 9 || mes == 11))
    {
        printf("Erro: Dia não pode ser maior que 30 para meses com 30 dias.\n");
        return 1;
    }

    if (dia > 29 && mes == 2)
    {
        printf("Erro: Dia não pode ser maior que 29 para fevereiro.\n");
        return 1;
    }

    if (mes == 2 && dia > 28 && (ano % 4 != 0 || (ano % 100 == 0 && ano % 400 != 0)))
    {
        printf("Erro: Dia não pode ser maior que 28 para fevereiro em anos não bissextos.\n");
        return 1;
    }

    time_t timestamp_alvo = converter_para_timestamp(dia, mes, ano, hora, minuto, segundo);

    char nome_arquivo[50];
    snprintf(nome_arquivo, sizeof(nome_arquivo), "%s.txt", sensor);

    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo)
    {
        printf("Erro ao abrir o arquivo '%s'. Verifique se o sensor é válido.\n", nome_arquivo);
        return 1;
    }

    Leitura leituras[MAX_MEDIDAS];
    int total = 0;
    char linha[MAX_LINHA];
    int line_number = 0;

    while (fgets(linha, sizeof(linha), arquivo))
    {
        time_t ts;
        float valor;

        line_number++;

        // Remover quebra de linha e caracteres de retorno
        linha[strcspn(linha, "\r\n")] = 0;

        // Remover BOM UTF-8 se presente na primeira linha
        char *line_ptr = linha;
        if (line_number == 1 && (unsigned char)linha[0] == 0xEF &&
            (unsigned char)linha[1] == 0xBB && (unsigned char)linha[2] == 0xBF)
        {
            line_ptr = linha + 3;
        }

        // Pular espaços em branco no início
        while (*line_ptr == ' ' || *line_ptr == '\t')
        {
            line_ptr++;
        }

        // Pular linhas vazias
        if (strlen(line_ptr) == 0)
            continue;

        // Verificar se há pelo menos dois espaços na linha
        int espacos = 0;
        for (const char *p = line_ptr; *p != '\0'; p++)
        {
            if (*p == ' ')
                espacos++;
        }

        if (espacos != 1)
        {
            printf("Aviso: Linha %d com separador inválido: '%s'\n", line_number, line_ptr);
            continue;
        }

        int parsed = sscanf(line_ptr, "%ld %f", &ts, &valor);

        if (parsed != 2)
        {
            printf("Aviso: Linha %d com formato inválido (parsed=%d): '%s'\n", line_number, parsed, line_ptr);
            continue;
        }

        leituras[total].timestamp = ts;
        leituras[total].valor = valor;
        total++;
    }
    fclose(arquivo);

    if (total == 0)
    {
        printf("Nenhuma leitura encontrada no arquivo '%s'.\n", nome_arquivo);
        return 1;
    }

    int indice_proximo = busca_binaria_mais_proxima(leituras, total, timestamp_alvo);
    if (indice_proximo != -1)
    {
        struct tm *tm_info = localtime(&leituras[indice_proximo].timestamp);
        char buffer[26];
        strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", tm_info);

        printf("Leitura mais próxima encontrada:\n");
        printf("  Timestamp: %ld (%s)\n", leituras[indice_proximo].timestamp, buffer);
        printf("  Valor: %.2f\n", leituras[indice_proximo].valor);
    }
    else
    {
        printf("Nenhuma leitura próxima encontrada.\n");
    }

    return 0;
}
