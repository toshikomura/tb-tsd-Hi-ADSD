#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include"smpl.h"
#include"cisj.h"

#define test 1
#define fault 2
#define repair 3

typedef struct {
    int id;
    int cluster;
    int *testedup;
    int *nodos_testados;
    int *latencia;
    int qtd_testes;
}tnodo;

typedef struct {
    int qtd_testes;
    int latencia;
}tmudancas;

tnodo *nodo;

tmudancas *mudancas;

int pos = 0;

Inicializacao_do_Termina_Diagnostico_Nodos( int *termina_diagnostico_nodos, int n){

    int i;

    for (i = 0; i < n; i++){
        termina_diagnostico_nodos[i] = 1;
    }

}

Atualizacao_do_Inicia_Diagnostico_Nodos( int *inicia_diagnostico_nodos, int n){

    int i;

    for (i = 0; i < n; i++){
        inicia_diagnostico_nodos[i] = status( nodo[i].id);
    }

}

Atualizacao_do_Termina_Diagnostico_Nodos( int *termina_diagnostico_nodos, int n){

    int i;
    for (i = 0; i < n; i++){
        termina_diagnostico_nodos[i] = status( nodo[i].id);
    }

}

Calcula_Latencia( int token, int n){

    int i;

    /* verifica mudanças de estado dos nodos (latencia) */
    for ( i = 0; i < n; i++){
        if ( ( (nodo[token].testedup[i] % 2) != status(nodo[i].id) ) /*&& nodo[token].timestamp[i] != -1*/ ){
            nodo[token].latencia[i]++;
        }
    }

}

int Checa_se_Terminou_o_Diagnostico( int n){

    int i, j,
    terminou_diagnostico = 1; // chuta que terminou

    /* checa se o diagnostico terminou */
    for ( i = 0; i < n; i++ ){ // para cada nodo
        for ( j = 0; j < n; j++ ){ // para cada posição do timestamp do nodo

            /* se i esta sem falha e no timestamp de i na posição j é diferente do estado de j */
            if ( status( nodo[i].id) == 0 && ( nodo[i].testedup[j] % 2 ) != status( nodo[j].id) ){
                terminou_diagnostico = 0;
            }

        }
    }

    return terminou_diagnostico;
}

int Checa_se_Iniciou_o_Diagnostico( int *inicia_diagnostico_nodos, int *termina_diagnostico_nodos, int n){

    int i, iniciou_diagnostico = 0;

    for (i = 0; i < n; i++){

        /* se um nodo mudou de estado se inicia um novo diagnostico */
        if ( inicia_diagnostico_nodos[i] != termina_diagnostico_nodos[i] )
            iniciou_diagnostico = 1;
    }

    return iniciou_diagnostico;
}

Iniciando_Simulacao( int *termina_diagnostico_nodos, int n){

    char fa_name[5];
    int i, j;

    smpl(0, "Simulação");
    reset();
    stream(1);
    nodo = (tnodo *) malloc (sizeof(tnodo)*n);

    for ( i = 0; i < n ; i++) {
        memset(fa_name, '\0', 5);
        sprintf(fa_name, "%d", i);
        nodo[i].id = facility(fa_name, 1);
        nodo[i].testedup = (int *) malloc (sizeof(int)*n);
        nodo[i].nodos_testados = (int *) malloc (sizeof(int)*n);
        nodo[i].latencia = (int *) malloc(sizeof(int)*n);

        /* inicia no cluster 1 */
        nodo[i].cluster = 1;

        /* seta vetor de teste para -1 */
        for (j = 0; j < n; j++) {
            nodo[i].testedup[j] = -1;
            nodo[i].nodos_testados[j] = 0;
        }
        /* valor de teste dele mesmo */
        nodo[i].testedup[i] = 0;

        /* seta vetor de latencia para 0 */
        for (j = 0; j < n; j++){
            nodo[i].latencia[j] = 0;
        }

    }

    /* primeiro escalonamento de todos os nodos */
    for (i = 0; i< n ; i++) {
        schedule(test, 30.0, i);
    }

    /* definição de tempo de erro e reparo */

    schedule(fault, 1.0, 4);
    schedule(fault, 1.0, 5);
    schedule(fault, 1.0, 6);
    schedule(fault, 1.0, 7);

//    schedule(repair, 50.0, 0);
//    schedule(repair, 50.0, 1);
//    schedule(repair, 50.0, 2);

    Inicializacao_do_Termina_Diagnostico_Nodos( termina_diagnostico_nodos, n);

}

