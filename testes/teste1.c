// teste 1: as duas arvores mantidas ao mesmo tempo
// versoes: so_b, so_splay e duas (insere nas duas, busca na que o monitor escolher)
// compilar (da raiz do repositorio):
//   gcc -std=c99 -O2 -Wall testes/teste1.c -o teste1 -lm
// rodar: ./teste1   (escreve resultados/teste1.csv)

#define _POSIX_C_SOURCE 199309L // pro clock_gettime no linux com -std=c99

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../arvores/arvore_b.c"
#include "../arvores/splay.c"
#include "cargas.c"
#include "monitor.c"

// parametros do monitor (padrao do teste 1)
#define JANELA 1000
#define SOBE 0.75
#define DESCE 0.55
#define MIN_OPS 5000

// o que cada versao mede
typedef struct Resultado {
    double nsInsercao; // tempo medio pra inserir uma chave
    double nsBusca;    // tempo medio por busca
    double nos;        // nos visitados por busca
    int trocas;        // trocas de arvore (so na versao duas)
} Resultado;

long long achados = 0; // soma das buscas que acharam (confere se ta tudo certo)

// tempo atual em segundos
double agora(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

// so arvore B
Resultado rodarSoB(int *ins, int n, int *carga, int m) {
    Resultado r;
    NoB *b = NULL;

    double t0 = agora();
    for (int i = 0; i < n; i++)
        b = inserirB(b, ins[i]);
    r.nsInsercao = (agora() - t0) * 1e9 / n;

    visitadosB = 0;
    t0 = agora();
    for (int i = 0; i < m; i++)
        achados += buscarB(b, carga[i]);
    r.nsBusca = (agora() - t0) * 1e9 / m;

    r.nos = (double) visitadosB / m;
    r.trocas = 0;
    liberarB(b);
    return r;
}

// so splay
Resultado rodarSoSplay(int *ins, int n, int *carga, int m) {
    Resultado r;
    No *s = NULL;
    int achou;

    double t0 = agora();
    for (int i = 0; i < n; i++)
        s = inserir(s, ins[i]);
    r.nsInsercao = (agora() - t0) * 1e9 / n;

    visitadosS = 0;
    t0 = agora();
    for (int i = 0; i < m; i++) {
        s = buscar(s, carga[i], &achou);
        achados += achou;
    }
    r.nsBusca = (agora() - t0) * 1e9 / m;

    r.nos = (double) visitadosS / m;
    r.trocas = 0;
    liberar(s);
    return r;
}

// as duas: insere nas duas, cada busca vai pra arvore que o monitor escolher
Resultado rodarDuas(int *ins, int n, int *carga, int m) {
    Resultado r;
    NoB *b = NULL;
    No *s = NULL;
    int achou;

    // insere toda a B e depois toda a splay (nao intercalar)
    // intercalando, os nos das duas ficam misturados na memoria e o cache piora
    double t0 = agora();
    for (int i = 0; i < n; i++)
        b = inserirB(b, ins[i]);
    for (int i = 0; i < n; i++)
        s = inserir(s, ins[i]);
    r.nsInsercao = (agora() - t0) * 1e9 / n;

    Monitor *mon = novoMonitor(n, JANELA, SOBE, DESCE, MIN_OPS);
    visitadosB = 0;
    visitadosS = 0;
    t0 = agora();
    for (int i = 0; i < m; i++) {
        registrarBusca(mon, carga[i]); // atualiza a janela e talvez troque
        if (mon->ativa == ARVORE_B) {
            achados += buscarB(b, carga[i]);
        } else {
            s = buscar(s, carga[i], &achou);
            achados += achou;
        }
    }
    r.nsBusca = (agora() - t0) * 1e9 / m; // inclui o custo do monitor

    r.nos = (double) (visitadosB + visitadosS) / m;
    r.trocas = mon->trocas;
    liberarMonitor(mon);
    liberarB(b);
    liberar(s);
    return r;
}

// as 8 cargas do teste (o nome vai pro csv)
#define NUM_CARGAS 8
const char *nomeCarga[NUM_CARGAS] = {"U", "Z0.8", "Z1.2", "Z1.5", "F10", "F20", "F40", "F2"};

// gera a carga numero c (picos de F e F2 com s = 1.5)
int *gerarCarga(int c, int n, int m) {
    switch (c) {
        case 0: return cargaUniforme(n, m);
        case 1: return cargaZipf(n, m, 0.8);
        case 2: return cargaZipf(n, m, 1.2);
        case 3: return cargaZipf(n, m, 1.5);
        case 4: return cargaFases(n, m, 0.1, 1.5);
        case 5: return cargaFases(n, m, 0.2, 1.5);
        case 6: return cargaFases(n, m, 0.4, 1.5);
        default: return cargaDoisEventos(n, m, 1.5);
    }
}

// escreve uma linha no csv
void escreverLinha(FILE *csv, const char *versao, int c, int n, const char *limiar, int semente, Resultado r) {
    fprintf(csv, "1,%s,%s,%d,%s,%d,%.2f,%.2f,%d,%.2f\n",
            versao, nomeCarga[c], n, limiar, semente, r.nsBusca, r.nos, r.trocas, r.nsInsercao);
}

int main(void) {
    int tamanhos[3] = {10000, 100000, 1000000}; // valores de n
    int m = 1000000;                            // buscas por carga
    int sementes = 5;

    FILE *csv = fopen("resultados/teste1.csv", "w");
    if (csv == NULL) {
        printf("Erro: nao abriu resultados/teste1.csv (rode da raiz do repositorio)\n");
        return 1;
    }
    fprintf(csv, "teste,versao,carga,n,limiar,semente,ns_por_op,nos_visitados,trocas,ns_insercao\n");

    // aquecimento: uma rodada que eh jogada fora
    printf("aquecendo...\n");
    srand(999);
    int *ins = ordemInsercao(1000000);
    int *carga = cargaUniforme(1000000, m);
    rodarSoB(ins, 1000000, carga, m);
    rodarSoSplay(ins, 1000000, carga, m);
    free(ins);
    free(carga);
    achados = 0;

    long long esperado = 0; // quantas buscas deveriam achar
    for (int t = 0; t < 3; t++) {
        int n = tamanhos[t];
        for (int c = 0; c < NUM_CARGAS; c++) {
            for (int sem = 1; sem <= sementes; sem++) {
                // mesma semente -> mesma ordem de insercao e mesma carga pras 3 versoes
                srand(sem);
                ins = ordemInsercao(n);
                carga = gerarCarga(c, n, m);

                Resultado rb = rodarSoB(ins, n, carga, m);
                Resultado rs = rodarSoSplay(ins, n, carga, m);
                Resultado rd = rodarDuas(ins, n, carga, m);
                esperado += 3LL * m;

                escreverLinha(csv, "so_b", c, n, "-", sem, rb);
                escreverLinha(csv, "so_splay", c, n, "-", sem, rs);
                escreverLinha(csv, "duas", c, n, "0.75-0.55", sem, rd);

                printf("n=%-8d %-5s semente %d | ns/busca: B %6.1f  S %6.1f  duas %6.1f | trocas %d\n",
                       n, nomeCarga[c], sem, rb.nsBusca, rs.nsBusca, rd.nsBusca, rd.trocas);

                free(ins);
                free(carga);
            }
        }
    }
    fclose(csv);

    // todas as chaves da carga existem, entao todas as buscas tem que achar
    if (achados != esperado)
        printf("ERRO: %lld buscas acharam, esperado %lld\n", achados, esperado);
    else
        printf("ok: todas as buscas acharam. resultado em resultados/teste1.csv\n");
    return 0;
}
