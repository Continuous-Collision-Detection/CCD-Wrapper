This is a visualization tool for CCD dataset of the paper "A Large Scale Benchmark and an Inclusion-Based Algorithm for Continuous Collision Detection" (https://archive.nyu.edu/handle/2451/61518) 

The tool is written in Python, requiring matplotlib and gmpy2 installed.

On the top of the file, you can modify:

filename (to select a query file), 

query_id (to select a query in this file), 

is_edge_edge (if you are checking edge-edge CCD, set it as True; Otherwise set it as False. This should be corresponding to the file you selected),

show_trajectories (set it as True, then you can see the trajectories the edges sweep in the 3D).

Then run the script, you will see an animation in a pop-up window, telling you if there is a collision, and showing the motions of the edges. A GIF figure will also be generated in the folder where the script is located.

The authors of "A Large Scale Benchmark and an Inclusion-Based Algorithm for Continuous Collision Detection"
