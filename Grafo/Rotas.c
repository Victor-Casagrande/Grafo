#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

#define Inicial_capacidade_vila 4
#define Tam_Nome     100

// Estruturas do TAD Grafo
typedef struct AdjNode {
    size_t destIndex;
    struct AdjNode *next;
} AdjNode;

typedef struct vila {
    char *name;
    AdjNode *head;
} vila;

typedef struct Grafo {
    vila **vilas;
    size_t numvilas;
    size_t capacidade_vila;
} Grafo;

// Prototipos do TAD
Grafo* createGrafo(void);
void destroyGrafo(Grafo *g);
bool addconexao(Grafo *g, const char *src, const char *dest);
bool removeconexao(Grafo *g, const char *src, const char *dest);
void listconexaos(const Grafo *g, const char *src);
void printGrafo(const Grafo *g);
bool exportaDot(const Grafo *g, const char *filename);

// Função auxiliar para ordenação de ponteiros de vila em ordem alfabetica
static int cmpvilaPtr(const void *a, const void *b) {
    vila *const *c1 = a;
    vila *const *c2 = b;
    return strcmp((*c1)->name, (*c2)->name);
}

// Implementação do TAD
Grafo* createGrafo(void) {
    Grafo *g = malloc(sizeof(*g));
    if (!g) return NULL;
    g->capacidade_vila = Inicial_capacidade_vila;
    g->numvilas = 0;
    g->vilas = malloc(sizeof(*g->vilas) * g->capacidade_vila);
    if (!g->vilas) { free(g); return NULL; }
    return g;
}

void destroyGrafo(Grafo *g) {
    if (!g) return;
    for (size_t i = 0; i < g->numvilas; i++) {
        vila *c = g->vilas[i];
        for (AdjNode *p = c->head; p; ) {
            AdjNode *tmp = p;
            p = p->next;
            free(tmp);
        }
        free(c->name);
        free(c);
    }
    free(g->vilas);
    free(g);
}

// Procura indice com busca linear
static ssize_t findvila(const Grafo *g, const char *name) {
    for (size_t i = 0; i < g->numvilas; i++) {
        if (strcmp(g->vilas[i]->name, name) == 0)
            return (ssize_t)i;
    }
    return -1;
}

static size_t addvila(Grafo *g, const char *name) {
    if (g->numvilas == g->capacidade_vila) {
        size_t newCap = g->capacidade_vila * 2;
        vila **tmp = realloc(g->vilas, sizeof(*g->vilas) * newCap);
        if (!tmp) return g->numvilas;
        g->vilas = tmp;
        g->capacidade_vila = newCap;
    }
    vila *c = malloc(sizeof(*c));
    c->name = strdup(name);
    c->head = NULL;
    g->vilas[g->numvilas] = c;
    return g->numvilas++;
}

/*
Procura índice das vilas de origem e destino com busca linear (findvila); se não existir, chama addvila para criar nova vila.
Evita duplicata varrendo a lista de adjacência.
Insere um novo AdjNode no início da lista ligada — tempo constante.
*/
bool addconexao(Grafo *g, const char *src, const char *dest) {
    ssize_t s = findvila(g, src);
    if (s < 0) s = (ssize_t)addvila(g, src);
    ssize_t d = findvila(g, dest);
    if (d < 0) d = (ssize_t)addvila(g, dest);
    for (AdjNode *p = g->vilas[s]->head; p; p = p->next) {
        if (p->destIndex == (size_t)d)
            return false;
    }
    AdjNode *node = malloc(sizeof(*node));
    node->destIndex = (size_t)d;
    node->next = g->vilas[s]->head;
    g->vilas[s]->head = node;
    return true;
}

/*
Localiza índices de origem e destino.
Percorre a lista ligada guardando prev.
Ao encontrar destIndex, “desconecta” o nó e libera sua memória.
*/
bool removeconexao(Grafo *g, const char *src, const char *dest) {
    ssize_t s = findvila(g, src);
    ssize_t d = findvila(g, dest);
    if (s < 0 || d < 0) return false;
    AdjNode *p = g->vilas[s]->head, *prev = NULL;
    while (p) {
        if (p->destIndex == (size_t)d) {
            if (prev) prev->next = p->next;
            else g->vilas[s]->head = p->next;
            free(p);
            return true;
        }
        prev = p; p = p->next;
    }
    return false;
}

