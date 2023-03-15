#include "bplustree.h"

Node::Node(std::initializer_list<char> list): Node()
{
  for(auto k: list) {
    keys[key_count++] = k;
  }
}

/**
 * Esegue uno shift a destra di @num_shift in modo da liberare spazio per inserire una chiave nelle prima @num_shift posizioni.
 * Se il nodo è interno vengono spostati anche i figli.
 * Il numero di chiavi nel nodo aumenta di @num_shift
 * Attenzione: se il nodo è pieno allora si perderanno chiave e figlio più a destra
*/
void Node::right_shift(const int num_shift) {
  for(int i = key_count+num_shift-1; i > num_shift-1; i -= 1) {
    keys[i] = keys[i-num_shift];
  }

  if(!leaf) { //se è anche un nodo interno
    for(int i = key_count+num_shift; i > num_shift-1; i -= 1) {
      children[i] = children[i-num_shift];
    }
  }
  key_count += num_shift;
}

/**
 * Eseguo uno shift a sinistra in modo da eliminare la chiave e il figlio alla posizione 0.
 * Se il nodo è interno vengono spostati anche i figli.
 * Il numeri di chiavi diminuisce di 1
*/
void Node::left_shift() {
  for(int i = 0; i < key_count-1; i += 1) {
    keys[i] = keys[i+1];
  }

  if(!leaf) {
    for(int i = 0; i < key_count; i += 1) {
      children[i] = children[i+1];
    }
  }
  key_count -= 1;
}

std::pair<const Node*,int> BPlusTree::search(const char key, const Node* current) const {
  if(current == nullptr) {
    current = root;
  }

  while(!current->leaf) { //Scende lungo l'albero finché non trova la foglia contenente o meno la chiave
    int i{0};

    while( i < current->key_count && key > current->keys[i] ) {
      i += 1;
    }
    current = current->children[i];
  }

  int i{0}; //Esegue la ricerca all'interno del nodo foglia
  while( i < current->key_count && key > current->keys[i] ) {
    i += 1;
  }

  if(i < current->key_count && key == current->keys[i] ) {
    return {current, i};
  }
  else {
    return {nullptr, 0};
  }
}

/**
 * @parent: un nodo non-full
 * @child_index: indice del figlio di @parent pieno e che vogliamo dividere
 * Divide il nodo @parent->children[@child_index] in due nodi #y e #z spostando la mediana di #y nel #parent
*/
void BPlusTree::split_child(Node* parent, const int child_index) {
  Node* z = new Node{}; //nuovo nodo foglia
  Node* y = parent->children[child_index];  //nodo da dividere
  z->leaf = y->leaf;
  const int t = CORMEN_ORDER; //primo elemento dopo la mediana //FIXME bisogna essere indipendenti da questa costante simbolica

  z->key_count = t - 1; //numero di figli che avrà #z
  for(int i = 0; i < z->key_count; i += 1) { //sposto le chiavi da #y -> #z
    z->keys[i] = y->keys[t+i];
  }

  for(int i = 0; (!y->leaf) && i < t; i += 1) { //sposto eventualmente anche i figli
    z->children[i] = y->children[t+i];//EDIT: c'era t-1+i
  }
  y->key_count = y->leaf ? t : t-1; //* B+Tree: se #y è foglia deve contenere anche la mediana

  for(int i = parent->key_count; i > child_index; i -= 1) {  //shifto i children di @parent a destra per far spazio alla mediana
    parent->children[i+1] = parent->children[i];
  }
  parent->children[child_index+1] = z;  //inserisco il nuovo figlio in @parent

  for(int i = parent->key_count-1; i >= child_index; i-= 1) { //shifto anche le chiavi di @parent
    parent->keys[i+1] = parent->keys[i];
  }
  parent->keys[child_index] = y->keys[t-1]; //inserisco la mediana

  parent->key_count += 1; //ora il @parent ha una chiave in più

  if(y->leaf) { //* B+Tree: sposto i puntatori next
    z->next = y->next;
    y->next = z;
  }
}

