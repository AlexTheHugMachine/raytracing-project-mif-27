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

int main( )
{
    const char *filename= "data/geometry.obj";    // pour la scene
    //const char *filename= "data/robot.obj";       // pour le robot
   
    std::vector<Point> positions;
    std::vector<int> materials_ids;
    Point triangle_p_proche[3];
    Materials materials;

    if(!read_positions(filename, positions))
        std::cout << "Erreur chargement positions" << std::endl;
    if(!read_materials(filename, materials, materials_ids))
        std::cout << "Erreur chargement materials" << std::endl;
   
    // deplace tous les sommets devant la camera
    for(unsigned i= 0; i < positions.size(); i++) {
        positions[i]= positions[i] + Vector(0, -110, -400);   // pour la scene
        //positions[i]= positions[i] + Vector(0, -2, -4);     // pour le robot
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

        Point Pos_Light = Point(200, 200, 100); // pour la scene
        //Point Pos_Light = Point(0, 2, -2); // pour le robot

        // rayon centre camera
        float x = float(px)/image.width() * 2 - 1;
        float y = float(py)/image.height() * 2 - 1;
        float z = -1;       
        Point p = Point(x, y, z);

        Point o = Point(0, 0, 0);
        Vector dist = Vector(o, p);    // ou Vector d= p - o; // si vous preferrez...
        Color col;
        int m_id;

        // parcours tous les triangles
        for(unsigned i= 0; i +2 < positions.size(); i+= 3)
        {
            Point t[3]= {
                positions[ i    ],
                positions[ i +1 ],
                positions[ i +2 ]
            };

            Vector normPlan = normalize(cross( Vector(t[0],t[1]), Vector(t[0],t[2]) ));

            float t_coef = dot(normPlan, Vector(o, t[0])) / dot(normPlan, dist);
            Point p_coef = o + t_coef * dist;

            if (t_coef > 0 && t_coef < t_proche)
            {
                if(appartient_tetraedre(dist, p_coef, t[0], t[1], t[2])) {
                    intersectionPoint = p_coef;
                    t_proche = t_coef;
                    normal = normPlan;
                    triangle_p_proche[0] = t[0];
                    triangle_p_proche[1] = t[1];
                    triangle_p_proche[2] = t[2];
                    m_id= materials_ids[i/3];
                }
            }
        }

        Material& new_material= materials(m_id);
        for(unsigned i= 0; i +2 < positions.size(); i+= 3)
        {
            Point t[3]= {
                positions[ i    ],
                positions[ i +1 ],
                positions[ i +2 ]
            };

            Vector normPlan = normalize(cross( Vector(t[0],t[1]), Vector(t[0],t[2]) ));

            Vector directionLight = normalize(Vector(intersectionPoint, Pos_Light));
            float t_coef = dot(normPlan, Vector(intersectionPoint, t[0])) / dot(normPlan, directionLight);

            Point p_coef_light = intersectionPoint + t_coef * directionLight;
            float distanceToLight = sqrt(pow(Pos_Light.x - intersectionPoint.x, 2) + pow(Pos_Light.y - intersectionPoint.y, 2) + pow(Pos_Light.z - intersectionPoint.z, 2));
            float distanceOriginIntersect = sqrt(pow(p_coef_light.x - intersectionPoint.x, 2) + pow(p_coef_light.y - intersectionPoint.y, 2) + pow(p_coef_light.z - intersectionPoint.z, 2));

            if (t_coef > 0 && distanceOriginIntersect < distanceToLight)
            {
                if(appartient_tetraedre(directionLight, intersectionPoint, t[0], t[1], t[2])) {
                    col = new_material.diffuse * 0;
                    image(px, py) = Color(col, 1);
                    break;
                }
            }
            else {
                col = new_material.diffuse * dot(normal, normalize(Vector(intersectionPoint, Pos_Light)));
                image(px, py) = Color(col, 1);
            }
        }
    }
    
    write_image_png(image, "image.png"); // par defaut en .png
    return 0;
}