Imprime_Log( node_set *nodes, int *num_cluster, int token, int max_cluster, int j, int n){

    int i;

    /* imprimi log */

    /* checa cluster */
    if ( *num_cluster != nodo[token].cluster){
        printf("CLUSTER %d\n", nodo[token].cluster);
        *num_cluster = nodo[token].cluster;
    }

    printf("sou o nodo %d no tempo %5.1f \n", token, time());
    printf("testei os nodo(s): ");
    for (i = 0; i < n; i++) {
        if (nodo[token].nodos_testados[i] == 1)
            printf(" %d ", i);
    }
    printf("\n");

    printf("[%d] state: ", token);
    for (i = 0; i < n; i++) {
        printf(" [%d] ", nodo[token].testedup[i]);
    }
    printf("\n\n");

}

Algoritmo_de_Testes( int *num_cluster, int token, int max_cluster, int n){

    int i, j, l, m,
    encontrou_nodo_sem_falha = 0,
    atualizou_nodo_em_cluster_anterior;
    node_set *nodes, *aux_nodes;

    nodes = cis(token, nodo[token].cluster);

    j = 0;
    while( j < nodes->size && encontrou_nodo_sem_falha == 0){

        /* se o nodo existe */
        if (nodes->nodes[j] < n){

            nodo[token].qtd_testes++;
            nodo[token].nodos_testados[nodes->nodes[j]] = 1;

            /* testa se o nodo esta sem falha */
            if ( status(nodo[nodes->nodes[j]].id) == 0 ){

                encontrou_nodo_sem_falha = 1;
                nodo[token].testedup[nodes->nodes[j]] = 0;

                /* atualiza o testedup */
                for (i = 0; i < n; i++){

                    /* desconsiderando clusters anteriores */
                    atualizou_nodo_em_cluster_anterior = 0;

                    // percorre todos os cluster anteriores //
                    for(l = 1; l < nodo[token].cluster; l++){

                        aux_nodes = cis(token, l);

                        for(m = 0; m < aux_nodes->size; m++)
                            // se o nodo já foi atualizado em um cluster anterior //
                            if(aux_nodes->nodes[m] == i)
                                atualizou_nodo_em_cluster_anterior = 1;

                    }
                    // se o token não tem informação do nodo e não é ele mesmo e não é o nodo testado //
                    if ( atualizou_nodo_em_cluster_anterior != 1 && i != token & i != nodes->nodes[j])
                       nodo[token].testedup[i] = nodo[nodes->nodes[j]].testedup[i];
               }
            }
            else
                nodo[token].testedup[nodes->nodes[j]] = 1;

        }
        j++;
    }

    Imprime_Log( nodes, num_cluster, token, max_cluster, j, n);

}

Verifica_Diagnostico( int *inicia_diagnostico_nodos, int *termina_diagnostico_nodos, int *maior, int n){

    int i, j,
    latencia_maior, latencia_testes;

    /* Verifica se um novo diagnostico foi reiniado para termina-lo */
    if (Checa_se_Iniciou_o_Diagnostico( inicia_diagnostico_nodos, termina_diagnostico_nodos, n)){
        if ( Checa_se_Terminou_o_Diagnostico( n ) ){
            Atualizacao_do_Termina_Diagnostico_Nodos( termina_diagnostico_nodos, n);

            for (i = 0; i < n; i++){ // para cada posição do timestamp do vetor
                maior[i] = nodo[0].latencia[i];
                for (j = 1; j < n; j++){ // para cada nodo
                    /* se o nodo não esta falho */
                    if ( status( nodo[j].id ) == 0 ){

                        if ( maior[i] < nodo[j].latencia[i]){
                            maior[i] = nodo[j].latencia[i];
                        }

                    }
                }
            }

            latencia_maior = maior[0];
            for (i = 1; i < n; i++){
                if ( latencia_maior < maior[i] ){
                    latencia_maior = maior[i];
                }
            }

            latencia_testes = 0;
            for (i = 0; i < n; i++){
                latencia_testes = latencia_testes + nodo[i].qtd_testes;
            }

            printf("latencia_maior = %d latencia_testes %d\n\n", latencia_maior, latencia_testes);

            mudancas[pos].latencia = latencia_maior;
            mudancas[pos].qtd_testes = latencia_testes;
            pos++;

            /* zera todo mundo */
            for (i = 0; i < n; i++){
                for (j = 0; j < n; j++){
                    nodo[i].latencia[j] = 0;
                }
            }
            for (i = 0; i < n; i++){
                maior[i] = 0;
            }
            for (i = 0; i < n; i++){
                nodo[i].qtd_testes = 0;
            }

        }
    }
}

