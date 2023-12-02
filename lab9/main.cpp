#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>


using namespace std;
using var =  int64_t;
using graph = vector<vector<int>>;

void bfs(int u, const graph& g, vector<char>& d, graph& ans) {
    queue<int> q;
    q.push(u);
    ans.push_back({});
    ans.back().push_back(u);
    d[u] = 1;
    while (!q.empty()) {
        u = q.front();
        q.pop();
        for (int v: g[u]) {
            if (d[v] == 0) {
                d[v] = 1;
                ans.back().push_back(v);
                q.push(v);
            }
        }
    }
}

void solve() {
    int n, m;
    cin >> n >> m;
    graph g(n);
    for (int i = 0; i < m; ++i) {
        int u, v;
        cin >> u >> v;
        --u;--v;
        g[u].push_back(v);
        g[v].push_back(u);
    }
    
    graph ans;
    ans.reserve(n);
    vector<char> d(n, 0);
    for (int i = 0; i < n; ++i) {
        if (!d[i]) {
            bfs(i, g, d, ans);
            sort(ans.back().begin(), ans.back().end());
        }
    }
    sort(ans.begin(), ans.end());
    for (const auto &x: ans) {
        for (auto y: x) {
            cout << y + 1 << ' ';
        }
        cout << '\n';
    }
}

int main() {
    ios_base::sync_with_stdio(false);
    cout.tie(nullptr);
    cin.tie(nullptr);
    int tt(1);
    //cin >> tt;
    while (tt--) {
        solve();
    }
    return 0;
}

