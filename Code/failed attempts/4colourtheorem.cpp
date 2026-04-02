#include <iostream>
#include <cstring>

// Need a tool that allows you to draw a graph and then obtain the adjacency matrix.

const int nodes = 10;

bool iskempeChain(int graph [][nodes+1], int prev, int node1, int node2, int colours [], int c) {

    if(node1 == node2)
        return true;
    
    // O(n)
    for(int i = 0 ; i < nodes; i++) {

        if(graph[node1][i] == 1 && i != prev && colours[i] == c) {
            if(iskempeChain(graph, node1, i, node2, colours, colours[node1]))
                return true;
        }

    }
    return false;

}

void KempeMethod (int graph [nodes][nodes+1], int colours [], int node) {

    if(graph[node][nodes] ==1) {
        for(int i = 0 ; i < nodes; i++) {
            if(graph[node][i]) {

                colours[node] = (colours[i] + 1) % 5;
                if(colours[node] == 0)
                    colours[node] = 1;
            }

        }
    }

    else if(graph[node][nodes] <4) {

        int colors [4] = {0};

        for(int i = 0 ; i < nodes; i++) {
            if(graph[node][i]) 
                colors[colours[i]] = 1;
        }    

        for(int i = 0; i < 4;i++) {
            if(colors[i] == 0) {
                colours[node] = i;
                return;
            }
        }
    }

    else if(graph[node][nodes] == 4) {

        int colors [4] = {0};
        int adjacency [nodes] = {0};

        for(int i = 0 ; i < nodes; i++) {
            if(graph[node][i]) 
                colors[colours[i]] = 1;
        }    

        for(int i = 0; i < 4;i++) {
            if(colors[i] == 0) {
                colours[node] = i;
                return;
            }
        }

        for(int )

        if(iskempeChain(graph, -1, colours,));
    }

    else if(graph[node][nodes] == 5) {


    }

    
}

int availableColoring(int graph [][nodes+1], int colours [], int node) {

    int newcolours [5] = {0};

    for(int i = 0 ; i < nodes; i++) {

        if(graph[node][i]) {
            newcolours[colours[i]] = 1;
        }

    }

    for(int i = 1 ; i < 5;i++) {
        if(newcolours[i] == 0)
            return i;
    }

    return 0;

}


int main () {

    // assume graph is planar
    int graph [nodes] [nodes + 1]= {

        {0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0},
        {1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0},
        {1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0}

    };

    // red:1, blue:2, green:3, yellow: 4
    int colours [nodes] = {0};
    int backtrack [nodes] = {0};

    int sum;

    for(int i = 0 ; i < nodes; i++) {

        sum = 0;

        for(int j = 0 ; j < nodes;j++) {
            sum += graph[i][j];
        }

        graph[i][nodes] = sum;

    }

    int graph1[nodes][nodes + 1];

    // deep copy:
    std::memcpy(graph1, graph, sizeof(graph));

    int watchdog = 0;
    int index = 0;
    int backIndex = nodes-1;

    int temp = 0;

    while (watchdog < nodes) {

        if(graph1[index%nodes][nodes] == 0 || graph1[index%nodes][nodes] > 5)
            watchdog++;

        else if(!colours[index] && graph1[index%nodes][nodes] < 6) {
            watchdog = 0;

            for(int i = 0 ; i < nodes; i++) {
                if(graph1[index][i] && colours[i] == 0) {

                    temp = availableColoring(graph1, colours, index);

                    colours[i] = temp;

                    // leave uncoloured to go back
                    if(temp == 0)
                        colours[i] = 1;

                }
            }

            // edge case finish up
            // cases 4/5
            // check the way for opposite neighbours, should be a fairly trivial check. ->is Kempe...
            // main functino integration.
            //change colour to be size 4 as well, simpler.

            // store the node into the end of backtrack
            // decrease the sum of all neighbours of this node.
            
        }

        index++;
        
    }







    return 0;
}