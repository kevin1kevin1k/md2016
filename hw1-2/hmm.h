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

static void copy_model(HMM *h1, HMM *h2) {
    for (int i = 0; i < h1->state_num; i++) {
        h1->initial[i] = h2->initial[i];
    }
    
    for (int i = 0; i < h1->state_num; i++) {
        for (int j = 0; j < h1->state_num; j++) {
            h1->transition[i][j] = h2->transition[i][j];
        }
    }
    
    for (int j = 0; j < h1->state_num; j++) {
        for (int k = 0; k < h1->observ_num; k++) {
            h1->observation[k][j] = h2->observation[k][j];
        }
    }
}

static double diff_model(HMM *h1, HMM *h2) {
    double sum = 0;
    for (int i = 0; i < h1->state_num; i++) {
        sum += fabs(h1->initial[i] - h2->initial[i]);
    }
    
    for (int i = 0; i < h1->state_num; i++) {
        for (int j = 0; j < h1->state_num; j++) {
            sum += fabs(h1->transition[i][j] - h2->transition[i][j]);
        }
    }
    
    for (int j = 0; j < h1->state_num; j++) {
        for (int k = 0; k < h1->observ_num; k++) {
            sum += fabs(h1->observation[k][j] - h2->observation[k][j]);
        }
    }
    
    return sum;
}

static void check0(double d) {
    if (d == 0) {
        printf("Error: divided by zero\n");
        exit(-1);
    }
}

static void calc_alpha(HMM *hmm, int seq[MAX_SEQ], int len, double alpha[][MAX_STATE]) {
    // forward
    
    for (int i = 0; i < MAX_STATE; i++) {
        alpha[0][i] = hmm->initial[i] * hmm->observation[seq[0]][i];
    }

    for (int t = 0; t < len - 1; t++) {
        for (int j = 0; j < hmm->state_num; j++) {
            // for alpha[t+1][j]
            double sum = 0;
            for (int i = 0; i < hmm->state_num; i++) {
                sum += alpha[t][i] * hmm->transition[i][j];
            }
            alpha[t+1][j] = sum * hmm->observation[seq[t+1]][j];
        }
    }
}

static void calc_beta(HMM *hmm, int seq[MAX_SEQ], int len, double beta[][MAX_STATE]) {
    // backward
    
    for (int i = 0; i < MAX_STATE; i++) {
        beta[len - 1][i] = 1;
    }

    for (int t = len - 2; t >= 0; t--) {
        for (int i = 0; i < hmm->state_num; i++) {
            // for beta[t][i]
            double sum = 0;
            for (int j = 0; j < hmm->state_num; j++) {
                sum += hmm->transition[i][j] * hmm->observation[seq[t+1]][j] * beta[t+1][j];
            }
            beta[t][i] = sum;
        }
    }
}

static void calc_gamma(HMM *hmm, int seq[MAX_SEQ], int len, double alpha[][MAX_STATE], double beta[][MAX_STATE], double gamma[][MAX_STATE], double first_state[MAX_STATE], double from_state[MAX_STATE], double at_state[MAX_STATE], double at_state_observ[][MAX_OBSERV]) {
    int state_num = hmm->state_num;
    
    for (int t = 0; t < len; t++) {
        double sum = 0;
        for (int _i = 0; _i < state_num; _i++) {
            sum += alpha[t][_i] * beta[t][_i];
        }
        
        for (int i = 0; i < state_num; i++) {
            // for gamma[t][i]
            check0(sum);
            gamma[t][i] = alpha[t][i] * beta[t][i] / sum;
            
            if (t == 0) {
                first_state[i] += gamma[t][i];
            }
            if (t < len - 1) {
                from_state[i] += gamma[t][i];
            }
            at_state_observ[i][seq[t]] += gamma[t][i];
            at_state[i] += gamma[t][i];
        }
    }
}

static void calc_epsilon(HMM *hmm, int seq[MAX_SEQ], int len, double alpha[][MAX_STATE], double beta[][MAX_STATE], double epsilon[][MAX_STATE][MAX_STATE], double from_state_to_state[][MAX_STATE]) {
    int state_num = hmm->state_num;
    
    for (int t = 0; t < len - 1; t++) {
        double sum = 0;
        for (int _i = 0; _i < state_num; _i++) {
            for (int _j = 0; _j < state_num; _j++) {
                sum += alpha[t][_i] * hmm->transition[_i][_j] * hmm->observation[seq[t+1]][_j] * beta[t+1][_j];
            }
        }
        
        for (int i = 0; i < state_num; i++) {
            for (int j = 0; j < state_num; j++) {
                // for epsilon[t][i][j]
                check0(sum);
                epsilon[t][i][j] = alpha[t][i] * hmm->transition[i][j] * hmm->observation[seq[t+1]][j] * beta[t+1][j] / sum;
                
                from_state_to_state[i][j] += epsilon[t][i][j];
            }
        }
    }
}

static void update(HMM *hmm, int n_seq, double first_state[MAX_STATE], double from_state[MAX_STATE], double from_state_to_state[][MAX_STATE], double at_state[MAX_STATE], double at_state_observ[][MAX_OBSERV]) {
    for (int i = 0; i < hmm->state_num; i++) {
        hmm->initial[i] = first_state[i] / n_seq;
    }
    
    for (int i = 0; i < hmm->state_num; i++) {
        for (int j = 0; j < hmm->state_num; j++) {
            check0(from_state[i]);
            hmm->transition[i][j] = from_state_to_state[i][j] / from_state[i];
        }
    }
    
    for (int k = 0; k < hmm->observ_num; k++) {
        for (int j = 0; j < hmm->state_num; j++) {
            check0(at_state[j]);
            hmm->observation[k][j] = at_state_observ[j][k] / at_state[j];
        }
    }
}

static void Baum_Welch(HMM *hmm, FILE *input) {
    double alpha[MAX_SEQ][MAX_STATE];
    double beta[MAX_SEQ][MAX_STATE];
    double gamma[MAX_SEQ][MAX_STATE];
    double epsilon[MAX_SEQ][MAX_STATE][MAX_STATE];

    double first_state[MAX_STATE] = {0};
    double from_state[MAX_STATE] = {0};
    double at_state[MAX_STATE] = {0};
    double at_state_observ[MAX_STATE][MAX_OBSERV] = {{0}};
    double from_state_to_state[MAX_STATE][MAX_STATE] = {{0}};
    
    int seq[MAX_SEQ];
    int n_seq = 0;
    fseek(input, 0, SEEK_SET);
    int len = 0;
    int SPACE = hmm->state_num;
    while (1) {
        int res = fscanf(input, "%d", &seq[len]);
        if (res <= 0 || seq[len] == SPACE) {
            calc_alpha(hmm, seq, len, alpha);
            calc_beta(hmm, seq, len, beta);
            calc_gamma(hmm, seq, len, alpha, beta, gamma, first_state, from_state, at_state, at_state_observ);
            calc_epsilon(hmm, seq, len, alpha, beta, epsilon, from_state_to_state);
            
            len = 0;
            n_seq++;
            
            if (res <= 0) {
                break;
            }
        }
        else {
            len++;
        }
    }
    
    update(hmm, n_seq, first_state, from_state, from_state_to_state, at_state, at_state_observ);
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
