#ifndef HMM_HEADER_
#define HMM_HEADER_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef MAX_STATE
#	define MAX_STATE	60
#endif

#ifndef MAX_OBSERV
#	define MAX_OBSERV	60
#endif

#ifndef MAX_SEQ
#	define	MAX_SEQ		30
#endif

#ifndef MAX_LINE
#	define MAX_LINE 	100
#endif

typedef struct {
    int state_num;
    int observ_num;
    double initial[MAX_STATE];
    double transition[MAX_STATE][MAX_STATE];
    double observation[MAX_OBSERV][MAX_STATE];
} HMM;

static FILE *open_or_die(const char *filename, const char *ht) {
    FILE *fp = fopen(filename, ht);
    if (fp == NULL){
        perror(filename);
        exit(1);
    }

    return fp;
}

static void loadHMM(HMM *hmm, const char *filename) {
    int i, j;
    FILE *fp = open_or_die(filename, "r");

    char token[MAX_LINE] = "";
    while(fscanf(fp, "%s", token) > 0) {
        if (token[0] == '\0' || token[0] == '\n') {
            continue;
        }

        if (strcmp(token, "initial:") == 0) {
            fscanf(fp, "%d", &hmm->state_num);

            for (i = 0; i < hmm->state_num; i++) {
                fscanf(fp, "%lf", &(hmm->initial[i]));
            }
        }
        else if (strcmp(token, "transition:") == 0) {
            fscanf(fp, "%d", &hmm->state_num);

            for (i = 0; i < hmm->state_num; i++) {
                for (j = 0; j < hmm->state_num; j++) {
                    fscanf(fp, "%lf", &(hmm->transition[i][j]));
                }
            }
        }
        else if (strcmp(token, "observation:") == 0) {
            fscanf(fp, "%d", &hmm->observ_num);

            for (i = 0; i < hmm->observ_num; i++) {
                for (j = 0; j < hmm->state_num; j++) {
                    fscanf(fp, "%lf", &(hmm->observation[i][j]));
                }
            }
        }
    }
    fclose(fp);
}

static void dumpHMM(FILE *fp, HMM *hmm) {
    int i, j;

    //fprintf(fp, "model name: %s\n", hmm->model_name);
    fprintf(fp, "initial: %d\n", hmm->state_num);
    for (i = 0 ; i < hmm->state_num; i++) {
        fprintf(fp, "%.5lf ", hmm->initial[i]);
    }
    fprintf(fp, "\n");

    fprintf(fp, "\ntransition: %d\n", hmm->state_num);
    for (i = 0; i < hmm->state_num; i++) {
        for (j = 0; j < hmm->state_num; j++) {
            fprintf(fp, "%.5lf ", hmm->transition[i][j]);
        }
        fprintf(fp, "\n");
    }

    fprintf(fp, "\nobservation: %d\n", hmm->observ_num);
    for (i = 0; i < hmm->observ_num; i++){
        for (j = 0; j < hmm->state_num; j++) {
            fprintf(fp, "%.5lf ", hmm->observation[i][j]);
        }
        fprintf(fp, "\n");
    }
}

static int load_models(const char *listname, HMM *hmm, const int max_num) {
    FILE *fp = open_or_die(listname, "r");

    int count = 0;
    char filename[MAX_LINE] = "";
    while(fscanf(fp, "%s", filename) == 1) {
        loadHMM(&hmm[count], filename);
        count++;

        if (count >= max_num) {
            return count;
        }
    }
    fclose(fp);

    return count;
}

static void dump_models(HMM *hmm, const int num) {
    int i = 0;
    for ( ; i < num; i++) { 
        dumpHMM(stderr, &hmm[i]);
    }
}

static void init_model(HMM *hmm, int encode[MAX_OBSERV][MAX_STATE]) {
    srand(time(NULL));
    
    double sum = 0;
    // printf("Initial:\n");
    for (int i = 0; i < hmm->state_num; i++) {
        hmm->initial[i] = rand();
        sum += hmm->initial[i];
    }
    for (int i = 0; i < hmm->state_num; i++) {
        hmm->initial[i] /= sum;
        // printf("%.2lf ", hmm->initial[i]);
    }
    // printf("\n");
    
    // printf("Transition:\n");
    for (int i = 0; i < hmm->state_num; i++) {
        sum = 0;
        for (int j = 0; j < hmm->state_num; j++) {
            hmm->transition[i][j] = rand();
            sum += hmm->transition[i][j];
        }
        for (int j = 0; j < hmm->state_num; j++) {
            hmm->transition[i][j] /= sum;
            // printf("%.2lf ", hmm->transition[i][j]);
        }
        // printf("\n");
    }
    
    for (int j = 0; j < hmm->state_num; j++) {
        sum = 0;
        for (int k = 0; k < hmm->observ_num; k++) {
            hmm->observation[k][j] = encode[k][j] ? rand() : 0;
            sum += hmm->observation[k][j];
        }
        for (int k = 0; k < hmm->observ_num; k++) {
            hmm->observation[k][j] /= sum;
        }
    }
    
    // printf("Observation:\n");
    for (int k = 0; k < hmm->observ_num; k++) {
        for (int j = 0; j < hmm->state_num; j++) {
            // printf("%.2lf ", hmm->observation[k][j]);
        }
        // printf("\n");
    }
}

#endif
