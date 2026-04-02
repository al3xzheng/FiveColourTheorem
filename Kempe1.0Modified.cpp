#include <iostream>
#include <vector>
#include <set>
#include <cstring>
#include <algorithm>

#include "viz.hpp"

// const int nodes = 28;

// Toggles kempe chain of colours c1, c2 containing node: node, where c1 is the colour of node.
void toggleKempeChain (std::vector <std::vector<int>>& graph , std::vector <int>& colors, int c1, int c2, int node) {
    colors[node] = c2;
    for(int i = 0 ; i < graph[0].size()-1; i++) {
        if(graph[node][i] && colors[i] == c2) {
            toggleKempeChain(graph, colors, c2, c1, i);
        }
    }
}

// Populates Kempechain vector with kempechain of colours: color, and c2, where c2 is the colour of the node: node.
void KempeSet (std::vector <std::vector<int>>& graph , std::vector <int>& colors, int prev, int node, int c1, int c2, std::set <int>& Kempechain) {

    Kempechain.insert(node);

    for(int i = 0 ; i < graph[0].size()-1; i++) {
        if(graph[node][i] && Kempechain.find(i) == Kempechain.end() && colors[i] == c1) {
            KempeSet(graph, colors, node, i, c2, c1, Kempechain);
        }
    }

}

// Returns the colour of an uncoloured node based on Kempe's method. degree (node) < 6.
int colour(std::vector <std::vector<int>>& graph , std::vector <int>& colors , int node) {

    if(colors[node] != 0)
        return colors[node];

    int colours [5] = {0};

    // neighbours of node: node that are coloured
    int neighbours [5] = {-1};

    // number of coloured neighbours of node: node
    int numColouredNodes = 0;


    // Count the neighbouring colours and track these nodes.
    for(int i = 0; i < graph[0].size()-1;i++){
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
        std::cerr<<"error in 4 neighbours of diff colours"<<std::endl;
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
        std::cerr<<"error in 5 neighbours of diff colours"<<std::endl;

    }

    // kempe method error. should never happen
    std::cerr<<"Non-planar"<<std::endl;
    return -2;

}

// Function that returns if the neighbours of an uncoloured node: node have all been successfully coloured.
bool colourNeighbours (std::vector <std::vector<int>>& graph , std::vector <int>& colors , int node, int * colouredNodes) {
    std::vector <int> uncolouredNeighbours;
    for(int i = 0 ; i < graph[0].size()-1; i++) {
        if(graph[node][i] && colors[i] == 0) {
            uncolouredNeighbours.push_back(i);
            colors[i] = colour(graph, colors, i);

            viz::waitForStep(); 
            viz::printState(graph, colors, i, graph[0].size()-1, false ? "done" : "running");

            *colouredNodes++;
            // the below cases should never occur on an originally uncoloured graph
            if(colors[i] == -1) {
                std::cerr<<"Not kempe-based colourable\n";
                return false;
            }            
            if(colors[i] == -2) {
                std::cerr<<"Non planar\n";
                return false;
            }
            // if(colors[i] == -1 || colors[i] == -2) {
            //     //reset coloured neighbours
            //     if(colors[i] == -1)
                
            //     colouredNodes-= uncolouredNeighbours.size();
            //     for(int j = 0; j < uncolouredNeighbours.size(); j++) {
            //         colors[uncolouredNeighbours[j]] = 0;
            //     }
            //     return false;
            // }
        }
    }
    return true;
}


// Function to delete a node, once it and its neighbours have been coloured.
void deleteNode(std::vector <std::vector<int>>& graph, int node, std::vector <int>& deletedNodes) {

    // delete connection from neighbours
    for(int i = 0 ; i < graph[0].size()-1; i++) {
        if(graph[node][i] == 1) {
            graph[i][node] = 0;
            graph[i][graph[0].size()-1]--;
            graph[node][i] = 0;
        }
    }

    // nullify total degree count at end as well
    graph[node][graph[0].size()-1] = -1;

    // insert deleted node into tracker for post processing
    deletedNodes.push_back(node);

}

