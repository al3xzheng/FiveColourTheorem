#include <iostream>
#include <vector>
#include <set>
#include <cstring>

// INPUT: NUMBER OF NODES
const int nodes = 5;

// Toggles kempe chain of colours c1, c2 containing node: node, where c1 is the colour of node.
void toggleKempeChain (int graph[][nodes+1], int colors [], int c1, int c2, int node) {
    colors[node] = c2;
    for(int i = 0 ; i < nodes; i++) {
        if(graph[node][i] && colors[i] == c2) {
            toggleKempeChain(graph, colors, c2, c1, i);
        }
    }
}

// Populates Kempechain vector with kempechain of colours: color, and c2, where c2 is the colour of the node: node.
void KempeSet (int graph [nodes][nodes +1], int colors [], int prev, int node, int c1, int c2, std::set <int>& Kempechain) {

    Kempechain.insert(node);

    for(int i = 0 ; i < nodes; i++) {
        if(graph[node][i] && Kempechain.find(i) == Kempechain.end() && colors[i] == c1) {
            KempeSet(graph, colors, node, i, c2, c1, Kempechain);
        }
    }

}

// Returns the colour of an uncoloured node based on Kempe's method. degree (node) < 6.
int colour(int graph [][nodes+1], int colors [] , int node) {

    if(colors[node] != 0)
        return colors[node];

    int colours [5] = {0};

    // neighbours of node: node that are coloured
    int neighbours [nodes] = {-1};

    // number of coloured neighbours of node: node
    int numColouredNodes = 0;


    // Count the neighbouring colours and track these nodes.
    for(int i = 0; i < nodes;i++){
        if(graph[node][i]) {
            colours[colors[i]]++;
            if(colors[i]!=0) {
                neighbours[numColouredNodes] = i;
                numColouredNodes++;
            }
        }
    }

    // Kempe method not applicable, try again later.
    if(numColouredNodes >5)
        return -1;

    // Colour the node an open colour.
    for(int i = 1 ; i < 5; i++) {
        if(colours[i] == 0)
            return i;
    }

    // Need to apply kempe chain method since no open, available colour, but < 6 coloured neighbours.
    std::set <int> KempeChain;
    int toggledColor;

    // Case: 4 coloured neighbours
    if(numColouredNodes == 4) {
        // choose a node.
        for(int i = 0 ; i < 4 ; i++) {
            KempeChain.clear();

            // choose another node
            for(int j = i+1 ; j < 4; j++) {
                
                // Find Kempe chain
                KempeSet(graph, colors, -1, neighbours[i], colors[neighbours[j]], colors[neighbours[i]], KempeChain);

                // If there's no kempe chain, we can toggle and colour the unknown node: node the toggled colour.
                if(KempeChain.find(neighbours[j]) == KempeChain.end()) {
                    toggledColor = colors[neighbours[i]];
                    toggleKempeChain(graph, colors, colors[neighbours[i]], colors[neighbours[j]], neighbours[i]);
                    return toggledColor;
                }
            }
        }
        std::cout<<"error in 4 neighbours of diff colours"<<std::endl;
    }

    // Case: 5 coloured neighbours
    if(numColouredNodes == 5) {

        int node1 = -1;
        int node1temp;
        int node2 = -1;
        int node2temp;

        // obtain node1, node2
        for(int i = 0 ; i < 5; i++) {
            if(colours[colors[neighbours[i]]] == 2){
                if(node1 == -1)
                    node1 = neighbours[i];
                else if(node2 == -1) {
                    node2 = neighbours[i];
                }
            }
        }


        // Scenario 1: checking unique coloured nodes
        // Choose a node
        for(int i = 0 ; i < 5;i++) {

            // This node has to be unique-colored. We want to toggle one of the 3 unique coloured neighbours.
            if(neighbours[i] == node1 || neighbours[i] == node2)
                continue;

            // Choose another node.
            for(int j = i + 1; j < 5; j++) {

                // This node has to be unique-colored. We want to toggle one of the 3 unique coloured neighbours.
                if(neighbours[j] == node1 || neighbours[j] == node2)
                    continue;

                KempeChain.clear();

                // Obtain kempe chain of the two nodes.
                KempeSet(graph, colors, -1, neighbours[i], colors[neighbours[j]], colors[neighbours[i]], KempeChain);

                // If there's no kempe chain, we can toggle and colour the unknown node: node, the toggled colour.
                if(KempeChain.find(neighbours[j]) == KempeChain.end()) {
                    toggledColor = colors[neighbours[i]];

                    toggleKempeChain(graph, colors, colors[neighbours[i]], colors[neighbours[j]], neighbours[i]);
                    return toggledColor;
                }
            }

        }

        // Scenario 2: toggling the 2 nodes with same colouring
        // node: node1, node2 are the 2 neighbours with same colouring

        // Solve for second scenario
        // Choose a node.

        toggledColor = colors[node1];

        node1temp = node1;
        node2temp = node2;

        for(int i = 0; i < 5;i++) {


            // Choose a unique coloured node.
            // we need to find a colour that is not the same color as the double-colored node or else
            // even if there is no kempe chain between the 2 diff coloured nodes, the kempe chain contains
            // 2 nodes in the set of neighbours, which errors out, this is due to every node in neighbours being
            // adjacent to a node that is colored the colour of the double-colourign.
            if(neighbours[i] == node1temp || neighbours[i] == node2temp)
                continue;
            
            // Check if node1 has already been toggled...
            if(node1 != -1) {

                KempeChain.clear();
                KempeSet(graph, colors, -1, node1, colors[neighbours[i]], toggledColor, KempeChain);

                // If there's no kempe chain, we can toggle and colour node1 this colour on [i].
                if(KempeChain.find(neighbours[i]) == KempeChain.end()) {
                    toggleKempeChain(graph, colors, colors[node1], colors[neighbours[i]], node1);

                    // if node2 is already toggled, then return the colour that was doubly coloured.
                    if(node2 == -1)
                        return toggledColor;
                    node1 = -1;
                }
            }

            if(node2 != -1) {
                KempeChain.clear();
                KempeSet(graph, colors, -1, node2, colors[neighbours[i]], toggledColor, KempeChain);
                if(KempeChain.find(neighbours[i]) == KempeChain.end()) {
                    toggleKempeChain(graph, colors, colors[node2], colors[neighbours[i]], node2);
                    if(node1 == -1)
                        return toggledColor;
                    node2 = -1;
                }
            }
            
        }
        std::cout<<"error in 5 neighbours of diff colours"<<std::endl;

    }

    std::cout<<"has counterexample case or a K5 minor"<<std::endl;
    return 5;

}