main ( int argc, char *argv[]) {
    static int n,
    token, aux_token,
    event;
    int r, s,
    i, j, k,
    max_cluster, num_cluster = 0,
    *inicia_diagnostico_nodos, *termina_diagnostico_nodos, *maior;

    if (argc != 2) {
        puts ("Número errado de parametros, uso correto, ./temp |número de nodos|");
        exit(1);
    }

    n = atoi(argv[1]);
    max_cluster =(int) ceil( log2(n) );

    /* inicio da alocação de ponteiros */
    maior = malloc(sizeof(int)*n);
    inicia_diagnostico_nodos = malloc(sizeof(int)*n);
    termina_diagnostico_nodos = malloc(sizeof(int)*n);
    mudancas =(tmudancas *) malloc(sizeof(tmudancas)* max_cluster);
    /* final da alocação de ponteiros */

    Iniciando_Simulacao( termina_diagnostico_nodos, n);

    printf("\n<<<<<<<<<<<<<<<<<<<START SIMULATION>>>>>>>>>>>>>>>>>>>\n\n");

    while (time() < 80.0) {

        cause(&event, &token);

        switch(event){

        case test:

            if ( status(nodo[token].id) != 0) break;

            Atualizacao_do_Inicia_Diagnostico_Nodos( inicia_diagnostico_nodos, n);

            Calcula_Latencia( token, n);

            Algoritmo_de_Testes( &num_cluster, token, max_cluster, n);

            /* apaga os nodos testados */
            for (k = 0; k < n; k++) {
                nodo[token].nodos_testados[k] = 0;
            }

            /* checa se esta no último cluster ou não */
            if ( nodo[token].cluster < max_cluster ) {
                nodo[token].cluster++;
                schedule(test, 0.0, token);
            }
            else {
                nodo[token].cluster = 1;
                schedule(test, 30.0, token);
            }

            Verifica_Diagnostico( inicia_diagnostico_nodos, termina_diagnostico_nodos, maior, n);

            break;

        case fault:
            r = request( nodo[token].id, token, 0);
            if (r != 0) {
                printf ("Não consegui falhar nodo %d\n", token);
                exit(1);
            }
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            printf("O nodo %d falha no tempo %5.1f \n", token, time());
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
            break;

        case repair:
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            printf("o nodo %d reparou no tempo %5.1f \n", token, time());
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
            release(nodo[token].id, token);
            schedule(test, 30.0, token);
            break;

        }
    }

    printf("<<<<<<<<<<<<<<<<<<<FINISH SIMULATION>>>>>>>>>>>>>>>>>>>\n\n");

    printf("<<<<<<<<<<<<<<INICIO LATENCIA E QTD TESTES>>>>>>>>>>>>>>\n\n");
    printf("Considerando a sequencia de eventos de diagnosticos as\n");
    printf("respectivas latencia e quantidade de testes foram:\n\n");
    for (i = 0; i < pos; i++){
        printf("latencia %d qtd_testes %d\n", mudancas[i].latencia, mudancas[i].qtd_testes);
    }
    printf("\n");
    printf("<<<<<<<<<<<<<<<FIM LATENCIA E QTD TESTES>>>>>>>>>>>>>>>\n\n");

}