// Postprocessor that returns if a deleted node no longer satisfies 4-colouring => 5 colour theorem.
bool deletedNodeError(std::vector <std::vector<int>>& graph, int node, std::vector <int>& colors) {

    // Checks neighbours and see if there's a same colouring of adjacent nodes.
    for(int i = 0 ; i < (graph[0].size()-1); i++) {
        if(graph[node][i] && colors[node] == colors[i])
            return true;
    }

    return false;
}

// PreProcessing function that returns if the original graph has any colouring conflict and 
// removes coloured nodes whose neighbours are also coloured (properly) for postprocessing.
bool preProcessing (std::vector <std::vector<int>>& graph, std::vector <int>& colors, std::vector <int>& deletedNodes, int * colouredNodes) {

    int colour = 0;
    bool remove;

    // Go through all nodes of graph
    for(int i = 0 ; i < graph[0].size()-1;i++) {

        // obtain colour
        colour = colors[i];

        // if uncoloured, no possible colouring conflict is possible nor is it reducible.
        if(colour == 0)
            continue;

        // flag to remove a node
        remove = true;

        // go through neighbours of node [i]
        for(int j = 0 ; j < graph[0].size()-1; j++) {

            // if an edge exists between node i and node j...
            if(graph[i][j]) {

                // if node j is coloured...
                if(colors[j] != 0) {

                    // if node i and node j are same colouring...
                    if(colors[j] == colour) {

                        // colouring conflict
                        std::cerr<<"this graph has a coloring conflict\n"<<std::endl;
                        return false;
                    }
                }

                // if even one neighbour is not coloured, node i can not be removed.
                else {
                    remove = false;
                }
            }   
        }
        // after going through neighbours, if no colouring conflict, and all its neighbours, as well as itself, is coloured....
        if(remove) {
            //remove the node; store for postprocessing
            deleteNode(graph, i, deletedNodes);
        
            viz::waitForStep(); 
            viz::printState(graph, colors, i, graph[0].size()-1, false ? "done" : "running");
        }

    }
    
    //no colouring conflicts.
    return true;
}

int numColouredNodes (std::vector <int>& colors) {
    int numColours = 0;
    for(int i = 0 ; i < size(colors);i++) {

        
        numColours += (colors[i] != 0);

    }

    return numColours;
}

