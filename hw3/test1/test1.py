
# coding: utf-8

import numpy as np

def file_to_mat(filename, m, n):
    mat = np.zeros( (m, n) )
    user = []
    item = []
    rats = []
    
    with open(filename) as f:
        for line in f:
            uir = line.split()
            u, i, r = int(uir[0]), int(uir[1]), float(uir[2])
            user.append(u)
            item.append(i)
            rats.append(r)
            mat[u, i] = r

    lst = list(zip(user, item))
    
    return mat, lst


M = 50000
N = 5000

R1, R1_lst = file_to_mat('train.txt', M, N)   # target domain
R2, R2_lst = file_to_mat('source.txt', M, N)  # source domain


rate = 0.05
lamb = 0.01 # different for P, Q ???
eps = 1.0
K = 10


from time import time

def PQ(R, user_item, k=10, maxiter=1000):    
    m, n = R.shape
    P = np.random.rand(m, k)
    Q = np.random.rand(n, k)
    prevsum = -1
    iters = 0
    while iters < maxiter:
        start = time()

        diffsum = 0
        iters += 1
        for i, j in user_item:
            diff = P[i].dot(Q[j]) - R[i, j]
            gP = diff * Q[j] + lamb * P[i]
            gQ = diff * P[i] + lamb * Q[j]
            diffsum += diff ** 2

            P[i] -= rate * gP
            Q[j] -= rate * gQ

        end = time()
        print('iter {} diff {} time {}'.format(iters, diffsum, end - start))

        if abs(prevsum - diffsum) < eps:
            break
        prevsum = diffsum
    
    return P, Q


def error(R, P, Q, user_item):
    s = 0
    for i, j in user_item:
        s += (P[i].dot(Q[j]) - R[i, j]) ** 2
    
    return s


P1, Q1 = PQ(R1, R1_lst, maxiter=2000)
P2, Q2 = PQ(R2, R2_lst, maxiter=2000)


print(error(R1, P1, Q1, R1_lst))
print(error(R2, P2, Q2, R2_lst))


def SVD(X):
    U, d, V = np.linalg.svd(X, full_matrices=False) # full_matrices ???
    D = np.diag(d)
    V = V.T
    
    return U, D, V


def PQ2SVD(P, Q):
    U_P, D_P, V_P = SVD(P)
    U_Q, D_Q, V_Q = SVD(Q)

    X = D_P.dot(V_P.T).dot(V_Q).dot(D_Q.T)
    U_X, D_X, V_X = SVD(X)

    U = U_P.dot(U_X)
    D = D_X
    V = U_Q.dot(V_X)

    return U, D, V


U1, D1, V1 = PQ2SVD(P1, Q1)
U2, D2, V2 = PQ2SVD(P2, Q2)


# without using T for caching
def NN(Z1, Z2):
    # G = argmin | G * Z2 - Z1 |
    #   = argmin | Z2^T * G^T - Z1^T |
    # G^T ~ (Z2^T^+) * Z1^T
    # G ~ [ (Z2^T^+) * Z1^T ]^T
    G = np.linalg.pinv(Z2.T).dot(Z1.T).T
    
    # alpha = min | G * Z2 - Z1 |
    alpha = np.linalg.norm(G.dot(Z2) - Z1) # without squaring
    
    return alpha, G


# without using T for caching
def Matching(Z1, Z2):
    S = np.eye(K)
    G = []
    for k in range(K):
        S[k, k] = +1
        alpha_plus,  G_plus  = NN(Z1[:, :k+1], Z2[:, :k+1].dot(S[:k+1, :k+1]))
        S[k, k] = -1
        alpha_minus, G_minus = NN(Z1[:, :k+1], Z2[:, :k+1].dot(S[:k+1, :k+1]))
        
        if alpha_plus <= alpha_minus:
            G.append(G_plus)
            S[k, k] = +1
        else:
            G.append(G_minus)
            S[k, k] = -1
    
    return G


def Algo1(U1, D1, V1, U2, D2, V2, R1, R1_lst):
    G_user = Matching(U1.dot(np.sqrt(D1)), U2.dot(np.sqrt(D2)))
    G_item = Matching(V1.dot(np.sqrt(D1)), V2.dot(np.sqrt(D2)))
    
    errors = []
    R2_hat = U2.dot(D2).dot(V2.T)
    for k in range(K):
        print('iter {} starts'.format(k))
        R1_hat = G_user[k].dot(R2_hat).dot(G_item[k].T)
        diff = R1 - R1_hat
        error = 0
        for i, j in R1_lst:
            error += diff[i, j] ** 2
        errors.append(error)
        print('iter {} ends'.format(k))
    
    k_min = np.argmin(errors)
    return G_user[k_min], G_item[k_min]


Gu, Gi = Algo1(U1, D1, V1, U2, D2, V2, R1, R1_lst)


R1_hat = U1.dot(D1).dot(V1.T)
R2_hat = U2.dot(D2).dot(V2.T)
R = ( R1_hat + Gu.dot(R2_hat).dot(Gi.T) ) / 2

lst = []
for i in range(50000):
    for j in range(5000):
        if R[i, j] != 0:
            lst.append( (i, j) ) 
P, Q = PQ(R, lst, maxiter=2000)


def predict(P, Q):
    with open('test.txt', 'r') as test, open('pred.txt', 'w') as output:
        for line in test:
            u, i, q = line.split()
            u = int(u)
            i = int(i)
            output.write('%d %d %.3f\n' % (u, i, P[u].dot(Q[i])))


##### Baseline 
# predict(P1, Q1)

##### Paper's approach
predict(P, Q)

