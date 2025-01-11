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
void displayMenu(OwnerNode *owner)
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
        displayBFS(owner->pokedexRoot);
        break;
    case 2:
        preOrderTraversal(owner->pokedexRoot);
        break;
    case 3:
        inOrderTraversal(owner->pokedexRoot);
        break;
    case 4:
        postOrderTraversal(owner->pokedexRoot);
        break;
    case 5:
        displayAlphabetical(owner->pokedexRoot);
        break;
    default:
        printf("Invalid choice.\n");
    }
}

// --------------------------------------------------------------
// Sub-menu for existing Pokedex
// --------------------------------------------------------------
void enterExistingPokedexMenu()
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
    while (node != NULL){ //while the node is nor NULL
        printf("%d. %s\n", count, node->ownerName);
        node = node->next; //next one
        count++;
    }
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
            addPokemon(current);
            break;
        case 2:
            displayMenu(current);
            break;
        case 3:
            freePokemon(current);
            break;
        case 4:
            pokemonFight(current);
            break;
        case 5:
            evolvePokemon(current);
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
void mainMenu()
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
            openPokedexMenu();
            break;
        case 2:
            enterExistingPokedexMenu();
            break;
        case 3:
            deletePokedex();
            break;
        case 4:
            //mergePokedexMenu();
            break;
        case 5:
            //sortOwners();
            break;
        case 6:
            //printOwnersCircular();
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
    mainMenu();
    //freeAllOwners();
    return 0;
}