int main () {

    std::vector <std::vector<int>> graph;
    std::vector <int> colors;

    int n;

    if (!viz::loadGraph(graph, n)) {
        std::cerr << "[main] No graph received from server. Run via: python server.py --binary ./program.exe" << std::endl;
        return 1;
    }

    std::vector <std::vector<int>> copy (graph);

    colors.assign(n, 0);
    colors = {0, 3, 4, 2, 1, 5, 1, 5, 3, 4, 5, 1, 2, 3, 2, 5, 4, 2, 5, 4, 3, 2, 3, 1, 4, 2, 1, 3, 2, 5, 5, 2, 4, 4, 3};

    std::vector <int> deletedNodes;

    int colouredNodes = 0;

    int index  = 0;

    viz::printState(graph, colors, index, graph[0].size()-1, false ? "done" : "running");

    // Kempes method says 
    if(preProcessing(graph, colors, deletedNodes, &colouredNodes)) {

        // add failing condiitoin so no infinite loop on colouredNodes.
        while(numColouredNodes(colors) < (graph[0].size()-1)) {
            // if an uncoloured node has a degree < 6
            if(graph[index%(graph[0].size()-1)][graph[0].size()-1] < 6 && colors[index%(graph[0].size()-1)] == 0) {
                // std::cout<<index%graph[0].size()-1<<std::endl;

                if(colourNeighbours(graph, colors, index%(graph[0].size()-1), &colouredNodes)) {
                    // colouredNodes += graph[index%nodes][nodes];
                    colors[index%(graph[0].size()-1)] = colour(graph, colors, index%(graph[0].size()-1));

                    viz::waitForStep(); 
                    viz::printState(graph, colors, index, graph[0].size()-1, false ? "done" : "running");

                    // If kempe method error... should never happen
                    if(colors[index%(graph[0].size()-1)] == -2) {
                        std::cerr<<"kempe method algorithm error"<<std::endl;
                        break;
                    }

                    // If unsuccessful colouring... This should never happen!!! Since we're using our reduced/induction-type
                    // method with node removal. Should never happen as well. This case can only happen with the neighbours, not
                    // with the emphaiszed node itself.
                    if(colors[index%(graph[0].size()-1)] == -1) {
                        colors[index%(graph[0].size()-1)] = 0;
                        std::cerr<<"Node should be colourable but isn't for some reason\n";
                        break;
                    }

                    // otherwise, SUCCESSFUL colouring
                    else {
                        deleteNode(graph, index%(graph[0].size()-1), deletedNodes);
                        colouredNodes++;
                    }

                    viz::waitForStep(); 
                    viz::printState(graph, colors, index, graph[0].size()-1, false ? "done" : "running");
                }

                else {
                    for(int k = 0 ; k < graph[0].size()-1;k++) {
                        std::cerr<<colors[k] << + " ";
                    }
                    std::cerr<<"the graph is not 5-colourable using kempe-based method"<<std::endl;
                    break;
                }
            }

            index++;

        }
        viz::waitForStep(); 
        viz::printState(copy, colors, 0, graph[0].size()-1, false ? "done" : "running");

        // Go through deleted nodes and see if a colouring error exists, if so, use 5 colouring.
        // Go through the deleted nodes in an inductive, recursive order from the recently deleted node to the first deleted node
        // # stackstack.
        for(int i = deletedNodes.size()-1; i >= 0; i--) {
            if(deletedNodeError(copy, deletedNodes[i], colors)) {
                colors[deletedNodes[i]] = 5;

                viz::waitForStep(); 
                viz::printState(copy, colors, i, graph[0].size()-1, false ? "done" : "running");
            }

        }
        viz::waitForStep(); 
        viz::printState(copy, colors, 0, graph[0].size()-1, true ? "done" : "running");
        // print colours.

        for(int i = 0 ; i < graph[0].size()-1;i++) {
            std::cerr<<colors[i] << + " ";
            //        std::cout<<"Node: " << i << + " Colour: " <<colors[i] << + " ";

        }

    }

    //     Heawood counterexample:
    //     {0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0},
    //         {1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    //         {0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    //         {0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0},
    //         {0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    //         {1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    //         {0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    //         {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    //         {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0},
    //         {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
    //         {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    //         {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
    //         {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0}
    // // };

    return 1;
}













// #include <iostream>
// #include <vector>   // For push_back and []
// #include <cstring>  // For memcpy
// #include <algorithm>
// #include <set>


// const int nodes = 35;

// // Toggles kempe chain of colours c1, c2 containing node: node, where c1 is the colour of node.
// void toggleKempeChain (int graph[][nodes+1], int colors [], int c1, int c2, int node) {
//     colors[node] = c2;
//     for(int i = 0 ; i < nodes; i++) {
//         if(graph[node][i] && colors[i] == c2) {
//             toggleKempeChain(graph, colors, c2, c1, i);
//         }
//     }
// }

// // Populates Kempechain vector with kempechain of colours: color, and c2, where c2 is the colour of the node: node.
// void KempeSet (int graph [nodes][nodes +1], int colors [], int prev, int node, int c1, int c2, std::set <int>& Kempechain) {

//     Kempechain.insert(node);

//     for(int i = 0 ; i < nodes; i++) {
//         if(graph[node][i] && Kempechain.find(i) == Kempechain.end() && colors[i] == c1) {
//             KempeSet(graph, colors, node, i, c2, c1, Kempechain);
//         }
//     }

// }

// // Returns the colour of an uncoloured node based on Kempe's method. degree (node) < 6.
// int colour(int graph [][nodes+1], int colors [] , int node) {

//     if(colors[node] != 0)
//         return colors[node];

//     int colours [5] = {0};

//     // neighbours of node: node that are coloured
//     int neighbours [nodes] = {-1};

//     // number of coloured neighbours of node: node
//     int numColouredNodes = 0;


