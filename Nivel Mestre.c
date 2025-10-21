#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NOME 80
#define MAX_PISTA 200
#define HASH_SIZE 101   // tamanho da tabela hash (primo pequeno)

// -----------------------------
// Estruturas
// -----------------------------

// Struct: Sala
// Representa um cômodo da mansão (nó da árvore binária).
typedef struct Sala {
    char nome[MAX_NOME];
    char *pista;             // string alocada dinamicamente (NULL se não houver)
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// Nó da BST que armazena pistas coletadas (sem duplicatas)
typedef struct PistaNode {
    char *pista;
    struct PistaNode *esq;
    struct PistaNode *dir;
} PistaNode;

// Entradas da tabela hash (lista encadeada em cada bucket)
typedef struct HashEntry {
    char *chave;            // pista (key)
    char *valor;            // suspeito (value)
    struct HashEntry *prox;
} HashEntry;

// Tabela hash
typedef struct HashTable {
    HashEntry *buckets[HASH_SIZE];
} HashTable;

// -----------------------------
// Funções utilitárias de string
// -----------------------------

// copiaString: aloca e retorna uma cópia da string (similar a strdup)
char *copiaString(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *c = (char*) malloc(n);
    if (!c) {
        fprintf(stderr, "Erro: sem memória para copiar string.\n");
        exit(EXIT_FAILURE);
    }
    memcpy(c, s, n);
    return c;
}

// limpaEspaco: remove '\n' final de fgets (se houver)
void limpaNovaLinha(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    if (len == 0) return;
    if (s[len - 1] == '\n') s[len - 1] = '\0';
}

// funçao para leitura segura de linha (stdin)
void leLinha(char *buffer, size_t tamanho) {
    if (fgets(buffer, (int)tamanho, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }
    limpaNovaLinha(buffer);
}

// -----------------------------
// criarSala()
// Cria dinamicamente um cômodo (Sala) com nome e pista opcional.
// -----------------------------
Sala* criarSala(const char *nome, const char *pista) {
    Sala *s = (Sala*) malloc(sizeof(Sala));
    if (!s) {
        fprintf(stderr, "Erro ao alocar memória para Sala.\n");
        exit(EXIT_FAILURE);
    }
    strncpy(s->nome, nome, MAX_NOME - 1);
    s->nome[MAX_NOME - 1] = '\0';
    if (pista != NULL && strlen(pista) > 0) {
        s->pista = copiaString(pista);
    } else {
        s->pista = NULL;
    }
    s->esquerda = s->direita = NULL;
    return s;
}

// -----------------------------
// BST: inserirPista()
// Insere uma pista na BST de pistas coletadas em ordem alfabética.
// Evita duplicatas (se já existe, não insere).
// Retorna a raiz (possivelmente nova).
// -----------------------------
PistaNode* inserirPista(PistaNode *raiz, const char *pista) {
    if (pista == NULL) return raiz;
    if (raiz == NULL) {
        PistaNode *n = (PistaNode*) malloc(sizeof(PistaNode));
        if (!n) {
            fprintf(stderr, "Erro ao alocar nó de pista.\n");
            exit(EXIT_FAILURE);
        }
        n->pista = copiaString(pista);
        n->esq = n->dir = NULL;
        return n;
    }
    int cmp = strcmp(pista, raiz->pista);
    if (cmp < 0) raiz->esq = inserirPista(raiz->esq, pista);
    else if (cmp > 0) raiz->dir = inserirPista(raiz->dir, pista);
    // else: igual => duplicata; não inserir
    return raiz;
}

// -----------------------------
// exibirPistas()
// Imprime as pistas coletadas (in-order traversal => ordem alfabética)
// -----------------------------
void exibirPistas(PistaNode *raiz) {
    if (raiz == NULL) return;
    exibirPistas(raiz->esq);
    printf(" - %s\n", raiz->pista);
    exibirPistas(raiz->dir);
}

// -----------------------------
// liberarPistas()
// Libera memória da BST de pistas
// -----------------------------
void liberarPistas(PistaNode *raiz) {
    if (!raiz) return;
    liberarPistas(raiz->esq);
    liberarPistas(raiz->dir);
    free(raiz->pista);
    free(raiz);
}

// -----------------------------
// Hash: funções básicas
// -----------------------------

// função hash simples (djb2)
static unsigned long hashString(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++) != 0) hash = ((hash << 5) + hash) + c;
    return hash % HASH_SIZE;
}

// inicializa tabela hash (zera buckets)
void inicializaHash(HashTable *ht) {
    for (int i = 0; i < HASH_SIZE; ++i) ht->buckets[i] = NULL;
}

