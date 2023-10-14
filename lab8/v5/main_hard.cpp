#include <iostream>
#include <vector>

using namespace std;

void dfs_cycle(vector<vector<size_t>>& mtrx, size_t u, size_t p, vector<int>& color, vector<size_t>& par, vector<vector<size_t>>& cycles) {
    if (color[u] == 2) return;
    if (color[u] == 1) {
        vector<size_t> v;
        size_t cur = p;
        v.push_back(u);
        v.push_back(cur);
        while (cur != u) {
            cur = par[cur];
            v.push_back(cur);
        }
        cycles.push_back(v);
        return;
    }
    par[u] = p;
    color[u] = 1;
    for (size_t v = 0; v < mtrx.size(); ++v) {
        if (mtrx[u][v] == 0) continue;
        dfs_cycle(mtrx, v, u, color, par, cycles);
    }
    color[u] = 3;
}

int main () {
    const size_t mx_d = 3;
    size_t n, tmp;
    cin >> n;

    vector<int> v(n);
    size_t cnt [mx_d] = {0};
    vector<vector<size_t>> pos (mx_d, vector<size_t> (mx_d));

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
    vector<size_t> par(mx_d);
    vector<int> color(mx_d);
    vector<vector<size_t>> cycles;
    for (size_t i = 0; i < mx_d; ++i) {
        if (color[i] == 0) dfs_cycle(pos, i, i, color, par, cycles);
    }
    for (vector<size_t> c: cycles) {
        size_t m = pos[c[1]][c[0]];
        for (size_t i = 0; i + 1 < c.size(); ++i) {
            m = min(m, pos[c[i + 1]][c[i]]);
        }
        for (size_t i = 0; i + 1 < c.size(); ++i) {
            pos[c[i + 1]][c[i]] -= m;
        }
        ans += (c.size() - 2) * m;
    }
    cout << ans << '\n';
    return 0;
}

