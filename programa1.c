#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAMANHO_MAX_LINHA 256
#define MAX_SENSOR_ID 32
#define TAMANHO_MAX_VALOR 64
#define NUM_SENSORS 8

// Estrutura para armazenar uma leitura
typedef struct
{
    long timestamp;
    char sensor_id[MAX_SENSOR_ID];
    char valor[TAMANHO_MAX_VALOR];
} Leitura;

// Estrutura para armazenar dados de um sensor específico
typedef struct
{
    char sensor_id[MAX_SENSOR_ID];
    Leitura *leituras;
    int count;
    int capacity;
} SensorData;

typedef enum
{
    TIPO_FLOAT,
    TIPO_INT,
    TIPO_BOOL,
    TIPO_STRING
} TipoDado;

// Lista de sensores suportados
typedef struct
{
    char id[MAX_SENSOR_ID];
    char filename[MAX_SENSOR_ID + 4]; // id + ".txt"
    char description[64];
    TipoDado tipo_dado;
} SensorType;

// Definindo os tipos de sensores suportados
SensorType supported_sensors[] = {
    {"TEMP", "TEMP.txt", "Sensor de Temperatura", TIPO_FLOAT},
    {"PRES", "PRES.txt", "Sensor de Pressão", TIPO_FLOAT},
    {"VIBR", "VIBR.txt", "Sensor de Vibração", TIPO_STRING},
    {"UMID", "UMID.txt", "Sensor de Umidade", TIPO_FLOAT},
    {"VOLT", "VOLT.txt", "Sensor de Voltagem", TIPO_FLOAT},
    {"CORR", "CORR.txt", "Sensor de Corrente", TIPO_FLOAT},
    {"NIVEL", "NIVEL.txt", "Sensor de Nível", TIPO_INT},
    {"FLOW", "FLOW.txt", "Sensor de Fluxo", TIPO_BOOL}};

#define NUM_SUPPORTED_SENSORS (sizeof(supported_sensors) / sizeof(SensorType))

int is_valid_float(const char *str)
{
    char *endptr;
    strtof(str, &endptr);
    return (*endptr == '\0');
}

int is_valid_int(const char *str)
{
    char *endptr;
    strtol(str, &endptr, 10);
    return (*endptr == '\0');
}

int is_valid_bool(const char *str)
{
    return (strcmp(str, "true") == 0 || strcmp(str, "false") == 0);
}

int is_valid_string(const char *str)
{
    return strlen(str) > 0; // Aceita qualquer string não vazia
}

int validar_valor_por_tipo(const SensorType *sensor, const char *valor)
{
    switch (sensor->tipo_dado)
    {
    case TIPO_FLOAT:
        return is_valid_float(valor);
    case TIPO_INT:
        return is_valid_int(valor);
    case TIPO_BOOL:
        return is_valid_bool(valor);
    case TIPO_STRING:
        return is_valid_string(valor);
    default:
        return 0;
    }
}

SensorType *get_sensor_type(const char *sensor_id)
{
    for (int i = 0; i < NUM_SUPPORTED_SENSORS; i++)
    {
        if (strcmp(sensor_id, supported_sensors[i].id) == 0)
        {
            return &supported_sensors[i];
        }
    }
    return NULL;
}

// Função para validar se um timestamp Unix epoch é válido
int is_valid_unix_timestamp(long timestamp)
{
    // Unix epoch começou em 1 de janeiro de 1970 00:00:00 UTC
    // Timestamp 0 = 1970-01-01 00:00:00 UTC

    // Verificar se não é negativo (antes de 1970)
    if (timestamp < 0)
    {
        return 0;
    }

    // Verificar limite superior razoável
    // 4102444800 = 2100-01-01 00:00:00 UTC (limite mais generoso)
    if (timestamp > 4102444800L)
    {
        return 0;
    }

    return 1;
}

// Função para converter timestamp para string legível (para debug)
void timestamp_to_string(long timestamp, char *buffer, size_t buffer_size)
{
    time_t time = (time_t)timestamp;
    struct tm *tm_info = gmtime(&time);

    if (tm_info != NULL)
    {
        strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S UTC", tm_info);
    }
    else
    {
        snprintf(buffer, buffer_size, "INVALID");
    }
}

