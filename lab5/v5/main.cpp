#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <assert.h>
#include <unordered_map>
#include <time.h>
#include <queue>
#include <stack>

using namespace std;

const char SENTINEL = 'z' + 1;
const char SENTINELS[] = {'z' + 2, 'z' + 3, 'z' + 4, 'z' + 5};

class TSuffTree {
private:
    struct TNode {
        //unordered_map <char, size_t> edges;
        map <char, size_t> edges;
        int suff_link;
        int leaf_id;                       // start index in text for this suffix
        char color;
        TNode() {
            color = 0;
            suff_link = -1;                // has no suffix link yet
            leaf_id = -1;                  // by default is not a leaf
        }
    };

    struct TEdge {
        size_t dest_node;
        size_t begin_index;                // index in text of the first char on edge
        size_t length;                     // number of char on edge

        TEdge() {
            length = 0;
        };

        TEdge(size_t dest_node, size_t begin_index, size_t length) {
            this->dest_node = dest_node;
            this->begin_index = begin_index;
            this->length = length;
        }
    };

    // structure for storing the current position in the tree
    struct TState {
        size_t node;                       // current node
        size_t edge;                       // current edge
        size_t pos_on_edge;                // position from the beginning of the edge (zero-based)
        bool on_edge;                      // flag of standing on the edge
        /* 
         * when on_edge = false 
         * it's assumed that the Edge leads to the current Node
         * !!! exept after break_edge() !!!
         * Then Edge starts from Node (as in the case below)
         * 
         * when on_edge = true
         * it's assumed that current Edge starts from Node
         */

        TState() {
            on_edge = false;               // initial state in node (root)
            // before build() required to set: state.node = Root!
        }
    };

    string data;                           // text for building the Suffix Tree
    TState state;                          // current position in the tree
    size_t root;                           // root of the tree (cannot make it const although it does not change)
    vector<TNode> nodes;                   // storage for all nodes of the tree
    vector<TEdge> edges;                   // storage for all edges of the tree
    size_t leaves_count = 0;
    size_t current_color = 0;

    size_t create_node() {
        TNode new_node; 
        nodes.emplace_back(new_node);
        return nodes.size() - 1;
    }

    size_t create_edge(size_t dest, int start_index, int length) {
        TEdge new_edge(dest, start_index, length); 
        edges.emplace_back(new_edge);
        return edges.size() - 1;
    }

    /*
     * is there a way with char `c` in the current state
     */
    bool has_way(const TState& st, char c) {
        if (st.on_edge) {
            /* 
             * never boundaries of the text
             * assert(edges[st.edge].begin_index + st.pos_on_edge + 1 < data.size());
             */
            return (c == data[edges[st.edge].begin_index + st.pos_on_edge + 1]);
        } else {
            return nodes[st.node].edges.find(c) != nodes[st.node].edges.end();
        }
    }
    bool has_way(char c) {
        return has_way(state, c);
    }

    /*
     * move down in tree only for one char
     */
    void go_one_char(TState& st, char c) {
        assert(has_way(st, c));
        if (st.on_edge) {
            ++st.pos_on_edge;
        } else {
            st.pos_on_edge = 0;
            st.on_edge = true;
            st.edge = nodes[st.node].edges[c];
        }
        /*
         * the end of the current edge has been reached
         * regardless of the previous st, a check is needed
         * because there can be only 1 char on the edge and
         * we immediately move on to the next Node
         */
        if (st.pos_on_edge + 1 == edges[st.edge].length) {
            st.on_edge = false;
            st.node = edges[st.edge].dest_node;
        }
    }
    void go_one_char(char c) {
        go_one_char(state, c);
    }

    /*
     * goes down no more than One edge and no more than `depth`
     * chars (the first of them is `c`)
     *
     * this function assumes that the initial state is
     *                           state.on_edge = False
     * also, the Algorithm guarantees that a path for
     * descent will exist
     *
     * Return
     * `step` = how many characters were went down
     */
    int go_one_edge(TState& st, char c, int depth) {
        //assert(!st.on_edge && has_way(c) && depth > 0);
        
        /* take appropriate edge */
        st.edge = nodes[st.node].edges[c];

        int step;
        if (edges[st.edge].length <= depth) {
            /* there are fewer characters on the edge than `depth` */
            /* go down only for length of the Edge (not `depth`) */
            st.node = edges[st.edge].dest_node;
            step = edges[st.edge].length;
        } else {
            /* there are more characters on the edge than `depth` */
            /* go down only for `depth` */
            st.on_edge = true;
            st.pos_on_edge = depth - 1;
            step = depth;
        }
        return step;
    }
    int go_one_edge(char c, int depth) {
        return go_one_edge(state, c, depth);
    }

