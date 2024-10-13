# **csg**  
## Boolean operations on polygon meshes  
Boolean operations on 3D models (objects) are very difficult: I tried using meshlab, VTK, CGAL, and other well-known tools, but nothing works or in some cases crashes during execution if it depends on the correctness of the model topology.

We thought it would be less restrictive and more stable to use only the simple structure of a set of triangles as an object, but we encountered cases where numerical calculations broke down. To avoid this, we attempted to use high precision instead of double precision.

Although the algorithm was double-precision (double), there was already a similar algorithm with a high degree of completion. *1
The same result was obtained by modifying it to high precision, so we made improvements based on this algorithm. The most troublesome case, however, is when triangles are attached to each other. The most troublesome case is when the triangles are attached to each other, which can be avoided by tilting the triangles slightly (10^-12) so that they intersect.
Otherwise, avoid the problem and try again.

This is not problem solving, but problem avoidance, and although the processing speed is considerably reduced when retry processing is involved, boolean operations can be performed with confidence on a wide variety of objects.

*1)  The original was written in Java and is in the public domain. Author Danilo Balby
https://sourceforge.net/projects/unbboolean/
 
http://createuniverses.blogspot.com/2008/09/constructive-solid-geometry-program.html  
A C++ port of the J3DBool library, an application that demonstrates the algorithm for CSG operations. It is also a C++ rewrite of a Java implementation by Greg Santucci in 2008. .The license is LGPL.

Many of the improvements I have made are in the use of QD (double precision), enhanced triangulation, and enhanced intersection determination.

---
## algorithm   
The algorithm is outlined as follows
All triangles of one object are sliced with all triangles of the other object and vice versa.
Once this is done, all triangles are classified as inside or outside of the other object.
Determining whether inside or outside can be done by using ray tracing to compare the ray from 
the normal direction of one triangle with the normal of the triangle of the other object that the ray will hit first.
If the ray hits nowhere, it is outside; 
if it does not hit in the same direction as the normal of the other object, it is inside.  

---  
## Overlapping areas make calculations very difficult
However, the calculation itself completes normally without abnormal termination.
The output looks correct, but upon closer inspection, the mesh topology is quite broken.  

<img src=./images/image01.png width=80%>
<img src=./images/image02.png width=80%>
<img src=./images/image03.png width=80%>
---


<img src=./images/image04.png width=80%>
<img src=./images/image05.png width=80%>

---
On the other hand, without duplication, very powerful boolean calculations can be performed without failure.
This is largely due to the accuracy provided by the QD library I added and the persistent retrying of calculations.  

The calculation speed may be extremely slow under some conditions because of the many retries that may be performed.

The example (sample) below is applied to a cutting simulation.
It is an image obtained by sequentially performing boolean operations on the material and cutting tool geometry.

<img src=./images/sim20120524.gif width=40%>
<img src=./images/sim2.gif width=40%>

This calculation usually takes about a week.
Thus, it is not real-time at all, and practical problems remain.