// Função para comparar leituras por timestamp (para qsort)
int comparar_leituras(const void *a, const void *b)
{
    Leitura *leitura_a = (Leitura *)a;
    Leitura *leitura_b = (Leitura *)b;

    if (leitura_a->timestamp < leitura_b->timestamp)
        return -1;
    if (leitura_a->timestamp > leitura_b->timestamp)
        return 1;
    return 0;
}

// Função para verificar se um sensor é suportado
int is_supported_sensor(const char *sensor_id)
{
    for (int i = 0; i < NUM_SUPPORTED_SENSORS; i++)
    {
        if (strcmp(sensor_id, supported_sensors[i].id) == 0)
        {
            return 1;
        }
    }
    return 0;
}

// Função para obter o nome do arquivo de um sensor
const char *get_sensor_filename(const char *sensor_id)
{
    for (int i = 0; i < NUM_SUPPORTED_SENSORS; i++)
    {
        if (strcmp(sensor_id, supported_sensors[i].id) == 0)
        {
            return supported_sensors[i].filename;
        }
    }
    return NULL;
}

// Função para inicializar dados do sensor
void init_sensor_data(SensorData *sensor, const char *sensor_id)
{
    strcpy(sensor->sensor_id, sensor_id);
    sensor->capacity = 1000;
    sensor->leituras = malloc(sensor->capacity * sizeof(Leitura));
    sensor->count = 0;
}

// Função para adicionar leitura aos dados do sensor
void add_leituras(SensorData *sensor, long timestamp, const char *valor)
{
    // Redimensionar array se necessário
    if (sensor->count >= sensor->capacity)
    {
        sensor->capacity *= 2;
        sensor->leituras = realloc(sensor->leituras, sensor->capacity * sizeof(Leitura));
        if (sensor->leituras == NULL)
        {
            fprintf(stderr, "Erro: Falha na alocação de memória\n");
            exit(1);
        }
    }

    // Adicionar nova leitura
    sensor->leituras[sensor->count].timestamp = timestamp;
    strcpy(sensor->leituras[sensor->count].sensor_id, sensor->sensor_id);
    strcpy(sensor->leituras[sensor->count].valor, valor);
    sensor->count++;
}

// Função para encontrar sensor na lista ou criar novo
SensorData *find_or_create_sensor(SensorData **sensors, int *num_sensors, const char *sensor_id)
{
    // Procurar sensor existente
    for (int i = 0; i < *num_sensors; i++)
    {
        if (strcmp(sensors[i]->sensor_id, sensor_id) == 0)
        {
            return sensors[i];
        }
    }

    // Criar novo sensor
    sensors[*num_sensors] = malloc(sizeof(SensorData));
    init_sensor_data(sensors[*num_sensors], sensor_id);
    (*num_sensors)++;

    return sensors[*num_sensors - 1];
}

// Função para salvar dados do sensor em arquivo
void save_sensor_data(SensorData *sensor)
{
    const char *filename = get_sensor_filename(sensor->sensor_id);
    if (filename == NULL)
    {
        printf("Aviso: Sensor %s não reconhecido, pulando...\n", sensor->sensor_id);
        return;
    }

    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        fprintf(stderr, "Erro: Não foi possível criar arquivo %s\n", filename);
        return;
    }

    // Ordenar readings por timestamp
    qsort(sensor->leituras, sensor->count, sizeof(Leitura), comparar_leituras);

    // Escrever dados ordenados no arquivo
    for (int i = 0; i < sensor->count; i++)
    {
        fprintf(file, "%ld %s\n",
                sensor->leituras[i].timestamp,
                sensor->leituras[i].valor);
    }

    fclose(file);
    printf("Sensor %s: %d leituras salvas em %s\n", sensor->sensor_id, sensor->count, filename);
}

// Função para liberar memória dos sensores
void free_sensors(SensorData **sensors, int num_sensors)
{
    for (int i = 0; i < num_sensors; i++)
    {
        free(sensors[i]->leituras);
        free(sensors[i]);
    }
}

// Função para exibir informações sobre sensores suportados
void print_supported_sensors()
{
    printf("Sensores suportados:\n");
    for (int i = 0; i < NUM_SUPPORTED_SENSORS; i++)
    {
        printf(" %s - %s\n", supported_sensors[i].id, supported_sensors[i].description);
    }
    printf("\n");
}

