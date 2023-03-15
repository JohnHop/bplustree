#include "bplustree.h"

int main(int argc, char const *argv[])
{
  Node* root = new Node{'P'};
  root->leaf = false;

  Node* cgm = new Node{'C', 'G', 'M'};
  cgm->leaf = false;
  root->children[0] = cgm;

  Node* tx = new Node{'T', 'X'};
  tx->leaf = false;
  root->children[1] = tx;

  Node* abc = new Node{'A', 'B', 'C'};
  cgm->children[0] = abc;
  Node* dfg = new Node{'D', 'F', 'G'};
  cgm->children[1] = dfg;
  Node* jklm = new Node{'J', 'K', 'L', 'M'};
  cgm->children[2] = jklm;
  Node* nop = new Node{'N', 'O', 'P'};
  cgm->children[3] = nop;

  Node* qrst = new Node{'Q', 'R', 'S', 'T'};
  tx->children[0] = qrst;
  Node* ux = new Node{'U', 'X'};
  tx->children[1] = ux;
  Node* yz = new Node{'Y', 'Z'};
  tx->children[2] = yz;

  abc->next = dfg;
  dfg->next = jklm;
  jklm->next = nop;
  nop->next = qrst;
  qrst->next = ux;
  ux->next = yz;

  BPlusTree tree{root};

  // tree.print();

  tree.remove('Y').remove('M').remove('L').remove('K');

  tree.remove('J').remove('C').remove('Z').remove('U').remove('T').remove('Q').remove('R').remove('O').remove('B').remove('G').remove('D');

  tree.remove('N').remove('F');
  tree.remove('P'); //qui la radice diventa foglia

  tree.remove('A').remove('S');

  tree.remove('X'); //con questo elimino anche l'ultimo nodo dall'albero

  // tree.print();

  return 0;
}
