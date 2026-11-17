// menu pra demonstrar a splay
// compilar: gcc -std=c99 -Wall demo/menu_splay.c -o menu_splay

#include "../arvores/splay.c"

int main() {
    No *raiz = NULL;
    int op, x, achou;
 
    do {
        printf("\n1 - Inserir\n2 - Buscar\n3 - Remover\n4 - Imprimir\n0 - Sair\n> ");
        if (scanf("%d", &op) != 1) {
            printf("Erro: entrada invalida\n");
            exit(1);
        }
 
        switch (op) {
            case 1:
                printf("Valor: ");
                scanf("%d", &x);
                raiz = inserir(raiz, x);
                break;
            case 2:
                printf("Valor: ");
                scanf("%d", &x);
                raiz = buscar(raiz, x, &achou);
                if (achou)
                    printf("%d encontrado (agora ta na raiz)\n", x);
                else
                    printf("%d nao encontrado\n", x);
                break;
            case 3:
                printf("Valor: ");
                scanf("%d", &x);
                raiz = remover(raiz, x);
                break;
            case 4:
                if (raiz == NULL) {
                    printf("Arvore vazia\n");
                } else {
                    printf("Raiz: %d\n", raiz->chave);
                    printf("Em ordem: ");
                    em_ordem(raiz);
                    printf("\nPre ordem: ");
                    preOrdem(raiz);
                    printf("\n");
                }
                break;
            case 0:
                break;
            default:
                printf("Opcao invalida\n");
        }
    } while (op != 0);
 
    liberar(raiz);
    return 0;
}
 