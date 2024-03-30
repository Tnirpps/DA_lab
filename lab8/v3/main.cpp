#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <math.h>

using namespace std;

int main () {
    size_t n;
    cin >> n;
    vector<int64_t> v(n);
    for (int64_t & x : v) {
        cin >> x;
    }
    sort(v.rbegin(), v.rend());
    double max_s = 0;
    int ind_max_s = -1;
    for (size_t i = 0; i + 2 < n; ++i) {
        if (!(v[i] < v[i + 1] + v[i + 2])) continue;
        double p = (v[i] + v[i + 1] + v[i + 2])/2.0;
        double cur_s = sqrt(p * (p - v[i]) * (p - v[i + 1]) * (p - v[i + 2]));
        if (cur_s > max_s) {
            max_s = cur_s;
            ind_max_s = i;
        }
    }
    if (ind_max_s == -1) {
        cout << "0\n";
    } else {
        cout << fixed << setprecision(3) << max_s << '\n';
        cout << v[ind_max_s + 2] << ' '  << v[ind_max_s + 1]  << ' ' <<  v[ind_max_s] << '\n';
    }
    return 0;
}