    /*
     * goes down in tree for `depth`
     * chars (the first of them is data[char_ind])
     * 
     * this function assumes that the initial state is
     *                           state.on_edge = False
     * also, the Algorithm guarantees that a path for
     * descent will exist
     */
    void go_down(TState& st, int char_ind, int depth) {
        //assert(!state.on_edge && char_ind < data.size() && "bi bu ba");
        char c;
        while (depth > 0) {
            c = data[char_ind];
            int step = go_one_edge(st, c, depth);
            char_ind += step;
            depth -= step;
        }
        return;
    }
    void go_down(int char_ind, int depth) {
        go_down(state, char_ind, depth);
    }

    /* 
     * the Root doesn't have a suffix link
     * nothing needs to be done
     */
    void go_suff_link(TState& st) {
        assert(!st.on_edge);
        if (st.node == root) return;
        st.node = nodes[st.node].suff_link;
    }
    void go_suff_link() {
        go_suff_link(state);
    }

    /*
     * creates a new Node between:
     * `state.node` and `state.edge.dest_node`
     *
     * Note:
     * 1) after execution, the `state.node` points
     *    to the same node as before.
     * 2) only one new edge is created, the second
     *    one just changes its parameters
     */
    void break_edge() {
        assert(state.on_edge);
        /* state.node -        - - - state.edge
         *             |      |
         *             |      |
         *             v      v    pos = ..           
         * Before: parent ------------------> son
         *
         * After:  {parent} ------> me -----> son
         *             ^      ^
         *             |      |
         * state.node -        - - - state.edge
         */
        size_t parent = state.node;
        size_t son    = edges[state.edge].dest_node;
        size_t me     = create_node();
        size_t char_ind = edges[state.edge].begin_index + state.pos_on_edge + 1;
        char c = data[char_ind];
        nodes[me].edges[c] = create_edge(
                son, char_ind,
                edges[state.edge].length - state.pos_on_edge - 1);

        edges[state.edge].dest_node = me;
        edges[state.edge].length = state.pos_on_edge + 1;

        state.on_edge = false;
    }

    void create_leaf(size_t parent, int char_ind) {
        size_t leaf = create_node();
        nodes[leaf].leaf_id = leaves_count++;
        nodes[leaf].color |= (1<<current_color);

        /* acording to Alforithm set length = INF on Edge to the leaf */
        size_t e = create_edge(leaf, char_ind, 1e9); 
        nodes[parent].edges[data[char_ind]] = e;
    }

    void set_suff_link(int from, size_t to) {
        /* only if it's not there yet */
        if (from != -1 && nodes[from].suff_link == -1) {
            nodes[from].suff_link = to;
        }
    }

    void build() {
        /* sequentially add one by one chars to the tree */
        for (int i = 0; i < data.size(); ++i) {
            add_part_of_str(i);
            current_color += (data[i] == SENTINELS[current_color]);
        }
    }

    /* the main function of the Algorithm */
    /*
     * Ukkonen's algorithm is a linear-time for constructing Suffix Tree.
     * Add one char `C` to the existing tree from the last step:
     * Cases:
     *     - In current state we have way by `C`.
     *       (it doesn't matter if we are the Node or on the Edge)
     *       It means that all current suffixes ending with the `C` char
     *       has already been added earlier. Just do one step by this
     *       char `C` and stop.
     *     - In current state we have no way by `C`.
     *           1) We are in the Node:
     *              Create a new leaf by char `C`, than go through
     *              suffix link.
     *
     *           2) We are on the Edge:
     *              Breake the Edge and create a leaf from the new node,
     *              new node doesn't have suffix link yet, so we can't
     *              go through it. Trick - use your parent's* siffix link
     *              and than go down to the same depth as it was from
     *              the parent (state.pos_on_edge).
     *              
     *              *if parent = root, just go down from root to (depth-1)
     *                                 Edge string but without first char.
     *        Than check all cases from the beginning. 
     *        And always in a Node try to set suffix link from the last
     *        created Node.
     */
    void add_part_of_str(int current_len) {
        char a = data[current_len];
        int prev_node = -1;                // the last created node (to set its suff link in next step)

        while (true) {
            if (has_way(a)) {
                if (!state.on_edge) {
                    /* try to set suff link */
                    set_suff_link(prev_node, state.node);
                }
                go_one_char(a);
                break;
            }

            if (state.on_edge) {
                size_t char_ind = edges[state.edge].begin_index;
                int depth = state.pos_on_edge + 1;
                
                break_edge(); // new node has been created => try to set suff link from prev to new one
                set_suff_link(prev_node, edges[state.edge].dest_node);

                create_leaf(edges[state.edge].dest_node, current_len);
                prev_node = edges[state.edge].dest_node;  // strore new node to set suff link later

                if (state.node == root) {
                    ++char_ind;
                    --depth;
                }

                go_suff_link();            // is does nothing when state.node = root
                go_down(char_ind, depth);
            } else {
                set_suff_link(prev_node, state.node);
                create_leaf(state.node, current_len);
                if (state.node == root) {
                    /* suffixes of all lengths have already been added in current phase */
                    break;
                }
                go_suff_link();
            }
        }
    }

