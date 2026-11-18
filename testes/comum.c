// partes comuns dos testes: relogio, resultado e as 3 versoes (so_b, so_splay, duas)
// cada teste da #include neste arquivo e tem o seu proprio main

#define _POSIX_C_SOURCE 200112L // pro clock_gettime e snprintf no linux com -std=c99

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../arvores/arvore_b.c"
#include "../arvores/splay.c"
#include "cargas.c"
#include "monitor.c"

// parametros fixos do monitor (os limiares sao passados pra rodarDuas)
#define JANELA 1000
#define MIN_OPS 5000
#define AMOSTRA 10 // monitor olha 1 a cada AMOSTRA buscas

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
// sobe e desce = limiares do monitor
Resultado rodarDuas(int *ins, int n, int *carga, int m, double sobe, double desce) {
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

    Monitor *mon = novoMonitor(n, JANELA, sobe, desce, MIN_OPS, AMOSTRA);
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
