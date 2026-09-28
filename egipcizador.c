#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define TOTAL_SIMBOLOS 16

/*
 * Cada byte do arquivo C e dividido em dois blocos de 4 bits.
 *
 * Exemplo:
 *
 * byte = 0x41
 *
 * 4 = 0100
 * 1 = 0001
 *
 * Cada numero vira um hieroglifo.
 */

const char *egipcio[TOTAL_SIMBOLOS] = {
    "𓂀",
    "𓁐",
    "𓂻",
    "𓃀",
    "𓆣",
    "𓇋",
    "𓈖",
    "𓉐",
    "𓊪",
    "𓋴",
    "𓌙",
    "𓍿",
    "𓎛",
    "𓏏",
    "𓐍",
    "𓂋"
};


/*
 * Mostra como usar o programa.
 */

void ajuda(const char *programa)
{
    printf("\n");
    printf("EGIPCIZADOR\n");
    printf("===========\n\n");

    printf("Gerar arquivo egipcio:\n");
    printf("  %s -e calculadora.c calculadora_egipcia.egy\n\n",
           programa);

    printf("Traduzir e compilar:\n");
    printf("  %s -c calculadora_egipcia.egy calculadora\n\n",
           programa);
}


/*
 * Escreve um hieroglifo no arquivo.
 */

int escrever_simbolo(FILE *saida, int numero)
{
    if (fputs(egipcio[numero], saida) == EOF)
        return 0;

    return 1;
}


/*
 * CONVERTER C -> EGIPCIO
 *
 * Cada byte do arquivo original vira dois hieroglifos.
 */

int gerar_egipcio(const char *entrada_nome,
                  const char *saida_nome)
{
    FILE *entrada;
    FILE *saida;

    int caractere;

    unsigned long quantidade = 0;


    entrada = fopen(entrada_nome, "rb");

    if (entrada == NULL)
    {
        perror("Erro abrindo arquivo");

        return 1;
    }


    saida = fopen(saida_nome, "wb");

    if (saida == NULL)
    {
        perror("Erro criando arquivo");

        fclose(entrada);

        return 1;
    }


    /*
     * Lemos byte por byte.
     */

    while ((caractere = fgetc(entrada)) != EOF)
    {
        unsigned char byte;

        int parte1;
        int parte2;


        byte = (unsigned char)caractere;


        /*
         * Pega os 4 bits superiores.
         */

        parte1 = (byte >> 4) & 0x0F;


        /*
         * Pega os 4 bits inferiores.
         */

        parte2 = byte & 0x0F;


        if (!escrever_simbolo(saida, parte1))
        {
            printf("Erro escrevendo arquivo.\n");

            fclose(entrada);
            fclose(saida);

            return 1;
        }


        if (!escrever_simbolo(saida, parte2))
        {
            printf("Erro escrevendo arquivo.\n");

            fclose(entrada);
            fclose(saida);

            return 1;
        }


        quantidade++;
    }


    fclose(entrada);
    fclose(saida);


    printf("\n");
    printf("=================================\n");
    printf("      EGIPCIZACAO CONCLUIDA\n");
    printf("=================================\n\n");

    printf("Entrada : %s\n", entrada_nome);
    printf("Saida   : %s\n", saida_nome);
    printf("Bytes   : %lu\n", quantidade);
    printf("Simbolos: %lu\n\n", quantidade * 2);


    return 0;
}


/*
 * Lê um hieroglifo UTF-8.
 *
 * Cada hieroglifo usado neste programa possui 3 bytes UTF-8.
 *
 * Retorna:
 *
 *  0  = fim do arquivo
 *  1  = simbolo encontrado
 * -1  = arquivo invalido
 */

