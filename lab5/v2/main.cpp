#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class TSuffArray {
private:
    vector<int> str;
    vector<int> sa;
    vector<int> lcp_arr;
    vector<int> lcp_query;
    bool ready_to_find = false;

    vector<int> string_to_bites(const string& s) {
        vector<int> res(s.size());
        for (int i = 0; i < s.size(); ++i) {
            res[i] = (int)s[i];
        }
        return res;
    }

    vector<bool> get_types(const vector<int>& s) {
        vector<bool> l_type(s.size() + 1, false);
        if (l_type.size() == 1) return l_type;

        l_type[l_type.size() - 2] = true;
        for (int i = s.size() - 2; i >= 0; --i) {
            if (s[i] > s[i + 1] || (s[i] == s[i + 1] && l_type[i + 1])) {
                l_type[i] = true;
            }
        }
        return l_type;
    }

    bool is_lms(const vector<bool>& l_type, int ind) {
        if (ind == 0) return false;
        if (!l_type[ind] && l_type[ind - 1]) return true;
        return false;
    }

    bool lms_substr_eq(const vector<int>& s, const vector<bool>& l_type, int ind_a, int ind_b) {
        if (ind_a == s.size() || ind_b == s.size()) return false;
        int i = 0;
        while (true) {
            bool lms_a = is_lms(l_type, ind_a + i);
            bool lms_b = is_lms(l_type, ind_b + i);

            if (i > 0 && lms_a && lms_b) {
                return true;
            }

            if (lms_a != lms_b || s[ind_a + i] != s[ind_b + i]) {
                return false;
            }
            ++i;
        }
    }

    vector<int> count_bucket_sizes(const vector<int>& s, size_t alphabet_size) {
        vector<int> res(alphabet_size);
        for (int c: s) {
            ++res[c];
        }
        return res;
    }

    vector<int> find_bucket_heads(const vector<int>& buckets) {
        vector<int> res(buckets.size());
        int offset = 1;
        for (int i = 0; i < res.size(); ++i) {
            res[i] = offset;
            offset += buckets[i];
        }
        return res;
    }

    vector<int> find_bucket_tails(const vector<int>& buckets) {
        vector<int> res(buckets.size());
        int offset = 1;
        for (int i = 0; i < res.size(); ++i) {
            offset += buckets[i];
            res[i] = offset - 1;
        }
        return res;
    }

    vector<int> sort_lms(const vector<int>& s, const vector<bool>& l_type, const vector<int>& buckets) {
        vector<int> s_arr(s.size() + 1, -1);
        vector<int> bucket_tails = find_bucket_tails(buckets);
        for (int i = 0; i < s.size(); ++i) {
            if (!is_lms(l_type, i)) continue;

            int bucket_ind = s[i];
            s_arr[bucket_tails[bucket_ind]] = i;
            --bucket_tails[bucket_ind];
        }
        s_arr[0] = s.size();
        return s_arr;
    }

    void induce_sort_l(const vector<int>& s, vector<int>& s_arr, const vector<bool>& l_type, const vector<int>& buckets) {
       vector<int> bucket_heads = find_bucket_heads(buckets);
       for (int i = 0; i < s_arr.size(); ++i) {
           if (s_arr[i] <= 0) continue;
           int j = s_arr[i] - 1;
           if (!l_type[j]) continue;
           int bucket_ind = s[j];
           s_arr[bucket_heads[bucket_ind]] = j;
           ++bucket_heads[bucket_ind];
       }
    }

    void induce_sort_s(const vector<int>& s, vector<int>& s_arr, const vector<bool>& l_type, const vector<int>& buckets) {
       vector<int> bucket_tails = find_bucket_tails(buckets);
       for (int i = s_arr.size() - 1; i >= 0; --i) {
           if (s_arr[i] == 0) continue;
           int j = s_arr[i] - 1;
           if (l_type[j]) continue;
           int bucket_ind = s[j];
           s_arr[bucket_tails[bucket_ind]] = j;
           --bucket_tails[bucket_ind];
       }
    }

    int rename_lms_substr(const vector<int>& s, const vector<int>& s_arr, const vector<bool>& l_type, vector<int>& new_s, vector<int>& pos_new_s) {
        vector<int> lms_names(s.size() + 1, -1);
        int current_name = 0;
        int last_lms_suffix_offset = -1;
        lms_names[s_arr[0]] = current_name;
        last_lms_suffix_offset = s_arr[0];
        for (int i = 1; i < s.size(); ++i) {
            int suffix_offset = s_arr[i];
            if (!is_lms(l_type, suffix_offset)) continue;
            if (!lms_substr_eq(s, l_type, last_lms_suffix_offset, suffix_offset)) {
                ++current_name;
            }
            last_lms_suffix_offset = suffix_offset;
            lms_names[suffix_offset] = current_name;
        }

        new_s.clear();
        pos_new_s.clear();
        for (int i = 0; i < lms_names.size(); ++i) {
            if (lms_names[i] == -1) continue; 
            new_s.push_back(lms_names[i]);
            pos_new_s.push_back(i);
        }
        return current_name + 1;
    }

    vector<int> fix_lms_sort(const vector<int>& s, int alphabet_size) {
        if (alphabet_size == s.size()) {
            vector<int> res(s.size() + 1, -1);
            res[0] = s.size();
            for (int i = 0; i < s.size(); ++i) {
                int j = s[i];
                res[j + 1] = i;
            }
            return res;
        }
        
        return create_suffix_array(s, alphabet_size);
    }

    vector<int> accurate_lms_sort(const vector<int>& s, const vector<int>& buckets, const vector<bool>& l_type, const vector<int>&new_sa, const vector<int>&pos_new_s) {
        vector<int> sof(s.size() + 1, -1);
        vector<int> bucket_tails = find_bucket_tails(buckets);
        for (int i = new_sa.size() - 1; i > 1; --i) {
            int ind = pos_new_s[new_sa[i]];
            int bucket_ind = s[ind];
            sof[bucket_tails[bucket_ind]] = ind;
            --bucket_tails[bucket_ind];
        }
        sof[0] = s.size();
        return sof;
    }

    /* 
     * time:  O(n)
     * space: O(n)
     *
     * read more about this State-of-the-Art
     * https://arxiv.org/pdf/1610.08305.pdf
     */
    vector<int> create_suffix_array(const vector<int>& s, int alphabet_size) {
        vector<bool> l_type = get_types(s);
        vector<int> buckets = count_bucket_sizes(s, alphabet_size);
        vector<int> s_arr = sort_lms(s, l_type, buckets);
        induce_sort_l(s, s_arr, l_type, buckets);
        induce_sort_s(s, s_arr, l_type, buckets);
        vector<int> new_s;
        vector<int> pos_new_s;
        int new_alphabet_size = rename_lms_substr(s, s_arr, l_type, new_s, pos_new_s);
        vector<int> new_sa = fix_lms_sort(new_s, new_alphabet_size);
        vector<int> result = accurate_lms_sort(s, buckets, l_type, new_sa, pos_new_s);
        induce_sort_l(s, result, l_type, buckets);
        induce_sort_s(s, result, l_type, buckets);
        return result;
    }

    /* O(n*log^2(n)) */
    vector<int> create_suffix_array(vector<int> c) {
        int n = c.size();
        vector<pair<pair<int, int>, int>> t(n);
        for (int i = 1; i < n; i <<= 1) {
            for (int j = 0; j < c.size(); j++)
                t[j] = { { c[j], c[(j + i) % n] }, j };
            sort(t.begin(), t.end());
            for (int cnt = 0, j = 0; j < n; j++) {
                if (j && t[j].first != t[j - 1].first)
                    cnt++;
                c[t[j].second] = cnt;
            }
        } 

        vector<int> p(n);
        for (int i = 0; i < n; i++)
            p[c[i]] = i;
        return p;
    }

    /* Kasai algo O(n) */
    void calc_lcp_array() {
        int n = str.size();
        int k = 0;
        lcp_arr.resize(n);
        vector<int> rank(n);
        for(int i = 0; i < n; ++i) {
            rank[sa[i]] = i;
        }
        for(int i = 0; i < n; ++i) {
            k = max(0 , k - 1);
            if(rank[i] == n - 1) {
                k = 0;
                continue;
            }

            int j = sa[rank[i] + 1];
            while(i + k < n && j + k < n && str[i+k] == str[j+k]) k++;
            lcp_arr[rank[i] + 1]=k;
        }
    }

    void fill_lcp_query(int i, int l, int r) {
        if (l + 1 == r) {
            lcp_query[i] = lcp_arr[r];
            return;
        }
        int m = (l + r) / 2;
        fill_lcp_query(2 * i + 1, l, m);
        fill_lcp_query(2 * i + 2, m, r);
        lcp_query[i] = min(lcp_query[2 * i + 1], lcp_query[2 * i + 2]);
    }


    int find(const vector<int>& p) {
        vector<size_t> res;
        int l_lcp = 0;
        int r_lcp = 0;
        int n = str.size();
        int m = p.size();
        while (sa.front() + l_lcp < n && l_lcp < m && str[sa.front() + l_lcp] == p[l_lcp]) ++l_lcp;
        while (sa.back() + r_lcp < n && r_lcp < m && str[sa.back() + r_lcp] == p[r_lcp]) ++r_lcp;
        if (l_lcp >= m) return 0;
        if (r_lcp >= m) return n - 1;
        if (l_lcp < m && str[sa.front() + l_lcp] > p[l_lcp]) return -1;
        if (r_lcp < m && str[sa.back() + r_lcp] < p[r_lcp]) return -1;

        int L = 0;
        int R = n - 1;
        int M;
        int index_lcp_query = 0;
        int c = 0;

        while (R - L > 1) {
            M = (L + R) / 2;
            if (l_lcp >= r_lcp) {
                if (lcp_query[2 * index_lcp_query + 1] >= l_lcp) {
                    int tmp = 0;
                    while (tmp + l_lcp < m && sa[M] + l_lcp + tmp < n && str[sa[M] + l_lcp + tmp] == p[tmp + l_lcp]) {
                        ++tmp;
                    }
                    c = l_lcp + tmp;
                } else {
                    c = lcp_query[2 * index_lcp_query + 1];
                }
            } else {
                if (lcp_query[2 * index_lcp_query + 2] >= r_lcp) {
                    int tmp = 0;
                    while (tmp + r_lcp < m && sa[M] + r_lcp + tmp < n && str[sa[M] + r_lcp + tmp] == p[tmp + r_lcp]) {
                        ++tmp;
                    }
                    c = r_lcp + tmp;
                } else {
                    c = lcp_query[2 * index_lcp_query + 2];
                }
            }
            if (c >= m) {
                return M;
            }
            if (p[c] < str[sa[M] + c]) {
                R = M;
                r_lcp = c;
                index_lcp_query = 2 * index_lcp_query + 1;
            } else {
                L = M;
                l_lcp = c;
                index_lcp_query = 2 * index_lcp_query + 2;
            }
        }
        if (c < m) return -1;
        return L;
    }

    int log_2_ceil(int n) {
        int x = n;
        int r = 0;
        while (n >>= 1) {
            r++;
        }
        return r + ((1<<r) < x);
    }