    void print(int n, int h) {
        TNode node = nodes[n];
        for (auto x : node.edges) {
            TEdge e  = edges[x.second];
            for (int i = 0; i < (h - 1); ++i) {
                cout << "    ";
            }
            string tmp;
            for (int i = 0; i < e.length && e.begin_index + i < data.size(); ++i) {
                tmp.push_back(data[i + e.begin_index]);
            }
            cout << "|-> {" << tmp <<", " << e.dest_node << ", " << nodes[e.dest_node].leaf_id <<  "}"<< "\n";
            print(e.dest_node, h + 1);
        }
    }

    bool find(const string& s, TState& st) {
        st.node = root;
        st.on_edge = false;

        int n = s.size();
        int i = 0;
        while (i < n) {
            if (!has_way(st, s[i])) {
                return false;
            }
            go_one_char(st, s[i]);
            ++i;
        }
        return true;
    }

    void set_colors(size_t u, size_t depth, size_t& common_depth) {
        for (pair<char, size_t> x: nodes[u].edges) {
            set_colors(edges[x.second].dest_node, depth + edges[x.second].length, common_depth);
            nodes[u].color |= nodes[edges[x.second].dest_node].color;
            if (nodes[u].color + 1 == (1<<(current_color))) {
                common_depth = max(common_depth, depth);
            }
        }
    }

    void recover_ans(size_t u, size_t depth, size_t common_depth, vector<pair<size_t, char>>& path, vector<string>& result) {
        if (depth == common_depth && nodes[u].color + 1 == (1<<(current_color))) {
            string s;
            TEdge e;
            for (pair<size_t, char> x: path) {
                e = edges[nodes[x.first].edges[x.second]];
                s += data.substr(e.begin_index, e.length);
            }
            result.push_back(s);
            return;
        }
        for (pair<char, size_t> x: nodes[u].edges) {
            path.push_back({u, x.first});
            recover_ans(edges[x.second].dest_node, depth + edges[x.second].length, common_depth, path, result);
            path.pop_back();
        }
    }

public:
    TSuffTree(string s) {
        data = s;
        nodes.reserve(2 * data.size());
        edges.reserve(2 * data.size());
        
        root = create_node();
        state.node = root;
        build();
    }

    void Print() {
        print(root, 1);
        return;
    }

    vector<size_t> find_occurrences(const string& s) {
        TState st;
        vector<size_t> res;
        if (find(s, st) == false) return res;
        if (st.on_edge) {
            st.node = edges[st.edge].dest_node;
        }
        // BFS
        queue<size_t> q;
        q.push(st.node);
        while (!q.empty()) {
            size_t u = q.front();
            q.pop();
            if (nodes[u].edges.empty()) {
                res.push_back(nodes[u].leaf_id);
                continue;
            }
            for (pair<char, size_t> x: nodes[u].edges) {
                q.push(edges[x.second].dest_node);
            }
        }
        return res;
    }

    string lexic_min_cut(int n) {
        string res;
        size_t cur = root;
        while (n > 0) {
            // find lexical min edge
            char min_char = SENTINEL;
            size_t next;
            for (pair<char, size_t> x: nodes[cur].edges) {
                if (x.first < min_char) {
                    min_char = x.first;
                    next = x.second;
                }
            }
            TEdge e = edges[next];

            // go through it
            for (int i = 0; i < e.length && n--; ++i) {
                res.push_back(data[e.begin_index + i]);
            }
            cur = e.dest_node;
        }
        return res;
    }

    vector<string> find_all_longest_common_substrings() {
        vector<string> res;
        size_t len = 0;
        set_colors(root, 0, len);
        if (len == 0) return res;
        vector<pair<size_t, char>> path;
        recover_ans(root, 0, len, path, res);
        return res;
    }
    

};


int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
    string s, tmp;
    size_t str_count = 2;                  // amount strings in input to find common substring (no more Than 5 !!!!)
    for (size_t i = 0; i < str_count; ++i) {
        cin >> tmp;
        s = s + tmp + SENTINELS[i];
    }
    TSuffTree tree(s);

    // USE map (instead unordered_map) in TNode to have lexical sorted result
    vector<string> result = tree.find_all_longest_common_substrings();
    if (result.empty()) {
        cout << "0\n";
    } else {
        cout << result.back().size() << '\n';
    }
    for (string& s: result) {
        cout << s << '\n';
    }
    return 0;
}


