#include <stdio.h>
#include <stdlib.h>

#define T 2            // grau minimo (T = 2 eh a arvore 2-3-4)
#define MAX (2 * T - 1) // maximo de chaves num no

long long visitadosB = 0; // contador de nos visitados (pros testes)

// no da arvore B
typedef struct NoB {
    int n;                       // quantas chaves o no tem
    int chaves[MAX];             // chaves em ordem
    struct NoB *filhos[MAX + 1]; // n + 1 filhos
    int folha;                   // 1 se nao tem filhos
} NoB;

// cria um no novo
NoB *novoNoB(int folha) {
    NoB *no = (NoB *) malloc(sizeof(NoB));
    if (no == NULL) {
        printf("Erro: sem memoria\n");
        exit(1);
    }
    no->n = 0;
    no->folha = folha;
    return no;
}

// busca: em cada no acha a primeira chave >= a procurada, se nao for ela desce
int buscarB(NoB *raiz, int chave) {
    NoB *no = raiz;
    while (no != NULL) {
        visitadosB++;
        int i = 0;
        while (i < no->n && chave > no->chaves[i])
            i++;
        if (i < no->n && chave == no->chaves[i])
            return 1;
        if (no->folha)
            return 0;
        no = no->filhos[i];
    }
    return 0;
}

// divide o filho i do pai, que ta cheio
// a chave do meio sobe pro pai, a metade de cima vai pra um no novo
void dividirB(NoB *pai, int i) {
    NoB *esq = pai->filhos[i];
    NoB *dir = novoNoB(esq->folha);

    // metade de cima vai pro no novo
    dir->n = T - 1;
    for (int j = 0; j < T - 1; j++)
        dir->chaves[j] = esq->chaves[j + T];
    if (!esq->folha)
        for (int j = 0; j < T; j++)
            dir->filhos[j] = esq->filhos[j + T];
    esq->n = T - 1;

    // abre espaco no pai pro no novo e pra chave do meio
    for (int j = pai->n; j > i; j--)
        pai->filhos[j + 1] = pai->filhos[j];
    pai->filhos[i + 1] = dir;
    for (int j = pai->n - 1; j >= i; j--)
        pai->chaves[j + 1] = pai->chaves[j];
    pai->chaves[i] = esq->chaves[T - 1];
    pai->n++;
}

// insere e retorna a raiz (muda quando a raiz ta cheia)
// vai descendo e dividindo todo no cheio no caminho, assim sempre tem espaco na folha
NoB *inserirB(NoB *raiz, int chave) {
    if (raiz == NULL)
        raiz = novoNoB(1);

    // raiz cheia: cria uma raiz nova em cima e divide (a arvore cresce pra cima)
    if (raiz->n == MAX) {
        NoB *nova = novoNoB(0);
        nova->filhos[0] = raiz;
        dividirB(nova, 0);
        raiz = nova;
    }

    NoB *no = raiz;
    while (1) {
        visitadosB++;
        int i = 0;
        while (i < no->n && chave > no->chaves[i])
            i++;

        // ja existe, nao insere repetido
        if (i < no->n && chave == no->chaves[i])
            return raiz;

        // chegou na folha: empurra as maiores pra direita e coloca a chave
        if (no->folha) {
            for (int j = no->n; j > i; j--)
                no->chaves[j] = no->chaves[j - 1];
            no->chaves[i] = chave;
            no->n++;
            return raiz;
        }

        // filho cheio: divide antes de descer
        if (no->filhos[i]->n == MAX) {
            dividirB(no, i);
            if (chave == no->chaves[i]) // eh a chave que subiu
                return raiz;
            if (chave > no->chaves[i])
                i++;
        }
        no = no->filhos[i];
    }
}

// coloca as chaves em ordem no vetor (usado pra passar os dados pra splay)
void emOrdemB(NoB *no, int *vet, int *pos) {
    if (no == NULL) return;
    for (int i = 0; i < no->n; i++) {
        if (!no->folha) emOrdemB(no->filhos[i], vet, pos);
        vet[(*pos)++] = no->chaves[i];
    }
    if (!no->folha) emOrdemB(no->filhos[no->n], vet, pos);
}

// libera tudo no final
void liberarB(NoB *no) {
    if (no == NULL) return;
    if (!no->folha)
        for (int i = 0; i <= no->n; i++)
            liberarB(no->filhos[i]);
    free(no);
}
