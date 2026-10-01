#```c
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#define TOTAL_SIMBOLOS 16

/*
 * Cada simbolo egipcio usado pela Egy ocupa 4 bytes em UTF-8.
 *
 * Cada byte do arquivo original e dividido em dois grupos de 4 bits.
 * Cada grupo de 4 bits corresponde a um simbolo egipcio.
 */

const char *egipcio[TOTAL_SIMBOLOS] = {
    "𓂀",
    "𓁐",
    "𓂻",
    "𓃀",
    "𓆣",
    "𓅓",
    "𓇋",
    "𓏏",
    "𓎛",
    "𓂋",
    "𓊪",
    "𓋴",
    "𓈖",
    "𓅱",
    "𓄿",
    "𓎼"
};


/*
 * Escreve um simbolo egipcio correspondente a um valor
 * de 0 a 15.
 */
void escrever_simbolo(FILE *saida, int valor)
{
    if (valor < 0 || valor >= TOTAL_SIMBOLOS)
        return;

    fputs(egipcio[valor], saida);
}


/*
 * Converte um arquivo normal para a representacao Egy.
 *
 * Cada byte:
 *
 *   1010 0110
 *   ---- ----
 *      10   6
 *
 * vira dois simbolos egipcios.
 */
int gerar_egipcio(const char *arquivo_entrada,
                  const char *arquivo_saida)
{
    FILE *entrada = fopen(arquivo_entrada, "rb");

    if (!entrada)
    {
        perror("Erro ao abrir arquivo de entrada");
        return 1;
    }

    FILE *saida = fopen(arquivo_saida, "wb");

    if (!saida)
    {
        perror("Erro ao criar arquivo de saida");
        fclose(entrada);
        return 1;
    }

    int caractere;

    while ((caractere = fgetc(entrada)) != EOF)
    {
        unsigned char byte = (unsigned char)caractere;

        int parte1 = (byte >> 4) & 0x0F;
        int parte2 = byte & 0x0F;

        escrever_simbolo(saida, parte1);
        escrever_simbolo(saida, parte2);
    }

    if (ferror(entrada))
    {
        perror("Erro ao ler arquivo de entrada");

        fclose(entrada);
        fclose(saida);

        return 1;
    }

    if (fclose(entrada) != 0)
    {
        perror("Erro ao fechar arquivo de entrada");
        fclose(saida);
        return 1;
    }

    if (fclose(saida) != 0)
    {
        perror("Erro ao fechar arquivo de saida");
        return 1;
    }

    return 0;
}


/*
 * Le um simbolo egipcio UTF-8.
 *
 * Os simbolos utilizados pela Egy ocupam 4 bytes UTF-8.
 *
 * Retorna:
 *
 *  0  = fim do arquivo
 * -1  = simbolo invalido
 *  1  = simbolo valido
 *
 * O valor correspondente ao simbolo e armazenado em *valor.
 */
int ler_simbolo(FILE *arquivo, int *valor)
{
    unsigned char bytes[5];

    int c1 = fgetc(arquivo);

    if (c1 == EOF)
    {
        if (ferror(arquivo))
            return -1;

        return 0;
    }

    int c2 = fgetc(arquivo);
    int c3 = fgetc(arquivo);
    int c4 = fgetc(arquivo);

    if (c2 == EOF || c3 == EOF || c4 == EOF)
        return -1;

    bytes[0] = (unsigned char)c1;
    bytes[1] = (unsigned char)c2;
    bytes[2] = (unsigned char)c3;
    bytes[3] = (unsigned char)c4;
    bytes[4] = '\0';

    for (int i = 0; i < TOTAL_SIMBOLOS; i++)
    {
        if (strcmp((char *)bytes, egipcio[i]) == 0)
        {
            *valor = i;
            return 1;
        }
    }

    return -1;
}


/*
 * Converte um arquivo Egy de volta para o arquivo original.
 *
 * Dois simbolos egipcios representam um byte.
 */
