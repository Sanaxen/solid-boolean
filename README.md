<center><img src=./images/image00.png width=100%></center>

# **csg**  
## Boolean operations on polygon meshes  
Boolean operations on 3D models (objects) are very difficult: I tried meshlab, VTK, CGAL, and other well-known tools, but nothing worked, and in some cases, depending on the correctness of the model topology, it crashed during execution.

We thought it would be less restrictive and more stable to use only the simple structure of a set of triangles as an object (*1), but we encountered cases where numerical calculations broke down. To avoid this, we decided to use high precision (double-double precision, quad-double precision) instead of double precision.

The algorithm was double precision (double), but there was already a similar algorithm with a high degree of completion. *2
Modifying it to high precision (double-double or quad-double double precision) gave the same results, so we made improvements based on this algorithm. The most troublesome case, however, was when triangles were attached to each other, which could be avoided by tilting the object slightly (10^-12) so that the triangles intersected. The original idea was that if the precision of the object was float or double, then changing this shape could be seen as not changing the shape of the object, so a high precision (double-double or quad-double) calculation prevented the triangles from sticking to each other. Thus, a mechanism to avoid the problem and try again is introduced without deep thought.

This is problem avoidance rather than problem solving, and although the processing speed of retries is considerably slower, it allows boolean operations to be performed with confidence on a wide variety of objects.  

*2)  The original was written in Java and is in the public domain. Author Danilo Balby
https://sourceforge.net/projects/unbboolean/
 
http://createuniverses.blogspot.com/2008/09/constructive-solid-geometry-program.html  
A C++ port of the J3DBool library, an application that demonstrates the algorithm for CSG operations. It is also a C++ rewrite of a Java implementation by Greg Santucci in 2008. .The license is LGPL.

Many of the improvements I have made are in the use of QD (double precision), enhanced triangulation, and enhanced intersection determination.

---
## algorithm   (*1)
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
