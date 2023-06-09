#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <unordered_map>
#include <time.h>

using namespace std;

const char SENTINEL = 'z' + 1;

class TSuffTree {
private:
    struct TNode {
        unordered_map <char, size_t> edges;
        //map <char, size_t> edges;
        int suff_link;
        int leaf_id;                       // start index in text for this suffix
        TNode() {
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
    bool has_way(char c) {
        if (state.on_edge) {
            /* 
             * never boundaries of the text
             * assert(edges[state.edge].begin_index + state.pos_on_edge + 1 < data.size());
             */
            return (c == data[edges[state.edge].begin_index + state.pos_on_edge + 1]);
        } else {
            return nodes[state.node].edges.find(c) != nodes[state.node].edges.end();
        }
    }

    /*
     * move down in tree only for one char
     */
    void go_one_char(char c) {
        assert(has_way(c));
        if (state.on_edge) {
            ++state.pos_on_edge;
        } else {
            state.pos_on_edge = 0;
            state.on_edge = true;
            state.edge = nodes[state.node].edges[c];
        }
        /*
         * the end of the current edge has been reached
         * regardless of the previous state, a check is needed
         * because there can be only 1 char on the edge and
         * we immediately move on to the next Node
         */
        if (state.pos_on_edge + 1 == edges[state.edge].length) {
            state.on_edge = false;
            state.node = edges[state.edge].dest_node;
        }
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
    int go_one_edge(char c, int depth) {
        //assert(!state.on_edge && has_way(c) && depth > 0);
        
        /* take appropriate edge */
        state.edge = nodes[state.node].edges[c];

        int step;
        if (edges[state.edge].length <= depth) {
            /* there are fewer characters on the edge than `depth` */
            /* go down only for length of the Edge (not `depth`) */
            state.node = edges[state.edge].dest_node;
            step = edges[state.edge].length;
        } else {
            /* there are more characters on the edge than `depth` */
            /* go down only for `depth` */
            state.on_edge = true;
            state.pos_on_edge = depth - 1;
            step = depth;
        }
        return step;
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
    void go_down(int char_ind, int depth) {
        //assert(!state.on_edge && char_ind < data.size() && "bi bu ba");
        char c;
        while (depth > 0) {
            c = data[char_ind];
            int step = go_one_edge(c, depth);
            char_ind += step;
            depth -= step;
        }
        return;
    }

    /* 
     * the Root doesn't have a suffix link
     * nothing needs to be done
     */
    void go_suff_link() {
        assert(!state.on_edge);
        if (state.node == root) return;
        state.node = nodes[state.node].suff_link;
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
        nodes[leaf].leaf_id = char_ind;    // char_ind by definition is leaf_id

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

public:
    TSuffTree(string s) {
        data = s;
        nodes.reserve(2 * data.size());
        edges.reserve(2 * data.size());
        
        root = create_node();
        state.node = root;
        build();
    }

    int TAB_SIZE = 4;
    void print(int n, int h) {
        TNode node = nodes[n];
        for (auto x : node.edges) {
            TEdge e  = edges[x.second];
#if 0
            // pretty Tree output
            for (int i = 0; i < TAB_SIZE * (h - 1); ++i) {
                cout << ' ';
            }
            string tmp;
            for (int i = 0; i < e.length && e.begin_index + i < data.size(); ++i) {
                tmp.push_back(data[i + e.begin_index]);
            }
            cout << "|-> {" << tmp <<", " << e.dest_node <<  "}"<< "\n";
#else
            // special for testing
            if (e.begin_index + e.length > data.size()) {
                cout << e.begin_index << ' ' << data.size() - e.begin_index << '\n';
            } else {
                cout << e.begin_index << ' ' << e.length << '\n';
            }
#endif
            print(e.dest_node, h + 1);
        }
    }

    void Print() {
        cout << "root" << '\n';
        print(root, 1);
        return;
    }

    string lexic_min_cut() {
        string res;
        int n = (data.size() >> 1);
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
};

#define BENCHMARK
#undef BENCHMARK

int main() {
#ifdef BENCHMARK
    struct timespec begin;
    struct timespec end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin);
    ios_base::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
#endif

    string s;
    cin >> s;
    TSuffTree t(s + s + SENTINEL);
    //t.Print();
    cout << t.lexic_min_cut() << '\n';

#ifdef BENCHMARK
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    fprintf(stderr,"%lf\n", ((end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec)/1000000000.0));
#endif
    return 0;
}