public:
    TSuffArray(const string& s) {
        str = string_to_bites(s);
        sa = create_suffix_array(str, 256);
        str.push_back(1);
    }

    vector<size_t> find_occurrences(const string& pattern) {
        if (!ready_to_find) {
            calc_lcp_array();
            lcp_query.resize((1<<(log_2_ceil(str.size() - 1) + 1)));
            fill_lcp_query(0, 0, str.size() - 1);
            ready_to_find = true;
        }
        vector<int> p;
        vector<size_t> res;
        p = string_to_bites(pattern);
        int i = find(p);
        if (i == -1) return res;
        for (int j = i; j > 0 && lcp_arr[j] >= p.size(); --j) {
            res.push_back(sa[j - 1]);
        }
        for (int j = i + 1; j < lcp_arr.size() && lcp_arr[j] >= p.size(); ++j) {
            res.push_back(sa[j]);
        }
        res.push_back(sa[i]);
        return res;
    }

    vector<int> get_array() {
        vector<int> res(sa);
        res.erase(res.begin());
        return res;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    string text, pattern;
    cin >> text;
    size_t pattern_id = 0;
    TSuffArray sa(text);
    string format[] = {", ", "\n"};
    while (cin >> pattern) {
        ++pattern_id;
        vector<size_t> res = sa.find_occurrences(pattern);
        if (res.empty()) continue;

        sort(res.begin(), res.end());
        cout << pattern_id << ": ";
        for (size_t i = 0; i < res.size(); ++i) {
            cout << res[i] + 1 << format[i+1==res.size()];
        }
    }
}

