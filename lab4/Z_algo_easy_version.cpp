#include <iostream>
#include <vector>
#include <string>

using namespace std;

vector<int> Zfunction(const string & s) {
    int n = s.size();
    vector<int> z (n, 0);
    int l = -1;
    int r = -1;
    z[0] = n;
    for (int i = 1; i < n; ++i) {
        if (i <= r) {
            z[i] = min(r - i, z[i - l]);
        }

        while ( i + z[i] < n && s[z[i]] == s[i + z[i]]) {
            ++z[i];
        }
        if (i + z[i] > r) {
            r = i + z[i];
            l = i;
        }
    }
    return z;
}

int main() {
    ios_base::sync_with_stdio(false);
    cout.tie(nullptr);
    cin.tie(nullptr);
    string pattern;
    string text;
    cin >> text >> pattern;
    text = pattern + "$" + text;
    vector<int> zVectro = Zfunction(text);
    for (int i = pattern.size(); i < text.size(); ++i) {
        if (zVectro[i] == pattern.size()) {
            cout << i - pattern.size() - 1 << "\n";
        }
    }
    return 0;
}

