#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "hmm.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./train DIR #ITER[default 1000]\n");
        printf("(Assume there are DIR/test.num and DIR/encode.bin)\n");
        exit(-1);
    }
    
    int iter = (argc == 2) ? 1000 : atoi(argv[2]);
    char dir_name[MAX_LINE], test_num_name[MAX_LINE], encode_bin_name[MAX_LINE];
    strcpy(dir_name, argv[1]);
    add_file_name(test_num_name, dir_name, "/test.num");
    add_file_name(encode_bin_name, dir_name, "/encode.bin");
    
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
    double max_prob_log = log(0); // -inf
    int VALID = !strncmp(dir_name, "valid", 5);
    for (int i = 0; i < iter; i++) {
        Baum_Welch(&hmm, test_num);
        double prob_log = 0, acc = calc_acc(&hmm, dir_name, &prob_log);
        if (VALID) {
            printf("%s: iter=%d, acc=%lf, max_prob_log=%g\n", dir_name, i, acc, prob_log);
        }
        else {
            printf("%s: iter=%d, max_prob_log=%g\n", dir_name, i, prob_log);
        }
        
        if (prob_log > max_prob_log) {
            max_prob_log = prob_log;
            
            char pred_tmp_num_name[MAX_LINE], pred_num_name[MAX_LINE];
            add_file_name(pred_tmp_num_name, dir_name, "/pred_tmp.num");
            add_file_name(pred_num_name, dir_name, "/pred.num");
            FILE *pred_tmp_num = open_or_die(pred_tmp_num_name, "r");
            FILE *pred_num = open_or_die(pred_num_name, "w");
            
            int num;
            while (fscanf(pred_tmp_num, "%d", &num) > 0) {
                fprintf(pred_num, "%d ", num);
            }
            fprintf(pred_num, "\n");
            
            fclose(pred_tmp_num);
            fclose(pred_num);
        }
    }
    printf("train: %s ok\n", dir_name);
    
    fclose(test_num);
    fclose(encode_bin);
    
    return 0;
}