// inserirNaHash()
// Insere associação pista -> suspeito na tabela hash.
// Se já existir a chave, sobrescreve o valor.
// -----------------------------
void inserirNaHash(HashTable *ht, const char *pista, const char *suspeito) {
    if (!pista || !suspeito) return;
    unsigned long idx = hashString(pista);
    HashEntry *e = ht->buckets[idx];
    while (e) {
        if (strcmp(e->chave, pista) == 0) {
            // já existe: atualizar valor
            free(e->valor);
            e->valor = copiaString(suspeito);
            return;
        }
        e = e->prox;
    }
    // não achou: cria novo entry e insere no início
    HashEntry *novo = (HashEntry*) malloc(sizeof(HashEntry));
    if (!novo) {
        fprintf(stderr, "Erro ao alocar hash entry.\n");
        exit(EXIT_FAILURE);
    }
    novo->chave = copiaString(pista);
    novo->valor = copiaString(suspeito);
    novo->prox = ht->buckets[idx];
    ht->buckets[idx] = novo;
}

// encontrarSuspeito()
// Consulta a tabela hash retornando o suspeito associado a uma pista.
// Retorna NULL se não houver associação.
// Nota: retorna ponteiro interno; NÃO deve ser liberado pelo chamador.
// -----------------------------
const char* encontrarSuspeito(HashTable *ht, const char *pista) {
    if (!pista) return NULL;
    unsigned long idx = hashString(pista);
    HashEntry *e = ht->buckets[idx];
    while (e) {
        if (strcmp(e->chave, pista) == 0) return e->valor;
        e = e->prox;
    }
    return NULL;
}

// liberarHash()
// Libera todas as entradas da tabela hash
// -----------------------------
void liberarHash(HashTable *ht) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        HashEntry *e = ht->buckets[i];
        while (e) {
            HashEntry *tmp = e;
            e = e->prox;
            free(tmp->chave);
            free(tmp->valor);
            free(tmp);
        }
        ht->buckets[i] = NULL;
    }
}

// -----------------------------
// explorarSalas()
// Navega interativamente pela árvore de salas, coleta pistas automaticamente
// e as adiciona na BST de pistas coletadas.
// -----------------------------
void explorarSalas(Sala *atual, PistaNode **arvorePistas) {
    if (!atual) return;
    char opc;
    Sala *pos = atual;

    while (pos != NULL) {
        printf("\nVocê está na sala: %s\n", pos->nome);

        // Se existir pista, coleta automaticamente (insere na BST e marca coletada)
        if (pos->pista != NULL) {
            printf("Pista encontrada: \"%s\"\n", pos->pista);
            *arvorePistas = inserirPista(*arvorePistas, pos->pista);
            // Marca como coletada liberando a string
            free(pos->pista);
            pos->pista = NULL;
        } else {
            printf("Nenhuma pista nova nesta sala.\n");
        }

        // Mostra opções
        printf("\nOpções:\n");
        if (pos->esquerda) printf(" (e) Ir para %s (esquerda)\n", pos->esquerda->nome);
        if (pos->direita)  printf(" (d) Ir para %s (direita)\n", pos->direita->nome);
        printf(" (s) Sair e ir ao julgamento\n");
        printf("Escolha: ");

        if (scanf(" %c", &opc) != 1) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
            printf("Entrada inválida. Tente novamente.\n");
            continue;
        }

        if (opc == 'e' || opc == 'E') {
            if (pos->esquerda) pos = pos->esquerda;
            else printf("Não há caminho à esquerda.\n");
        } else if (opc == 'd' || opc == 'D') {
            if (pos->direita) pos = pos->direita;
            else printf("Não há caminho à direita.\n");
        } else if (opc == 's' || opc == 'S') {
            printf("Exploração encerrada pelo jogador.\n");
            break;
        } else {
            printf("Opção inválida. Use 'e', 'd' ou 's'.\n");
        }
    }
}

// -----------------------------
// verificarSuspeitoFinal()
// Avalia se existem pelo menos 'limite' pistas coletadas que apontam para o suspeito indicado.
// Percorre a BST de pistas coletadas e conta quantas correspondem ao suspeito consultando a hash.
// Retorna o número de pistas que apontam para o suspeito.
// -----------------------------
typedef struct ContadorCtx {
    HashTable *ht;
    const char *suspeito;
    int contador;
} ContadorCtx;

void contarCallback(PistaNode *n, ContadorCtx *ctx) {
    if (!n || !ctx) return;
    const char *s = encontrarSuspeito(ctx->ht, n->pista);
    if (s && strcmp(s, ctx->suspeito) == 0) ctx->contador += 1;
}

// percorre BST in-order e aplica callback (recursivo)
void percorreBST_e_conta(PistaNode *raiz, ContadorCtx *ctx) {
    if (!raiz) return;
    percorreBST_e_conta(raiz->esq, ctx);
    contarCallback(raiz, ctx);
    percorreBST_e_conta(raiz->dir, ctx);
}