/*
Conta grau (quantidade de arestas).
Copia ponteiros para os vizinhos num vetor temporário.
Ordena esse vetor alfabeticamente com qsort + cmpvilaPtr.
Imprime cada nome em formato de lista.
*/
void listconexaos(const Grafo *g, const char *src) {
    ssize_t s = findvila(g, src);
    if (s < 0) {
        printf("Cidade '%s' nao encontrada.\n", src);
        return;
    }
    vila *c = g->vilas[s];
    size_t count = 0;
    for (AdjNode *p = c->head; p; p = p->next) count++;
    if (count == 0) {
        printf("Conexoes de %s: (nenhuma)\n", src);
        return;
    }
    vila **viz = malloc(sizeof(*viz) * count);
    size_t idx = 0;
    for (AdjNode *p = c->head; p; p = p->next)
        viz[idx++] = g->vilas[p->destIndex];
    qsort(viz, count, sizeof(*viz), cmpvilaPtr);
    printf("Conexoes de %s:\n", src);
    for (size_t i = 0; i < count; i++)
        printf("  - %s\n", viz[i]->name);
    free(viz);
}

/*
Clona o vetor de todas as vilas num array auxiliar.
Ordena alfabeticamente as vilas com qsort.
Para cada vila, repete o processo de coleta e ordenação de vizinhos e imprime nomes separados por virgula.
*/
void printGrafo(const Grafo *g) {
    if (g->numvilas == 0) {
        printf("(nenhuma cidade cadastrada)\n");
        return;
    }
    vila **sorted = malloc(sizeof(*sorted) * g->numvilas);
    for (size_t i = 0; i < g->numvilas; i++) sorted[i] = g->vilas[i];
    qsort(sorted, g->numvilas, sizeof(*sorted), cmpvilaPtr);
    printf("\n=== GRAFO DE ROTAS ===\n");
    for (size_t i = 0; i < g->numvilas; i++) {
        vila *c = sorted[i];
        printf("%s: ", c->name);
        size_t deg = 0;
        for (AdjNode *p = c->head; p; p = p->next) deg++;
        if (deg == 0) {
            printf("(sem rotas)\n");
            continue;
        }
        vila **viz = malloc(sizeof(*viz) * deg);
        size_t j = 0;
        for (AdjNode *p = c->head; p; p = p->next)
            viz[j++] = g->vilas[p->destIndex];
        qsort(viz, deg, sizeof(*viz), cmpvilaPtr);
        for (size_t k = 0; k < deg; k++)
            printf("%s%s", viz[k]->name, k + 1 < deg ? ", " : "");
        printf("\n");
        free(viz);
    }
    printf("\n Para melhor visualização acesse: https://dreampuf.github.io/GrafovizOnline/ \n");
    free(sorted);
}

bool exportaDot(const Grafo *g, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return false;
    fprintf(f, "diGrafo Rotas {\n");
    for (size_t i = 0; i < g->numvilas; i++)
        fprintf(f, "  \"%s\";\n", g->vilas[i]->name);
    for (size_t i = 0; i < g->numvilas; i++) {
        vila *c = g->vilas[i];
        for (AdjNode *p = c->head; p; p = p->next)
            fprintf(f, "  \"%s\" -> \"%s\";\n",
                    c->name, g->vilas[p->destIndex]->name);
    }
    fprintf(f, "}\n");
    fclose(f);
    return true;
}