/**
 * @current: nodo dove inserire la chiave
 * @key: chiave da inserire in @current
 * Si assume che @current sia un nodo non-pieno, interno o foglia.
*/
void BPlusTree::insert_nonfull(Node* current, const char key) {
  int i{current->key_count-1};  //posizione dell'ultima chiave nel nodo

  if(current->leaf) {
    while(i >= 0 && key < current->keys[i]) { //shifto a destra tutte le chiavi maggiori di @key
      current->keys[i+1] = current->keys[i];
      i -= 1;
    }

    current->keys[i+1] = key; //e inserisco la nuova chiave
    current->key_count += 1;
  }
  else {  //altrimenti bisogna inserire @key nel sottoalbero appropriato
    while(i >= 0 && key < current->keys[i]) {
      i -= 1;
    }
    i += 1; //è la posizione del children dove inserire @key

    if(current->children[i]->full()) {
      split_child(current, i);
      if(key > current->keys[i]) {
        i += 1;
      }
    }
    insert_nonfull(current->children[i], key);
  }
} 

BPlusTree& BPlusTree::insert(const char key) {
  Node* r = root;

  if(root->full()) {
    Node* s = new Node(false);  //crea un nuovo nodo non-foglia
    root = s;
    s->children[0] = r;
    split_child(s, 0);
    insert_nonfull(s, key);
  }
  else {
    insert_nonfull(root, key);
  }
  return *this;
}

/**
 * Elimina semplicemente una chiave in un nodo foglia
*/
void BPlusTree::delete_key_in_leaf(Node* current, const int key_pos) {
  for(int j = key_pos; j < current->key_count-1; j += 1) {  //eliminazione della chiave @key dal nodo foglia @current mediante traslazione
    current->keys[j] = current->keys[j+1];
  }
  current->key_count -= 1;  //ora @current ha una chiave in meno
}

/**
 * @parent: genitore dei nodi vicini
 * @children_pos: indice del nodo figlio di @parent che ha meno di t-1 chiavi
 * Trasferisce una chiave da un vicino al figlio di @parent alla posizione @children_pos. Aggiorna anche il link
 * Precondizione: uno dei fratelli di #y ha necessariamente almeno t chiavi !
*/
void BPlusTree::rotate(Node* parent, const int y_pos_in_parent) {
  Node* y = parent->children[y_pos_in_parent];
  Node* s;  //fratello di #y con almeno t chiavi
  int s_pos_in_parent;  //posizione del nodo #s in @parent
  int s_old_key_pos;  //posizione in #s della chiave che andrà in #y
  int y_new_key_pos;  //posizione della nuova chiave in #y proveniente da #s

  //1. Capisco il sibling che possiede almeno t chiavi. Uno dei due per forza
  if( (y_pos_in_parent > 0) && parent->children[y_pos_in_parent-1]->more_than_min_keys() ) {  //se quello a sinistra può dare una chiave
    s = parent->children[y_pos_in_parent-1];
    s_pos_in_parent = y_pos_in_parent - 1;
    s_old_key_pos = s->key_count - 1;
    y_new_key_pos = 0;
  }
  else {  //oppure quello a destra
    s_pos_in_parent = y_pos_in_parent + 1;
    s = parent->children[s_pos_in_parent];
    s_old_key_pos = 0;
    y_new_key_pos = y->key_count;
  }

  //2. Sposto la nuova chiave da #s verso #y
  if(y_new_key_pos == 0) {  //se #s sta alla sinistra di #y allora traslo per far spazio alla nuova chiave
    y->right_shift();
  }
  else
    y->key_count += 1;

  y->keys[y_new_key_pos] = s->keys[s_old_key_pos];  //sposto la chiave

  //se è un nodo interno allora devo spostare anche il figlio
  if(!s->leaf) {
    if(y_new_key_pos == 0)  //se #s è alla sinistra di #y
      y->children[y_new_key_pos] = s->children[s_old_key_pos+1];
    else  //#s a destra di #y
      y->children[y_new_key_pos+1] = s->children[s_old_key_pos];
  }

  //3. Elimino la chiave prelevata da #s
  if(s_old_key_pos == 0) {  //se lo spostamento è avvenuto a sinistra
    s->left_shift();
  }
  else {  //se lo spostamento è avvenuto a destra
    s->key_count -= 1;
  }

  /**
   * È necessario aggiornare:
   *  - nel #parent
   *    - se lo spostamento è avvenuto a sinistra (#y a sinistra di #s), si aggiorna la chiave in #y_pos_in_parent
   *    - se lo spostamento è avvenuto a destra, si aggiorna #s_pos_in_parent
   * - se lo spostamento è avvenuto tra nodi interni, in #y il valore della nuova chiave, presente in #y_new_key_pos
  */
    
  if(s_old_key_pos == 0) //spostamento avvenuto a sinistra
    update_key(parent, y_pos_in_parent);
  else
    update_key(parent, s_pos_in_parent);

  if(!y->leaf) {  //spsotamento avvenuto tra nodi interni
    update_key(y, y_new_key_pos);
  }
}

