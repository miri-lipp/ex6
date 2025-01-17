#include "ex6.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# define INT_BUFFER 128

// ================================================
// Basic struct definitions from ex6.h assumed:
//   PokemonData { int id; char *name; PokemonType TYPE; int hp; int attack; EvolutionStatus CAN_EVOLVE; }
//   PokemonNode { PokemonData* data; PokemonNode* left, *right; }
//   OwnerNode   { char* ownerName; PokemonNode* pokedexRoot; OwnerNode *next, *prev; }
//   OwnerNode* ownerHead;
//   const PokemonData pokedex[];
// ================================================

// --------------------------------------------------------------
// 1) Safe integer reading
// --------------------------------------------------------------

void trimWhitespace(char *str)
{
    // Remove leading spaces/tabs/\r
    int start = 0;
    while (str[start] == ' ' || str[start] == '\t' || str[start] == '\r')
        start++;

    if (start > 0)
    {
        int idx = 0;
        while (str[start])
            str[idx++] = str[start++];
        str[idx] = '\0';
    }

    // Remove trailing spaces/tabs/\r
    int len = (int)strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\r'))
    {
        str[--len] = '\0';
    }
}

char *myStrdup(const char *src)
{
    if (!src)
        return NULL;
    size_t len = strlen(src);
    char *dest = (char *)malloc(len + 1);
    if (!dest)
    {
        printf("Memory allocation failed in myStrdup.\n");
        return NULL;
    }
    strcpy(dest, src);
    return dest;
}

