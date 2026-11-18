// teste 2: a partir de qual concentracao de acessos vale trocar pra splay?
// cargas F 20% e F 40% (U -> pico -> U) com o s do pico variando
// pra cada s roda so_b, so_splay e a duas com varios limiares de subida
// a arvore B eh a principal: o sistema comeca nela
// compilar (da raiz do repositorio):
//   gcc -std=c99 -O2 -Wall testes/teste2.c -o teste2 -lm
// rodar: ./teste2   (escreve resultados/teste2.csv)

#include "comum.c"

#define N 1000000   // chaves na arvore
#define M 1000000   // buscas por carga
#define SEMENTES 5
#define FOLGA 0.20  // descida = subida - folga (histerese)

// tamanhos do pico (fracao das buscas)
#define NUM_PICOS 2
double picos[NUM_PICOS] = {0.2, 0.4};

// concentracoes do pico
#define NUM_S 8
double valoresS[NUM_S] = {1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.7, 2.0};

// limiares de subida (b -> splay)
#define NUM_LIMIARES 6
double limiares[NUM_LIMIARES] = {0.60, 0.65, 0.70, 0.75, 0.80, 0.85};

// escreve uma linha no csv
void escreverLinha(FILE *csv, const char *versao, double pico, double s, const char *limiar, int semente, Resultado r) {
    fprintf(csv, "2,%s,F%.0f-s%.1f,%d,%s,%d,%.2f,%.2f,%d,%.2f\n",
            versao, pico * 100, s, N, limiar, semente, r.nsBusca, r.nos, r.trocas, r.nsInsercao);
}

int main(void) {
    FILE *csv = fopen("resultados/teste2.csv", "w");
    if (csv == NULL) {
        printf("Erro: nao abriu resultados/teste2.csv (rode da raiz do repositorio)\n");
        return 1;
    }
    fprintf(csv, "teste,versao,carga,n,limiar,semente,ns_por_op,nos_visitados,trocas,ns_insercao\n");

    // aquecimento: uma rodada que eh jogada fora
    printf("aquecendo...\n");
    srand(999);
    int *ins = ordemInsercao(N);
    int *carga = cargaUniforme(N, M);
    rodarSoB(ins, N, carga, M);
    rodarSoSplay(ins, N, carga, M);
    free(ins);
    free(carga);
    achados = 0;

    long long esperado = 0; // quantas buscas deveriam achar
    for (int sem = 1; sem <= SEMENTES; sem++) {
        for (int p = 0; p < NUM_PICOS; p++) {
            for (int k = 0; k < NUM_S; k++) {
                double pico = picos[p];
                double s = valoresS[k];

                // mesma semente -> mesma insercao e mesma carga pra todas as versoes
                srand(sem);
                ins = ordemInsercao(N);
                carga = cargaFases(N, M, pico, s);

                // referencias
                Resultado rb = rodarSoB(ins, N, carga, M);
                Resultado rs = rodarSoSplay(ins, N, carga, M);
                escreverLinha(csv, "so_b", pico, s, "-", sem, rb);
                escreverLinha(csv, "so_splay", pico, s, "-", sem, rs);
                esperado += 2LL * M;
                printf("semente %d F%.0f s=%.1f | B %6.1f  S %6.1f | duas:", sem, pico * 100, s, rb.nsBusca, rs.nsBusca);

                // a duas com cada limiar
                for (int l = 0; l < NUM_LIMIARES; l++) {
                    double sobe = limiares[l];
                    double desce = sobe - FOLGA;
                    char limiar[16];
                    snprintf(limiar, sizeof(limiar), "%.2f-%.2f", sobe, desce);

                    Resultado rd = rodarDuas(ins, N, carga, M, sobe, desce);
                    escreverLinha(csv, "duas", pico, s, limiar, sem, rd);
                    esperado += M;
                    printf(" %.2f->%.1f(%d)", sobe, rd.nsBusca, rd.trocas);
                }
                printf("\n");

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
        printf("ok: todas as buscas acharam. resultado em resultados/teste2.csv\n");
    return 0;
}