int traduzir_egipcio(const char *arquivo_entrada,
                     const char *arquivo_saida)
{
    FILE *entrada = fopen(arquivo_entrada, "rb");

    if (!entrada)
    {
        perror("Erro ao abrir arquivo Egy");
        return 1;
    }

    FILE *saida = fopen(arquivo_saida, "wb");

    if (!saida)
    {
        perror("Erro ao criar arquivo de saida");

        fclose(entrada);

        return 1;
    }

    while (1)
    {
        int primeiro;
        int segundo;

        int resultado = ler_simbolo(entrada, &primeiro);

        if (resultado == 0)
            break;

        if (resultado == -1)
        {
            fprintf(stderr,
                    "Erro: simbolo Egy invalido ou arquivo corrompido.\n");

            fclose(entrada);
            fclose(saida);

            return 1;
        }

        resultado = ler_simbolo(entrada, &segundo);

        if (resultado != 1)
        {
            fprintf(stderr,
                    "Erro: arquivo Egy possui quantidade invalida de simbolos.\n");

            fclose(entrada);
            fclose(saida);

            return 1;
        }

        unsigned char byte =
            (unsigned char)((primeiro << 4) | segundo);

        fputc(byte, saida);

        if (ferror(saida))
        {
            perror("Erro ao escrever arquivo de saida");

            fclose(entrada);
            fclose(saida);

            return 1;
        }
    }

    if (ferror(entrada))
    {
        perror("Erro ao ler arquivo Egy");

        fclose(entrada);
        fclose(saida);

        return 1;
    }

    if (fclose(entrada) != 0)
    {
        perror("Erro ao fechar arquivo Egy");
        fclose(saida);
        return 1;
    }

    if (fclose(saida) != 0)
    {
        perror("Erro ao fechar arquivo de saida");
        return 1;
    }

    return 0;
}


/*
 * Compila um arquivo C temporario utilizando GCC.
 *
 * O arquivo Egy e primeiro convertido para C temporario.
 * Depois o GCC compila esse C para o executavel final.
 */
int compilar_egipcio(const char *arquivo_egy,
                     const char *programa_saida)
{
    char caminho_temp[] = "/tmp/egy_XXXXXX";

    int fd = mkstemp(caminho_temp);

    if (fd == -1)
    {
        perror("Erro ao criar arquivo temporario");
        return 1;
    }

    FILE *temporario = fdopen(fd, "wb");

    if (!temporario)
    {
        perror("Erro ao abrir arquivo temporario");

        close(fd);
        unlink(caminho_temp);

        return 1;
    }

    /*
     * Converte Egy para C.
     */
    FILE *entrada = fopen(arquivo_egy, "rb");

    if (!entrada)
    {
        perror("Erro ao abrir arquivo Egy");

        fclose(temporario);
        unlink(caminho_temp);

        return 1;
    }

    while (1)
    {
        int primeiro;
        int segundo;

        int resultado = ler_simbolo(entrada, &primeiro);

        if (resultado == 0)
            break;

        if (resultado == -1)
        {
            fprintf(stderr,
                    "Erro: simbolo Egy invalido ou arquivo corrompido.\n");

            fclose(entrada);
            fclose(temporario);
            unlink(caminho_temp);

            return 1;
        }

        resultado = ler_simbolo(entrada, &segundo);

        if (resultado != 1)
        {
            fprintf(stderr,
                    "Erro: arquivo Egy possui quantidade invalida de simbolos.\n");

            fclose(entrada);
            fclose(temporario);
            unlink(caminho_temp);

            return 1;
        }

        unsigned char byte =
            (unsigned char)((primeiro << 4) | segundo);

        fputc(byte, temporario);

        if (ferror(temporario))
        {
            perror("Erro ao escrever arquivo temporario");

            fclose(entrada);
            fclose(temporario);
            unlink(caminho_temp);

            return 1;
        }
    }

    if (ferror(entrada))
    {
        perror("Erro ao ler arquivo Egy");

        fclose(entrada);
        fclose(temporario);
        unlink(caminho_temp);

        return 1;
    }

    fclose(entrada);

    if (fclose(temporario) != 0)
    {
        perror("Erro ao fechar arquivo temporario");
        unlink(caminho_temp);

        return 1;
    }

    /*
     * Cria processo filho para executar GCC.
     */
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("Erro ao criar processo");

        unlink(caminho_temp);

        return 1;
    }

    if (pid == 0)
    {
        /*
         * Processo filho.
         *
         * gcc -x c arquivo_temporario -o programa -lm
         */
        execlp(
            "gcc",
            "gcc",
            "-x",
            "c",
            caminho_temp,
            "-o",
            programa_saida,
            "-lm",
            (char *)NULL
        );

        perror("Nao foi possivel executar GCC");

        _exit(127);
    }

    /*
     * Processo pai espera o GCC terminar.
     */
    int status;

    if (waitpid(pid, &status, 0) == -1)
    {
        perror("Erro ao esperar pelo GCC");

        unlink(caminho_temp);

        return 1;
    }

    unlink(caminho_temp);

    if (WIFEXITED(status))
    {
        int codigo = WEXITSTATUS(status);

        if (codigo == 0)
            return 0;

        fprintf(stderr,
                "GCC terminou com codigo %d.\n",
                codigo);

        return codigo;
    }

    if (WIFSIGNALED(status))
    {
        fprintf(stderr,
                "GCC foi encerrado pelo sinal %d.\n",
                WTERMSIG(status));

        return 1;
    }

    return 1;
}


