#include <stdio.h>
#include <math.h>

int fatorial(int n) {
    int resultado = 1;

    for (int i = 1; i <= n; i++) {
        resultado *= i;
    }

    return resultado;
}

int main() {
    float num1, num2, resultado;
    char operador;

    printf("Digite a operacao (+, -, *, /, ^, !): ");
    scanf(" %c", &operador);

    if (operador == '!') {
        printf("Digite o numero: ");
        scanf("%f", &num1);

        resultado = fatorial((int)num1);

        printf("Resultado: %.0f\n", resultado);
    }
    else {
        printf("Digite o primeiro numero: ");
        scanf("%f", &num1);

        printf("Digite o segundo numero: ");
        scanf("%f", &num2);

        switch (operador) {
            case '+':
                resultado = num1 + num2;
                break;

            case '-':
                resultado = num1 - num2;
                break;

            case '*':
                resultado = num1 * num2;
                break;

            case '/':
                if (num2 == 0) {
                    printf("Erro: divisao por zero!\n");
                    return 1;
                }

                resultado = num1 / num2;
                break;

            case '^':
                resultado = pow(num1, num2);
                break;

            default:
                printf("Operador invalido!\n");
                return 1;
        }

        printf("Resultado: %.2f\n", resultado);
    }

    return 0;
}