/**
 * Esegue l'aggiornamento della chiave del nodo #parent alla posizione #child_pos.
 * Nella pratica, scorre tutto il sottoalbero individuando l'elemento più grande dello stesso. Quindi poi aggiorna la chiave.
 * Precodnizioni: @child_pos deve essere necessariamente l'indice di una chiave
*/
void BPlusTree::update_key(Node* parent, const int child_pos) {
  Node* current = parent->children[child_pos];

  while(!current->leaf) { //finché non arrivo alla foglia
    current = current->children[current->key_count];  //mi sposto nel sottoalbero più a destra
  }

  parent->keys[child_pos] = current->keys[current->key_count-1];  //aggiorno
}

/**
 * @parent: il genitore dei nodi che verranno fusi
 * @children_pos: posizione del figlio che ora ha un nodo in meno
 * Esegue la funzione di #y = parent->children[children_pos] con un fratello.
 * Precondizione: nessun fratello di #y ha almeno t chiavi
*/
Node* BPlusTree::merge(Node* parent, const int children_pos) {
  Node* y = parent->children[children_pos];
  Node* s;
  int s_new_key_pos;  //posizione in #s di dove iniziare a mettere le chiavi provenienti da #y

  //1. seleziono il nodo #s
  if(children_pos == parent->key_count) {  //se #y è l'ultimo figlio più a destra seleziono quello a sinistra
    s = parent->children[children_pos-1];
    s_new_key_pos = s->key_count;
  }
  else {  //#s a destra di #y
    s = parent->children[children_pos+1];
    s_new_key_pos = 0;  
  }

  //2. Faccio la merge di #y dentro #s
  if(!s->leaf) {  //se la merge avviene tra router-link allora è necessario un passaggio in più
    int parent_key_pos = (s_new_key_pos == 0) ? children_pos : children_pos - 1;  //l'indice della chiave in parent che deve "scendere"

    if(s_new_key_pos == 0) {
      s->right_shift(); //faccio spazio per far "scendere" la chiave nel @parent in #s
    }
    else {
      s->key_count += 1;
    }
    s->keys[s_new_key_pos] = parent->keys[parent_key_pos];  //la chiave del parent "scende"
    
    if(s_new_key_pos == 0) {  //sposto anche l'ultimo figlio di #y. Se #s sta a destra di #y
      s->children[s_new_key_pos] = y->children[y->key_count];
      s->keys[s_new_key_pos] = s->children[s_new_key_pos]->last_key();  //e aggiorno con la chiave maggiore del nodo
    }
    else {
      s_new_key_pos += 1;
    }
  }

  //se #s si trova a destra, le chiavi provenienti da #y si posizioneranno all'inizio di #s: bisogna liberare prima dello spazio
  if(s_new_key_pos == 0) {
    s->right_shift(y->key_count);
  }
  //TODO dovresi aggiungere un else s->key_count += 1

  for(int i = 0; i < y->key_count; i += 1) {  //sposto le chiavi di #y dentro #s
    s->keys[s_new_key_pos+i] = y->keys[i];
  }

  if(!y->leaf) {  //se nodo interno devo spostare anche i figli
    for(int i = 0; i < y->key_count; i += 1) {
      s->children[s_new_key_pos+i] = y->children[i];
    }

    if(s_new_key_pos != 0)  //se #s è alla sinistra di #y allora va spostato pure l'ultimo figlio di #y
      s->children[s->key_count + y->key_count] = y->children[y->key_count];
  }

  if(s_new_key_pos != 0)  //se ho aggiunto una chiave in coda ad #s va incrementato il numero di chiavi
    s->key_count += y->key_count;

  //3. Elimino una chiave dal @parent eventualmente shiftando le altre chiavi
  for(int i = children_pos; i < parent->key_count-1; i += 1) {
    parent->keys[i] = parent->keys[i+1];
  }
  //se è un nodo interno shifto anche i figli
  if(!parent->leaf) {
    for(int i = children_pos; i < parent->key_count; i += 1) {
      parent->children[i] = parent->children[i+1];
    }
  }
  parent->key_count -= 1;

  //Infine, l'aggiornamento del next
  if(s->leaf) {
    if(s_new_key_pos != 0)  //se #s sta a sinistra di #y è semplice
      s->next = y->next;
    else {  //altrimenti bisogna trovare il precedessore di #y e farlo puntare ad #s
      if(children_pos > 0) {  //se #y ha un precedessore
        parent->children[children_pos-1]->next = s;
      } 
    }
  }

  delete y;

  return s; //restituisce il nuovo nodo
}