void openPokedexMenu(void) {
    OwnerNode *owner = (OwnerNode *)malloc(sizeof(OwnerNode));
    PokemonNode *node = (PokemonNode *)malloc(sizeof(PokemonNode));
    if (owner == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    if (node == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    printf("Your name:\n");
    if (ownerHead == NULL) {
        ownerHead = owner; //if head of the linked list is null then the linked list is empty
        ownerHead->ownerName = getDynamicInput(); //need to add node to the binary tree
        ownerHead->prev = NULL; //previous node becomes the last one i added to linked list
        ownerHead->next = NULL; //next node is null to know when the linked list ends
        //add data to root of binary tree
        //HOW? i have an array of pokemon data so i need to add pokemondata[] to data struct. da fuck?
        ownerHead->pokedexRoot = StarterPokemon(node);
        printf("New Pokedex created for %s with starter %s.\n",ownerHead->ownerName, ownerHead->pokedexRoot->data->name);
    }
    else { //same thing but when linked list is not empty
        OwnerNode *current = ownerHead;
        owner->ownerName = getDynamicInput();
        owner->prev = NULL;
        owner->next = NULL;
        //going to the end of linked list
        while (current->next != NULL)
            current = current->next;
        current->next = owner; //next in line
        owner->prev = current; //previous becomes current
        owner->pokedexRoot = StarterPokemon(node); //initializing binqry tree. i hope
        printf("New Pokedex created for %s with starter %s.\n",owner->ownerName, owner->pokedexRoot->data->name);
    }
}

PokemonNode *StarterPokemon(PokemonNode *node) {
    int choice;
    printf("Choose Starter:\n\
                1. Bulbasaur\n\
                2. Charmander\n\
                3. Squirtle\n");
    choice = readIntSafe("Your choice: ");
    switch (choice) {
        case 1: {
            return createPokemonNode(&pokedex[0]);
        }
        case 2: {
            return createPokemonNode(&pokedex[3]);
        }
        case 3: {
            return createPokemonNode(&pokedex[6]);
        }
        default:
            printf("Invalid choice.\n");
            return StarterPokemon(node);
    } //recursion supremacy
}

PokemonNode *createPokemonNode(const PokemonData *data) {
    PokemonNode *node = (PokemonNode *)malloc(sizeof(PokemonNode));
    if (node == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    node->data = data;
    node->left = node->right = NULL;//right and left nodes == NULL because it's the first one in tree
    return node;
}

OwnerNode *ListLookUp(int choice) { //looking for a specific person in list and returning their name
    OwnerNode *current = ownerHead;
    int count = 1;
    while (current != NULL) {
        if (count == choice)
            break;
        current = current->next;
        count++;
    }
    return current;
}

void addPokemon(OwnerNode *owner) {
    int pokemonId;
    printf("Enter ID to add:\n");
    pokemonId = readIntSafe("");
    if (pokemonId < 1|| pokemonId > 151) {
        printf("Invalid ID.\n");
        return;
    }
    //printf("Current Node ID: %d, Inserting Pokemon ID: %d\n", owner->pokedexRoot->data->id, pokemonId);
    owner->pokedexRoot = insertPokemonNode(owner->pokedexRoot, pokemonId, 0);
    //printf("Pokemon %s (ID %d) added.\n", owner->pokedexRoot->data->name, pokemonId);
}

PokemonNode *insertPokemonNode(PokemonNode *newNode, int pokemonId, int subChoice) {
    if (newNode == NULL) {
        //if tree empty return node
        // printf("Tree is empty. Creating new node for Pokemon ID: %d\n", pokemonId);
        // printf("Pokemon ID %d added to the Pokedex.\n", pokemonId);
        PokemonNode *node = createPokemonNode(&pokedex[pokemonId - 1]);
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
        newNode->right = insertPokemonNode(newNode->right, pokemonId, subChoice);
    }
    else if (newNode->data->id > pokedex[pokemonId - 1].id) {
        //if id is smaller then left node
        //printf("Going left from Node ID: %d\n", newNode->data->id);
        newNode->left = insertPokemonNode(newNode->left, pokemonId, subChoice);
    }
    return newNode;
}

void displayBFS(PokemonNode *root) {
    int h = Height(root);
    for (int i = 1; i <= h; i++) {
        PrintLevel(root, i);
    }
}
//need to find *height* of a tree
int Height(PokemonNode *node) {
    if (node == NULL)
        return 0;
    int leftHeight = Height(node->left); //height of left nodes
    int rightHeight = Height(node->right); //height of right nodes
    if (leftHeight > rightHeight) //if left node is bigger than right return leftheight + 1
        return leftHeight + 1;
    return rightHeight + 1;//else return rightheight + 1
}

void PrintLevel(PokemonNode *root, int level) {
    if (root == NULL)
        return;
    if (level == 1)
        PrintPokemon(root);
    PrintLevel(root->left, level - 1);
    PrintLevel(root->right, level - 1);
}

void preOrderTraversal(PokemonNode *root) {
    if (root == NULL)
        return;
    PrintPokemon(root);
    preOrderTraversal(root->left);
    preOrderTraversal(root->right);
}

void inOrderTraversal(PokemonNode *root) {
    if (root == NULL)
        return;
    inOrderTraversal(root->left);
    PrintPokemon(root);
    inOrderTraversal(root->right);
}

void postOrderTraversal(PokemonNode *root) {
    if (root == NULL)
        return;
    postOrderTraversal(root->left);
    postOrderTraversal(root->right);
    PrintPokemon(root);
}

void displayAlphabetical(PokemonNode *root) {
    NodeArray pokemons;
    initNodeArray(&pokemons, 1);
    //printf("Adding Pokemon ID: %d, Name: %s to NodeArray.\n", root->data->id, root->data->name);
    collectAll(root, &pokemons);
    qsort(pokemons.nodes,pokemons.size, sizeof(PokemonNode*), compareByNameNode);
    for (int i = 0; i < pokemons.size; i++) {
        PrintPokemon(pokemons.nodes[i]);
    }
}

void initNodeArray(NodeArray *na, int cap) {
    na->nodes = malloc(cap * sizeof(PokemonNode *)); //initializing array inside of struct
    if (na->nodes == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    na->size = 0;
}

void addNode(NodeArray *na, PokemonNode *node) { //i'm very not sure in my doings here
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

void collectAll(PokemonNode *root, NodeArray *na) { //bro i'm trying to destroy these data structures it just happens that they somehow end up working
    if (root == NULL) //base case
        return;
   // printf("Adding Pokemon ID: %d, Name: %s to NodeArray.\n", root->data->id, root->data->name);
    addNode(na, root);
   // printf("Traversing left from Pokemon ID: %d, Name: %s\n", root->data->id, root->data->name);
    collectAll(root->left, na);
    //printf("Traversing right from Pokemon ID: %d, Name: %s\n", root->data->id, root->data->name);
    collectAll(root->right, na);
}

int compareByNameNode(const void *a, const void *b) { //how the fuck do i implement qsort here?
    PokemonNode *nodeA = *(PokemonNode **)a;
    PokemonNode *nodeB = *(PokemonNode **)b;
    return strcmp(nodeA->data->name, nodeB->data->name);
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
void freePokemon(OwnerNode *owner) {
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
    if (searchPokemonBFS(owner->pokedexRoot, pokemonId) == NULL) {
        printf("No Pokemon with ID %d found.\n", pokemonId);
        return;
    }
    printf("Removing Pokemon %s (ID %d)\n", searchPokemonBFS(owner->pokedexRoot, pokemonId)->data->name, pokemonId);
    owner->pokedexRoot = ReleasePokemon(owner->pokedexRoot, searchPokemonBFS(owner->pokedexRoot, pokemonId)->data);
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

void freePokemonTree(PokemonNode *root) {
    if (root == NULL)
        return;
    freePokemonTree(root->left);
    freePokemonTree(root->right);
    free(root);
}

void freeOwnerNode(OwnerNode *owner) { //how am i contring if the owner in the middle of the list?
    if (owner == NULL)
        return;
    if (owner == ownerHead) { //if deleteing from start of the list
        ownerHead = owner->next; //owner head becomnes next one
        ownerHead->prev = NULL; //owner previous null
        freePokemonTree(owner->pokedexRoot);
        free(owner->ownerName);
        free(owner);
        owner = NULL;
        return;
    }
    if (owner->next == NULL) { //if next one is NULL then put NULL in the current
        owner->prev->next = NULL; //next pointer to previous becomes null
        owner->prev = owner->prev->prev;//pointer to previous becomes previous before previous
        freePokemonTree(owner->pokedexRoot);
        free(owner->ownerName);
        free(owner);
        owner = NULL;
        return;
    } //if owner next not NULL then next becomes next after next
    owner->prev->next = owner->next; //next after prev becomes next after current
    owner->next->prev = owner->prev; //perv before next becomes prev after current
    freePokemonTree(owner->pokedexRoot);
    free(owner->ownerName);
    free(owner);
    owner = NULL;
}

void FreeHead() {
    if (ownerHead == NULL)
        return;
    freePokemonTree(ownerHead->pokedexRoot);
    free(ownerHead->ownerName);
    free(ownerHead);
    ownerHead = NULL;
}

void deletePokedex(void) {
    printf("\n=== Delete a Pokedex ===\n");
    if (ownerHead == NULL) {
        printf("No existing Pokedexes.\n");
        return;
    }
    int count = 1;
    int choice;
    OwnerNode *node = ownerHead;//first one in the list
    while (node != NULL){ //while the node is nor NULL
        printf("%d. %s\n", count, node->ownerName);
        node = node->next; //next one
        count++;
    }
    printf("Choose a Pokedex to delete by number: \n");
    choice = readIntSafe("");
    while (choice > count - 1 || choice < 1) {
        printf("Invalid choice.\n");
        choice = readIntSafe("");
    }
    OwnerNode *current = ListLookUp(choice);
    printf("Deleting %s's entire Pokedex...\n", current->ownerName);
    if (current == ownerHead && current->next == NULL) {
        FreeHead();
        printf("Pokedex deleted.\n");
        return;
    }
    freeOwnerNode(current);
    printf("Pokedex deleted.\n");
}

PokemonNode *searchPokemonBFS(PokemonNode *root, int id) {
    if (root == NULL || root->data->id == id)
        return root;
    if (root->data->id > id) //bigger search right
        return searchPokemonBFS(root->left, id);
    return searchPokemonBFS(root->right, id); //smaller search left
}

void pokemonFight(OwnerNode *owner) {
    if (owner->pokedexRoot == NULL) {
        printf("Pokedex is empty.\n");
        return;
    }
    int id1, id2;
    printf("Enter ID of the first Pokemon: \n");
    id1 = readIntSafe("");
    printf("Enter ID of the second Pokemon: \n");
    id2 = readIntSafe("");
    if ((id1 || id2) < 1 || (id1 || id2) > 151) {
        printf("Invalid ID.\n");
        return;
    }
    if (searchPokemonBFS(owner->pokedexRoot, id1) == NULL || searchPokemonBFS(owner->pokedexRoot, id2) == NULL) {
        printf("One or both Pokemon IDs not found.\n");
        return;
    }
    double score1, score2;
    printf("Pokemon 1: %s ", searchPokemonBFS(owner->pokedexRoot, id1)->data->name);
    score1 = searchPokemonBFS(owner->pokedexRoot, id1)->data->hp * 1.2 + searchPokemonBFS(owner->pokedexRoot, id1)->data->attack * 1.5;
    printf("(Score = %.2f)\n", score1);
    printf("Pokemon 2: %s ", searchPokemonBFS(owner->pokedexRoot, id2)->data->name);
    score2 = searchPokemonBFS(owner->pokedexRoot, id2)->data->hp * 1.2 + searchPokemonBFS(owner->pokedexRoot, id2)->data->attack * 1.5;
    printf("(Score = %.2f)\n", score2);
    if (score1 > score2)
        printf("%s wins!\n", searchPokemonBFS(owner->pokedexRoot, id1)->data->name);
    else if (score2 > score1)
        printf("%s wins!\n", searchPokemonBFS(owner->pokedexRoot, id2)->data->name);
    else
        printf("It's a tie!\n");
}

void evolvePokemon(OwnerNode *owner) {
    if (owner->pokedexRoot == NULL) {
        printf("Pokedex is empty.\n");
        return;
    }
    int id;
    printf("Enter ID of Pokemon to evolve: \n");
    id = readIntSafe("");
    if (id < 1 || id > 151) { //id out of bounds
        printf("Invalid ID.\n");
        return;
    }
    if (searchPokemonBFS(owner->pokedexRoot, id) == NULL) { //there is no pokemon in pokedex
        printf("Pokemon ID not found.\n");
        return;
    }
    if (searchPokemonBFS(owner->pokedexRoot, id)->data->CAN_EVOLVE == CANNOT_EVOLVE) {//pokemon cannot evolve
        printf("Pokemon cannot evolve.\n");
        return;
    }
    else if (searchPokemonBFS(owner->pokedexRoot, id)->data->CAN_EVOLVE == CAN_EVOLVE) { //pokemon can evolve
        printf("Pokemon evolved from %s (ID %d) to %s (ID %d)\n", searchPokemonBFS(owner->pokedexRoot, id)->data->name, id,
                                                                pokedex[id].name, id + 1);
        owner->pokedexRoot = ReleasePokemon(owner->pokedexRoot, searchPokemonBFS(owner->pokedexRoot, id)->data); //case where evolving happens
        owner->pokedexRoot = insertPokemonNode(owner->pokedexRoot, id + 1, 1);
    }
}
