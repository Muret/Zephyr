
Controls:

WSAD and EQ moves camera
With right mouse click down, you can rotate the camera
m key makes it iterate
n key changes between CC_V1 and CC_V2 (default is CC_V1)	 !!ONLY FOR THE NEXT ITERATION
If CC_V1 is enabled, mesh will be blue, otherwiese it will be red
b key changes between Icosahedron and cube, default is Icosahedron 
v key changes between wireframe and solid
c key resets meshes

Thoughts:

My term project subject was adaptive Catmull-Clark subdivision/smoothing wrt heightmap data. When I started coding Catmull Clark(CC), I did not know that with triangle meshes, it is not that efficent and robust. I implemented the original algorithm and saw that the results are not that smooth(even though it subdivides very aggresively) and a cube can not become a sphere after many iterations.

So, instead of implementing adaptive subdivision , I wanted to find another CC variation which would work well with a triangle based mesh and implement it too.(My rendering engine only supports triangle meshes). 
After research I found this : https://graphics.stanford.edu/~mdfisher/subdivision.html .I will call the first method CC_V1 and this new algorithm CC_V2. I also made a icosahedron mesh ,as mentioned in the article, with maya and started working on that. CC_V2 was not that much different from the original one. The original one used barycenter of F , P , R. F is the average of newly added face center points. P is current position and R is the average of newly added edge midpoints , from only the faces and edges that touches the P. CC_V2 is more linear and just averages the old neighbours of old vertices. Their most important difference is that CC_V1 adds one more vertex to every center of every face, thus subdivides the mesh more every iteration.

After testing the CC_V2 with a cube, although being much metter and having a faster convergence rate, I saw that CC_V2 too is not roboust enough at cube mesh. Moral of the story, the more spherical nature of Icosahedron makes it a more suitable start up subdivision for the Catmull Clark algorithm when it is used at triangle meshes. Thus, robostnuss of CC algorithms at triangle meshes are majorly dependent on initial mesh structure. 


