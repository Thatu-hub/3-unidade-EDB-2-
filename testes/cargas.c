// geracao das cargas dos testes
// carga = lista de chaves a buscar, em ordem
// chave i = i-esimo titulo em ordem alfabetica
// todas as funcoes usam rand(), entao chamar srand(semente) antes
// os vetores sao criados com malloc, quem chama da free

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// cria um vetor com m posicoes
int *novoVetor(int m) {
    int *v = (int *) malloc(sizeof(int) * m);
    if (v == NULL) {
        printf("Erro: sem memoria\n");
        exit(1);
    }
    return v;
}

// embaralha o vetor (fisher-yates): troca cada posicao com uma sorteada antes dela
void embaralhar(int *v, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int aux = v[i];
        v[i] = v[j];
        v[j] = aux;
    }
}

// chaves 0..n-1 embaralhadas (titulos chegando fora de ordem)
// tambem evita a splay virar fila e estourar a pilha
int *ordemInsercao(int n) {
    int *v = novoVetor(n);
    for (int i = 0; i < n; i++)
        v[i] = i;
    embaralhar(v, n);
    return v;
}

// U: preenche v[ini..fim-1] com chaves sorteadas, todas com a mesma chance
void trechoUniforme(int *v, int ini, int fim, int n) {
    for (int i = ini; i < fim; i++)
        v[i] = rand() % n;
}

// carga U (dia normal)
int *cargaUniforme(int n, int m) {
    int *v = novoVetor(m);
    trechoUniforme(v, 0, m, n);
    return v;
}

// zipf: preenche v[ini..fim-1], a posicao i do ranking tem peso 1/(i+1)^s
// s maior = acessos mais concentrados nas primeiras posicoes
// cada chamada sorteia novos populares (na F2 o pico A e o B sao diferentes)
void trechoZipf(int *v, int ini, int fim, int n, double s) {
    // acum[i] = soma dos pesos das posicoes 0..i
    double *acum = (double *) malloc(sizeof(double) * n);
    if (acum == NULL) {
        printf("Erro: sem memoria\n");
        exit(1);
    }
    double total = 0;
    for (int i = 0; i < n; i++) {
        total += 1.0 / pow(i + 1, s);
        acum[i] = total;
    }

    // posicao do ranking -> chave (populares espalhados pelo alfabeto)
    int *perm = ordemInsercao(n);

    for (int k = ini; k < fim; k++) {
        // sorteia um numero entre 0 e total
        double u = rand() / (RAND_MAX + 1.0) * total;

        // busca binaria: primeira posicao com acum > u
        int a = 0, b = n - 1;
        while (a < b) {
            int meio = (a + b) / 2;
            if (acum[meio] > u)
                b = meio;
            else
                a = meio + 1;
        }
        v[k] = perm[a];
    }

    free(acum);
    free(perm);
}

// carga Z (s = 0.8 fraca, 1.2 empate, 1.5 forte)
int *cargaZipf(int n, int m, double s) {
    int *v = novoVetor(m);
    trechoZipf(v, 0, m, n, s);
    return v;
}

// carga F (dia do evento): U -> zipf -> U
// pico = fracao das buscas que eh pico (0.1, 0.2 ou 0.4), fica no meio
// s = concentracao do pico (padrao 1.5)
int *cargaFases(int n, int m, double pico, double s) {
    int *v = novoVetor(m);
    int tam = (int) (m * pico); // buscas do pico
    int a = (m - tam) / 2;      // inicio do pico
    int b = a + tam;            // fim do pico

    trechoUniforme(v, 0, a, n);
    trechoZipf(v, a, b, n, s);
    trechoUniforme(v, b, m, n);
    return v;
}

// carga F2 (dois eventos): U 30% -> pico A 20% -> U 30% -> pico B 20%
// cada pico sorteia seus populares, entao os artigos em alta mudam
// s = concentracao dos picos (padrao 1.5)
int *cargaDoisEventos(int n, int m, double s) {
    int *v = novoVetor(m);
    int a = (int) (m * 0.3); // inicio do pico A
    int b = (int) (m * 0.5); // fim do pico A
    int c = (int) (m * 0.8); // inicio do pico B

    trechoUniforme(v, 0, a, n);
    trechoZipf(v, a, b, n, s);
    trechoUniforme(v, b, c, n);
    trechoZipf(v, c, m, n, s);
    return v;
}
