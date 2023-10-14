#include <iostream>
#include <vector>

using namespace std;

int main () {
    const size_t mx_d = 3;
    size_t n, tmp;
    cin >> n;

    vector<int> v(n);
    size_t cnt [mx_d] = {0};
    size_t pos [mx_d][mx_d] = {0};

    for (size_t i = 0; i < n; ++i) {
        cin >> v[i];
        ++cnt[--v[i]];
    }

    pair<size_t, size_t> intervals [mx_d];
    tmp = 0;
    for (size_t i = 0; i < mx_d; ++i) {
        intervals[i] = {tmp, tmp + cnt[i]};
        tmp += cnt[i];
    }

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < mx_d; ++j) {
            if (intervals[j].first <= i && i < intervals[j].second && v[i] != j) {
                ++pos[v[i]][j];
            }
        }
    }

    size_t ans = 0;
    for (size_t i = 0; i < mx_d; ++i) {
        for (size_t j = 0; j < mx_d; ++j) {
            tmp = min(pos[i][j], pos[j][i]);
            pos[i][j] -= tmp;
            pos[j][i] -= tmp;
            ans += tmp;
        }
    }
    for (size_t i = 0; i < mx_d; ++i) {
        ans += 2 * pos[0][i];
    }
    cout << ans << "\n";
    return 0;
}
