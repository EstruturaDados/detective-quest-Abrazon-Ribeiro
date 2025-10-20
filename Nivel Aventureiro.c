#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NOME 60
#define MAX_PISTA 120

// ----------------------
// Estruturas de dados
// ----------------------

// Nó da árvore que representa uma sala da mansão
typedef struct Sala {
    char nome[MAX_NOME];
    char *pista;             // NULL se não houver pista ou se já coletada
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// Nó da BST que armazena pistas coletadas (ordenadas alfabeticamente)
typedef struct PistaNode {
    char *pista;
    struct PistaNode *esq;
    struct PistaNode *dir;
} PistaNode;

// ----------------------
// Funções auxiliares
// ----------------------

// Aloca e retorna uma cópia de uma string (substituto seguro para strdup)
char* copiarString(const char *s) {
    if (s == NULL) return NULL;
    size_t n = strlen(s) + 1;
    char *c = (char*) malloc(n);
    if (!c) {
        printf("Erro: sem memória ao copiar string.\n");
        exit(1);
    }
    strcpy(c, s);
    return c;
}

// Converte primeira letra para maiúscula apenas para exibir (não altera a string original)
void exibirTitulo(const char *s) {
    if (!s) return;
    if (islower((unsigned char)s[0])) {
        putchar(toupper((unsigned char)s[0]));
        printf("%s", s+1);
    } else {
        printf("%s", s);
    }
}

// ----------------------
// criarSala()
// Cria dinamicamente uma sala com nome e opcionalmente uma pista.
// Se pista == NULL ou string vazia -> pista será NULL.
// ----------------------
Sala* criarSala(const char *nome, const char *pista) {
    Sala *nova = (Sala*) malloc(sizeof(Sala));
    if (!nova) {
        printf("Erro ao alocar memória para sala.\n");
        exit(1);
    }
    strncpy(nova->nome, nome, MAX_NOME-1);
    nova->nome[MAX_NOME-1] = '\0';
    if (pista != NULL && strlen(pista) > 0) {
        nova->pista = copiarString(pista);
    } else {
        nova->pista = NULL;
    }
    nova->esquerda = NULL;
    nova->direita = NULL;
    return nova;
}

// ----------------------
// inserirPista()
// Insere uma pista (string) na BST de pistas de forma ordenada.
// Evita duplicatas (se igual, não insere).
// ----------------------
PistaNode* inserirPista(PistaNode *raiz, const char *pista) {
    if (pista == NULL) return raiz;
    if (raiz == NULL) {
        PistaNode *n = (PistaNode*) malloc(sizeof(PistaNode));
        if (!n) {
            printf("Erro ao alocar memória para nó de pista.\n");
            exit(1);
        }
        n->pista = copiarString(pista);
        n->esq = n->dir = NULL;
        return n;
    }
    int cmp = strcmp(pista, raiz->pista);
    if (cmp < 0) {
        raiz->esq = inserirPista(raiz->esq, pista);
    } else if (cmp > 0) {
        raiz->dir = inserirPista(raiz->dir, pista);
    } else {
        // Igual: já foi coletada — não inserir duplicata
    }
    return raiz;
}

// ----------------------
// exibirPistas()
// Percorre a BST em ordem (in-order) e imprime as pistas em ordem alfabética.
// ----------------------
void exibirPistas(PistaNode *raiz) {
    if (raiz == NULL) return;
    exibirPistas(raiz->esq);
    printf(" - %s\n", raiz->pista);
    exibirPistas(raiz->dir);
}

// ----------------------
// liberarPistas()
// Libera memória da BST de pistas.
// ----------------------
void liberarPistas(PistaNode *raiz) {
    if (raiz == NULL) return;
    liberarPistas(raiz->esq);
    liberarPistas(raiz->dir);
    free(raiz->pista);
    free(raiz);
}

// ----------------------
// liberarSalas()
// Libera memória da árvore de salas (mapa).
// Observação: cada sala tem campo pista que pode ser NULL (já coletada) ou não.
// ----------------------
void liberarSalas(Sala *raiz) {
    if (raiz == NULL) return;
    liberarSalas(raiz->esquerda);
    liberarSalas(raiz->direita);
    if (raiz->pista) free(raiz->pista);
    free(raiz);
}

// ----------------------
// explorarSalasComPistas()
// Navegação interativa: em cada sala informa nome, mostra pista (se houver) e
// adiciona a pista à BST. O usuário escolhe 'e' (esquerda), 'd' (direita) ou 's' (sair).
// ----------------------
void explorarSalasComPistas(Sala *atual, PistaNode **arvorePistas) {
    char opcao;
    while (atual != NULL) {
        printf("\nVocê está em: %s\n", atual->nome);

        // Se houver pista e ainda não coletada, coleta automaticamente
        if (atual->pista != NULL) {
            printf("Você encontrou uma pista: \"%s\"\n", atual->pista);
            *arvorePistas = inserirPista(*arvorePistas, atual->pista);
            // Marca como coletada liberando a string e tornando NULL
            free(atual->pista);
            atual->pista = NULL;
        } else {
            printf("Nenhuma pista nova aqui.\n");
        }

        // Mostra opções
        printf("\nOpções:\n");
        if (atual->esquerda)  printf(" (e) Ir para %s (esquerda)\n", atual->esquerda->nome);
        if (atual->direita)   printf(" (d) Ir para %s (direita)\n", atual->direita->nome);
        printf(" (s) Sair da exploração e ver pistas coletadas\n");
        printf("Escolha: ");

        // Ler opção ignorando espaços em branco
        if (scanf(" %c", &opcao) != 1) {
            // entrada inválida: consumir resto e continuar
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
            printf("Entrada inválida. Tente novamente.\n");
            continue;
        }

        if (opcao == 'e' || opcao == 'E') {
            if (atual->esquerda) atual = atual->esquerda;
            else printf("Não há caminho à esquerda. Escolha outra opção.\n");
        } else if (opcao == 'd' || opcao == 'D') {
            if (atual->direita) atual = atual->direita;
            else printf("Não há caminho à direita. Escolha outra opção.\n");
        } else if (opcao == 's' || opcao == 'S') {
            printf("Encerrando exploração por opção do jogador.\n");
            break;
        } else {
            printf("Opção inválida. Use 'e', 'd' ou 's'.\n");
        }
    }
}

// ----------------------
// main()
// Monta o mapa fixo, inicia exploração, exibe pistas ordenadas e libera memória.
// ----------------------
int main(void) {
    // Montagem do mapa (árvore fixa) com pistas já associadas
    // Criamos salas usando criarSala(nome, pista)
    Sala *hall = criarSala("Hall de Entrada", "Bilhete rasgado com a hora marcada");
    Sala *salaEstar = criarSala("Sala de Estar", "Pegadas molhadas perto da lareira");
    Sala *cozinha = criarSala("Cozinha", "Faca com monograma X");
    Sala *biblioteca = criarSala("Biblioteca", "Livro apontando para passagem secreta");
    Sala *jardim = criarSala("Jardim", "Foto antiga da família no arbusto");
    Sala *pirao = criarSala("Porão", "Raspas de tinta da mesma cor da mansão");
    Sala *escritorio = criarSala("Escritório", "Carta com uma assinatura parcial");
    Sala *sotao = criarSala("Sótão", "Caixa trancada com três chaves faltando");

    // Conectar salas (árvore binária)
    hall->esquerda = salaEstar;
    hall->direita  = cozinha;

    salaEstar->esquerda = biblioteca;
    salaEstar->direita  = jardim;

    cozinha->esquerda = pirao;
    cozinha->direita  = escritorio;

    biblioteca->esquerda = sotao;  // exemplo de profundidade extra

    // Árvore BST para pistas inicialmente vazia
    PistaNode *arvorePistas = NULL;

    // Mensagem inicial
    printf("====================================\n");
    printf(" 🕵️ DETECTIVE QUEST - MODO AVENTUREIRO\n");
    printf("====================================\n");
    printf("Explore a mansão, as pistas serão coletadas automaticamente.\n");
    printf("Navegue com 'e' (esquerda), 'd' (direita) ou 's' (sair).\n");

    // Inicia exploração a partir do hall
    explorarSalasComPistas(hall, &arvorePistas);

    // Ao finalizar, exibe todas as pistas coletadas em ordem alfabética
    printf("\n\n===== PISTAS COLETADAS (ORDEM ALFABÉTICA) =====\n");
    if (arvorePistas == NULL) {
        printf("Nenhuma pista coletada.\n");
    } else {
        exibirPistas(arvorePistas);
    }

    // Liberar memória
    liberarPistas(arvorePistas);
    liberarSalas(hall);

    printf("\nObrigado por jogar Detective Quest! Boa investigação.\n");
    return 0;
}
