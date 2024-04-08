
//! \file tuto7_camera.cpp reprise de tuto7.cpp mais en derivant AppCamera, avec gestion automatique d'une camera.

#include "wavefront.h"
#include "texture.h"

#include "orbiter.h"
#include "draw.h"        
#include "app_camera.h"        // classe Application a deriver
#include "uniforms.h"


// utilitaire. creation d'une grille / repere.
Mesh make_grid( const int n= 10 )
{
    Mesh grid= Mesh(GL_LINES);
    
    // grille
    grid.color(White());
    for(int x= 0; x < n; x++)
    {
        float px= float(x) - float(n)/2 + .5f;
        grid.vertex(Point(px, 0, - float(n)/2 + .5f)); 
        grid.vertex(Point(px, 0, float(n)/2 - .5f));
    }

    for(int z= 0; z < n; z++)
    {
        float pz= float(z) - float(n)/2 + .5f;
        grid.vertex(Point(- float(n)/2 + .5f, 0, pz)); 
        grid.vertex(Point(float(n)/2 - .5f, 0, pz)); 
    }
    
    // axes XYZ
    grid.color(Red());
    grid.vertex(Point(0, .1, 0));
    grid.vertex(Point(1, .1, 0));
    
    grid.color(Green());
    grid.vertex(Point(0, .1, 0));
    grid.vertex(Point(0, 1, 0));
    
    grid.color(Blue());
    grid.vertex(Point(0, .1, 0));
    grid.vertex(Point(0, .1, 1));
    
    glLineWidth(2);
    
    return grid;
}


class TP : public AppCamera
{
public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    TP( ) : AppCamera(1024, 640) {}
    