//     // Count the neighbouring colours and track these nodes.
//     for(int i = 0; i < nodes;i++){
//         if(graph[node][i]) {
//             colours[colors[i]]++;
//             if(colors[i]!=0) {
//                 neighbours[numColouredNodes] = i;
//                 numColouredNodes++;
//             }
//         }
//     }

//     // Kempe method not applicable, try again later.
//     if(numColouredNodes >5)
//         return -1;

//     // Colour the node an open colour.
//     for(int i = 1 ; i < 5; i++) {
//         if(colours[i] == 0)
//             return i;
//     }

//     // Need to apply kempe chain method since no open, available colour, but < 6 coloured neighbours.
//     std::set <int> KempeChain;
//     int toggledColor;

//     // Case: 4 coloured neighbours
//     if(numColouredNodes == 4) {
//         // choose a node.
//         for(int i = 0 ; i < 4 ; i++) {
//             KempeChain.clear();

//             // choose another node
//             for(int j = i+1 ; j < 4; j++) {
                
//                 // Find Kempe chain
//                 KempeSet(graph, colors, -1, neighbours[i], colors[neighbours[j]], colors[neighbours[i]], KempeChain);

//                 // If there's no kempe chain, we can toggle and colour the unknown node: node the toggled colour.
//                 if(KempeChain.find(neighbours[j]) == KempeChain.end()) {
//                     toggledColor = colors[neighbours[i]];
//                     toggleKempeChain(graph, colors, colors[neighbours[i]], colors[neighbours[j]], neighbours[i]);
//                     return toggledColor;
//                 }
//             }
//         }
//         std::cerr<<"error in 4 neighbours of diff colours"<<std::endl;
//     }

//     // Case: 5 coloured neighbours
//     if(numColouredNodes == 5) {

//         int node1 = -1;
//         int node1temp;
//         int node2 = -1;
//         int node2temp;

//         // obtain node1, node2
//         for(int i = 0 ; i < 5; i++) {
//             if(colours[colors[neighbours[i]]] == 2){
//                 if(node1 == -1)
//                     node1 = neighbours[i];
//                 else if(node2 == -1) {
//                     node2 = neighbours[i];
//                 }
//             }
//         }


//         // Scenario 1: checking unique coloured nodes
//         // Choose a node
//         for(int i = 0 ; i < 5;i++) {

//             // This node has to be unique-colored. We want to toggle one of the 3 unique coloured neighbours.
//             if(neighbours[i] == node1 || neighbours[i] == node2)
//                 continue;

//             // Choose another node.
//             for(int j = i + 1; j < 5; j++) {

//                 // This node has to be unique-colored. We want to toggle one of the 3 unique coloured neighbours.
//                 if(neighbours[j] == node1 || neighbours[j] == node2)
//                     continue;

//                 KempeChain.clear();

//                 // Obtain kempe chain of the two nodes.
//                 KempeSet(graph, colors, -1, neighbours[i], colors[neighbours[j]], colors[neighbours[i]], KempeChain);

//                 // If there's no kempe chain, we can toggle and colour the unknown node: node, the toggled colour.
//                 if(KempeChain.find(neighbours[j]) == KempeChain.end()) {
//                     toggledColor = colors[neighbours[i]];

//                     toggleKempeChain(graph, colors, colors[neighbours[i]], colors[neighbours[j]], neighbours[i]);
//                     return toggledColor;
//                 }
//             }

//         }

//         // Scenario 2: toggling the 2 nodes with same colouring
//         // node: node1, node2 are the 2 neighbours with same colouring

//         // Solve for second scenario
//         // Choose a node.

//         toggledColor = colors[node1];

//         node1temp = node1;
//         node2temp = node2;

//         for(int i = 0; i < 5;i++) {


//             // Choose a unique coloured node.
//             // we need to find a colour that is not the same color as the double-colored node or else
//             // even if there is no kempe chain between the 2 diff coloured nodes, the kempe chain contains
//             // 2 nodes in the set of neighbours, which errors out, this is due to every node in neighbours being
//             // adjacent to a node that is colored the colour of the double-colourign.
//             if(neighbours[i] == node1temp || neighbours[i] == node2temp)
//                 continue;
            
