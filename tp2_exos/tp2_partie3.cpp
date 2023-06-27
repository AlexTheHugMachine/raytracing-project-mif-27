#include "color.h"
#include "image.h"
#include "image_io.h"
#include "mesh_io.h"
#include <stdlib.h>
#include "vec.h"

bool appartient_tetraedre(const Vector &d, const Point &o, const Point &a, const Point &b, const Point &c) {
    Vector norm1 = normalize(cross(Vector(a,b), Vector(o,a)));
    Vector norm2 = normalize(cross(Vector(b,c), Vector(o,b)));
    Vector norm3 = normalize(cross(Vector(c,a), Vector(o,c)));

    float det1 = dot(d, norm1);
    float det2 = dot(d, norm2);
    float det3 = dot(d, norm3);

    if((det1 >= 0 && det2 >= 0 && det3 >= 0)) {
        return true;
    }
    else
        return false;
}

Vector coord_barycentrique(const Vector &d, const Point &o, const Point &a, const Point &b, const Point &c) {
    Vector points[3] = {
        Vector(o, a),
        Vector(o, b),
        Vector(o, c)};

    float volume[3] = {};
    float lambda[3] = {};
    Vector normals[3] = {};

    for(unsigned int i = 0; i < 3; i++) {
        normals[i] = Vector(normalize(cross(points[(i+2)%3], points[(i+1)%3])));
        volume[i] = dot(d, normals[i]);
    }
    for(unsigned int i = 0; i < 3; i++) {
        lambda[i] = volume[i]/(volume[0]+volume[1]+volume[2]);
    }
    return Vector(lambda[0], lambda[1], lambda[2]);
}

Point pointDistance(const Vector &lambda, const Point &a, const Point &b, const Point &c) {
    return Point((lambda.x*a.x + lambda.y*b.x + lambda.z*c.x), (lambda.x*a.y + lambda.y*b.y + lambda.z*c.y), (lambda.x*a.z + lambda.y*b.z + lambda.z*c.z));
}

void zbuffer(const float &zPd, Image &zbuff, Image &im, const int &px, const int &py, const Color color) {
    if(zPd > zbuff(px, py).r) {
        zbuff(px, py) = Color(zPd);
        im(px, py) = color;
    }
}

bool intersctionRayonAvecPlanBarycentrique(Point o, Vector d, Point &a, Point &b, Point &c, Point &intersectionPoint, float &t_proche, Vector &normal)
{
    Vector normPlan = normalize(cross( Vector(a,b), Vector(a,c) ));

    float t = dot(normPlan, Vector(o, a)) / dot(normPlan, d);
    Point p = o + t * d;

    if (t > 0 && t < t_proche)
    {
        if(appartient_tetraedre(d, p, a, b, c)) {
            intersectionPoint = p;
            t_proche = t;
            normal = normPlan;
            return true;
        }
        else
            return false;
    }
    else {
        return false;
    }
}

int main( )
{
    //const char *filename= "data/geometry.obj";    // pour la scene
    const char *filename= "data/robot.obj";       // pour le robot
   
    std::vector<Point> positions;
    std::vector<int> materials_ids;
    Materials materials;

    if(!read_positions(filename, positions))
        std::cout << "Erreur chargement positions" << std::endl;
    if(!read_materials(filename, materials, materials_ids))
        std::cout << "Erreur chargement materials" << std::endl;
   
    // deplace tous les sommets devant la camera
    for(unsigned i= 0; i < positions.size(); i++) {
        //positions[i]= positions[i] + Vector(0, -110, -400);   // pour la scene
        positions[i]= positions[i] + Vector(0, -2, -4);     // pour le robot
    }
   
    // englobant des points, verifier qu'ils sont bien devant la camera...
    Point pmin= positions[0];
    Point pmax= positions[0];
    for(unsigned i= 1; i < positions.size(); i++)
    {
        pmin= min(pmin, positions[i]);
        pmax= max(pmax, positions[i]);
    }
    printf("bounds [%f %f %f]x[%f %f %f]\n", pmin.x, pmin.y, pmin.z, pmax.x, pmax.y, pmax.z);

    // cree l'image resultat
    Image image(1024, 1024);

    for(int py= 0; py < image.height(); py++)
    for(int px= 0; px < image.width(); px++)
    {

        // Le pixel est noir par defaut
        image(px, py)= Black();//Color(0.5, 0.5, 0.5);

        float t_proche = 1000000000.0;
        Point intersectionPoint(100000, 100000, 100000);
        Vector normal = Vector(0, 0, 0);

        // rayon centre camera
        float x = float(px)/image.width() * 2 - 1;
        float y = float(py)/image.height() * 2 - 1;
        float z = -1;       
        Point p = Point(x, y, z);

        Point o = Point(0, 0, 0);
        Vector dist = Vector(o, p);    // ou Vector d= p - o; // si vous preferrez...

        // parcours tous les triangles
        for(unsigned i= 0; i +2 < positions.size(); i+= 3)
        {
            Point t[3]= {
                positions[ i    ],
                positions[ i +1 ],
                positions[ i +2 ]
            };

            int m_id= materials_ids[i/3];
            Material& new_material= materials(m_id);

            // triangle
            if(intersctionRayonAvecPlanBarycentrique(o, dist, t[0], t[1], t[2], intersectionPoint, t_proche, normal)) {
                //Color col = new_material.diffuse * dot(normal, normalize(Vector(intersectionPoint, Point(0, 200, 0))));  // pour la scene
                Color col = new_material.diffuse * dot(normal, normalize(Vector(intersectionPoint, Point(0, 2, -2))));    // pour le robot
                image(px, py) = Color(col, 1);
            }
        }
    }
    
    write_image_png(image, "image.png"); // par defaut en .png
    return 0;
}