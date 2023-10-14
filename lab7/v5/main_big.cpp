#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>

using namespace std;

int main() {
    vector<vector<int>> d = {{1, -1}, {1, 0}, {1, 1}};
    int64_t n, m;
    cin >> n >> m;
    vector<vector<int64_t>> mat (n, vector<int64_t>(m));
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < m; ++j) {
            cin >> mat[i][j];
        }
    }

    for (int i = n - 1; i >= 0; --i) {
        for (int j = 0; j < m; ++j) {
            int64_t tmp = INT64_MAX;
            for (size_t k = 0; k < d.size(); ++k) {
                if ((i + d[k][0]) >= 0 && (i + d[k][0]) < n && j + d[k][1] >= 0 && j + d[k][1] < m) {
                    tmp = min(tmp, mat[i + d[k][0]][j + d[k][1]]);
                }
            }
            mat[i][j] += (tmp == INT64_MAX) ? 0 : tmp;
        }
    }

    int index = 0;
    for (size_t i = 0; i < m; i++) {
        if (mat[0][index] > mat[0][i]) {
            index = i;
        }
    }
    cout << mat[0][index] << "\n(1," << index + 1 << ")";
    for (int i = 0; i < n - 1; ++i) {
        int64_t tmp = INT64_MAX, next_index;
        for (int k = 0; k < d.size(); ++k) {
            if ((i + d[k][0]) >= 0 && (i + d[k][0]) < n && index + d[k][1] >= 0 && index + d[k][1] < m) {
                if (tmp > mat[i + d[k][0]][index + d[k][1]]) {
                    tmp = mat[i + d[k][0]][index + d[k][1]];
                    next_index = index + d[k][1];
                }
            }
        }
        index = next_index;
        cout << " (" << (i + 1) + 1 << "," << index + 1 << ")";
    }
    cout << '\n';
}