// Função principal
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Uso: %s <arquivo_entrada>\n\n", argv[0]);
        print_supported_sensors();
        return 1;
    }

    FILE *input_file = fopen(argv[1], "r");
    if (input_file == NULL)
    {
        fprintf(stderr, "Erro: Não foi possível abrir o arquivo %s\n", argv[1]);
        return 1;
    }

    printf("Processando arquivo: %s\n\n", argv[1]);
    print_supported_sensors();

    // Array para armazenar dados de diferentes sensores
    SensorData *sensors[NUM_SUPPORTED_SENSORS];
    int num_sensors = 0;

    char line[TAMANHO_MAX_LINHA];
    int line_number = 0;
    int total_leituras = 0;
    int ignored_leituras = 0;
    int invalid_timestamps = 0;

    // Processar cada linha do arquivo
    while (fgets(line, sizeof(line), input_file))
    {
        line_number++;

        // Remover quebra de linha e caracteres de retorno
        line[strcspn(line, "\r\n")] = 0;

        // Remover BOM UTF-8 se presente na primeira linha
        char *line_ptr = line;
        if (line_number == 1 && (unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF)
        {
            line_ptr = line + 3;
        }

        // Pular espaços em branco no início
        while (*line_ptr == ' ' || *line_ptr == '\t')
        {
            line_ptr++;
        }

        // Pular linhas vazias
        if (strlen(line_ptr) == 0)
            continue;

        // Parsear linha: timestamp sensor_id valor
        long timestamp;
        char sensor_id[MAX_SENSOR_ID];
        char valor[TAMANHO_MAX_VALOR];

        // Verificar se há pelo menos dois espaços na linha
        int espacos = 0;
        for (const char *p = line_ptr; *p != '\0'; p++)
        {
            if (*p == ' ')
                espacos++;
        }

        if (espacos != 2)
        {
            printf("Aviso: Linha %d com separadores inválidos: '%s'\n", line_number, line_ptr);
            continue;
        }

        int parsed = sscanf(line_ptr, "%ld %s %s", &timestamp, sensor_id, valor);

        if (parsed != 3)
        {
            printf("Aviso: Linha %d com formato inválido (parsed=%d): '%s'\n", line_number, parsed, line_ptr);
            continue;
        }

        // Validar timestamp Unix epoch
        if (!is_valid_unix_timestamp(timestamp))
        {
            char time_str[64];
            timestamp_to_string(timestamp, time_str, sizeof(time_str));
            printf("Aviso: Linha %d com timestamp inválido %ld (%s): '%s'\n",
                   line_number, timestamp, time_str, line_ptr);
            invalid_timestamps++;
            continue;
        }

        // Verificar se o sensor é suportado
        if (!is_supported_sensor(sensor_id))
        {
            ignored_leituras++;
            continue;
        }

        // Encontrar ou criar dados do sensor
        SensorData *sensor = find_or_create_sensor(sensors, &num_sensors, sensor_id);
        SensorType *sensor_type = get_sensor_type(sensor_id);

        if (!sensor || !sensor_type || !validar_valor_por_tipo(sensor_type, valor))
        {
            printf("Aviso: Linha %d com valor inválido para o sensor %s: '%s'\n\n", line_number, sensor_id, valor);
            ignored_leituras++;
            continue;
        }

        // Adicionar leitura
        add_leituras(sensor, timestamp, valor);
        total_leituras++;
    }

    fclose(input_file);

    printf("Processamento concluído:\n");
    printf("  Total de linhas processadas: %d\n", line_number);
    printf("  Leituras válidas: %d\n", total_leituras);
    printf("  Leituras ignoradas (sensor não suportado): %d\n", ignored_leituras);
    printf("  Leituras ignoradas (timestamp inválido): %d\n", invalid_timestamps);
    printf("  Sensores únicos encontrados: %d\n\n", num_sensors);

    // Salvar dados de cada sensor em arquivo separado
    printf("Salvando dados ordenados por timestamp:\n");
    for (int i = 0; i < num_sensors; i++)
    {
        save_sensor_data(sensors[i]);
    }

    // Liberar memória
    free_sensors(sensors, num_sensors);

    printf("\nOrganização dos dados concluída com sucesso!\n");

    return 0;
}