//             // Check if node1 has already been toggled...
//             if(node1 != -1) {

//                 KempeChain.clear();
//                 KempeSet(graph, colors, -1, node1, colors[neighbours[i]], toggledColor, KempeChain);

//                 // If there's no kempe chain, we can toggle and colour node1 this colour on [i].
//                 if(KempeChain.find(neighbours[i]) == KempeChain.end()) {
//                     toggleKempeChain(graph, colors, colors[node1], colors[neighbours[i]], node1);

//                     // if node2 is already toggled, then return the colour that was doubly coloured.
//                     if(node2 == -1)
//                         return toggledColor;
//                     node1 = -1;
//                 }
//             }

//             if(node2 != -1) {
//                 KempeChain.clear();
//                 KempeSet(graph, colors, -1, node2, colors[neighbours[i]], toggledColor, KempeChain);
//                 if(KempeChain.find(neighbours[i]) == KempeChain.end()) {
//                     toggleKempeChain(graph, colors, colors[node2], colors[neighbours[i]], node2);
//                     if(node1 == -1)
//                         return toggledColor;
//                     node2 = -1;
//                 }
//             }
            
//         }
//         std::cerr<<"error in 5 neighbours of diff colours"<<std::endl;

//     }

//     // kempe method error. should never happen
//     std::cout<<"Non-planar"<<std::endl;
//     return -2;

// }

// // Function that returns if the neighbours of an uncoloured node: node have all been successfully coloured.
// bool colourNeighbours (int graph [][nodes+1], int colors [], int node, int& colouredNodes) {
    
//     std::vector <int> uncolouredNeighbours;
    
//     for(int i = 0 ; i < nodes; i++) {
//         if(graph[node][i] && colors[i] == 0) {
//             uncolouredNeighbours.push_back(i);
//             colors[i] = colour(graph, colors, i);
//             colouredNodes++;
//             // the below cases should never occur on an originally uncoloured graph
//             if(colors[i] == -1) {
//                 std::cout<<"Not kempe-based colourable\n";
//                 return false;
//             }            
//             if(colors[i] == -2) {
//                 std::cout<<"Non planar\n";
//                 return false;
//             }
//             // if(colors[i] == -1 || colors[i] == -2) {
//             //     //reset coloured neighbours
//             //     if(colors[i] == -1)
                
//             //     colouredNodes-= uncolouredNeighbours.size();
//             //     for(int j = 0; j < uncolouredNeighbours.size(); j++) {
//             //         colors[uncolouredNeighbours[j]] = 0;
//             //     }
//             //     return false;
//             // }
//         }
//     }
//     return true;
// }

// // Function to delete a node, once it and its neighbours have been coloured.
// void deleteNode(int graph [][nodes+1], int node, std::vector <int>& deletedNodes) {

//     // delete connection from neighbours
//     for(int i = 0 ; i < nodes; i++) {
//         if(graph[node][i] == 1) {
//             graph[i][node] = 0;
//             graph[i][nodes]--;
//             graph[node][i] = 0;
//         }
//     }

//     // nullify total degree count at end as well
//     graph[node][nodes] = -1;

//     // insert deleted node into tracker for post processing
//     deletedNodes.push_back(node);

// }

// // Postprocessor that returns if a deleted node no longer satisfies 4-colouring => 5 colour theorem.
// bool deletedNodeError(int graph[][nodes+1], int node, int colors []) {

//     // Checks neighbours and see if there's a same colouring of adjacent nodes.
//     for(int i = 0 ; i < nodes; i++) {
//         if(graph[node][i] && colors[node] == colors[i])
//             return true;
//     }

//     return false;
// }

// // PreProcessing function that returns if the original graph has any colouring conflict and 
// // removes coloured nodes whose neighbours are also coloured (properly) for postprocessing.
// bool preProcessing (int graph [][nodes+1], int colors [], std::vector <int>& deletedNodes) {

//     int colour = 0;
//     bool remove;