// Function to delete a node, once it and its neighbours have been coloured.
void deleteNode(int graph [][nodes+1], int node, std::vector <int>& deletedNodes) {

    // delete connection from neighbours
    for(int i = 0 ; i < nodes; i++) {
        if(graph[node][i] == 1) {
            graph[i][node] = 0;
            graph[i][nodes]--;
            graph[node][i] = 0;
        }
    }

    // nullify total degree count at end as well
    graph[node][nodes] = -1;

    // insert deleted node into tracker for post processing
    deletedNodes.push_back(node);

}

// Restores a previously deleted node to colour it.
void restoreNode (int node, int graph [][nodes+1], int copy [][nodes+1], std::set <int>& restoredNodes) {

    for(int i = 0 ; i < nodes;i++) {
        if(copy[node][i] && restoredNodes.find(i) != restoredNodes.end()) {
            graph[node][i] = 1;
        }
    }

    restoredNodes.insert(node);

}

int main () {

    int graph [nodes][nodes+1]=  {

// INPUT: uncolourd GRAPH!!!
// {
        {0, 1, 1, 1, 1, 0},
        {1, 0, 1, 1, 1, 0},
        {1, 1, 0, 1, 1, 0},
        {1, 1, 1, 0, 1, 0},
        {1, 1, 1, 1, 0, 0}
// };

    };

    int copy [nodes][nodes+1];

    std::memcpy(copy, graph, sizeof(graph));

    int degree;

    // Store degree of each node at end
    for(int i = 0 ; i < nodes;i++) {
        degree = 0;
        for(int j = 0 ; j < nodes;j++) {
            degree += graph[i][j];
        }
        graph[i][nodes] = degree;
    }

    std::vector <int> deletedNodes;
    std::set <int> recoveredNodes = {0};

    int colors [nodes] = {0};

    int numDeletedNodes = 0;
    int index  = 0;

    while(numDeletedNodes < nodes) {
        if(graph[index%nodes][nodes] < 6 && graph[index%nodes][nodes] >= 0 && colors[index%nodes] == 0) {
            std::cout<<" "<< index%nodes;
            deleteNode(graph, index%nodes, deletedNodes);
            numDeletedNodes++;
        }
        index++;
    }

    for(int i = nodes-1; i >= 0 ; i--) {
        restoreNode(deletedNodes[i], graph, copy, recoveredNodes);
        colors[deletedNodes[i]] = colour(graph, colors, deletedNodes[i]);
        if(colors[deletedNodes[i]] == 0) {
            std::cout<<"err";
        }
    }
    std::cout<<" \n\n";

        // print colours.
    for(int i = 0 ; i < nodes;i++) {
        std::cout<<colors[i] << + " ";

    }

    return 1;
}