#include <stdio.h>
#include <string.h>
#include "hmm.h"

double Viterbi(HMM *hmm, int seq[MAX_SEQ], int len) {
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./test DIR\n");
        printf("(Assume there are DIR/model.txt and DIR/test.num)\n");
        exit(-1);
    }
    
    char dir_name[MAX_LINE], model_name[MAX_LINE], test_num_name[MAX_LINE], pred_num_name[MAX_LINE];
    strcpy(dir_name, argv[1]);
    strcpy(model_name, dir_name);
    strcpy(test_num_name, dir_name);
    strcpy(pred_num_name, dir_name);
    strcat(model_name, "/model.txt");
    strcat(test_num_name, "/test.num");
    strcat(pred_num_name, "/pred.num");
    
    HMM hmm;
    loadHMM(&hmm, model_name);
    
    FILE *test_num = open_or_die(test_num_name, "r");
    FILE *pred_num = open_or_die(pred_num_name, "w");
    int seq[MAX_SEQ];
    int SPACE = hmm.state_num;
    int len = 0;
    while (1) {
        int res = fscanf(test_num, "%d", &seq[len]);
        if (res <= 0 || seq[len] == SPACE) {
            Viterbi(&hmm, seq, len);
            for (int i = 0; i <= len; i++) {
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
    
    return 0;
}