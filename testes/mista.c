// versao mista: a arvore B guarda todas as chaves e uma splay pequena so existe no pico
// o teste da #include neste arquivo depois do comum.c
//
// modo normal (monitor na B): busca direto na B, igual a so_b
// modo pico (monitor na splay): olha primeiro na splay pequena
//   achou     -> acerto
//   nao achou -> busca na B e coloca a chave na splay (falta)
// se a splay chegar em 'cap' chaves ela eh jogada fora e recomeca vazia
// quando o monitor volta pra B a splay tambem eh jogada fora

// o que a mista mede a mais que as outras versoes
typedef struct ResultadoMista {
    Resultado r;         // tempo, nos visitados, trocas (igual as outras versoes)
    long long acertos;   // buscas do pico resolvidas na splay
    long long faltas;    // buscas do pico que tiveram que ir na B
    long long descartes; // vezes que a splay encheu e foi jogada fora
} ResultadoMista;

// sobe e desce = limiares do monitor, cap = maximo de chaves na splay
ResultadoMista rodarMista(int *ins, int n, int *carga, int m, double sobe, double desce, int cap) {
    ResultadoMista rm;
    NoB *b = NULL;
    No *s = NULL;  // splay pequena (comeca vazia)
    int tamS = 0;  // chaves na splay
    int achou;

    rm.acertos = 0;
    rm.faltas = 0;
    rm.descartes = 0;

    // insere so na B (a splay se monta sozinha durante o pico)
    double t0 = agora();
    for (int i = 0; i < n; i++)
        b = inserirB(b, ins[i]);
    rm.r.nsInsercao = (agora() - t0) * 1e9 / n;

    Monitor *mon = novoMonitor(n, JANELA, sobe, desce, MIN_OPS, AMOSTRA);
    visitadosB = 0;
    visitadosS = 0;
    t0 = agora();
    for (int i = 0; i < m; i++) {
        int chave = carga[i];

        // acabou o pico: joga a splay fora
        if (registrarBusca(mon, chave) && mon->ativa == ARVORE_B) {
            liberar(s);
            s = NULL;
            tamS = 0;
        }

        // modo normal: igual a so_b
        if (mon->ativa == ARVORE_B) {
            achados += buscarB(b, chave);
            continue;
        }

        // modo pico: tenta a splay primeiro
        if (s != NULL) {
            s = buscar(s, chave, &achou);
            if (achou) {
                rm.acertos++;
                achados++;
                continue;
            }
        }

        // falta: busca na B e coloca a chave na splay
        rm.faltas++;
        if (!buscarB(b, chave))
            continue;
        achados++;

        // splay cheia: recomeca do zero
        if (tamS >= cap) {
            liberar(s);
            s = NULL;
            tamS = 0;
            rm.descartes++;
        }
        s = inserir(s, chave);
        tamS++;
    }
    rm.r.nsBusca = (agora() - t0) * 1e9 / m; // inclui o monitor e a montagem da splay

    rm.r.nos = (double) (visitadosB + visitadosS) / m;
    rm.r.trocas = mon->trocas;
    liberarMonitor(mon);
    liberarB(b);
    liberar(s);
    return rm;
}