int readIntSafe(const char *prompt)
{
    char buffer[INT_BUFFER];
    int value;
    int success = 0;

    while (!success)
    {
        printf("%s", prompt);

        // If we fail to read, treat it as invalid
        if (!fgets(buffer, sizeof(buffer), stdin))
        {
            printf("Invalid input.\n");
            clearerr(stdin);
            continue;
        }

        // 1) Strip any trailing \r or \n
        //    so "123\r\n" becomes "123"
        size_t len = strlen(buffer);
        if (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r'))
            buffer[--len] = '\0';
        if (len > 0 && (buffer[len - 1] == '\r' || buffer[len - 1] == '\n'))
            buffer[--len] = '\0';

        // 2) Check if empty after stripping
        if (len == 0)
        {
            printf("Invalid input.\n");
            continue;
        }

        // 3) Attempt to parse integer with strtol
        char *endptr;
        value = (int)strtol(buffer, &endptr, 10);

        // If endptr didn't point to the end => leftover chars => invalid
        // or if buffer was something non-numeric
        if (*endptr != '\0')
        {
            printf("Invalid input.\n");
        }
        else
        {
            // We got a valid integer
            success = 1;
        }
    }
    return value;
}

// --------------------------------------------------------------
// 2) Utility: Get type name from enum
// --------------------------------------------------------------
const char *getTypeName(PokemonType type)
{
    switch (type)
    {
    case GRASS:
        return "GRASS";
    case FIRE:
        return "FIRE";
    case WATER:
        return "WATER";
    case BUG:
        return "BUG";
    case NORMAL:
        return "NORMAL";
    case POISON:
        return "POISON";
    case ELECTRIC:
        return "ELECTRIC";
    case GROUND:
        return "GROUND";
    case FAIRY:
        return "FAIRY";
    case FIGHTING:
        return "FIGHTING";
    case PSYCHIC:
        return "PSYCHIC";
    case ROCK:
        return "ROCK";
    case GHOST:
        return "GHOST";
    case DRAGON:
        return "DRAGON";
    case ICE:
        return "ICE";
    default:
        return "UNKNOWN";
    }
}

// --------------------------------------------------------------
// Utility: getDynamicInput (for reading a line into malloc'd memory)
// --------------------------------------------------------------
char *getDynamicInput()
{
    char *input = NULL;
    size_t size = 0, capacity = 1;
    input = (char *)malloc(capacity);
    if (!input)
    {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        if (size + 1 >= capacity)
        {
            capacity *= 2;
            char *temp = (char *)realloc(input, capacity);
            if (!temp)
            {
                printf("Memory reallocation failed.\n");
                free(input);
                return NULL;
            }
            input = temp;
        }
        input[size++] = (char)c;
    }
    input[size] = '\0';

    // Trim any leading/trailing whitespace or carriage returns
    trimWhitespace(input);

    return input;
}

// Function to print a single Pokemon node
// --------------------------------------------------------------
// Display Menu
// --------------------------------------------------------------
void DisplayMenu(OwnerNode *owner)
{
    if (owner->pokedexRoot == NULL)
    {
        printf("Pokedex is empty.\n");
        return;
    }

    printf("Display:\n");
    printf("1. BFS (Level-Order)\n");
    printf("2. Pre-Order\n");
    printf("3. In-Order\n");
    printf("4. Post-Order\n");
    printf("5. Alphabetical (by name)\n");

    int choice = readIntSafe("Your choice: ");

    switch (choice)
    {
    case 1:
        DisplayBFS(owner->pokedexRoot);
        break;
    case 2:
        PreOrderTraversal(owner->pokedexRoot);
        break;
    case 3:
        InOrderTraversal(owner->pokedexRoot);
        break;
    case 4:
        PostOrderTraversal(owner->pokedexRoot);
        break;
    case 5:
        DisplayAlphabetical(owner->pokedexRoot);
        break;
    default:
        printf("Invalid choice.\n");
    }
}

// --------------------------------------------------------------
// Sub-menu for existing Pokedex
// --------------------------------------------------------------
void EnterExistingPokedexMenu()
{
    // list owners
    printf("\nExisting Pokedexes:\n");
    if (ownerHead == NULL) {
        printf("No existing Pokedexes.\n");
        return;
    }
    int count = 1;
    int choice;
    OwnerNode *node = ownerHead;//first one in the list
    do { //while the node is not first one
        printf("%d. %s\n", count, node->ownerName);
        node = node->next; //next one
        count++;
    } while (node != ownerHead);
    printf("Choose a Pokedex by number: \n");
    choice = readIntSafe("");
    while (choice > count - 1 || choice < 1) {
        printf("Invalid choice.\n");
        choice = readIntSafe("");
    }
    OwnerNode *current = ListLookUp(choice);
    printf("Entering %s Pokedex...", current->ownerName);

    int subChoice;
    do
    {
        printf("\n-- %s's Pokedex Menu --\n", current->ownerName);
        printf("1. Add Pokemon\n");
        printf("2. Display Pokedex\n");
        printf("3. Release Pokemon (by ID)\n");
        printf("4. Pokemon Fight!\n");
        printf("5. Evolve Pokemon\n");
        printf("6. Back to Main\n");

        subChoice = readIntSafe("Your choice: ");

        switch (subChoice)
        {
        case 1:
            AddPokemon(current);
            break;
        case 2:
            DisplayMenu(current);
            break;
        case 3:
            FreePokemon(current);
            break;
        case 4:
            PokemonFight(current);
            break;
        case 5:
            EvolvePokemon(current);
            break;
        case 6:
            printf("Back to Main Menu.\n");
            break;
        default:
            printf("Invalid choice.\n");
        }
    } while (subChoice != 6);
}

// --------------------------------------------------------------
// Main Menu
// --------------------------------------------------------------
void MainMenu()
{
    int choice;
    do
    {
        printf("\n=== Main Menu ===\n");
        printf("1. New Pokedex\n");
        printf("2. Existing Pokedex\n");
        printf("3. Delete a Pokedex\n");
        printf("4. Merge Pokedexes\n");
        printf("5. Sort Owners by Name\n");
        printf("6. Print Owners in a direction X times\n");
        printf("7. Exit\n");
        choice = readIntSafe("Your choice: ");

        switch (choice)
        {
        case 1:
            OpenPokedexMenu();
            break;
        case 2:
            EnterExistingPokedexMenu();
            break;
        case 3:
            DeletePokedex();
            break;
        case 4:
            MergePokedexMenu();
            break;
        case 5:
            SortOwners();
            break;
        case 6:
            PrintOwnersCircular();
            break;
        case 7:
            printf("Goodbye!\n");
            break;
        default:
            printf("Invalid.\n");
        }
    } while (choice != 7);
}

int main()
{
    MainMenu();
    FreeAllOwners();
    return 0;
}

void OpenPokedexMenu(void) {
    printf("Your name:\n");
    char *name = getDynamicInput();
    if (ownerHead == NULL) {
        //add data to root of binary tree
        //HOW? i have an array of pokemon data so i need to add pokemondata[] to data struct.
        ownerHead = CreateOwnerNode(name);
        printf("New Pokedex created for %s with starter %s.\n",ownerHead->ownerName, ownerHead->pokedexRoot->data->name);
    }
    else { //same thing but when linked list is not empty
        AddOwner(name);
    }
    free(name);
}

OwnerNode *CreateOwnerNode(char *ownerName) {
    OwnerNode *owner = (OwnerNode *)malloc(sizeof(OwnerNode));
    if (owner == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    owner->next = owner;
    owner->prev = owner;
    owner->ownerName = myStrdup(ownerName);
    owner->pokedexRoot = StarterPokemon();
    return owner;
}

void AddOwner(char *ownerName) {//same thing but when linked list is not empty
    OwnerNode *current = ownerHead;
        //going to the end of linked list
    if (FindOwnerByName(ownerName) != NULL) {
        printf("Owner '%s' already exists. Not creating a new Pokedex.\n", ownerName);
        return;
    }
    OwnerNode *owner = CreateOwnerNode(ownerName);
    current = ownerHead;
     while (current->next != ownerHead)
         current = current->next; //find the last one in the list
     owner->prev = current; //previous becomes current
     owner->next = ownerHead; //next becomes pointer to the first one
     current->next = owner; //next to current becomes pointer to the current one
     ownerHead->prev = owner; //previous to the pointer to head becomes pointer to the added owner
    printf("New Pokedex created for %s with starter %s.\n",owner->ownerName, owner->pokedexRoot->data->name);
} //make adding to list if there is no same owners

PokemonNode *StarterPokemon() {
    int choice;
    printf("Choose Starter:\n\
                1. Bulbasaur\n\
                2. Charmander\n\
                3. Squirtle\n");
    choice = readIntSafe("Your choice: ");
    switch (choice) {
        case 1: {
            return CreatePokemonNode(&pokedex[0]);
        }
        case 2: {
            return CreatePokemonNode(&pokedex[3]);
        }
        case 3: {
            return CreatePokemonNode(&pokedex[6]);
        }
        default:
            printf("Invalid choice.\n");
            return StarterPokemon();
    } //recursion supremacy
}

PokemonNode *CreatePokemonNode(const PokemonData *data) {
    PokemonNode *node = (PokemonNode *)malloc(sizeof(PokemonNode));
    if (node == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    node->data = (PokemonData*) data;
    node->left = node->right = NULL;//right and left nodes == NULL because it's the first one in tree
    return node;
}

OwnerNode *ListLookUp(int choice) { //looking for a specific person in list and returning their name
    OwnerNode *current = ownerHead;
    int count = 1;
    do {
        if (count == choice)
            break;
        current = current->next;
        count++;
    } while (current->next != ownerHead);
    return current;
}

void AddPokemon(OwnerNode *owner) {
    int pokemonId;
    printf("Enter ID to add:\n");
    pokemonId = readIntSafe("");
    if (pokemonId < 1|| pokemonId > 151) {
        printf("Invalid ID.\n");
        return;
    }
    //printf("Current Node ID: %d, Inserting Pokemon ID: %d\n", owner->pokedexRoot->data->id, pokemonId);
    owner->pokedexRoot = InsertPokemonNode(owner->pokedexRoot, pokemonId, 0);
    //printf("Pokemon %s (ID %d) added.\n", owner->pokedexRoot->data->name, pokemonId);
}

PokemonNode *InsertPokemonNode(PokemonNode *newNode, int pokemonId, int subChoice) {
    if (newNode == NULL) {
        //if tree empty return node
        // printf("Tree is empty. Creating new node for Pokemon ID: %d\n", pokemonId);
        // printf("Pokemon ID %d added to the Pokedex.\n", pokemonId);
        PokemonNode *node = CreatePokemonNode(&pokedex[pokemonId - 1]);
        if (subChoice == 1) //just that printf won't get printed when i'm doing evolving
            return node;
        printf("Pokemon %s (ID %d) added.\n", node->data->name, pokemonId);
        return node;
    }
    // else {
    //  printf("Current Node ID: %d, Inserting Pokemon ID: %d\n", newNode->data->id, pokemonId);
    // }
    if (newNode->data->id == pokedex[pokemonId - 1].id) {
        //base case if the node is present then return it
        if (subChoice == 1) //for when evolving to not print message
            return newNode;
        printf("Pokemon with ID %d is already in the Pokedex. No changes made.\n", pokemonId);
        return newNode;
    }
    if (newNode->data->id < pokedex[pokemonId - 1].id) {
        //if the id is bigger then right node
       // printf("Going right from Node ID: %d\n", newNode->data->id);
        newNode->right = InsertPokemonNode(newNode->right, pokemonId, subChoice);
    }
    else if (newNode->data->id > pokedex[pokemonId - 1].id) {
        //if id is smaller then left node
        //printf("Going left from Node ID: %d\n", newNode->data->id);
        newNode->left = InsertPokemonNode(newNode->left, pokemonId, subChoice);
    }
    return newNode;
}

Queue *CreateQueue() {
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    if (queue == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    queue->front = queue->rear = NULL;
    return queue;
}

void Enqueue(Queue *queue, PokemonNode *node) { //function to add elements to queue list
    QueueNode *newNode = (QueueNode *)malloc(sizeof(QueueNode));
    if (newNode == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    newNode->node = node; //adding pointer of the tree to queue
    newNode->next = NULL; //next element in queue is NULL
    if (queue->front == NULL) { //if last element of the queue is NULL means that queue is empty and pointer to the first element is the same as last one
        queue->front = queue->rear = newNode;
        return;
    }
    //else if queue is not empty i need to add next element to the next after last because first in first out !
    queue->rear->next = newNode;
    queue->rear = newNode; //updating the last element of the queue
}

PokemonNode *Dequeue(Queue *queue) {
    if (queue->front == NULL) {
        return NULL;
    }//i need to do it first in first out
    QueueNode *temp = queue->front;
    PokemonNode *node = temp->node;
    queue->front = queue->front->next; //first in first out babyyyy
    if (queue->front == NULL)
        queue->rear = NULL; //if queue is empty
    free(temp);
    return node;
}

void BFSGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (root == NULL)
        return;
    Queue *queue = CreateQueue();//creating queue
    Enqueue(queue, root);//adding root to queue
    while (queue->front != NULL) {
        PokemonNode *node = Dequeue(queue); //deleting from queue
        visit(node);
        if (node->left != NULL)
            Enqueue(queue, node->left);
        if (node->right != NULL)
            Enqueue(queue, node->right);
    }
    free(queue);
}

void DisplayBFS(PokemonNode *root) {
    BFSGeneric(root, PrintPokemon);
}

void preOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (root == NULL)
        return;
    visit(root);
    preOrderGeneric(root->left, visit);
    preOrderGeneric(root->right, visit);
}

void PreOrderTraversal(PokemonNode *root) {
    preOrderGeneric(root, PrintPokemon);
}

void inOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (root == NULL)
        return;
    inOrderGeneric(root->left, visit);
    visit(root);
    inOrderGeneric(root->right, visit);
}

void InOrderTraversal(PokemonNode *root) {
    inOrderGeneric(root, PrintPokemon);
}

void postOrderGeneric(PokemonNode *root, VisitNodeFunc visit) {
    if (root == NULL)
        return;
    postOrderGeneric(root->left, visit);
    postOrderGeneric(root->right, visit);
    visit(root);
}

void PostOrderTraversal(PokemonNode *root) {
    postOrderGeneric(root, PrintPokemon);
}

void DisplayAlphabetical(PokemonNode *root) {
    NodeArray pokemons;
    InitNodeArray(&pokemons, 1);
    //printf("Adding Pokemon ID: %d, Name: %s to NodeArray.\n", root->data->id, root->data->name);
    CollectAll(root, &pokemons);
    //qsort(pokemons.nodes, pokemons.size, sizeof(PokemonNode*), CompareByNameNode);
    QuickSort(&pokemons, 0, pokemons.size - 1);
    for (int i = 0; i < pokemons.size; i++) {
        PrintPokemon(pokemons.nodes[i]);
    }
    FreeNodeArray(&pokemons);
}

int Partition(NodeArray *pokemons, int low, int high) {
    PokemonNode *pivot = pokemons->nodes[low];//pivot the last element of the array
    int i = low; //i = 0
    int j = high; //j = num of last element
    while (i < j) {
        //printf("Pokemon name %s\n", pokemons->nodes[i]->data->name);
       // printf("Pivot name %s\n", pivot->data->name);
        while (CompareByNameNode(&pokemons->nodes[i], &pivot) <= 0 && i < high) {
            i++; //compare node i and last element till i last el before previous
        }
        while (CompareByNameNode(&pokemons->nodes[j], &pivot) > 0 && j > low) {
            j--; //compare node j and last element till j larger then 2 el in array
        }
        if (i < j) {
            Swap(pokemons->nodes[i], pokemons->nodes[j]);
        }
    }
    Swap(pokemons->nodes[low], pokemons->nodes[j]);
    return j;
}

void Swap(PokemonNode *a, PokemonNode *b) {
    PokemonNode temp = *a;
    *a = *b;
    *b = temp;
}

void QuickSort(NodeArray *pokemons, int low, int high) {
    if (low < high) {
        int pivot = Partition(pokemons, low, high);
        QuickSort(pokemons, low, pivot - 1);
        QuickSort(pokemons, pivot + 1, high);
    }
}

void InitNodeArray(NodeArray *na, int cap) {
    na->nodes = malloc(cap * sizeof(PokemonNode *)); //initializing array inside of struct
    if (na->nodes == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    na->size = 0;
    na->capacity = cap;
}

void FreeNodeArray(NodeArray *na) {
    //printf("Freeing NodeArray with size: %d and capacity: %d\n", na->size, na->capacity);
    free(na->nodes);
    na->nodes = NULL;
    na->size = 0;
}

void AddNode(NodeArray *na, PokemonNode *node) { //i'm very not sure in my doings here
    if (na->size >= na->capacity) {
        na->capacity *= 2;
        na->nodes = realloc(na->nodes, na->capacity * sizeof(PokemonNode *));
        if (na->nodes == NULL) {
            printf("Memory allocation failed.\n");
            exit(1);
        }
    }
    na->nodes[na->size] = node; //adding first element to my array in struct
    na->size++;//size increase
}

void CollectAll(PokemonNode *root, NodeArray *na) { //bro i'm trying to destroy these data structures it just happens that they somehow end up working
    if (root == NULL) //base case
        return;
   // printf("Adding Pokemon ID: %d, Name: %s to NodeArray.\n", root->data->id, root->data->name);
    AddNode(na, root);
   // printf("Traversing left from Pokemon ID: %d, Name: %s\n", root->data->id, root->data->name);
    CollectAll(root->left, na);
    //printf("Traversing right from Pokemon ID: %d, Name: %s\n", root->data->id, root->data->name);
    CollectAll(root->right, na);
}

int CompareByNameNode(const void *a, const void *b) { //how the fuck do i implement qsort here?
    PokemonNode *NodeA = *(PokemonNode **)a;
    PokemonNode *NodeB = *(PokemonNode **)b;
    return strcmp(NodeA->data->name, NodeB->data->name);
}
// Function to print a single Pokemon node
void PrintPokemon(PokemonNode *root) {
    printf("ID: %d, ", root->data->id);
    printf("Name: %s, ", root->data->name);
    printf("Type: %s, ", getTypeName(root->data->TYPE));
    printf("HP: %d, ", root->data->hp);
    printf("Attack: %d, ", root->data->hp);
    if (root->data->CAN_EVOLVE == 0)
        printf("Can Evolve: No\n");
    else
        printf("Can Evolve: Yes\n");
}

void FreePokemon(OwnerNode *owner) {
    int pokemonId = 0;
    if (owner->pokedexRoot == NULL) {
        printf("No Pokemon to release.\n");
        return;
    }
    printf("Enter Pokemon ID to release:\n");
    pokemonId = readIntSafe("");
    if (pokemonId <= 0 || pokemonId > 151) {
        printf("Invalid choice.\n");
        return;
    }
    if (SearchPokemonBFS(owner->pokedexRoot, pokemonId) == NULL) {
        printf("No Pokemon with ID %d found.\n", pokemonId);
        return;
    }
    printf("Removing Pokemon %s (ID %d)\n", SearchPokemonBFS(owner->pokedexRoot, pokemonId)->data->name, pokemonId);
    owner->pokedexRoot = ReleasePokemon(owner->pokedexRoot, SearchPokemonBFS(owner->pokedexRoot, pokemonId)->data);
}

PokemonNode* ReleasePokemon(PokemonNode *root, PokemonData *data) {
    if (root == NULL) //base case
        return root;
    if (data->id < root->data->id) {//going left
        //printf("Going left: Current Node ID: %d\n", root->data->id);
        root->left = ReleasePokemon(root->left, data);
    }
    if (data->id > root->data->id) {//going right
        //printf("Going right: Current Node ID: %d\n", root->data->id);
        root->right = ReleasePokemon(root->right, data);
    } //when the leaf is found
    else { //else if one child
       // printf("Found Pokemon to delete: ID %d, Name: %s\n", root->data->id, root->data->name);
        if (root->left == NULL) {//if there is only right node or 0
            PokemonNode *temp = root->right;
            //printf("Node has no left child. Replacing with right child.\n");
            free(root);
            return temp;
        }
        if (root->right == NULL) { //if only left node
            PokemonNode *temp = root->left;
            //printf("Node has no right child. Replacing with left child.\n");
            free(root);
            return temp;
        } //both present
        PokemonNode *temp = FindMax(root); //find maximum in bst
        // printf("Node has two children. Replacing with max from left subtree: ID %d, Name: %s\n",
        //        temp->data->id, temp->data->name);
        root->data = temp->data; //replace everything
        root->left = ReleasePokemon(root->left, temp->data);
    }
    return root;
}

PokemonNode* FindMax(PokemonNode *root) {
    root = root->left;
    while (root != NULL && root->right != NULL)
        root = root->right;
    return root;
}

void FreePokemonTree(PokemonNode *root) {
    if (root == NULL) //base case
        return;
    FreePokemonTree(root->left);
    FreePokemonTree(root->right);
    free(root);
}

void FreeOwnerNode(OwnerNode *owner) { //how am i contring if the owner in the middle of the list?
    OwnerNode *ownerTail = ownerHead; //made two pointers for the same one just for easier logic and reading
    if (owner == NULL)
        return;
    if (owner == ownerHead) { //if deleteing from start of the list
        if (owner->next == ownerHead) {
            FreeHead(owner); //if the only one
            return;
        }
        ownerTail = ownerHead->prev; //first one becomes next
        ownerTail->next = ownerHead->next; //next pointer to first becomes previous ownerhead
        ownerHead->next->prev = ownerTail; //previous pointer to owner head next is the last one
        ownerHead = ownerHead->next; //updating ownerhead
         // printf("Updated ownerHead: %s\n", ownerHead->ownerName);
         // printf("Updated ownerTail: %s\n", ownerTail->ownerName);
        FreePokemonTree(owner->pokedexRoot);
        free(owner->ownerName);
        free(owner);
        owner = NULL;
        return;
    }
    if (owner->next == ownerTail) { //if next one is pointer to first then put pointer to head in the current
        ownerTail = owner->prev; //pointer to previous becomes pointer to first
        ownerTail->next = ownerHead; //new pointer to the head !
        ownerHead->prev = ownerTail;
        FreePokemonTree(owner->pokedexRoot);
        free(owner->ownerName);
        free(owner);
        owner = NULL;
        return;
    } //if owner next not last then next becomes next after next
    else {
        owner->prev->next = owner->next; //next after prev becomes next after current
        owner->next->prev = owner->prev; //perv before next becomes prev after current
        FreePokemonTree(owner->pokedexRoot);
        free(owner->ownerName);
        free(owner);
        owner = NULL;
    }
}

void FreeHead() {
    if (ownerHead == NULL)
        return;
    FreePokemonTree(ownerHead->pokedexRoot);
    free(ownerHead->ownerName);
    free(ownerHead);
    ownerHead = NULL;
}

void DeletePokedex(void) {
    printf("\n=== Delete a Pokedex ===\n");
    if (ownerHead == NULL) {
        printf("No existing Pokedexes.\n");
        return;
    }
    int count = 1;
    int choice;
    OwnerNode *node = ownerHead;//first one in the list
    do { //while the node is not pointer to the first one
        printf("%d. %s\n", count, node->ownerName);
        node = node->next; //next one
        count++;
    } while (node != ownerHead);
    printf("Choose a Pokedex to delete by number: \n");
    choice = readIntSafe("");
    while (choice > count - 1 || choice < 1) {
        printf("Invalid choice.\n");
        choice = readIntSafe("");
    }
    OwnerNode *current = ListLookUp(choice);
    printf("Deleting %s's entire Pokedex...\n", current->ownerName);
    FreeOwnerNode(current);
    printf("Pokedex deleted.\n");
}

PokemonNode *SearchPokemonBFS(PokemonNode *root, int id) {
    if (root == NULL)
        return NULL;
    Queue *queue = CreateQueue();//creating queue
    Enqueue(queue, root);//adding root to queue
    while (queue->front != NULL) {
        PokemonNode *node = Dequeue(queue);
        if (node->data->id == id) {
            while (queue->front != NULL) {
                Dequeue(queue);
            }
            free(queue);
            return node;
        }
        if (node->left != NULL) {
           // printf("node left id: %d\n", node->left->data->id);
            Enqueue(queue, node->left);
        }
        if (node->right != NULL) {
           // printf("node right id: %d\n", node->right->data->id);
            Enqueue(queue, node->right);
        }
    }
    free(queue);
    return NULL;
}

void PokemonFight(OwnerNode *owner) {
    if (owner->pokedexRoot == NULL) {
        printf("Pokedex is empty.\n");
        return;
    }
    int id1, id2;
    printf("Enter ID of the first Pokemon: \n");
    id1 = readIntSafe("");
    printf("Enter ID of the second Pokemon: \n");
    id2 = readIntSafe("");
    if (id1 < 1 || id2 < 1 || id1 > 151 || id2 > 151) {
        printf("One or both Pokemon IDs not found.\n");
        return;
    }
    if (SearchPokemonBFS(owner->pokedexRoot, id1) == NULL || SearchPokemonBFS(owner->pokedexRoot, id2) == NULL) {
        printf("One or both Pokemon IDs not found.\n");
        return;
    }
    double score1, score2;
    printf("Pokemon 1: %s ", SearchPokemonBFS(owner->pokedexRoot, id1)->data->name);
    score1 = SearchPokemonBFS(owner->pokedexRoot, id1)->data->hp * 1.2 + SearchPokemonBFS(owner->pokedexRoot, id1)->data->attack * 1.5;
    printf("(Score = %.2f)\n", score1);
    printf("Pokemon 2: %s ", SearchPokemonBFS(owner->pokedexRoot, id2)->data->name);
    score2 = SearchPokemonBFS(owner->pokedexRoot, id2)->data->hp * 1.2 + SearchPokemonBFS(owner->pokedexRoot, id2)->data->attack * 1.5;
    printf("(Score = %.2f)\n", score2);
    if (score1 > score2)
        printf("%s wins!\n", SearchPokemonBFS(owner->pokedexRoot, id1)->data->name);
    else if (score2 > score1)
        printf("%s wins!\n", SearchPokemonBFS(owner->pokedexRoot, id2)->data->name);
    else
        printf("It's a tie!\n");
}

void EvolvePokemon(OwnerNode *owner) {
    if (owner->pokedexRoot == NULL) {
        printf("Pokedex is empty.\n");
        return;
    }
    int id;
    printf("Enter ID of Pokemon to evolve: \n");
    id = readIntSafe("");
    if (id < 1 || id > 151) { //id out of bounds
        printf("No pokemon with ID %d found.\n", id);
        return;
    }
    if (SearchPokemonBFS(owner->pokedexRoot, id) == NULL) { //there is no pokemon in pokedex
        printf("No pokemon with ID %d found.\n", id);
        return;
    }
    if (SearchPokemonBFS(owner->pokedexRoot, id)->data->CAN_EVOLVE == CANNOT_EVOLVE) {//pokemon cannot evolve
        printf("%s (ID %d) cannot evolve.\n", SearchPokemonBFS(owner->pokedexRoot, id)->data->name, id);
        return;
    }
    if (SearchPokemonBFS(owner->pokedexRoot, id)->data->CAN_EVOLVE == CAN_EVOLVE) { //pokemon can evolve
        printf("Pokemon evolved from %s (ID %d) to %s (ID %d)\n", SearchPokemonBFS(owner->pokedexRoot, id)->data->name, id,
                                                                pokedex[id].name, id + 1);
        owner->pokedexRoot = ReleasePokemon(owner->pokedexRoot, SearchPokemonBFS(owner->pokedexRoot, id)->data); //case where evolving happens
        owner->pokedexRoot = InsertPokemonNode(owner->pokedexRoot, id + 1, 1);
    }
}

void MergePokedexMenu(void) {
    OwnerNode *node = ownerHead;//first one in the list
    printf("\n=== Merge Pokedexes ===\n");
    if (ownerHead == NULL) {
        printf("No existing Pokedexes.\n");
        return;
    }
    if (ownerHead == node->next) {
        printf("Not enough owners to merge.\n");
        return;
    }
    int count = 1;
    char *owner1, *owner2;
    do{ //while the node is not first one
        printf("%d. %s\n", count, node->ownerName);
        node = node->next; //next one
        count++;
    } while (node != ownerHead);
    printf("Enter name of first owner: \n");
    owner1 = getDynamicInput();
    printf("Enter name of second owner: \n");
    owner2 = getDynamicInput();
    if (FindOwnerByName(owner1)->pokedexRoot == NULL && FindOwnerByName(owner2)->pokedexRoot == NULL) {
        printf("Both Pokedexes empty. Nothing to merge.\n");
    }
    printf("Merging %s and %s...\n", FindOwnerByName(owner1)->ownerName, FindOwnerByName(owner2)->ownerName);
    //i need to add all of the stuff from the second one and if it exists then it wouldn't be added
    //i need something like owner->pokemonroot = inseart(search) but for actually how long? for every pokemon????
    //i have no idea how to optimise this shit
    FindOwnerByName(owner1)->pokedexRoot = MergeTrees(FindOwnerByName(owner1)->pokedexRoot, FindOwnerByName(owner2)->pokedexRoot);
    printf("Merge completed.\n");
    FreeOwnerNode(FindOwnerByName(owner2));
    printf("Owner '%s' has been removed after merging.", owner2);
    free(owner1);
    free(owner2);
}

PokemonNode *MergeTrees(PokemonNode *root1, PokemonNode *root2) {
    //need to traverse in queue with bfs and insert into tree
    if (root2 == NULL)
        return root1;
    Queue *queue = CreateQueue();//creating queue
    Enqueue(queue, root2);//adding root of the second one to queue
    while (queue->front != NULL) {
        PokemonNode *node = Dequeue(queue); //deleting from queue
        root1 = InsertPokemonNode(root1, node->data->id, 1);//inserting first element into queue
        if (node->left != NULL)
            Enqueue(queue, node->left);
        if (node->right != NULL)
            Enqueue(queue, node->right);
    }
    free(queue);
    return root1;
}

OwnerNode *FindOwnerByName(const char *name) {
    OwnerNode *owner = ownerHead;
    do {
        if (strcmp(owner->ownerName, name) == 0)
            return owner;
        owner = owner->next;
    } while (owner != ownerHead);
    return NULL;
}

void SortOwners(void) {//i'm guessing i need to put list in the array dymanically allocated and do the same quicksort
    if (ownerHead == NULL) { //i need to do array from the data in the list
        printf("No existing owners.\n");
        return;
    }
    int size = 0;
    ownerNames = InitOwnerArray(ownerHead, &size);
    //i will scream i swear i will
    BubbleSort(ownerNames, &size); //putting everything back in the list?
    OwnerNode *owner = ownerHead;
    for (int i = 0; i < size; i++) {
        owner->ownerName = ownerNames[i]->ownerName;
        owner->pokedexRoot = ownerNames[i]->pokedexRoot;
        owner = owner->next;
    }
    printf("Owners sorted by name.");
    free(ownerNames); //freeing array
}

OwnerNode** InitOwnerArray(OwnerNode *owner, int *size) { //array of owner names i hope
    int capacity = 10;
    ownerNames = malloc(sizeof(OwnerNode *) * capacity);
    if (ownerNames == NULL) {
        printf("Memory allocation error.\n");
        exit(1);
    }
    do {
        if (*size >= capacity) {
            capacity *= 2;
            ownerNames = realloc(ownerNames, sizeof(OwnerNode *) * capacity);
            if (ownerNames == NULL) {
                printf("Memory allocation error.\n");
                exit(1);
            }
        }
        ownerNames[*size] = owner;
        (*size)++;
        owner = owner->next;
    } while (owner->next != ownerHead);
    return ownerNames;
}

void BubbleSort(OwnerNode **ownerNames, int *size) {
    for (int i = 0; i < *size - 1; i++) {
        for (int j = 0; j < *size - i - 1; j++) {
            if (CompareByNameOwners(&ownerNames[j]->ownerName, &ownerNames[j + 1]->ownerName) > 0) {
                SwapOwnerData(ownerNames[j], ownerNames[j + 1]);
            }
        }
    }
}

void SwapOwnerData(OwnerNode *a, OwnerNode *b) { //mega swap
    PokemonNode *temp = a->pokedexRoot;
    a->pokedexRoot = b->pokedexRoot;
    b->pokedexRoot = temp;
    char *temp2 = a->ownerName;
    a->ownerName = b->ownerName;
    b->ownerName = temp2;
}

int CompareByNameOwners(const void *a, const void *b) {
    char *name1 = *(char **)a;
    char *name2 = *(char **)b;
    return strcmp(name1, name2);
}

void PrintOwnersCircular(void) {
    OwnerNode *owner = ownerHead;
    if (owner == NULL) {
        printf("No existing owners.\n");
        return;
    }
    char direction;
    while (1) {
        printf("Enter direction (F or B):\n");
        scanf(" %c", &direction);
        if (direction != 'F' && direction != 'B' && direction != 'f' && direction != 'b') {
            printf("Invalid direction, must be F or B.\n");
        }
        else
            break;
    }
    scanf("%*c");
    printf("How many prints?\n");
    int num;
    num = readIntSafe("");
    if (num < 0) {
        printf("Invalid number.\n");
        return;
    }
    if (direction == 'F' || direction == 'f') {
        for (int i = 0; i < num; i++) {
            printf("[%d] ", i + 1);
            printf("%s\n", owner->ownerName);
            owner = owner->next;
        }
    }
    else {
        for (int i = 0; i < num; i++) {
            printf("[%d] ", i + 1);
            owner = owner->prev;
            printf("%s\n", owner->ownerName);
        }
    }
}

void FreeAllOwners(void) {
    if (ownerHead == NULL) {
        return;
    }
    OwnerNode *owner;
    OwnerNode *current = ownerHead;
    do {
        owner = current->next; //next owner
        FreePokemonTree(current->pokedexRoot); //freeing tree
        free(current->ownerName);//freeing name
        free(current); //freeing pointer
        current = owner;
    } while (owner != ownerHead);
    //printf("and we're golden\n");
    ownerHead = NULL;
}