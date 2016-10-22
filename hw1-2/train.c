#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "hmm.h"

void check0(double d) {
    if (d == 0) {
        printf("Error: divided by zero\n");
        exit(-1);
    }
}

void calc_alpha(HMM *hmm, int seq[MAX_SEQ], int len, double alpha[][MAX_STATE]) {
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

void calc_beta(HMM *hmm, int seq[MAX_SEQ], int len, double beta[][MAX_STATE]) {
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

void calc_gamma(HMM *hmm, int seq[MAX_SEQ], int len, double alpha[][MAX_STATE], double beta[][MAX_STATE], double gamma[][MAX_STATE], double first_state[MAX_STATE], double from_state[MAX_STATE], double at_state[MAX_STATE], double at_state_observ[][MAX_OBSERV]) {
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

void calc_epsilon(HMM *hmm, int seq[MAX_SEQ], int len, double alpha[][MAX_STATE], double beta[][MAX_STATE], double epsilon[][MAX_STATE][MAX_STATE], double from_state_to_state[][MAX_STATE]) {
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

void update(HMM *hmm, int n_seq, double first_state[MAX_STATE], double from_state[MAX_STATE], double from_state_to_state[][MAX_STATE], double at_state[MAX_STATE], double at_state_observ[][MAX_OBSERV]) {
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

void Baum_Welch(HMM *hmm, FILE *input) {
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

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: ./train DIR model.txt #ITER[default 1]\n");
        printf("(Assume there are DIR/test.num and DIR/encode.bin)\n");
        exit(-1);
    }
    
    int iter = (argc == 3) ? 1 : atoi(argv[3]);
    char dir_name[MAX_LINE], test_num_name[MAX_LINE], encode_bin_name[MAX_LINE], model_name[MAX_LINE];
    strcpy(dir_name, argv[1]);
    strcpy(test_num_name, dir_name);
    strcpy(encode_bin_name, dir_name);
    strcpy(model_name, dir_name);
    strcat(test_num_name, "/test.num");
    strcat(encode_bin_name, "/encode.bin");
    strcat(model_name, "/");
    strcat(model_name, argv[2]);
    
    FILE *test_num = open_or_die(test_num_name, "r");
    FILE *encode_bin = open_or_die(encode_bin_name, "r");
    
    int state, observ, poss;
    int vocab = 0;
    int encode[MAX_OBSERV][MAX_STATE];
    while (fscanf(encode_bin, "%d %d %d", &state, &observ, &poss) > 0) {
        encode[observ][state] = poss;
        if (state > vocab) {
            vocab = state;
        }
    }

    HMM hmm;
    hmm.state_num = hmm.observ_num = vocab;
    init_model(&hmm, encode);
    for (int i = 0; i < iter; i++) {
        Baum_Welch(&hmm, test_num);
    }
    printf("%s ok\n", dir_name);
    
    FILE *model = open_or_die(model_name, "w");
    dumpHMM(model, &hmm);
    
    fclose(test_num);
    fclose(encode_bin);
    fclose(model);
    
    return 0;
}
