
#include "vec.h"
#include "mat.h"

#include "mesh.h"
#include "wavefront.h"
#include "draw.h"

#include "program.h"
#include "uniforms.h"

#include "app_camera.h"


void transform_frustum( const Transform& inv, Point frustum[8] )
{
    Point positions[8]= { 
        { -1, -1, -1 },
        { -1, -1, 1 },
        { -1, 1, -1 },
        { -1, 1, 1 },
        { 1, -1, -1 },
        { 1, -1, 1 },
        { 1, 1, -1 },
        { 1, 1, 1 },
    };
    
    for(int i= 0; i < 8; i++) 
        frustum[i]= inv( positions[i] );
}

bool cull( const Point& pmin, const Point& pmax, const Point fustrum[8])
{
    //~ cull_box(pmin, pmax, mvp) || cull_frustum(pmin, pmax, frustum);
    Point pBoundingBox[8];
    vec4 pBoundingBoxW[8];
    vec4 fustrumWi[8];
    Transform pv = camera().projection() * camera().view();

    pBoundingBox[0] = pmin;
    pBoundingBox[1] = Point(pmin.x, pmin.y, pmax.z);
    pBoundingBox[2] = Point(pmin.x, pmax.y, pmax.z);
    pBoundingBox[3] = Point(pmin.x, pmax.y, pmin.z);
    pBoundingBox[4] = Point(pmax.x, pmax.y, pmin.z);
    pBoundingBox[5] = Point(pmax.x, pmin.y, pmin.z);
    pBoundingBox[6] = Point(pmax.x, pmin.y, pmax.z);
    pBoundingBox[7] = pmax;

    for (int i = 0; i < 8; i++)
    {
        pBoundingBoxW[i] = pv(vec4(pBoundingBox[i]));
    }

    for (int i = 0; i < 8; i++)
    {
        fustrumWi[i] = pvi(vec4(fustrum[i]));
    }

    for (int i = 0; i < 8; i++)
    {
        if (pBoundingBoxW[i].x < -pBoundingBoxW[i].w)
            return false;
        if (pBoundingBoxW[i].x > pBoundingBoxW[i].w)
            return false;
        if (pBoundingBoxW[i].y < -pBoundingBoxW[i].w)
            return false;
        if (pBoundingBoxW[i].y > pBoundingBoxW[i].w)
            return false;
        if (pBoundingBoxW[i].z < -pBoundingBoxW[i].w)
            return false;
        if (pBoundingBoxW[i].z > pBoundingBoxW[i].w)
            return false;
        if(pmin.x )
    }
}


Mesh make_grid( const int n= 10 )
{
    Mesh grid= Mesh(GL_LINES);
    
    // grille
    grid.color(White());
    for(int x= 0; x < n; x++)
    {
        float px= float(x) - float(n)/2 + .5f;
        grid.vertex(px, 0, - float(n)/2 + .5f); 
        grid.vertex(px, 0, float(n)/2 - .5f);
    }

    for(int z= 0; z < n; z++)
    {
        float pz= float(z) - float(n)/2 + .5f;
        grid.vertex(- float(n)/2 + .5f, 0, pz); 
        grid.vertex(float(n)/2 - .5f, 0, pz); 
    }
    
    // axes XYZ
    grid.color(Red());
    grid.vertex(0, .1, 0);
    grid.vertex(1, .1, 0);
    
    grid.color(Green());
    grid.vertex(0, .1, 0);
    grid.vertex(0, 1, 0);
    
    grid.color(Blue());
    grid.vertex(0, .1, 0);
    grid.vertex(0, .1, 1);
    
    glLineWidth(2);
    
    return grid;
}

Mesh make_frustum( )
{
    glLineWidth(2);    
    Mesh camera= Mesh(GL_LINES);
    
    camera.color(Yellow());
    // face avant
    camera.vertex(-1, -1, -1);
    camera.vertex(-1, 1, -1);
    camera.vertex(-1, 1, -1);
    camera.vertex(1, 1, -1);
    camera.vertex(1, 1, -1);
    camera.vertex(1, -1, -1);
    camera.vertex(1, -1, -1);
    camera.vertex(-1, -1, -1);
    
    // face arriere
    camera.vertex(-1, -1, 1);
    camera.vertex(-1, 1, 1);
    camera.vertex(-1, 1, 1);
    camera.vertex(1, 1, 1);
    camera.vertex(1, 1, 1);
    camera.vertex(1, -1, 1);
    camera.vertex(1, -1, 1);
    camera.vertex(-1, -1, 1);
    
    // aretes
    camera.vertex(-1, -1, -1);
    camera.vertex(-1, -1, 1);
    camera.vertex(-1, 1, -1);
    camera.vertex(-1, 1, 1);
    camera.vertex(1, 1, -1);
    camera.vertex(1, 1, 1);
    camera.vertex(1, -1, -1);
    camera.vertex(1, -1, 1);
    
    return camera;
}

