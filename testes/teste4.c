// teste 4: uma arvore por vez, migrando quando o monitor manda trocar
// versoes: so_b, so_splay, duas (as duas mantidas, igual teste 1) e migra
// migra: comeca so com a B; ao trocar, passa todas as chaves pra outra arvore e apaga a antiga
// compilar (da raiz do repositorio):
//   gcc -std=c99 -O2 -Wall testes/teste4.c -o teste4 -lm
// rodar: ./teste4   (escreve resultados/teste4.csv)

#include "comum.c"

// limiares do monitor (mesmos do teste 1)
#define SOBE 0.75
#define DESCE 0.55

// monta uma splay equilibrada a partir do vetor ordenado vet[ini..fim]
// o meio vira a raiz, a metade de baixo vai pra esquerda e a de cima pra direita
// (inserir em ordem uma a uma deixaria a splay uma fila e estouraria a pilha)
No *construirSplay(int *vet, int ini, int fim) {
    if (ini > fim)
        return NULL;
    int meio = (ini + fim) / 2;
    No *raiz = novoNo(vet[meio]);
    raiz->esq = construirSplay(vet, ini, meio - 1);
    raiz->dir = construirSplay(vet, meio + 1, fim);
    return raiz;
}

// migra: so uma arvore existe por vez
// msMigracao recebe o tempo total gasto migrando (em milissegundos)
Resultado rodarMigra(int *ins, int n, int *carga, int m, double *msMigracao) {
    Resultado r;
    NoB *b = NULL;
    No *s = NULL;
    int achou;
    int *vet = novoVetor(n); // vetor usado nas migracoes

    // comeca so com a B
    double t0 = agora();
    for (int i = 0; i < n; i++)
        b = inserirB(b, ins[i]);
    r.nsInsercao = (agora() - t0) * 1e9 / n;

    Monitor *mon = novoMonitor(n, JANELA, SOBE, DESCE, MIN_OPS, AMOSTRA);
    visitadosB = 0;
    visitadosS = 0;
    double tMig = 0; // tempo gasto migrando (segundos)
    t0 = agora();
    for (int i = 0; i < m; i++) {
        if (registrarBusca(mon, carga[i])) {
            // o monitor mandou trocar: migra todas as chaves
            double tm = agora();
            int pos = 0;
            if (mon->ativa == SPLAY) {
                // B -> splay: chaves em ordem e monta a splay equilibrada
                emOrdemB(b, vet, &pos);
                s = construirSplay(vet, 0, n - 1);
                liberarB(b);
                b = NULL;
            } else {
                // splay -> B: chaves em ordem e insere uma a uma na B
                emOrdemS(s, vet, &pos);
                long long guardado = visitadosB; // a reconstrucao nao conta como busca
                for (int k = 0; k < n; k++)
                    b = inserirB(b, vet[k]);
                visitadosB = guardado;
                liberar(s);
                s = NULL;
            }
            if (pos != n)
                printf("ERRO: migracao passou %d chaves, esperado %d\n", pos, n);
            tMig += agora() - tm;
        }

        if (mon->ativa == ARVORE_B) {
            achados += buscarB(b, carga[i]);
        } else {
            s = buscar(s, carga[i], &achou);
            achados += achou;
        }
    }
    r.nsBusca = (agora() - t0) * 1e9 / m; // inclui o monitor e as migracoes

    r.nos = (double) (visitadosB + visitadosS) / m;
    r.trocas = mon->trocas;
    *msMigracao = tMig * 1000;
    liberarMonitor(mon);
    liberarB(b);
    liberar(s);
    free(vet);
    return r;
}

// as 8 cargas (as mesmas do teste 1; o nome vai pro csv)
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
void escreverLinha(FILE *csv, const char *versao, int c, int n, const char *limiar, int semente, Resultado r, double msMig) {
    fprintf(csv, "4,%s,%s,%d,%s,%d,%.2f,%.2f,%d,%.2f,%.2f\n",
            versao, nomeCarga[c], n, limiar, semente, r.nsBusca, r.nos, r.trocas, r.nsInsercao, msMig);
}

int main(void) {
    int tamanhos[3] = {10000, 100000, 1000000}; // valores de n
    int m = 1000000;                            // buscas por carga
    int sementes = 5;

    FILE *csv = fopen("resultados/teste4.csv", "w");
    if (csv == NULL) {
        printf("Erro: nao abriu resultados/teste4.csv (rode da raiz do repositorio)\n");
        return 1;
    }
    fprintf(csv, "teste,versao,carga,n,limiar,semente,ns_por_op,nos_visitados,trocas,ns_insercao,ms_migracao\n");

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
                // mesma semente -> mesma ordem de insercao e mesma carga pras 4 versoes
                srand(sem);
                ins = ordemInsercao(n);
                carga = gerarCarga(c, n, m);

                double msMig;
                Resultado rb = rodarSoB(ins, n, carga, m);
                Resultado rs = rodarSoSplay(ins, n, carga, m);
                Resultado rd = rodarDuas(ins, n, carga, m, SOBE, DESCE);
                Resultado rm = rodarMigra(ins, n, carga, m, &msMig);
                esperado += 4LL * m;

                escreverLinha(csv, "so_b", c, n, "-", sem, rb, 0);
                escreverLinha(csv, "so_splay", c, n, "-", sem, rs, 0);
                escreverLinha(csv, "duas", c, n, "0.75-0.55", sem, rd, 0);
                escreverLinha(csv, "migra", c, n, "0.75-0.55", sem, rm, msMig);

                printf("n=%-8d %-5s semente %d | ns/busca: B %6.1f  S %6.1f  duas %6.1f  migra %6.1f | trocas %d | migracao %.1f ms\n",
                       n, nomeCarga[c], sem, rb.nsBusca, rs.nsBusca, rd.nsBusca, rm.nsBusca, rm.trocas, msMig);

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
        printf("ok: todas as buscas acharam. resultado em resultados/teste4.csv\n");
    return 0;
}
