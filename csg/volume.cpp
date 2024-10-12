/*  3D surface volume() function
    Author:   Anton Milev
    mail:     amil@pyramidproject.net, amil@abv.bg
    website:  www.pyramidproject.net

    Calculates signed solid body volume,
    the triangles mesh should be closed and all normals should point outside (or all inside)
    if the surface is not close or normals dont point outside result will be a nonsense
    use the isClosed() routine to determine if the volume is closed, 
    if normals are disordered use correctNormals() routine
    
    Volume is calculated by the Gauss theorem: 
    //V = 1/6 ((v2y-v1y)(v3z-v1z) - (v2z-v1z)(v3y-v1y))(v1x + v2x + v3x) 

    Input: - a list of triangles representing the mesh of the closed surface

    Output: - float value of the signed volume 
              > 0 if normals point outside
              < 0 if normals points inside

    Date: 2011-03-12
*/

float volume(const List<triangle3d>& triangles) 
{
    if(triangles.size() <= 3)
        return 0;

    float volume = 0;

    point3d p1,p2,p3 ;
    for(int k = 0; k < triangles.size(); k++)
    {
        p1 = triangles[k].p1 ;
        p2 = triangles[k].p2 ;
        p3 = triangles[k].p3 ;
        //apply Gauss Teorem
        volume += ((p2.y-p1.y)*(p3.z-p1.z) - (p2.z-p1.z)*(p3.y-p1.y) )*(p1.x+p2.x+p3.x);
    }

    return volume / 6.0 ;
}