// Interface de Usuario
int main(void) {
    Grafo *g = createGrafo();
    if (!g) return EXIT_FAILURE;

    // Pre-cadastro de conexões conforme mapa de territorios
    const char *edges[][2] = {
        {"A-vila","B-vila"},
        {"A-vila","C-vila"},
        {"A-vila","D-vila"},
        {"A-vila","F-vila"},
        {"A-vila","I-vila"},
        {"A-vila","J-vila"},
        {"A-vila","K-vila"},
        {"A-vila","M-vila"},
        {"A-vila","N-vila"},
        {"A-vila","P-vila"},
        {"A-vila","R-vila"},

        {"B-vila","C-vila"},
        {"B-vila","Q-vila"},
        {"B-vila","R-vila"},
        {"B-vila","Z-vila"},

        {"C-vila","E-vila"},
        {"C-vila","F-vila"},
        {"C-vila","Q-vila"},

        {"D-vila","P-vila"},
        {"D-vila","R-vila"},
        {"D-vila","S-vila"},

        {"E-vila","F-vila"},
        {"E-vila","Q-vila"},

        {"F-vila","G-vila"},
        {"F-vila","I-vila"},

        {"G-vila","I-vila"},

        {"H-vila","V-vila"},
        {"H-vila","X-vila"},
        {"H-vila","Y-vila"},
        {"H-vila","Z-vila"},

        {"I-vila","J-vila"},

        {"J-vila","K-vila"},
        {"J-vila","L-vila"},

        {"K-vila","L-vila"},
        {"K-vila","M-vila"},
        {"K-vila","N-vila"},

        {"L-vila","M-vila"},

        {"M-vila","N-vila"},
        {"M-vila","U-vila"},

        {"N-vila","O-vila"},
        {"N-vila","P-vila"},
        {"N-vila","U-vila"},

        {"O-vila","P-vila"},
        {"O-vila","U-vila"},

        {"Q-vila","Z-vila"},

        {"R-vila","S-vila"},
        {"R-vila","Y-vila"},
        {"R-vila","Z-vila"},

        {"S-vila","T-vila"},
        {"S-vila","V-vila"},
        {"S-vila","W-vila"},
        {"S-vila","Y-vila"},

        {"T-vila","V-vila"},
        {"T-vila","W-vila"},
        {"T-vila","X-vila"},
        {"T-vila","Y-vila"},

        {"V-vila","W-vila"},
        {"V-vila","X-vila"},

        {"W-vila","X-vila"},

        {"X-vila","Y-vila"},
        {"Y-vila","Z-vila"}
    };
    size_t numEdges = sizeof(edges) / sizeof(edges[0]);
    for (size_t i = 0; i < numEdges; i++) {
        addconexao(g, edges[i][0], edges[i][1]);
        addconexao(g, edges[i][1], edges[i][0]);
    }

    int opcao;
    char src[Tam_Nome], dest[Tam_Nome], filename[Tam_Nome];
    do {
        printf("\n=== MENU ROTAS ===\n"
               "1) Inserir conexao\n"
               "2) Remover conexao\n"
               "3) Listar conexoes de uma cidade\n"
               "4) Imprimir grafo completo\n"
               "5) Exportar para DOT\n"
               "0) Sair\n"
               "Escolha: ");
        if (scanf("%d", &opcao) != 1) break;
        getchar(); // limpa \n pendente
        switch (opcao) {
            case 1:
                printf("Origem: "); fgets(src, Tam_Nome, stdin);
                src[strcspn(src, "\n")] = 0;
                printf("Destino: "); fgets(dest, Tam_Nome, stdin);
                dest[strcspn(dest, "\n")] = 0;
                printf(addconexao(g, src, dest)
                       ? "Conexao adicionada.\n"
                       : "Conexao ja existe.\n");
                break;
            case 2:
                printf("Origem: "); fgets(src, Tam_Nome, stdin);
                src[strcspn(src, "\n")] = 0;
                printf("Destino: "); fgets(dest, Tam_Nome, stdin);
                dest[strcspn(dest, "\n")] = 0;
                printf(removeconexao(g, src, dest)
                       ? "Conexao removida.\n"
                       : "Conexao nao encontrada.\n");
                break;
            case 3:
                printf("Cidade: "); fgets(src, Tam_Nome, stdin);
                src[strcspn(src, "\n")] = 0;
                listconexaos(g, src);
                break;
            case 4:
                printGrafo(g);
                break;
            case 5:
                printf("DOT arquivo: "); fgets(filename, Tam_Nome, stdin);
                filename[strcspn(filename, "\n")] = 0;
                printf(exportaDot(g, filename)
                       ? "Arquivo gerado.\n"
                       : "Erro escrevendo arquivo.\n");
                break;
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opcao invalida.\n");
        }
    } while (opcao != 0);

    destroyGrafo(g);
    return EXIT_SUCCESS;
}