/**
 * @key: chiave da eliminare
 * @parent: radice del sotto-albero / nodo foglia dove cercare @key
 * @return: restituisce il valore più a destra nel nodo foglia @current
 * Elimina la chiave @key dal sottolbero la cui radice è @parent
 * 
*/
char BPlusTree::delete_helper(const char key, Node* current) {
  int i = 0;  //Cerco il sottoalbero dove proseguire la ricerca
  while( i < current->key_count && key > current->keys[i] ) {
    i += 1;
  }

  if(current->leaf) {
    if(i < current->key_count) {  //chiave trovata alla posizione #i
      delete_key_in_leaf(current, i);
      return current->keys[current->key_count-1]; //restituisco la chiave più a destra
    }
    //<-- Si raggiunge questo punto se la chiave non viene trovata
    return 0;
  }
  else {  //se è un nodo interno (router-link)
    char c = delete_helper(key, current->children[i]); //continuo la ricerca (ed eventualmente l'eliminazione) nel figlio #i-esimo
    if(current->children[i]->less_than_min_keys()) { //Caso B1: il nodo dove ho eliminato la chiave ha meno di t-1 chiavi
      if( (i > 0) && current->children[i-1]->more_than_min_keys() ||  //se un vicino di @current->children[#i] ha almeno t chiavi (minimo + 1)
          (i < current->key_count) && current->children[i+1]->more_than_min_keys()
      ) {
        rotate(current, i); //il figlio alla posizione #i ottiene una chiave da un vicino
      } 
      else { //entrambi i vicini di @current->children[#i] hanno il minimo numero di chiavi
        Node* merged = merge(current, i);

        if(current->key_count == 0) { //se ha zero chiavi significa che era la radice. La "aggiorno"
          root = current = merged;
        }
      }
    }
    if( i < current->key_count && key == current->keys[i] ) { //la chiave eliminata si trova anche nel nodo interno //FIXME dopo la merge forse non serve
      current->keys[i] = c; //lo aggiorno con la chiave "più a destra" //TODO migliora il commento  //TODO forse è da eliminare
    }
    return c; //faccio salire la chiave "più a destra" del nodo eliminato //TODO
  }
}

/**
 * @key: chiave da eliminare
 * Elimina la chiave @key dall'albero
*/
BPlusTree& BPlusTree::remove(const char key) {
  Node* current = root;

  if(current) {
    delete_helper(key, current);
  }

  return *this;
}

//Esegue la stampa di tutti i nodi dell'albero raggruppati per livello
void BPlusTree::print(void) {
  if(root == nullptr) {
    std::cout << "(empty)\n";
  }
  else {
    std::vector<Node*> figli{root};
    print(figli); //stampo il resto dell'albero
  }
}

/**
 * Stampa un vettore di nodi e mette i loro figli in un altro vettore per la stampa su livelli
*/
void BPlusTree::print(std::vector<Node*>& nodi) {
  std::vector<Node*> figli;  //conterrà tutti i figli dei nodi del vettore @nodi

  for(auto n: nodi) {  //Per ogni nodo del livello corrente
    for(int i = 0; (!n->leaf) && i < n->key_count+1; ++i) { //mette i figli in un vettore (se non è foglia)
      figli.push_back(n->children[i]);
    }
    std::cout << *n;  //stampo i figli del livello
  }
  std::cout << "\n";  //Dopo aver stampato un livello vado a capo

  if(figli.size() > 0)
    print(figli);  //Stampo i figli del livello successivo
}

void BPlusTree::print_leaves() {
  Node* current = root;

  while(!current->leaf) {
    current = current->children[0];
  }

  while(current) {
    std::cout << *current;
    current = current->next;
  }

  std::cout << "\n";
}

/**
 * Distruzione ricorsiva dell'albero utilizzando la funzione destructor_helper().
*/
BPlusTree::~BPlusTree() {
  if(root)
    destructor_helper(root);
}

void BPlusTree::destructor_helper(Node* current) {
  if(current->leaf) {
    delete current;
  }
  else {
    for(int i = 0; i < root->key_count+1; ++i) {
      destructor_helper(root->children[i]);
    }
  }
}