int ler_simbolo(FILE *arquivo, int *valor)
{
    unsigned char bytes[4];

    int c1;
    int c2;
    int c3;


    c1 = fgetc(arquivo);


    if (c1 == EOF)
        return 0;


    c2 = fgetc(arquivo);

    c3 = fgetc(arquivo);


    if (c2 == EOF || c3 == EOF)
        return -1;


    bytes[0] = (unsigned char)c1;
    bytes[1] = (unsigned char)c2;
    bytes[2] = (unsigned char)c3;
    bytes[3] = '\0';


    /*
     * Compara com cada hieroglifo.
     */

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
 * CONVERTER EGIPCIO -> C
 *
 * Dois hieroglifos formam um byte.
 */

int traduzir_egipcio(const char *entrada_nome,
                     FILE *saida)
{
    FILE *entrada;

    int primeiro;
    int segundo;

    unsigned long bytes = 0;


    entrada = fopen(entrada_nome, "rb");

    if (entrada == NULL)
    {
        perror("Erro abrindo arquivo egipcio");

        return 1;
    }


    while (1)
    {
        int resultado;


        /*
         * Primeiro meio-byte.
         */

        resultado = ler_simbolo(entrada, &primeiro);


        if (resultado == 0)
            break;


        if (resultado < 0)
        {
            printf("Arquivo egipcio invalido.\n");

            fclose(entrada);

            return 1;
        }


        /*
         * Segundo meio-byte.
         */

        resultado = ler_simbolo(entrada, &segundo);


        if (resultado <= 0)
        {
            printf("Arquivo egipcio esta incompleto.\n");

            fclose(entrada);

            return 1;
        }


        /*
         * Junta os dois blocos de 4 bits.
         */

        unsigned char byte;

        byte = (unsigned char)
               ((primeiro << 4) | segundo);


        /*
         * Escreve o byte original.
         */

        if (fputc(byte, saida) == EOF)
        {
            printf("Erro escrevendo C.\n");

            fclose(entrada);

            return 1;
        }


        bytes++;
    }


    fclose(entrada);


    return 0;
}


/*
 * TRADUZ E CHAMA O GCC
 */

int compilar_egipcio(const char *entrada_nome,
                     const char *programa_saida)
{
    char caminho_temp[] = "/tmp/egipcizador-XXXXXX";

    int descritor;

    FILE *arquivo_temp;

    pid_t processo;

    int status;


    /*
     * Cria arquivo temporario.
     */

    descritor = mkstemp(caminho_temp);


    if (descritor == -1)
    {
        perror("Erro criando temporario");

        return 1;
    }


    arquivo_temp = fdopen(descritor, "wb");


    if (arquivo_temp == NULL)
    {
        perror("Erro abrindo temporario");

        close(descritor);

        unlink(caminho_temp);

        return 1;
    }


    printf("\n");
    printf("Traduzindo hieroglifos para C...\n");


    /*
     * Reconstrói o código C.
     */

    if (traduzir_egipcio(entrada_nome,
                         arquivo_temp) != 0)
    {
        fclose(arquivo_temp);

        unlink(caminho_temp);

        return 1;
    }


    fclose(arquivo_temp);


    printf("C reconstruido.\n");
    printf("Chamando GCC...\n\n");


    /*
     * Cria processo para executar GCC.
     */

    processo = fork();


    if (processo == -1)
    {
        perror("Erro no fork");

        unlink(caminho_temp);

        return 1;
    }


    /*
     * Processo filho.
     */

    if (processo == 0)
    {
        /*
         * -x c:
         * fala para o GCC tratar o arquivo temporario
         * como codigo C.
         *
         * -lm:
         * necessario para pow().
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


        /*
         * Se chegou aqui, o GCC nao abriu.
         */

        perror("Nao foi possivel executar GCC");

        _exit(127);
    }


    /*
     * Processo principal espera o GCC.
     */

    if (waitpid(processo,
                &status,
                0) == -1)
    {
        perror("Erro esperando GCC");

        unlink(caminho_temp);

        return 1;
    }


    /*
     * Apaga o C temporario.
     */

    unlink(caminho_temp);


    if (WIFEXITED(status))
    {
        int codigo;

        codigo = WEXITSTATUS(status);


        if (codigo == 0)
        {
            printf("\n");
            printf("==============================\n");
            printf("       COMPILADO!\n");
            printf("==============================\n\n");

            printf("Executavel: %s\n\n",
                   programa_saida);

            return 0;
        }


        printf("\n");
        printf("O GCC terminou com codigo %d.\n",
               codigo);

        return codigo;
    }


    return 1;
}


/*
 * MAIN
 */

int main(int argc, char *argv[])
{
    /*
     * Precisamos de:
     *
     * -e entrada saida
     *
     * ou
     *
     * -c entrada_egipcia executavel
     */

    if (argc != 4)
    {
        ajuda(argv[0]);

        return 1;
    }


    /*
     * MODO EGIPCIO
     */

    if (strcmp(argv[1], "-e") == 0)
    {
        return gerar_egipcio(
            argv[2],
            argv[3]
        );
    }


    /*
     * MODO COMPILAR
     */

    if (strcmp(argv[1], "-c") == 0)
    {
        return compilar_egipcio(
            argv[2],
            argv[3]
        );
    }


    /*
     * Opcao desconhecida.
     */

    printf("Opcao desconhecida: %s\n\n",
           argv[1]);

    ajuda(argv[0]);

    return 1;
}