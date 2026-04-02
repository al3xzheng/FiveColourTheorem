#include <iostream>
#include <vector>
#include <algorithm>

//QUESTIONS: does this method 4-colour all planar graphs that are originally all unocloured?
// O(N^3) time complexity.

// Next: implement colouring of originally coloured, specific colored graphs. some changes. 
// Steps:
/**
 * Implement 5 colour theorem?, and know when to stop, e..g my example. If kempe fails, it keeps on alternating, we need to try new node (like the one that worked)
 * in the example.
 * 
 * need to implement the heavier brute force and then opitmizaitons.
 */

const int nodes = 14;

int minIndex (int availableColours[5]) {
    int min = 1;
    for(int i = 2 ; i < 5;i++) {
        if(availableColours[i] < availableColours[min])
            min = i;
    }
    return min;
}

void nodesOfMinColour (int graph [nodes][nodes], int node, int minColor, int nodesOfMinColor [nodes], int colors[]) {
    int index = 0; 
    for(int i = 0 ; i < nodes;i++) {
        if(graph[node][i] && colors[i] == minColor) {
            nodesOfMinColor[index] = i;
            index++;
        }
    }
}

// 
void toggleKempeChain (int graph[][nodes], int colors [], int c1, int c2, int node) { //c1 is the color of the node
    colors[node] = c2;
    for(int i = 0 ; i < nodes; i++) {
        if(graph[node][i] && colors[i] == c2) {
            toggleKempeChain(graph, colors, c2, c1, i);
        }
    }
}

void KempeSet (int graph [][nodes ], int colors [], int node, int color, int c2, std::vector <int>& Kempechain) {

    Kempechain.push_back(node);

    // make exclsuive
    for(int i = 0 ; i < nodes; i++) {
        if(graph[node][i] != 0 && colors[i] == color) {
            KempeSet(graph, colors, i, c2, color, Kempechain);
        }
    }

}

void colour (int graph [][nodes], int node, int colors []) {

    int availableColours [5] = {0};

    for(int i = 0 ; i < nodes; i++) {

        if(graph[node][i]) {
            availableColours[colors[i]]++;
        }

    }

    for(int i = 1;i<5;i++) {
        if(availableColours[i] == 0) {
            colors[node] = i;
            return;
        }
    }

    int minColor = minIndex(availableColours);
    int nodesOfMinColor [nodes] = {-1};

    nodesOfMinColour(graph, node, minColor, nodesOfMinColor, colors);


    for(int j = 0 ; ;j++) {
        if(nodesOfMinColor[j] == -1)
            break;

        for(int i = 0; i < nodes; i++) {
                if(graph[node][i] && colors[i] !=0 && colors[i] != minColor) {



                    std::vector<int> KempeChain;
                    KempeSet(graph, colors, i, minColor, colors[i], KempeChain);
                    if(std::find(KempeChain.begin(), KempeChain.end(), nodesOfMinColor[j]) == KempeChain.end()) {
                        colors[nodesOfMinColor[j]] = colors[i];
                        break;
                    }

                }
                if(i == nodes - 1)
                    std::cout<<"Kempe chain method failed for " << nodesOfMinColor[j] <<std::endl;
            }
    }

    colors[node] = minColor;
    return;

}

int main () {

    int graph [nodes] [nodes]= {

// Adjacency matrix (13 nodes, 18 edges)
// int adj[13][13] = {
// Adjacency matrix (13 nodes, 18 edges)
// int adj[13][13] = {
// Adjacency matrix (5 nodes, 9 edges)
// int adj[5][5] = {
        {0, 1, 1, 1, 1},
        {1, 0, 1, 1, 1},
        {1, 1, 0, 0, 1},
        {1, 1, 0, 0, 1},
        {1, 1, 1, 1, 0}
// };

// };

// };


    };

    //coloring higher degreed nodes can be implemented later
    // for(int i = 0 ; i < nodes; i++) {

    //     for(int j = 0 ; j < nodes;j++) {
    //         graph[i][nodes] += graph[i][j];
    //     }
    // }


    int colors [nodes] = {0};
    int colouredNodes = 0;

    int node = 0;


    // can add optimizaation later of choosing node wiht most neighbours first.
    for(int i = 0 ; i < nodes;i++) {
        colour(graph, i, colors);
    }

    for(int i = 0 ; i < nodes;i++) {
        std::cout<<colors[i] << + " ";
        //        std::cout<<"Node: " << i << + " Colour: " <<colors[i] << + " ";

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




}