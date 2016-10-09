#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

using namespace std;

#define WRITE_FILE true
#define ALNUM "0123456789abcdefghijklmnopqrstuvwxyz"
#define LEN_ALNUM 37

#define INF -log(0);
double bigram[LEN_ALNUM][LEN_ALNUM], encode[LEN_ALNUM][LEN_ALNUM];

// 0-9, a-z, ' '
int alnum_index(char c) {
    if (c == ' ') {
        return 36;
    }
    else if ('0' <= c && c <= '9') {
        return c - '0';
    }
    else {
        return c - 'a' + 10;
    }
}

void read_file(double table[LEN_ALNUM][LEN_ALNUM], string filename) {
    ifstream fin(filename);
    string line;
    while (getline(fin, line)) {
        int i1 = alnum_index(line[0]),
            i2 = alnum_index(line[2]);
        stringstream sin(line.substr(4));
        double prob;
        sin >> prob;
        table[i1][i2] = log(prob);
    }
    
    return;
}

void update(double old_prob[LEN_ALNUM], string old_str[LEN_ALNUM],
            double new_prob[LEN_ALNUM], string new_str[LEN_ALNUM],
            string prev_list, string this_list, int cipher_index) {
    
    for (size_t i = 0; i < this_list.size(); i++) {
        char this_plain = this_list[i];
        int this_index = alnum_index(this_plain);
        double max_prob = -INF;
        int max_prev_index = -1;
        
        for (size_t j = 0; j < prev_list.size(); j++) {
            char prev_plain = prev_list[j];
            int prev_index = alnum_index(prev_plain);
            double prob = old_prob[prev_index] + bigram[prev_index][this_index];
            if (prob > max_prob) {
                max_prob = prob;
                max_prev_index = prev_index;
            }
        }
        
        new_prob[this_index] = max_prob + encode[this_index][cipher_index];
        new_str[this_index] = old_str[max_prev_index];
        if (this_plain != ' ') {
            new_str[this_index] += this_plain;
        }
    }
    
    return;
}

void decode(string cipher_word, ostream& out) {
    cipher_word += ' ';
    // alternately being old or new
    double prob[2][LEN_ALNUM] = {{0.0}};
    string str[2][LEN_ALNUM] = {{""}};
    int OLD = 1, NEW = 0;
    
    for (size_t i = 0; i < cipher_word.size(); i++) {
        char cipher = cipher_word[i];
        int cipher_index = alnum_index(cipher);
        string prev_list = (i == 0 ? " " : ALNUM);
        string this_list = (i == cipher_word.size() - 1 ? " " : ALNUM);
        OLD = 1 - OLD;
        NEW = 1 - NEW;
        update(prob[OLD], str[OLD], prob[NEW], str[NEW], prev_list, this_list, cipher_index);
    }
    
    out << str[NEW][alnum_index(' ')];
    return;
}

int main() {
    
    for (int i = 0; i < LEN_ALNUM; i++) {
        for (int j = 0; j < LEN_ALNUM; j++) {
            bigram[i][j] = -INF;
            encode[i][j] = 0.0;
        }
    }
    
    read_file(bigram, "bigram.txt");
    read_file(encode, "encode.txt");
    
    ifstream fin("test.txt");
    ofstream fout("pred.txt");
    char c;
    string s = "";
    fin >> noskipws;
    ostream& out = WRITE_FILE ? fout : cout;
    while (fin >> c) {
        if (c == ' ' || c == '\n') {
            decode(s, out);
            out << c;
            s = "";
        }
        else {
            s += c;
        }
    }

    return 0;
}