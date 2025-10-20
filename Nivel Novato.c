#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estrutura que representa uma sala da mansão
typedef struct Sala {
    char nome[50];
    struct Sala* esquerda;  // Caminho à esquerda
    struct Sala* direita;   // Caminho à direita
} Sala;

//------------------------------------------------------
// Função: criarSala()
// Cria dinamicamente uma sala com um nome informado
//------------------------------------------------------
Sala* criarSala(const char* nome) {
    Sala* novaSala = (Sala*) malloc(sizeof(Sala));
    if (novaSala == NULL) {
        printf("Erro ao alocar memória para a sala.\n");
        exit(1);
    }
    strcpy(novaSala->nome, nome);
    novaSala->esquerda = NULL;
    novaSala->direita = NULL;
    return novaSala;
}

//------------------------------------------------------
// Função: explorarSalas()
// Permite que o jogador explore a mansão
//------------------------------------------------------
void explorarSalas(Sala* salaAtual) {
    char escolha;

    while (salaAtual != NULL) {
        printf("\nVocê está na: %s\n", salaAtual->nome);

        // Caso a sala não tenha saídas (nó-folha)
        if (salaAtual->esquerda == NULL && salaAtual->direita == NULL) {
            printf("Você chegou ao fim do caminho!\n");
            break;
        }

        printf("Escolha o caminho:\n");
        if (salaAtual->esquerda != NULL)
            printf(" (e) Ir para %s à esquerda\n", salaAtual->esquerda->nome);
        if (salaAtual->direita != NULL)
            printf(" (d) Ir para %s à direita\n", salaAtual->direita->nome);
        printf(" (s) Sair do jogo\n");
        printf("Digite sua opção: ");
        scanf(" %c", &escolha);

        if (escolha == 'e' || escolha == 'E') {
            if (salaAtual->esquerda != NULL)
                salaAtual = salaAtual->esquerda;
            else
                printf("Não há caminho à esquerda!\n");
        }
        else if (escolha == 'd' || escolha == 'D') {
            if (salaAtual->direita != NULL)
                salaAtual = salaAtual->direita;
            else
                printf("Não há caminho à direita!\n");
        }
        else if (escolha == 's' || escolha == 'S') {
            printf("Você decidiu encerrar a exploração.\n");
            break;
        }
        else {
            printf("Opção inválida! Tente novamente.\n");
        }
    }
}

//------------------------------------------------------
// Função: liberarSalas()
// Libera a memória da árvore binária recursivamente
//------------------------------------------------------
void liberarSalas(Sala* raiz) {
    if (raiz != NULL) {
        liberarSalas(raiz->esquerda);
        liberarSalas(raiz->direita);
        free(raiz);
    }
}

//------------------------------------------------------
// Função principal: main()
// Monta o mapa da mansão e inicia o jogo
//------------------------------------------------------
int main() {
    // Criação das salas (árvore fixa)
    Sala* hallEntrada = criarSala("Hall de Entrada");
    Sala* salaEstar   = criarSala("Sala de Estar");
    Sala* cozinha     = criarSala("Cozinha");
    Sala* biblioteca  = criarSala("Biblioteca");
    Sala* jardim      = criarSala("Jardim");
    Sala* porao       = criarSala("Porão");
    Sala* escritorio  = criarSala("Escritório");

    // Montagem manual da árvore binária
    hallEntrada->esquerda = salaEstar;
    hallEntrada->direita  = cozinha;

    salaEstar->esquerda   = biblioteca;
    salaEstar->direita    = jardim;

    cozinha->esquerda     = porao;
    cozinha->direita      = escritorio;

    // Início do jogo
    printf("====================================\n");
    printf(" 🕵️  DETECTIVE QUEST: A MANSÃO ENIGMA\n");
    printf("====================================\n");
    printf("Você começa sua investigação...\n");

    explorarSalas(hallEntrada);

    // Libera a memória alocada
    liberarSalas(hallEntrada);

    printf("\nObrigado por jogar Detective Quest!\n");
    return 0;
}
