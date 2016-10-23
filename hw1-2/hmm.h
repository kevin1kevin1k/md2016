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

static void init_model(HMM *hmm, int encode[MAX_OBSERV][MAX_STATE]) {
    srand(time(NULL));
    
    double sum = 0;
    for (int i = 0; i < hmm->state_num; i++) {
        hmm->initial[i] = rand();
        sum += hmm->initial[i];
    }
    for (int i = 0; i < hmm->state_num; i++) {
        hmm->initial[i] /= sum;
    }
    
    for (int i = 0; i < hmm->state_num; i++) {
        sum = 0;
        for (int j = 0; j < hmm->state_num; j++) {
            hmm->transition[i][j] = rand();
            sum += hmm->transition[i][j];
        }
        for (int j = 0; j < hmm->state_num; j++) {
            hmm->transition[i][j] /= sum;
        }
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
}

static double Viterbi(HMM *hmm, int seq[MAX_SEQ], int len) {
    double delta[MAX_SEQ][MAX_STATE];
    int psi[MAX_SEQ][MAX_STATE];
    for (int i = 0; i < hmm->state_num; i++) {
        delta[0][i] = hmm->initial[i] * hmm->observation[seq[0]][i];
    }
    
    for (int t = 1; t < len; t++) {
        for (int j = 0; j < hmm->state_num; j++) {
            double max_prob = 0;
            int max_prev;
            for (int i = 0; i < hmm->state_num; i++) {
                double prob = delta[t-1][i] * hmm->transition[i][j];
                if (prob > max_prob) {
                    max_prob = prob;
                    max_prev = i;
                }
            }
            delta[t][j] = max_prob * hmm->observation[seq[t]][j];
            psi[t][j] = max_prev;
        }
    }
    
    double max = 0;
    for (int i = 0; i < hmm->state_num; i++) {
        if (delta[len-1][i] > max) {
            max = delta[len-1][i];
            seq[len-1] = i;
        }
    }
    
    // store the decoded sequence back into seq
    for (int t = len - 2; t >= 0; t--) {
        seq[t] = psi[t+1][seq[t+1]];
    }
    
    return max;
}

static double calc_acc(HMM *hmm, char dir_name[MAX_LINE], char _pred_num_name[MAX_LINE]) {
    char test_num_name[MAX_LINE], pred_num_name[MAX_LINE], ans_num_name[MAX_LINE];
    strcpy(test_num_name, dir_name);
    strcpy(pred_num_name, dir_name);
    strcpy(ans_num_name, dir_name);
    strcat(test_num_name, "/test.num");
    strcat(pred_num_name, "/");
    strcat(pred_num_name, _pred_num_name);
    strcat(ans_num_name, "/ans.num");
    
    FILE *ans_num = open_or_die(ans_num_name, "r");
    FILE *test_num = open_or_die(test_num_name, "r");
    FILE *pred_num = open_or_die(pred_num_name, "w");
    
    int seq[MAX_SEQ];
    int SPACE = hmm->state_num;
    int len = 0, ans, n_num = 0, n_same = 0;
    while (1) {
        int res = fscanf(test_num, "%d", &seq[len]);
        if (res <= 0 || seq[len] == SPACE) {
            Viterbi(hmm, seq, len);
            n_num += len + 1;
            for (int i = 0; i <= len; i++) {
                fscanf(ans_num, "%d", &ans);
                if (ans == seq[i]) {
                    n_same++;
                }
                fprintf(pred_num, "%d ", seq[i]);
            }
            
            len = 0;
            
            if (res <= 0) {
                break;
            }
        }
        else {
            len++;
        }
    }
    
    fclose(test_num);
    fclose(pred_num);
    fclose(ans_num);
    
    return 1.0 * n_same / n_num;
}

#endif
