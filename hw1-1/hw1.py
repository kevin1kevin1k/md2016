from math import log
import sys

FILE = True
INF = float('inf')

bigram = {}
encode = {}
alphabet = [chr(i) for i in range(ord('a'), ord('z') + 1)]
number = [str(i) for i in range(0, 10)]
alnum = alphabet + number

def mylog(f):
    if f == 0.0:
        return -INF
    return log(f)

with open('bigram.txt') as bigram_file:
    for line in bigram_file:
        a, b, p = line[0], line[2], line[4:]
        bigram[a + b] = mylog(float(p))

with open('encode.txt') as encode_file:
    for line in encode_file:
        a, b, p = line[0], line[2], line[4:]
        encode[a + b] = mylog(float(p))

with open('pred.txt', 'w') as pred_file:
    with open('test.txt') as test_file:
        for line in test_file:
            for cipher_word in line.rstrip().split(' '):
                old_prob = {' ': 1.0}
                old_str = {' ': ''}
                
                for i, cipher in enumerate(cipher_word + ' '):
                    new_prob = {}
                    new_str = {}
                    prev_list = [' '] if i == 0 else alnum
                    this_list = [' '] if i == len(cipher_word) else alnum
                    
                    for this_plain in this_list:
                        max_prob = -INF
                        for prev_plain in prev_list:
                            prob = old_prob[prev_plain] + bigram.get(prev_plain + this_plain, -INF)
                            if prob > max_prob:
                                max_prob = prob
                                max_prev = prev_plain
                        
                        new_prob[this_plain] = max_prob + encode[this_plain + cipher]
                        new_str[this_plain] = old_str[max_prev] + this_plain
                    
                    old_prob = dict(new_prob)
                    old_str = dict(new_str)
                
                if FILE:
                    pred_file.write(new_str[' '])
                else:
                    print new_str[' '][:-1],
                    sys.stdout.flush()
            
            if FILE:
                pred_file.write('\n')
            else:
                print
