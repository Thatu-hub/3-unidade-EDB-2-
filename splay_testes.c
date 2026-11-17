
#include <stdio.h>
#include <stdlib.h>
 
// no da arvore splay
typedef struct No {
    int chave;
    struct No *esq;
    struct No *dir;
} No;
 
// cria um no novo
No *novoNo(int chave) {
    No *n = (No *) malloc(sizeof(No));
    if (n == NULL) {
        printf("Erro: sem memoria\n");
        exit(1);
    }
    n->chave = chave;
    n->esq = NULL;
    n->dir = NULL;
    return n;
}
 
// rotacao pra direita (filho esq sobe)
No *rotDir(No *x) {
    No *aux = x->esq;
    x->esq = aux->dir;
    aux->dir = x;
    return aux;
}
 
// rotacao pra esquerda (filho dir sobe)
No *rotEsq(No *x) {
    No *aux = x->dir;
    x->dir = aux->esq;
    aux->esq = x;
    return aux;
}
 
// traz a chave (ou o ultimo no visitado) pra raiz
// zig, zig-zig e zig-zag tudo aqui
No *splay(No *raiz, int chave) {
    // arvore vazia ou ja ta na raiz
    if (raiz == NULL || raiz->chave == chave)
        return raiz;
 
    // chave ta na subarvore esquerda
    if (chave < raiz->chave) {
        if (raiz->esq == NULL)
            return raiz; // nao achou, para aqui
 
        // zig-zig (esq esq)
        if (chave < raiz->esq->chave) {
            raiz->esq->esq = splay(raiz->esq->esq, chave);
            raiz = rotDir(raiz);
        }
        // zig-zag (esq dir)
        else if (chave > raiz->esq->chave) {
            raiz->esq->dir = splay(raiz->esq->dir, chave);
            if (raiz->esq->dir != NULL)
                raiz->esq = rotEsq(raiz->esq);
        }
 
        // ultima rotacao (zig)
        if (raiz->esq == NULL)
            return raiz;
        return rotDir(raiz);
    }
    // chave ta na subarvore direita
    else {
        if (raiz->dir == NULL)
            return raiz;
 
        // zag-zig (dir esq)
        if (chave < raiz->dir->chave) {
            raiz->dir->esq = splay(raiz->dir->esq, chave);
            if (raiz->dir->esq != NULL)
                raiz->dir = rotDir(raiz->dir);
        }
        // zag-zag (dir dir)
        else if (chave > raiz->dir->chave) {
            raiz->dir->dir = splay(raiz->dir->dir, chave);
            raiz = rotEsq(raiz);
        }
 
        if (raiz->dir == NULL)
            return raiz;
        return rotEsq(raiz);
    }
}
 
// busca: faz o splay e ve se a raiz eh a chave
No *buscar(No *raiz, int chave, int *achou) {
    raiz = splay(raiz, chave);
    if (raiz != NULL && raiz->chave == chave)
        *achou = 1;
    else
        *achou = 0;
    return raiz;
}
 
// insere e deixa o novo no na raiz
No *inserir(No *raiz, int chave) {
    if (raiz == NULL)
        return novoNo(chave);
 
    raiz = splay(raiz, chave);
 
    // ja existe, nao insere repetido
    if (raiz->chave == chave)
        return raiz;
 
    No *n = novoNo(chave);
 
    // a raiz atual vira filho do novo no
    if (chave < raiz->chave) {
        n->dir = raiz;
        n->esq = raiz->esq;
        raiz->esq = NULL;
    } else {
        n->esq = raiz;
        n->dir = raiz->dir;
        raiz->dir = NULL;
    }
    return n;
}
 
// remove: splay na chave, depois junta as duas subarvores
No *remover(No *raiz, int chave) {
    if (raiz == NULL)
        return NULL;
 
    raiz = splay(raiz, chave);
 
    // nao achou
    if (raiz->chave != chave)
        return raiz;
 
    No *aux = raiz;
    if (raiz->esq == NULL) {
        raiz = raiz->dir;
    } else {
        // splay na esq com a chave: o maior da esq sobe e fica sem filho dir
        raiz = splay(raiz->esq, chave);
        raiz->dir = aux->dir;
    }
    free(aux);
    return raiz;
}
 
// percursos pra conferir
void em_ordem(No *raiz) {
    if (raiz == NULL) return;
    em_ordem(raiz->esq);
    printf("%d ", raiz->chave);
    em_ordem(raiz->dir);
}
 
void preOrdem(No *raiz) {
    if (raiz == NULL) return;
    printf("%d ", raiz->chave);
    preOrdem(raiz->esq);
    preOrdem(raiz->dir);
}
 
// libera tudo no final
void liberar(No *raiz) {
    if (raiz == NULL) return;
    liberar(raiz->esq);
    liberar(raiz->dir);
    free(raiz);
}
 
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
 