struct TP : public AppCamera
{
    TP( ) : AppCamera(1024, 768) {}
    
    int init( )
    {
        m_objet= read_mesh("data/cube.obj");
        m_frustum= make_frustum();
        m_frustum_projection= Perspective(45, 1, float(1), float(4));
        m_frustum_inv_projection= Inverse( m_frustum_projection );
        
        m_grid= make_grid();
        Point pmin, pmax;
        m_grid.bounds(pmin, pmax);
        camera().lookat(pmin, pmax);
        
        m_vao= m_objet.create_buffers(false, true, false, false);
        m_program= read_program("src/shader/tuto_cull.glsl");
        program_print_errors(m_program);
        
        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest
        
        return 0;
    }
    
    int quit( )
    {
        // todo
        return 0;
    }
    
    int render( )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // modifie la position du frustum en fonction des fleches de direction
        if(key_state(SDLK_UP))
            m_position= m_position * Translation(0, 0, 0.05);     // en avant
        if(key_state(SDLK_DOWN))
            m_position= m_position * Translation(0, 0, -0.05);      // en arriere
        if(key_state(SDLK_PAGEUP))
            m_position= m_position * Translation(0, 0.05, 0);     // en haut
        if(key_state(SDLK_PAGEDOWN))
            m_position= m_position * Translation(0, -0.05, 0);      // en bas
        
        if(key_state(SDLK_LEFT))
            m_position= m_position * RotationY(4);     // tourne vers la droite
        if(key_state(SDLK_RIGHT))
            m_position= m_position * RotationY(-4);     // tourne vers la gauche
        
        // transformation du repere du frustum vers la scene
        Transform frustum_m= m_position * Translation(0, 4, 0) * RotationX(-90) * m_frustum_inv_projection;
        // transformation inverse : scene vers frustum
        Transform frustum_mvp= Inverse(frustum_m);
        
        // sommets du frustum dans le repere de la scene
        Point frustum[8];
        transform_frustum(frustum_m, frustum);
        
        // englobant de l'objet a tester
        Point pmin, pmax;
        m_objet.bounds(pmin, pmax);

        // test separation objet / frustum
        bool test= cull(pmin, pmax, frustum_mvp, frustum);
        
        // affiche le resutlat en couleur
        draw(m_grid, Identity(), camera());
        draw(m_frustum, frustum_m, camera());
        
    #if 0
        {
            if(test)
                m_objet.default_color( White() );
            else
                m_objet.default_color( Red() );
            
            draw(m_objet, Identity(), camera());
        }
    #else
        {
            Transform model= Identity();
            Transform view= camera().view();
            Transform projection= camera().projection();
            Transform mv= view * model;
            Transform mvp= projection * mv;
            Transform pv = projection * view;
            Transform pvi = view.inverse() * projection.inverse();
            
            glBindVertexArray(m_vao);
            glUseProgram(m_program);
            
            program_uniform(m_program, "mvpMatrix", mvp);
            program_uniform(m_program, "mvMatrix", mv);
            program_uniform(m_program, "frustumMatrix", frustum_mvp);
            
            if(test)
                program_uniform(m_program, "mesh_color", White());
            else
                program_uniform(m_program, "mesh_color", Red());
            
            m_objet.draw(m_program, /* position */ true, false, /* normal */ true, false, false);
        }
    #endif
        
        
        return 1;
    }

protected:
    Mesh m_objet;
    Mesh m_frustum;
    Mesh m_grid;
    
    Transform m_position;
    Transform m_frustum_projection;
    Transform m_frustum_inv_projection;
    
    GLuint m_program;
    GLuint m_vao;
};


int main( int argc, char **argv )
{
    TP app;
    app.run();
    
    return 0;
}
