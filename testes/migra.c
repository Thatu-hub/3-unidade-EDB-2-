// versao migra (a mesma do teste 4): uma arvore por vez
// comeca so com a B; quando o monitor manda trocar, passa todas as chaves pra outra arvore e apaga a antiga
// o teste da #include neste arquivo depois do comum.c

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

// sobe e desce = limiares do monitor
// msMigracao recebe o tempo total gasto migrando (em milissegundos)
Resultado rodarMigra(int *ins, int n, int *carga, int m, double sobe, double desce, double *msMigracao) {
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

    Monitor *mon = novoMonitor(n, JANELA, sobe, desce, MIN_OPS, AMOSTRA);
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