//     // Go through all nodes of graph
//     for(int i = 0 ; i < nodes;i++) {

//         // obtain colour
//         colour = colors[i];

//         // if uncoloured, no possible colouring conflict is possible nor is it reducible.
//         if(colour == 0)
//             continue;

//         // flag to remove a node
//         remove = true;

//         // go through neighbours of node [i]
//         for(int j = 0 ; j < nodes; j++) {

//             // if an edge exists between node i and node j...
//             if(graph[i][j]) {

//                 // if node j is coloured...
//                 if(colors[j] != 0) {

//                     // if node i and node j are same colouring...
//                     if(colors[j] == colour) {

//                         // colouring conflict
//                         std::cout<<"this graph has a coloring conflict\n"<<std::endl;
//                         return false;
//                     }
//                 }

//                 // if even one neighbour is not coloured, node i can not be removed.
//                 else {
//                     remove = false;
//                 }
//             }   
//         }
//         // after going through neighbours, if no colouring conflict, and all its neighbours, as well as itself, is coloured....
//         if(remove)
//             //remove the node; store for postprocessing
//             deleteNode(graph, i, deletedNodes);

//     }
    
//     //no colouring conflicts.
//     return true;
// }


// // bool hasColouredNeighbour (int graph [][nodes + 1], int colors[], int node) {
// //     for(int i = 0 ; i < nodes; i++) {
// //         if(graph[node][i] && colors[i] != 0)
// //     }
// // }


// int main () {

//     int graph [nodes][nodes+1]=  {

// //         {0, 1, 1, 0, 0, 0, 0},
// //         {1, 0, 0, 1, 1, 0, 0},
// //         {1, 0, 0, 0, 1, 0, 0},
// //         {0, 1, 0, 0, 0, 1, 0},
// //         {0, 1, 1, 0, 0, 1, 0},
// //         {0, 0, 0, 1, 1, 0, 0}
// // // };
// // Adjacency matrix + color: int adj[36][37]
// // Columns 0..35 = adjacency, column 36 = color
// // 0=none 1=red 2=green 3=blue 4=yellow 5=white
// // Adjacency matrix + color: int adj[36][37]
// // Columns 0..35 = adjacency, column 36 = color
// // 0=none 1=red 2=green 3=blue 4=yellow 5=white
// // {

// // Adjacency matrix + color: int adj[34][35]
// // Columns 0..33 = adjacency, column 34 = color
// // 0=none 1=red 2=green 3=blue 4=yellow 5=white
// // {

// // Adjacency matrix + color: int adj[24][25]
// // Columns 0..23 = adjacency, column 24 = color
// // 0=none 1=red 2=green 3=blue 4=yellow 5=white

// // Adjacency matrix + color: int adj[27][28]
// // Columns 0..26 = adjacency, column 27 = color
// // 0=none 1=red 2=green 3=blue 4=yellow 5=white
// // {

// // Adjacency matrix + color: int adj[5][6]
// // Columns 0..4 = adjacency, column 5 = color
// // 0=none 1=red 2=green 3=blue 4=yellow 5=white
// // {
// //         {0, 1, 1, 1, 1, 0},
// //         {1, 0, 1, 1, 1, 0},
// //         {1, 1, 0, 1, 1, 0},
// //         {1, 1, 1, 0, 1, 0},
// //         {1, 1, 1, 1, 0, 0}
// // // };


// //egypt
// //         {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
// //         {1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0},
// //         {0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0},
// //         {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0},
// //         {0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
// //         {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0},
// //         {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}
// // // };

// //uk
// // // {
// //         {0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
// //         {0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0}
// // // };

// // {
// {0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
// {1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
// {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
// {1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0},
// {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
// {0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
// {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
// {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0},
// {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
// {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
// {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
// // };


// // //china
//         // {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
//         // {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0},
//         // {0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
//         // {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0},
//         // {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
//         // {1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//         // {0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
// // // };
// //india
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
// //         {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0},
// //         {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
// //         {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
// // // };

//     // };

