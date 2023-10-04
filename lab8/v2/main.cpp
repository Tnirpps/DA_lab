#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct TSegment {
    int64_t l, r;
    size_t index;
};

int main () {
    size_t n;
    cin >> n;
    vector<TSegment> v(n), ans;
    ans.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        cin >> v[i].l >> v[i].r;
        v[i].index = i;
    }
    sort(v.begin(), v.end(), 
        [](const TSegment& lhs, const TSegment& rhs) {
            return lhs.l < rhs.l;
        });
    
    int64_t cover(0), ind(0), m;
    cin >> m;
    while (cover < m) {
        int64_t tmp(0), tmp_i(-1);
        while (ind < n && v[ind].l <= cover) {
            if (v[ind].r - cover > tmp) {
                tmp = v[ind].r - cover;
                tmp_i = ind;
            }
            ++ind;
        }
        if (tmp_i != -1) {
            ans.push_back(v[tmp_i]);
            cover = v[tmp_i].r;
        } else {
            break;
        }
    }
    if (cover < m) {
        ans.clear();
    }
    cout << ans.size() << '\n';
    sort(ans.begin(), ans.end(), 
    [](const TSegment& lhs, const TSegment& rhs) {
        return lhs.index < rhs.index;
    });
    for (const TSegment& x: ans) {
        cout << x.l << ' ' << x.r << '\n';
    }
    
    return 0;
}

