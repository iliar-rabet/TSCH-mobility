// // Node structure
// typedef struct Node {
//     int data;
//     struct Node* next;
// } Node;

// // Linked List structure
// typedef struct LinkedList {
//     Node* head;
// } LinkedList;

// // Function to create a new node
// Node* createNode(int data) {
//     Node* newNode = (Node*)malloc(sizeof(Node));
//     newNode->data = data;
//     newNode->next = NULL;
//     return newNode;
// }

// // Function to initialize the linked list
// void initializeList(LinkedList* list) {
//     list->head = NULL;
// }

// // Function to add a node to the end of the list
// void addNode(LinkedList* list, int data) {
//     Node* newNode = createNode(data);
    
//     if (list->head == NULL) {
//         // If the list is empty, set the new node as the head
//         list->head = newNode;
//     } else {
//         // Traverse to the end of the list
//         Node* current = list->head;
//         while (current->next != NULL) {
//             current = current->next;
//         }
//         // Add the new node at the end
//         current->next = newNode;
//     }
// }

// // Function to remove a node with a specific value from the list
// void removeNode(LinkedList* list, int data) {
//     Node* current = list->head;
//     Node* previous = NULL;
    
//     // Traverse the list to find the node to remove
//     while (current != NULL && current->data != data) {
//         previous = current;
//         current = current->next;
//     }
    
//     if (current == NULL) {
//         // Node with the specified data was not found
//         printf("Node with data %d not found.\n", data);
//         return;
//     }
    
//     if (previous == NULL) {
//         // Removing the head node
//         list->head = current->next;
//     } else {
//         // Bypass the node to remove it
//         previous->next = current->next;
//     }
    
//     // Free the memory of the removed node
//     free(current);
//     printf("Node with data %d removed.\n", data);
// }

// // Function to display the contents of the list
// void displayList(LinkedList* list) {
//     Node* current = list->head;
//     printf("List: ");
//     while (current != NULL) {
//         printf("%d ", current->data);
//         current = current->next;
//     }
//     printf("\n");
// }