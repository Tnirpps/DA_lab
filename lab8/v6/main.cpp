#include <iostream>
#include <vector>

using namespace std;

void dfs(const vector<vector<int>>& gr, int u, vector<bool>& vis, vector<int>& ans) {
    if (vis[u]) return;
    vis[u] = true;
    for (int v: gr[u]) {
        if (!vis[v]) dfs(gr, v, vis, ans);
    }
    ans.push_back(u);
}

vector<int> topological_sort(const vector<vector<int>>& gr) {
    vector<bool> vis(gr.size(), false);
    vector<int> ans;
    for (int i = 0; i < gr.size(); i++) {
        dfs(gr, i, vis, ans);
    }
    return ans;
}

int main () {
    int n, m, u, v;
    cin >> n >> m;
    vector<vector<int>> graph(n);
    // read graph
    for (int i = 0; i < m; ++i) {
        cin >> u >> v;
        graph[u - 1].push_back(v - 1);
    }
    // calc assumed answer
    vector<int> ans = topological_sort(graph); // reversed
    // check that answer
    vector<int> ans_inv(ans.size());
    bool correct_ans = true;
    for (int i = ans.size(); i > 0; --i) {
        ans_inv[ans[i - 1]] = ans.size() - i;
    }
    for (int u = 0; u < n; ++u) {
        for (int v: graph[u]) {
            correct_ans &= (ans_inv[u] < ans_inv[v]);
        }
    }
    // output result
    if (!correct_ans) ans = {-2};
    for (int i = ans.size(); i > 0; --i) {
        cout << ans[i - 1] + 1 << " \n"[i==0];
    }
    return 0;
}

