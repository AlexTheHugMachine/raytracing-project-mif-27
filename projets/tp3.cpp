#include <vector>
 #include <cfloat>
 #include <chrono>
 #include <random>
  
 #include "vec.h"
 #include "mat.h"
 #include "color.h"
 #include "image.h"
 #include "image_io.h"
 #include "image_hdr.h"
 #include "orbiter.h"
 #include "mesh.h"
 #include "wavefront.h"
  
 struct Ray
 {
     Point o;                // origine
     Vector d;               // direction
     float tmax;             // position de l'extremite, si elle existe. le rayon est un intervalle [0 tmax]
     
     // le rayon est un segment, on connait origine et extremite, et tmax= 1
     Ray( const Point& origine, const Point& extremite ) : o(origine), d(Vector(origine, extremite)), tmax(1) {}
     
     // le rayon est une demi droite, on connait origine et direction, et tmax= \inf
     Ray( const Point& origine, const Vector& direction ) : o(origine), d(direction), tmax(FLT_MAX) {}
     
     // renvoie le point sur le rayon pour t
     Point point( const float t ) const { return o + t * d; }
 };
  
 struct Hit
 {
     float t;            // p(t)= o + td, position du point d'intersection sur le rayon
     float u, v;         // p(u, v), position du point d'intersection sur le triangle
     int triangle_id;    // indice du triangle dans le mesh
     
     Hit( ) : t(FLT_MAX), u(), v(), triangle_id(-1) {}
     Hit( const float _t, const float _u, const float _v, const int _id ) : t(_t), u(_u), v(_v), triangle_id(_id) {}
     
     // renvoie vrai si intersection
     operator bool ( ) { return (triangle_id != -1); }
 };
  
 struct Triangle
 {
     Point p;            // sommet a du triangle
     Vector e1, e2;      // aretes ab, ac du triangle
     int id;             // indice du triangle
     
     Triangle( const TriangleData& data, const int _id ) : p(data.a), e1(Vector(data.a, data.b)), e2(Vector(data.a, data.c)), id(_id) {}
     
     /*  calcule l'intersection ray/triangle
         cf "fast, minimum storage ray-triangle intersection" 
         
         renvoie faux s'il n'y a pas d'intersection valide (une intersection peut exister mais peut ne pas se trouver dans l'intervalle [0 tmax] du rayon.)
         renvoie vrai + les coordonnees barycentriques (u, v) du point d'intersection + sa position le long du rayon (t).
         convention barycentrique : p(u, v)= (1 - u - v) * a + u * b + v * c
     */
     Hit intersect( const Ray &ray, const float tmax ) const
     {
         Vector pvec= cross(ray.d, e2);
         float det= dot(e1, pvec);
         
         float inv_det= 1 / det;
         Vector tvec(p, ray.o);
         
         float u= dot(tvec, pvec) * inv_det;
         if(u < 0 || u > 1) return Hit();        // pas d'intersection
         
         Vector qvec= cross(tvec, e1);
         float v= dot(ray.d, qvec) * inv_det;
         if(v < 0 || u + v > 1) return Hit();    // pas d'intersection
         
         float t= dot(e2, qvec) * inv_det;
         if(t > tmax || t < 0) return Hit();     // pas d'intersection
         
         return Hit(t, u, v, id);                // p(u, v)= (1 - u - v) * a + u * b + v * c
     }
 };
  
 // renvoie la normale au point d'intersection
 Vector normal( const Mesh& mesh, const Hit& hit )
 {
     // recuperer le triangle du mesh
     const TriangleData& data= mesh.triangle(hit.triangle_id);
     
     // interpoler la normale avec les coordonn�es barycentriques du point d'intersection
     float w= 1 - hit.u - hit.v;
     Vector n= w * Vector(data.na) + hit.u * Vector(data.nb) + hit.v * Vector(data.nc);
     return normalize(n);
 }
  
 int main( const int argc, const char **argv )
 {
     const char *mesh_filename= "data/cornell.obj";
     if(argc > 1)
         mesh_filename= argv[1];
         
     const char *orbiter_filename= "data/cornell_orbiter.txt";
     if(argc > 2)
         orbiter_filename= argv[2];
     
     Orbiter camera;
     if(camera.read_orbiter(orbiter_filename) < 0)
         return 1;
  
     Mesh mesh= read_mesh(mesh_filename);
     
     // recupere les triangles dans le mesh
     std::vector<Triangle> triangles;
     {
         int n= mesh.triangle_count();
         for(int i= 0; i < n; i++)
             triangles.emplace_back(mesh.triangle(i), i);
     }
  
     Image image(1024, 768);
  
     // recupere les transformations pour generer les rayons
     camera.projection(image.width(), image.height(), 45);
     Transform model= Identity();
     Transform view= camera.view();
     Transform projection= camera.projection();
     Transform viewport= camera.viewport();
     Transform inv= Inverse(viewport * projection * view * model);

     std::vector<Triangle> sources;

     for(int i = 0; i < int(triangles.size()); i++)
     {
        const Material& material = mesh.triangle_material(i);
        Color emission = material.emission;
        if(emission.r != 0.0)
            sources.push_back(triangles[i]);
     }
     
 auto start= std::chrono::high_resolution_clock::now();
     
     // parcours tous les pixels de l'image

     
     #pragma omp parallel for
     for(int y= 0; y < image.height(); y++)
     {
        std::random_device hwseed;
        // init d'un générateur de nombre de aleatoire std
        std::default_random_engine rng( hwseed() );
        std::uniform_real_distribution<float> uniform(0, 1);
        for(int x= 0; x < image.width(); x++)
        {
            // generer le rayon au centre du pixel
            Point origine= inv(Point(x + float(0.5), y + float(0.5), 0));
            Point extremite= inv(Point(x + float(0.5), y + float(0.5), 1));
            Ray ray(origine, extremite);
            Hit hit;                // proprietes de l'intersection
            float tmax= ray.tmax;   // extremite du rayon

           for(int i= 0; i < int(triangles.size()); i++)
           {
               if(Hit h= triangles[i].intersect(ray, tmax))
               {
                   // ne conserve que l'intersection *valide* la plus proche de l'origine du rayon
                   assert(h.t > 0);
                   hit= h;
                   tmax= h.t;
               }
           }

            Color color;
            const int N = 256;
            Color diffuse;
                
           for(int i = 0; i < N; i++)
           {
               TriangleData triangle_hit;
               Point Point_a_th;
               Point Point_b_th;
               Point Point_c_th;
               Vector e1_pt;
               Vector e2_pt;
               Vector normal_pt;
               float distance_carre_pt_q;
               float cos_theta;
               float cos_theta2;
               float pdf;
               float V = 1;
               Color emission;

               Point pt = ray.point(hit.t);
               if(hit.triangle_id != -1)
               {
                    diffuse = mesh.triangle_material(hit.triangle_id).diffuse;
                    triangle_hit = mesh.triangle(hit.triangle_id);
                    Point_a_th = triangle_hit.a;
                    Point_b_th = triangle_hit.b;
                    Point_c_th = triangle_hit.c;

                    e1_pt = Vector(Point_a_th, Point_b_th);
                    e2_pt = Vector(Point_a_th, Point_c_th);

                    normal_pt = normalize(cross(e1_pt, e2_pt));

                    int s= uniform( rng ) * sources.size();   // uniforme entre 0 et n
                    const Triangle& source= sources[s];
                    Vector direction_source = normalize(source.p - pt);
                    Vector normal_source = normalize(cross(source.e2, source.e1));

                    // selectionne un point sur la source
                    float u2= uniform( rng );
                    float u3= uniform( rng );

                    // place le point dans la source
                    Point q= source.p + source.e1 * u2 + source.e2 * u3;
                    float aire_source= length(cross(source.e1, source.e2)) / 2;

                    // evalue la densite de proba
                    pdf= 1 / float(sources.size()) * 1 / aire_source;

                    Ray ray2(pt, q);
                    ray2.o = ray2.point(0.001);

                    distance_carre_pt_q = distance2(pt, q);
                    cos_theta = dot(normal_pt, direction_source);
                    cos_theta2 = dot(normal_source, normalize(ray2.d));

                    Hit hit3;
                    float tmax3 = ray2.tmax;
                    for(int g = 0; g < int(triangles.size()); g++) 
                    {
                        V = 1;
                        if(Hit h2 = triangles[g].intersect(ray2, tmax3)) 
                        {
                            hit3= h2;
                            tmax3= h2.t;
                            const Material& material = mesh.triangle_material(h2.triangle_id);
                            emission = material.emission;
                            V = 0;
                        }
                    }
               }
               // moyenne
               color= color + diffuse/M_PI * emission * V *cos_theta * cos_theta2 * 1 / distance_carre_pt_q * 1 / pdf;
           }
           color= color / float(N);
           image(x, y)= Color(color, 1);
        }
     }
  
 auto stop= std::chrono::high_resolution_clock::now();
     int cpu= std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
     printf("%dms\n", cpu);
     
     write_image(image, "render.png");
     write_image_hdr(image, "render.hdr");
     return 0;
 }