    // creation des objets de l'application
    int init( )
    {
        camera().lookat(Point(-50, -50, -50), Point(50, 50, 50));
        program_cubemap= read_program("tutos/cubemap.glsl");
        program_print_errors(program_cubemap);

        program_texture = read_program("src/shader/shader_texture.glsl");
        program_print_errors(program_texture);

        program_shadow_map = read_program("src/shader/shader_shadow_map.glsl");
        program_print_errors(program_shadow_map);

        // decrire un repere / grille 
        m_repere= make_grid(10);
        
        // charge un objet
        m_cube= read_mesh("data/cube.obj");

        m_bistro= read_mesh("data/bistro-small-export/export.obj");
        Materials& materials_bistro= m_bistro.materials();
        if(materials_bistro.filename_count() == 0)
            return -1;     // pas de textures, pas d'affichage
        if(m_bistro.materials().count() == 0)
            return -1;     // pas de matieres, pas d'affichage
        m_group_bistro = m_bistro.groups();
        m_texture_bistro.resize(materials_bistro.filename_count());
        for(unsigned int i= 0; i < m_texture_bistro.size(); i++)
        {
            m_texture_bistro[i]= read_texture(0, materials_bistro.filename(i));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        // charge aussi une texture neutre pour les matieres sans texture...
        m_white_texture= read_texture(0, "data/grid.png");  // !! utiliser une vraie image blanche...
        m_transparent_texture= read_texture(0, "data/transparent.png");

        for(unsigned i= 0; i < m_group_bistro.size(); i++)
        {
            const TriangleGroup& group_= m_group_bistro[i];
            Point pmaxGroupp = Point(-100000, -100000, -100000);
            Point pminGroupp = Point(100000, 100000, 100000);
            for (int j = group_.first; j < group_.first + group_.n; j++)
            {
                Point p1 = m_bistro.triangle(j/3).a;
                Point p2 = m_bistro.triangle(j/3).b;
                Point p3 = m_bistro.triangle(j/3).c;

                pmaxGroupp = max(pmaxGroupp, p1);
                pmaxGroupp = max(pmaxGroupp, p2);
                pmaxGroupp = max(pmaxGroupp, p3);

                pminGroupp = min(pminGroupp, p1);
                pminGroupp = min(pminGroupp, p2);
                pminGroupp = min(pminGroupp, p3);
            }

            pmaxGroup.push_back(pmaxGroupp);
            pminGroup.push_back(pminGroupp);

            pBoundingBoxGroup.resize(m_group_bistro.size());
            pBoundingBoxGroupW.resize(m_group_bistro.size());
            pBoundingBoxGroup[i].resize(8);
            pBoundingBoxGroupW[i].resize(8);

            pBoundingBoxGroup[i][0] = pminGroup[i];
            pBoundingBoxGroup[i][1] = Point(pminGroup[i].x, pminGroup[i].y, pmaxGroup[i].z);
            pBoundingBoxGroup[i][2] = Point(pminGroup[i].x, pmaxGroup[i].y, pmaxGroup[i].z);
            pBoundingBoxGroup[i][3] = Point(pminGroup[i].x, pmaxGroup[i].y, pminGroup[i].z);
            pBoundingBoxGroup[i][4] = Point(pmaxGroup[i].x, pmaxGroup[i].y, pminGroup[i].z);
            pBoundingBoxGroup[i][5] = Point(pmaxGroup[i].x, pminGroup[i].y, pminGroup[i].z);
            pBoundingBoxGroup[i][6] = Point(pmaxGroup[i].x, pminGroup[i].y, pmaxGroup[i].z);
            pBoundingBoxGroup[i][7] = pmaxGroup[i];
        }

        pBoxFrustum[0] = Point(-1, -1, -1);
        pBoxFrustum[1] = Point(-1, -1, 1);
        pBoxFrustum[2] = Point(-1, 1, 1);
        pBoxFrustum[3] = Point(-1, 1, -1);
        pBoxFrustum[4] = Point(1, 1, -1);
        pBoxFrustum[5] = Point(1, -1, -1);
        pBoxFrustum[6] = Point(1, -1, 1);
        pBoxFrustum[7] = Point(1, 1, 1);
        
        // un autre objet
        m_objet= Mesh(GL_TRIANGLES);
        {
            // ajouter des triplets de sommet == des triangles dans objet...
        }

        //! bistro
        glGenBuffers(1, &vertex_buffer2);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer2);

        // Dimensionne le buffer actif sur array_buffer, l'alloue et l'initialise avec les positions des sommets de l'objet
        glBufferData(GL_ARRAY_BUFFER,
                    m_bistro.vertex_buffer_size() + m_bistro.normal_buffer_size() + m_bistro.texcoord_buffer_size(),
                    nullptr,
                    GL_STATIC_DRAW); // Utilisation par draw, sans modifications
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_bistro.vertex_buffer_size(), m_bistro.vertex_buffer());
        glBufferSubData(GL_ARRAY_BUFFER, m_bistro.vertex_buffer_size(), m_bistro.normal_buffer_size(), m_bistro.normal_buffer());
        glBufferSubData(GL_ARRAY_BUFFER, m_bistro.vertex_buffer_size() + m_bistro.normal_buffer_size(), m_bistro.texcoord_buffer_size(), m_bistro.texcoord_buffer());

        // Création d'un vertex array object
        glGenVertexArrays(1, &vao2);
        glBindVertexArray(vao2);