// -----------------------------
// liberarSalas()
// Libera memória da árvore de salas (inclui liberação de pistas que ainda existirem)
// -----------------------------
void liberarSalas(Sala *raiz) {
    if (!raiz) return;
    liberarSalas(raiz->esquerda);
    liberarSalas(raiz->direita);
    if (raiz->pista) free(raiz->pista);
    free(raiz);
}

// -----------------------------
// Função principal
// Monta o mapa, preenche a hash com associações (pista -> suspeito), realiza exploração,
// exibe pistas coletadas, pede acusação e verifica evidências.
// -----------------------------
int main(void) {
    // ---------- Montagem do mapa (árvore fixa) ----------
    Sala *hall = criarSala("Hall de Entrada", "Bilhete rasgado com hora marcada");
    Sala *salaEstar = criarSala("Sala de Estar", "Pegadas molhadas perto da lareira");
    Sala *cozinha = criarSala("Cozinha", "Faca com monograma X");
    Sala *biblioteca = criarSala("Biblioteca", "Livro apontando para passagem secreta");
    Sala *jardim = criarSala("Jardim", "Foto antiga da família com uma assinatura");
    Sala *pirao = criarSala("Porão", "Raspas de tinta da mesma cor da mansão");
    Sala *escritorio = criarSala("Escritório", "Carta com assinatura parcial");
    Sala *sotao = criarSala("Sótão", "Chave enferrujada com iniciais 'M.'");

    // conexões
    hall->esquerda = salaEstar;
    hall->direita  = cozinha;

    salaEstar->esquerda = biblioteca;
    salaEstar->direita  = jardim;

    cozinha->esquerda = pirao;
    cozinha->direita  = escritorio;

    biblioteca->esquerda = sotao;

    // ---------- Cria e popula a tabela hash (pista -> suspeito) ----------
    HashTable ht;
    inicializaHash(&ht);

    // Associações pré-definidas (definidas pela lógica do jogo)
    inserirNaHash(&ht, "Bilhete rasgado com hora marcada", "Suspeito A");
    inserirNaHash(&ht, "Pegadas molhadas perto da lareira", "Suspeito B");
    inserirNaHash(&ht, "Faca com monograma X", "Suspeito C");
    inserirNaHash(&ht, "Livro apontando para passagem secreta", "Suspeito A");
    inserirNaHash(&ht, "Foto antiga da família com uma assinatura", "Suspeito B");
    inserirNaHash(&ht, "Raspas de tinta da mesma cor da mansão", "Suspeito C");
    inserirNaHash(&ht, "Carta com assinatura parcial", "Suspeito A");
    inserirNaHash(&ht, "Chave enferrujada com iniciais 'M.'", "Suspeito D");

    // ---------- BST de pistas coletadas (inicialmente vazia) ----------
    PistaNode *arvorePistas = NULL;

    // ---------- Início do jogo ----------
    printf("=========================================\n");
    printf(" 🕵️  DETECTIVE QUEST - MODO MESTRE\n");
    printf("=========================================\n");
    printf("Explore a mansão e colete pistas. Ao final, acuse o suspeito.\n");
    printf("Navegue com: 'e' (esquerda), 'd' (direita) ou 's' (sair).\n");

    explorarSalas(hall, &arvorePistas);

    // Exibe pistas coletadas
    printf("\n\n===== PISTAS COLETADAS =====\n");
    if (arvorePistas == NULL) {
        printf("Nenhuma pista coletada.\n");
    } else {
        exibirPistas(arvorePistas);
    }

    // Solicita acusação do jogador
    char entrada[128];
    printf("\nDigite o nome do suspeito que você deseja acusar (ex.: Suspeito A):\n> ");
    // limpar buffer remanescente
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    leLinha(entrada, sizeof(entrada));
    if (strlen(entrada) == 0) {
        printf("Nenhum suspeito informado. Encerrando.\n");
    } else {
        // Conta quantas pistas coletadas apontam para o suspeito indicado
        ContadorCtx ctx;
        ctx.ht = &ht;
        ctx.suspeito = copiaString(entrada); // cópia para segurança
        ctx.contador = 0;

        percorreBST_e_conta(arvorePistas, &ctx);

        printf("\nVocê acusou: %s\n", ctx.suspeito);
        printf("Evidências encontradas que apontam para %s: %d\n", ctx.suspeito, ctx.contador);

        if (ctx.contador >= 2) {
            printf("\nResultado: ACUSAÇÃO SUSTENTADA. Parece que você tem evidências suficientes!\n");
        } else {
            printf("\nResultado: ACUSAÇÃO FRACA. Poucas evidências. Falta prova contundente.\n");
        }
        free((char*)ctx.suspeito);
    }

    // ---------- Limpeza de memória ----------
    liberarPistas(arvorePistas);
    liberarHash(&ht);
    liberarSalas(hall);

    printf("\nObrigado por jogar Detective Quest - Modo Mestre!\n");
    return 0;
}
