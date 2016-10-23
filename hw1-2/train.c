#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "hmm.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./train DIR #ITER[default 1]\n");
        printf("(Assume there are DIR/test.num and DIR/encode.bin)\n");
        exit(-1);
    }
    
    int iter = (argc == 2) ? 1 : atoi(argv[2]);
    char dir_name[MAX_LINE], test_num_name[MAX_LINE], encode_bin_name[MAX_LINE];
    strcpy(dir_name, argv[1]);
    strcpy(test_num_name, dir_name);
    strcpy(encode_bin_name, dir_name);
    strcat(test_num_name, "/test.num");
    strcat(encode_bin_name, "/encode.bin");
    
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

    HMM hmm, prev;
    hmm.state_num = hmm.observ_num = vocab;
    prev.state_num = prev.observ_num = vocab;
    init_model(&hmm, encode);
    for (int i = 0; i < iter; i++) {
        Baum_Welch(&hmm, test_num);
        if (i > 0) {
            double diff = diff_model(&hmm, &prev), max = 0, acc = calc_acc(&hmm, dir_name, "pred_tmp.num", &max);
            printf("iter:%d, diff:%lf, acc: %lf, max_prob_log: %g\n", i, diff, acc, max);
        }
        copy_model(&prev, &hmm);
    }
    printf("train: %s, iter=%d ok\n", dir_name, iter);
    
    fclose(test_num);
    fclose(encode_bin);
    
    return 0;
}
