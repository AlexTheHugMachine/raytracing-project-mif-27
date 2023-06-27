// exemple de code test a compiler pour verifier que tout est ok
#include <stdlib.h>
#include "color.h"
#include "image.h"
#include "image_io.h"
#include "vec.h"

bool appartient_triangle(const Point &p, const Point &a, const Point &b,const Point &c) {

    Vector ab(a,b);
    Vector ap(a,p);
    float detABP = (ab.x*ap.y) - (ap.x*ab.y);

    Vector bc(b, c); 
    Vector bp(b, p);
    float detBCP = (bc.x*bp.y) - (bp.x*bc.y);

    Vector ca(c, a);
    Vector cp(c, p);
    float detCAP = (ca.x*cp.y) - (cp.x*ca.y);

    //Vector ab(a, b);
    Vector ac(a, c);
    float detABC = (ab.x*ac.y) - (ac.x*ab.y);

    if((detABP >= 0 && detBCP >= 0 && detCAP >= 0 && detABC >= 0) || (detABP <= 0 && detBCP <= 0 && detCAP <= 0 && detABC <= 0)) {
        return true;
    }
    return false;
}

float aire_triangle(const Point & a, const Point & b, const Point & c) {
    float aire = 0.5 * length(cross(Vector(a,b), Vector(a,c)));
    //std::cout << "Aire : " << aire << std::endl;
    return aire;
}


Point interpole(const Point & p, const Point & a, const Point & b, const Point & c) {
    float lambdaA = aire_triangle(b, c, p)/aire_triangle(a, b, c);
    float lambdaB = aire_triangle(c, a, p)/aire_triangle(a, b, c);
    float lambdaC = aire_triangle(a, b, p)/aire_triangle(a, b, c);
    return Point(lambdaA, lambdaB, lambdaC);
}

int main( )
{

    //! PARTIE 1 ///////////////////////////////////////////////////
    // cree l'image resultat
    Image image(1024, 1024);

    Point a(724, 950, 0);
    Point b(512, 300, 0);
    Point c(300, 950, 0);

    // Code de base : mets tous les pixels au rouge

    for(int py= 0; py < image.height(); py++)
    for(int px= 0; px < image.width(); px++)
        image(px, py)= Black();

    
    // Exercice 1 : vérifie tous les pixels pour savoir s'ils font partie du triangle et les colorie en blanc si c'est le cas
    /*
    for(int j = 0; j < image.height(); j++) {
        for(int i = 0; i < image.width(); i++){
            Point p(i, j, 0);
            if(appartient_triangle(p, a, b, c)) {
                image((j*image.width()) + i) = White();
            }
            else {
                image((j*image.width()) + i) = Red();
            }
        }
    }*/

    // Exercice 2 : Utiliser un rectangle englobante
    /*Point maxp = max(max(a, b), c);
    Point minp = min(min(a, b), c);
    std::cout << "Max " << maxp << std::endl;
    std::cout << "Min " << minp << std::endl;
    for(int j = minp.y; j <= maxp.y; j++) {
        for(int i = minp.x; i <= maxp.x; i++){
            Point p(i, j, 0);
            if(appartient_triangle(p, a, b, c)) {
                image((j*image.width()) + i) = White();
            }
            else {
                image((j*image.width()) + i) = Red();
            }
        }
    }*/

    // Exercice 3 : Définition une couleur par sommet et interpolation au sein du triangle
    // cf coordonnées barycentriques

    /*Color ca = Red();
    Color cb = Blue();
    Color cc = Green();

    Point maxp = max(max(a, b), c);
    Point minp = min(min(a, b), c);
    for(int j = minp.y; j <= maxp.y; j++) {
        for(int i = minp.x; i <= maxp.x; i++){
            Point p(i, j, 0);
            if(appartient_triangle(p, a, b, c)) {
                Point interpole3 = interpole(p, a, b, c);
                std::cout << "Valeurs de lambdaA, lambdaB, lambdaC : " << interpole3 << std::endl;
                image(i, j) = Color(interpole3.x*ca + interpole3.y*cb + interpole3.z*cc);
            }
            else {
                image(i, j) = grey;
            }
        }
    }*/

    // Exercice 4 : Affichage de 2 triangles qui se superposent
    /// 1er triangle : abc
    /// 2ieme triangle : efg

    Point e(950, 724, 0);
    Point f(300, 512, 0);
    Point g(950, 300, 0);

    Color cabc = Red();
    Color cefg = Blue();

    Point maxp = max(max(max(a, b), c), max(max(e, f), g));
    Point minp = min(min(min(a, b), c), min(min(e, f), g));
    for(int j = minp.y; j <= maxp.y; j++) {
        for(int i = minp.x; i <= maxp.x; i++){
            Point p(i, j, 0);
            if(appartient_triangle(p, a, b, c) && appartient_triangle(p, e, f, g)) {
                image(i, j) = cabc + cefg;
            }
            else if(appartient_triangle(p, a, b, c)) {
                image(i, j) = cabc;
            }
            else if(appartient_triangle(p, e, f, g)) {
                image(i, j) = cefg;
            }
            else {
                image(i, j) = Black();
            }
        }
    }

    write_image_png(image, "image.png");

    return 0;
}