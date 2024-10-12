# **csg**  
## Boolean operations on polygon meshes  
The original was written in Java and is in the public domain. Author Danilo Balby
https://sourceforge.net/projects/unbboolean/
 
http://createuniverses.blogspot.com/2008/09/constructive-solid-geometry-program.html  
A C++ port of the J3DBool library, an application that demonstrates the algorithm for CSG operations. It is also a C++ rewrite of a Java implementation by Greg Santucci in 2008. .The license is LGPL.

Many of the improvements I have made are in the use of QD (double precision), enhanced triangulation, and enhanced intersection determination.

---  
# Overlapping areas make calculations very difficult
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