/*
 * Mostra a ajuda do programa.
 */
void mostrar_ajuda(const char *programa)
{
    printf("\n");
    printf("Egy - Egyptian Esoteric Programming Language\n");
    printf("\n");

    printf("Uso:\n");
    printf("  %s -e <entrada> <saida>\n", programa);
    printf("  %s -d <entrada> <saida>\n", programa);
    printf("  %s -c <entrada.egy> <executavel>\n", programa);

    printf("\n");

    printf("Opcoes:\n");
    printf("  -e    Converte um arquivo para Egy\n");
    printf("  -d    Converte um arquivo Egy para o formato original\n");
    printf("  -c    Converte Egy para C e compila com GCC\n");
    printf("  -h    Mostra esta ajuda\n");
    printf("  --help Mostra esta ajuda\n");

    printf("\n");

    printf("Exemplos:\n");
    printf("  %s -e programa.c programa.egy\n", programa);
    printf("  %s -d programa.egy programa.c\n", programa);
    printf("  %s -c programa.egy programa\n", programa);

    printf("\n");
}


/*
 * Funcao principal.
 */
int main(int argc, char *argv[])
{
    /*
     * Sem argumentos:
     * mostra ajuda.
     */
    if (argc == 1)
    {
        mostrar_ajuda(argv[0]);
        return 0;
    }

    /*
     * Ajuda.
     */
    if (strcmp(argv[1], "-h") == 0 ||
        strcmp(argv[1], "--help") == 0)
    {
        mostrar_ajuda(argv[0]);
        return 0;
    }

    /*
     * Todas as operacoes normais precisam de:
     *
     * programa
     * opcao
     * entrada
     * saida
     *
     * argc = 4
     */
    if (argc != 4)
    {
        fprintf(stderr,
                "Numero incorreto de argumentos.\n");

        mostrar_ajuda(argv[0]);

        return 1;
    }

    const char *opcao = argv[1];
    const char *entrada = argv[2];
    const char *saida = argv[3];

    /*
     * C -> Egy
     */
    if (strcmp(opcao, "-e") == 0)
    {
        printf("Convertendo para Egy...\n");

        int resultado =
            gerar_egipcio(entrada, saida);

        if (resultado == 0)
        {
            printf("Conversao concluida!\n");
            printf("Arquivo: %s\n", saida);
        }

        return resultado;
    }

    /*
     * Egy -> C
     */
    if (strcmp(opcao, "-d") == 0)
    {
        printf("Convertendo Egy...\n");

        int resultado =
            traduzir_egipcio(entrada, saida);

        if (resultado == 0)
        {
            printf("Conversao concluida!\n");
            printf("Arquivo: %s\n", saida);
        }

        return resultado;
    }

    /*
     * Egy -> C temporario -> GCC -> executavel
     */
    if (strcmp(opcao, "-c") == 0)
    {
        printf("Compilando Egy...\n");

        int resultado =
            compilar_egipcio(entrada, saida);

        if (resultado == 0)
        {
            printf("Compilacao concluida!\n");
            printf("Executavel: %s\n", saida);
        }

        return resultado;
    }

    /*
     * Opcao desconhecida.
     */
    fprintf(stderr,
            "Opcao desconhecida: %s\n",
            opcao);

    mostrar_ajuda(argv[0]);

    return 1;
}
```
