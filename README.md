# B+Tree (B-Plus Tree)

## Inserimento
È identica a quella presentata per gli alberi B-Tree. L'unica differenza sta nella funzione split_child

## Eliminazione

### Rotation
Si applica quando il nodo attuale ha meno di `t-1` chiavi e un vicino possiede almeno `t` chiavi.
Denominando **y** il nodo con meno di `t-1` chiavi ed **s** il suo vicino che gliene può fornire una, l'operazione consiste nel spostare una chiave da **s** --> **y**