        //Positions
        GLint attribute = 0;
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer2);
        // Tableau de 3 float.
        glVertexAttribPointer(attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(attribute);

        //Normales
        attribute = 2;
        glVertexAttribPointer(attribute, 3, GL_FLOAT, GL_FALSE, 0, (void*)m_bistro.vertex_buffer_size()); // in vec3 normal;
        glEnableVertexAttribArray(attribute);

        //Texture
        attribute = 1;
        glVertexAttribPointer(attribute, 2, GL_FLOAT, GL_FALSE, 0, (void*)(m_bistro.vertex_buffer_size() + m_bistro.normal_buffer_size())); // in vec3 normal;
        glEnableVertexAttribArray(attribute);

        vertex_count_bistro = m_bistro.vertex_count();
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        
        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest

  	    glEnable(GL_LIGHTING); 	// Active l'�clairage
  	    glEnable(GL_LIGHT0); 	// Allume la lumi�re n�1

        shadow_map= make_depth_texture( /* unit */ 0, /* width */ 1024, /* height */ 640 );

        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
    
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, /* attachment */ GL_DEPTH_ATTACHMENT, shadow_map, /* mipmap */ 0);
    
        // verification de la configuration du framebuffer
        if(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            return 0;

        // nettoyage...
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        return 0;   // pas d'erreur, sinon renvoyer -1
    }
    
    // destruction des objets de l'application
    int quit( )
    {
        release_program(program_texture);
        release_program(program_cubemap);
        m_objet.release();
        m_repere.release();
        m_bistro.release();
        glDeleteBuffers(1, &vertex_buffer2);
        glDeleteVertexArrays(1, &vao2);
        return 0;   // pas d'erreur
    }

    bool estDansLeFrustrum(std::vector<vec4> pBoundingBoxW)
    {
        bool testG = true;
        bool testD = true;
        bool testH = true;
        bool testB = true;
        bool testZD = true;
        bool testZG = true;
        for(int i = 0; i < 8; i++)
        {
            testG = testG && (pBoundingBoxW[i].x < -pBoundingBoxW[i].w);
            testD = testD && (pBoundingBoxW[i].x > pBoundingBoxW[i].w);
            testH = testH && (pBoundingBoxW[i].y < -pBoundingBoxW[i].w);
            testB = testB && (pBoundingBoxW[i].y > pBoundingBoxW[i].w);
            testZD = testZD && (pBoundingBoxW[i].z < -pBoundingBoxW[i].w);
            testZG = testZG && (pBoundingBoxW[i].z > pBoundingBoxW[i].w);
        }
        return !(testG || testD || testH || testB || testZD || testZG);
    }

    void saveTexture(GLuint & texture, size_t w, size_t h, size_t nbChannels, GLuint format, const std::string & path) {
        glBindTexture(GL_TEXTURE_2D, texture);
        std::vector<unsigned char> pixels(w * h * nbChannels, 0);
        glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, pixels.data());
        Image im(w, h, Black());
        if(nbChannels == 1) {
            for(int i = 0; i < 1024 * 640; i++) {
                Color c = Color(pixels[i]/255.0, pixels[i]/255.0, pixels[i]/255.0);
                im(i) = c;
            }
        } else {
            for(int i = 0; i < 1024 * 640; i++) {
                Color c = Color(pixels[i*3]/255.0, pixels[i*3+1]/255.0, pixels[i*3+2]/255.0);
                im(i) = c;
            }
        }
        write_image(im, path.c_str());
    }

    
    // dessiner une nouvelle image
    int render( )
    {
        draw(m_repere, /* model */ Identity(), camera());
        m_cube.default_color(White());
        Transform t= Translation(light);
        draw(m_cube, t, camera());

        Transform r= RotationX(-90);
        Transform t2= Translation(light);
        Transform m= r * t2;

        Transform model2 = Identity();
        Transform model_light = Translation(light);
        Transform mvp2 = camera().projection() * camera().view() * model2;
        Transform mvp_light = Ortho(-100, 100, -100, 100, float(-1), float(100)) * Inverse(model_light * m);
        Transform mp2 = camera().projection() * model2;
        Transform pv = camera().projection() * camera().view();
        Transform pvi = camera().view().inverse() * camera().projection().inverse();

        Transform decal_viewport= Viewport(1, 1);
        Transform decal_matrix= decal_viewport * Ortho(-100, 100, -100, 100, float(-1), float(100)) * Inverse(model_light * m);

        vec4 pBoxFrustumWi[8];

        for (int i = 0; i < 8; i++)
        {
            pBoxFrustumWi[i] = pvi(vec4(pBoxFrustum[i]));
        }

        glUseProgram(program_shadow_map);
        GLuint location;
        location = glGetUniformLocation(program_shadow_map, "mvpMatrix");

        //! bistro

        glBindVertexArray(vao2);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniformMatrix4fv(location, 1, GL_TRUE, mvp_light.data());

        const Materials& materials = m_bistro.materials();
        for(unsigned i= 0; i < m_group_bistro.size(); i++)
        {
            const Material& material= materials(m_group_bistro[i].index);

            // recuperer la couleur de la matiere du groupe
            const TriangleGroup& group_= m_group_bistro[i];

            for (int j = 0; j < 8; j++)
            {
                pBoundingBoxGroupW[i][j] = pv(vec4(pBoundingBoxGroup[i][j]));
            }
            
            if(estDansLeFrustrum(pBoundingBoxGroupW[i]))
            {
                program_uniform(program_shadow_map, "mvpMatrix", mvp_light);
                glDrawArrays(GL_TRIANGLES, group_.first, group_.n);
            }
        }
        saveTexture(shadow_map, 1024, 640, 1, GL_DEPTH_COMPONENT, "shadow_map.png");

        glUseProgram(program_texture);
        location = glGetUniformLocation(program_texture, "mvpMatrix");

        //! bistro

        glBindVertexArray(vao2);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniformMatrix4fv(location, 1, GL_TRUE, mvp2.data());

        program_uniform(program_texture, "mvpMatrix", mvp2);
        program_uniform(program_texture, "mpMatrix", mp2);

        vec4 pBoundingBoxBistroW[8];
        for (int i = 0; i < 8; i++)
        {
            pBoundingBoxBistroW[i] = pv(vec4(pBoundingBoxBistro[i]));
        }

        for(unsigned i= 0; i < m_group_bistro.size(); i++)
        {
            const Material& material= materials(m_group_bistro[i].index);
            const TriangleGroup& group_= m_group_bistro[i];

            for (int j = 0; j < 8; j++)
            {
                pBoundingBoxGroupW[i][j] = pv(vec4(pBoundingBoxGroup[i][j]));
            }
            
            if(estDansLeFrustrum(pBoundingBoxGroupW[i]))
            {                
                if(material.diffuse_texture != -1)
                    program_use_texture(program_texture, "texture0", 0, m_texture_bistro[material.diffuse_texture]);
                else
                    program_use_texture(program_texture, "texture0", 0, m_white_texture);

                program_uniform(program_texture, "decalMatrix", decal_matrix);
                program_use_texture(program_texture, "decal", 1, shadow_map);
                program_uniform(program_texture, "vertex_direction", vec3(light.x, light.y, light.z));
                program_uniform(program_texture, "level_number", 128);
                glDrawArrays(GL_TRIANGLES, group_.first, group_.n);
            }
        }
        
        if(key_state('a'))
            light.y += 0.1;
        if(key_state('e'))
            light.y -= 0.1;
        if(key_state('z'))
            light.z += 0.1;
        if(key_state('w'))
            light.z -= 0.1;
        if(key_state('q'))
            light.x += 0.1;
        if(key_state('d'))
            light.x -= 0.1;
        

        glUseProgram(0);
        glBindVertexArray(0);
        return 1;
    }

