#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    int n, m;
    cin >> n >> m;
    vector<vector<int64_t>> mat(n, vector<int64_t>(m));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            cin >> mat[i][j];
        }
    }

    for (int i = n - 1; i >= 1; --i) {
        for (int j = 0; j < m; ++j) {
            mat[i - 1][j] += *min_element(mat[i].begin() + max(0, j - 1), mat[i].begin() + min(j + 1, m - 1) + 1);
        }
    }

    int index = distance(mat[0].begin(), min_element(mat[0].begin(), mat[0].end()));
    cout << mat[0][index] << "\n(1," << index + 1 << ")";
    for (int i = 1; i < n; ++i) {
        index = distance(mat[i].begin(), min_element(
                                        mat[i].begin() + max(0, index - 1),
                                        mat[i].begin() + min(index + 1, m - 1) + 1));
        cout << " (" << i + 1 << "," << index + 1 << ")";
    }
    cout << '\n';
    return 0;
}








/*
https://github.com/Brinckley/DA_Labs/blob/main/Lab7/Lab7.cpp
https://github.com/kaefman/DA/blob/main/lab7/main.cpp
https://github.com/ZergTricky/DA/blob/main/lab7/main.cpp
https://github.com/muvekat/discreteanalysis/blob/master/lab7.cpp
https://github.com/whitewolf185/DA-labs/blob/master/DA%20lab7/main.cpp
https://github.com/Netter2/DA-labs/blob/main/Lab7/main.cpp
https://github.com/AppCrashExpress/DA-labs/blob/master/DA-lab-7/src/main.cpp
https://github.com/Prianikq/DA-Labs/blob/main/Lab7/Lab7.pdf
*/