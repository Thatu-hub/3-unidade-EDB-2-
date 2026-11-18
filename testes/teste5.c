// teste 5: o hibrido herda as limitacoes da splay quando nao ha pico?
// cargas sem pico: U, Z0.8 e Z1.0 (concentracao fraca, pode enganar o monitor)
// F40-pos: so o trecho espalhado depois do pico da F40 (o hibrido pode ainda estar na splay)
// versoes: so_b, so_splay, duas, mista (cap 4096) e migra
// compilar (da raiz do repositorio):
//   gcc -std=c99 -O2 -Wall testes/teste5.c -o teste5 -lm
// rodar: ./teste5   (escreve resultados/teste5.csv)

#include "comum.c"
#include "mista.c"
#include "migra.c"

#define M 1000000 // buscas por carga
#define SEMENTES 5

// limiares do monitor (os mesmos do teste 1)
#define SOBE 0.75
#define DESCE 0.55

#define CAP_MISTA 4096 // capacidade da splay da mista (a melhor do teste 3)
#define PICO 0.4       // tamanho do pico da F40

#define NUM_VERSOES 5
const char *nomeVersao[NUM_VERSOES] = {"so_b", "so_splay", "duas", "mista", "migra"};
const char *limiarVersao[NUM_VERSOES] = {"-", "-", "0.75-0.55", "0.75-0.55", "0.75-0.55"};

// roda a versao v com as m primeiras buscas da carga
Resultado rodar(int v, int *ins, int n, int *carga, int m) {
    double msMig;
    switch (v) {
        case 0: return rodarSoB(ins, n, carga, m);
        case 1: return rodarSoSplay(ins, n, carga, m);
        case 2: return rodarDuas(ins, n, carga, m, SOBE, DESCE);
        case 3: return rodarMista(ins, n, carga, m, SOBE, DESCE, CAP_MISTA).r;
        default: return rodarMigra(ins, n, carga, m, SOBE, DESCE, &msMig);
    }
}

// escreve uma linha no csv
void escreverLinha(FILE *csv, int v, const char *carga, int n, int semente, Resultado r) {
    fprintf(csv, "5,%s,%s,%d,%s,%d,%.2f,%.2f,%d,%.2f\n",
            nomeVersao[v], carga, n, limiarVersao[v], semente, r.nsBusca, r.nos, r.trocas, r.nsInsercao);
}

int main(void) {
    int tamanhos[3] = {10000, 100000, 1000000}; // valores de n

    FILE *csv = fopen("resultados/teste5.csv", "w");
    if (csv == NULL) {
        printf("Erro: nao abriu resultados/teste5.csv (rode da raiz do repositorio)\n");
        return 1;
    }
    fprintf(csv, "teste,versao,carga,n,limiar,semente,ns_por_op,nos_visitados,trocas,ns_insercao\n");

    // aquecimento: uma rodada que eh jogada fora
    printf("aquecendo...\n");
    srand(999);
    int *ins = ordemInsercao(1000000);
    int *carga = cargaUniforme(1000000, M);
    rodarSoB(ins, 1000000, carga, M);
    rodarSoSplay(ins, 1000000, carga, M);
    free(ins);
    free(carga);
    achados = 0;

    // onde o pico da F40 termina (a mesma conta do cargaFases)
    int tamPico = (int) (M * PICO);
    int fimPico = (M - tamPico) / 2 + tamPico;
    int depois = M - fimPico; // buscas espalhadas depois do pico

    long long esperado = 0; // quantas buscas deveriam achar
    for (int t = 0; t < 3; t++) {
        int n = tamanhos[t];
        for (int sem = 1; sem <= SEMENTES; sem++) {
            // cargas sem pico
            const char *nomes[3] = {"U", "Z0.8", "Z1.0"};
            for (int c = 0; c < 3; c++) {
                // mesma semente -> mesma insercao e mesma carga pras 5 versoes
                srand(sem);
                ins = ordemInsercao(n);
                if (c == 0)
                    carga = cargaUniforme(n, M);
                else
                    carga = cargaZipf(n, M, c == 1 ? 0.8 : 1.0);

                printf("n=%-8d %-7s semente %d |", n, nomes[c], sem);
                for (int v = 0; v < NUM_VERSOES; v++) {
                    Resultado r = rodar(v, ins, n, carga, M);
                    esperado += M;
                    escreverLinha(csv, v, nomes[c], n, sem, r);
                    printf(" %s %.1f(%d)", nomeVersao[v], r.nsBusca, r.trocas);
                }
                printf("\n");
                free(ins);
                free(carga);
            }

            // F40: roda a carga inteira e depois so ate o fim do pico
            // a diferenca entre as duas rodadas eh o trecho espalhado depois do pico
            srand(sem);
            ins = ordemInsercao(n);
            carga = cargaFases(n, M, PICO, 1.5);

            printf("n=%-8d %-7s semente %d |", n, "F40-pos", sem);
            for (int v = 0; v < NUM_VERSOES; v++) {
                Resultado inteira = rodar(v, ins, n, carga, M);
                Resultado ate = rodar(v, ins, n, carga, fimPico);
                esperado += M + fimPico;

                Resultado pos;
                pos.nsBusca = (inteira.nsBusca * M - ate.nsBusca * fimPico) / depois;
                pos.nos = (inteira.nos * M - ate.nos * fimPico) / depois;
                pos.trocas = inteira.trocas - ate.trocas;
                pos.nsInsercao = inteira.nsInsercao;

                escreverLinha(csv, v, "F40", n, sem, inteira);
                escreverLinha(csv, v, "F40-pos", n, sem, pos);
                printf(" %s %.1f(%d)", nomeVersao[v], pos.nsBusca, pos.trocas);
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
        printf("ok: todas as buscas acharam. resultado em resultados/teste5.csv\n");
    return 0;
}