protected:
    Mesh m_objet;
    Mesh m_cube;
    Mesh m_repere;
    Mesh m_bistro;

    Point pBoundingBoxBistro[8];
    Point pBoundingBoxBistroI[8];
    Point pBoxFrustum[8];
    std::vector<Point> pmaxGroup;
    std::vector<Point> pminGroup;
    std::vector<std::vector<Point>> pBoundingBoxGroup;
    std::vector<std::vector<vec4>> pBoundingBoxGroupW;

    Orbiter m_camera;

    vec3 light = vec3(0, 20, 10);

    GLuint program_texture;
    GLuint program_shadow_map;
    GLuint shadow_map;
    GLuint framebuffer=0;
    GLuint program_cubemap;
    GLuint vao2;
    GLuint vertex_buffer2;
    GLuint normal_buffer2;
    GLuint texture_buffer_bistro;
    GLuint m_white_texture;
    GLuint m_transparent_texture;
    std::vector<GLuint> m_texture_bistro;
    unsigned int vertex_count_laurus;
    unsigned int vertex_count_bistro;

    std::vector<TriangleGroup> m_group_laurus;
    std::vector<TriangleGroup> m_group_bistro;
};


int main( int argc, char **argv )
{
    // il ne reste plus qu'a creer un objet application et la lancer 
    TP tp;
    tp.run();
    
    return 0;
}