//     //   {0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0},
//     //     {1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
//     //     {1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
//     //     {0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0},
//     //     {1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0},
//     //     {0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0},
//     //     {0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0},
//     //     {1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0},
//     //     {0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0},
//     //     {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0},
//     //     {0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0}


// //us
// //        {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
// //         {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0},
// //         {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
// //         {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
// //         {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
// //         {0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
// //         {0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
// //         {1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
// //         {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
// //         {0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}
// // // };


// };

// // };


//     int copy [nodes][nodes+1];

//     std::memcpy(copy, graph, sizeof(graph));

//     int degree;

//     // Store degree of each node at end
//     for(int i = 0 ; i < nodes;i++) {
//         degree = 0;
//         for(int j = 0 ; j < nodes;j++) {
//             degree += graph[i][j];
//         }
//         graph[i][nodes] = degree;
//     }

//     std::vector <int> deletedNodes;

//     int colors [nodes] = {0, 3, 4, 2, 1, 5, 1, 5, 3, 4, 5, 1, 2, 3, 2, 5, 4, 2, 5, 4, 3, 2, 3, 1, 4, 2, 1, 3, 2, 5, 5, 2, 4, 4, 3};

//     // int colors [nodes] = {0};

//     int colouredNodes = 0;

//     for(int i = 0 ; i < nodes; i++) {
//         colouredNodes+= (colors[i] != 0);
//     }

//     int index  = 0;

//     // Kempes method says 
//     if(preProcessing(graph, colors, deletedNodes)) {

//         // add failing condiitoin so no infinite loop on colouredNodes.
//         while(colouredNodes < nodes) {


//             // if an uncoloured node has a degree < 6
//             if(graph[index%nodes][nodes] < 6 && colors[index%nodes] == 0) {

//                 std::cout<<index%nodes<<std::endl;

//                 if(colourNeighbours(graph, colors, index%nodes, colouredNodes)) {

//                     // colouredNodes += graph[index%nodes][nodes];
//                     colors[index%nodes] = colour(graph, colors, index%nodes);

//                     // If kempe method error... should never happen
//                     if(colors[index%nodes] == -2) {
//                         std::cout<<"kempe method algorithm error"<<std::endl;
//                         break;
//                     }

//                     // If unsuccessful colouring... This should never happen!!! Since we're using our reduced/induction-type
//                     // method with node removal. Should never happen as well. This case can only happen with the neighbours, not
//                     // with the emphaiszed node itself.
//                     if(colors[index%nodes] == -1) {
//                         colors[index%nodes] = 0;
//                         std::cout<<"Node should be colourable but isn't for some reason\n";
//                         break;
//                     }

//                     // otherwise, SUCCESSFUL colouring
//                     else {
//                         deleteNode(graph, index%nodes, deletedNodes);
//                         colouredNodes++;
//                     }
//                 }

//                 else {
//                     for(int k = 0 ; k < nodes;k++) {
//                         std::cout<<colors[k] << + " ";
//                     }
//                     std::cout<<"the graph is not 5-colourable using kempe-based method"<<std::endl;
//                     break;
//                 }
//             }

//             index++;

//         }

//         // Go through deleted nodes and see if a colouring error exists, if so, use 5 colouring.
//         // Go through the deleted nodes in an inductive, recursive order from the recently deleted node to the first deleted node
//         // # stackstack.
//         for(int i = deletedNodes.size()-1; i >= 0; i--) {
//             if(deletedNodeError(copy, deletedNodes[i], colors)) {
//                 colors[deletedNodes[i]] = 5;
//             }
//         }

//         // print colours.
//         for(int i = 0 ; i < nodes;i++) {
//             std::cout<<colors[i] << + " ";
//             //        std::cout<<"Node: " << i << + " Colour: " <<colors[i] << + " ";

//         }

//     }

//     //     Heawood counterexample:
//     //     {0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0},
//     //         {1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
//     //         {0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
//     //         {0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0},
//     //         {0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
//     //         {1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//     //         {0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//     //         {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
//     //         {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
//     //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0},
//     //         {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
//     //         {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
//     //         {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
//     //         {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0}
//     // // };

//     return 1;
// }
