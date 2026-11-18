// teste 3: arvore mista (B com todas as chaves + splay pequena so no pico)
// pergunta: manter so as chaves quentes numa splay sobre a B ganha onde a duas perdeu?
// versoes: so_b, so_splay, duas e mista com 3 capacidades de splay
// compilar (da raiz do repositorio):
//   gcc -std=c99 -O2 -Wall testes/teste3.c -o teste3 -lm
// rodar: ./teste3   (escreve resultados/teste3.csv)

#include "comum.c"
#include "mista.c"

#define N 1000000   // chaves na arvore
#define M 1000000   // buscas por carga
#define SEMENTES 5

// limiares do monitor (os mesmos do teste 1)
#define SOBE 0.75
#define DESCE 0.55

// capacidades da splay pequena
#define NUM_CAPS 3
int caps[NUM_CAPS] = {256, 1024, 4096};

// cargas: U pra ver se a mista herda algo sem pico, Z1.5 pico continuo, F e F2 picos temporarios
#define NUM_CARGAS 6
const char *nomeCarga[NUM_CARGAS] = {"U", "Z1.5", "F10", "F20", "F40", "F2"};

// gera a carga numero c (picos de F e F2 com s = 1.5)
int *gerarCarga(int c, int n, int m) {
    switch (c) {
        case 0: return cargaUniforme(n, m);
        case 1: return cargaZipf(n, m, 1.5);
        case 2: return cargaFases(n, m, 0.1, 1.5);
        case 3: return cargaFases(n, m, 0.2, 1.5);
        case 4: return cargaFases(n, m, 0.4, 1.5);
        default: return cargaDoisEventos(n, m, 1.5);
    }
}

// escreve uma linha no csv (as colunas do teste 1 + cap, acertos, faltas, descartes)
void escreverLinha(FILE *csv, const char *versao, int c, const char *limiar, int semente,
                   Resultado r, int cap, long long acertos, long long faltas, long long descartes) {
    fprintf(csv, "3,%s,%s,%d,%s,%d,%.2f,%.2f,%d,%.2f,%d,%lld,%lld,%lld\n",
            versao, nomeCarga[c], N, limiar, semente, r.nsBusca, r.nos, r.trocas, r.nsInsercao,
            cap, acertos, faltas, descartes);
}

int main(void) {
    FILE *csv = fopen("resultados/teste3.csv", "w");
    if (csv == NULL) {
        printf("Erro: nao abriu resultados/teste3.csv (rode da raiz do repositorio)\n");
        return 1;
    }
    fprintf(csv, "teste,versao,carga,n,limiar,semente,ns_por_op,nos_visitados,trocas,ns_insercao,"
                 "cap,acertos,faltas,descartes\n");

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
    for (int c = 0; c < NUM_CARGAS; c++) {
        for (int sem = 1; sem <= SEMENTES; sem++) {
            // mesma semente -> mesma insercao e mesma carga pra todas as versoes
            srand(sem);
            ins = ordemInsercao(N);
            carga = gerarCarga(c, N, M);

            // referencias (iguais ao teste 1)
            Resultado rb = rodarSoB(ins, N, carga, M);
            Resultado rs = rodarSoSplay(ins, N, carga, M);
            Resultado rd = rodarDuas(ins, N, carga, M, SOBE, DESCE);
            escreverLinha(csv, "so_b", c, "-", sem, rb, 0, 0, 0, 0);
            escreverLinha(csv, "so_splay", c, "-", sem, rs, 0, 0, 0, 0);
            escreverLinha(csv, "duas", c, "0.75-0.55", sem, rd, 0, 0, 0, 0);
            esperado += 3LL * M;

            printf("%-5s semente %d | B %6.1f  S %6.1f  duas %6.1f | mista:",
                   nomeCarga[c], sem, rb.nsBusca, rs.nsBusca, rd.nsBusca);

            // a mista com cada capacidade
            for (int k = 0; k < NUM_CAPS; k++) {
                ResultadoMista rm = rodarMista(ins, N, carga, M, SOBE, DESCE, caps[k]);
                escreverLinha(csv, "mista", c, "0.75-0.55", sem, rm.r,
                              caps[k], rm.acertos, rm.faltas, rm.descartes);
                esperado += M;

                // taxa de acerto da splay no pico (0 se nao entrou no pico)
                long long noPico = rm.acertos + rm.faltas;
                double taxa = noPico > 0 ? 100.0 * rm.acertos / noPico : 0;
                printf(" %d->%.1f(%.0f%%)", caps[k], rm.r.nsBusca, taxa);
            }
            printf("\n");

            free(ins);
            free(carga);
        }
    }
    fclose(csv);

    // todas as chaves da carga existem, entao todas as buscas tem que achar
    if (achados != esperado)
        printf("ERRO: %lld buscas acharam, esperado %lld\n", achados, esperado);
    else
        printf("ok: todas as buscas acharam. resultado em resultados/teste3.csv\n");
    return 0;
}
