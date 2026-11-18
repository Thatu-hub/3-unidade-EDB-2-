// monitor: olha as ultimas W buscas amostradas e decide qual arvore usar
// taxa de repeticao = 1 - (chaves distintas na janela / W)
// histerese: na B troca se taxa > sobe, na splay volta se taxa < desce
// e so pode trocar depois de minOps buscas desde a ultima troca
// amostragem: so olha 1 a cada 'amostra' buscas (as outras so contam), pra gastar menos

#include <stdio.h>
#include <stdlib.h>

#define ARVORE_B 0
#define SPLAY 1

typedef struct Monitor {
    int W;          // tamanho da janela
    int *janela;    // ultimas W chaves buscadas (vetor circular)
    int pos;        // onde entra a proxima chave
    int cheia;      // quantas posicoes da janela ja foram preenchidas
    int *cont;      // cont[k] = quantas vezes a chave k ta na janela
    int distintos;  // chaves diferentes na janela

    double sobe;    // limiar pra ir pra splay
    double desce;   // limiar pra voltar pra B
    int minOps;     // minimo de buscas entre trocas
    int desdeTroca; // buscas desde a ultima troca

    int amostra;    // olha 1 a cada 'amostra' buscas (1 = olha todas)
    int pulo;       // buscas desde a ultima que foi olhada

    int ativa;      // ARVORE_B ou SPLAY
    int trocas;     // quantas trocas aconteceram
} Monitor;

// cria o monitor (n = quantidade de chaves, pro vetor de contadores)
// comeca na arvore B
Monitor *novoMonitor(int n, int W, double sobe, double desce, int minOps, int amostra) {
    Monitor *mon = (Monitor *) malloc(sizeof(Monitor));
    if (mon == NULL) {
        printf("Erro: sem memoria\n");
        exit(1);
    }
    mon->janela = (int *) malloc(sizeof(int) * W);
    mon->cont = (int *) calloc(n, sizeof(int)); // calloc ja zera
    if (mon->janela == NULL || mon->cont == NULL) {
        printf("Erro: sem memoria\n");
        exit(1);
    }
    mon->W = W;
    mon->pos = 0;
    mon->cheia = 0;
    mon->distintos = 0;
    mon->sobe = sobe;
    mon->desce = desce;
    mon->minOps = minOps;
    mon->desdeTroca = 0;
    mon->amostra = amostra;
    mon->pulo = 0;
    mon->ativa = ARVORE_B;
    mon->trocas = 0;
    return mon;
}

// taxa de repeticao da janela atual
double taxaRepeticao(Monitor *mon) {
    return 1.0 - (double) mon->distintos / mon->W;
}

// registra uma busca e retorna 1 se trocou de arvore, 0 se nao
// chamar em toda busca: o proprio monitor decide se olha ou nao
int registrarBusca(Monitor *mon, int chave) {
    mon->desdeTroca++; // o minimo entre trocas conta buscas, nao amostras

    // so olha 1 a cada 'amostra' buscas
    mon->pulo++;
    if (mon->pulo < mon->amostra)
        return 0;
    mon->pulo = 0;

    // janela cheia: a chave mais antiga sai
    if (mon->cheia == mon->W) {
        int velha = mon->janela[mon->pos];
        mon->cont[velha]--;
        if (mon->cont[velha] == 0)
            mon->distintos--;
    } else {
        mon->cheia++;
    }

    // a chave nova entra no lugar dela
    mon->janela[mon->pos] = chave;
    if (mon->cont[chave] == 0)
        mon->distintos++;
    mon->cont[chave]++;
    mon->pos = (mon->pos + 1) % mon->W;

    // so decide com a janela cheia e depois do minimo de buscas
    if (mon->cheia < mon->W || mon->desdeTroca < mon->minOps)
        return 0;

    double taxa = taxaRepeticao(mon);
    if (mon->ativa == ARVORE_B && taxa > mon->sobe) {
        mon->ativa = SPLAY;
    } else if (mon->ativa == SPLAY && taxa < mon->desce) {
        mon->ativa = ARVORE_B;
    } else {
        return 0;
    }
    mon->trocas++;
    mon->desdeTroca = 0;
    return 1;
}

// libera tudo no final
void liberarMonitor(Monitor *mon) {
    free(mon->janela);
    free(mon->cont);
    free(mon);
}
