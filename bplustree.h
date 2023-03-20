#if !defined(BPLUSTREE)
#define BPLUSTREE

#include <iostream>
#include <vector>
#include <utility>

#define CORMEN_ORDER 3

struct Node {
  int key_count;
  char keys[2*CORMEN_ORDER-1];
  Node* children[2*CORMEN_ORDER];
  bool leaf;
  Node* next;

  Node(): key_count{0}, children{nullptr}, leaf{true}, next{nullptr} { }
  Node(const char k): key_count{1}, children{nullptr}, leaf{true}, next{nullptr} { keys[0] = k; }
  Node(std::initializer_list<char>);
  Node(const bool l): Node{} { leaf = l; };

  bool full() const { return (2*CORMEN_ORDER-1) == key_count; };  //FIXME dipendenza da costante simbolica
  bool less_than_min_keys() const { return CORMEN_ORDER-1 > key_count; }; //true se il numero do chiavi è minore del minimo richiesto
  bool more_than_min_keys() const { return key_count > CORMEN_ORDER-1; };

  void right_shift(const int = 1);
  void left_shift();

  char last_key() const { return keys[key_count-1]; };
};

class BPlusTree {
  Node* root;

public:
  BPlusTree(): root{new Node{}} { };  //di default l'albero, se vuoto, possiede comunque un nodo
  BPlusTree(Node* p): root{p} { };  //DEBUG //TODO rimuovi

  std::pair<Node*,int> search(const char) const;
  BPlusTree& insert(const char);
  BPlusTree& remove(const char);

  void print();
  void print_leaves();

  void destructor_helper(Node*);
  ~BPlusTree();

private:
  void print(std::vector<Node*>&);

  void split_child(Node*, const int);
  void insert_nonfull(Node*, const char);

  void delete_key_in_leaf(Node*, const int);
  char delete_helper(const char, Node*);
  void rotate(Node*, const int);
  void update_key(Node*, const int);
  Node* merge(Node*, const int);
};

inline std::ostream& operator<<(std::ostream& os, const Node& n) {
  os << "[";
  for(int i = 0; i < n.key_count-1; ++i) {
    os << n.keys[i] << ", ";
  }
  return os << n.keys[n.key_count-1] << "]";
}

#endif // BPLUSTREE
