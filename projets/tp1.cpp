
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
        //program = read_program("src/shader/shader.glsl");
        //program_print_errors(program);
        program_cubemap= read_program("tutos/cubemap.glsl");
        program_print_errors(program_cubemap);

        program_texture = read_program("src/shader/shader_texture.glsl");
        program_print_errors(program_texture);

        // decrire un repere / grille 
        m_repere= make_grid(10);
        
        // charge un objet
        m_cube= read_mesh("data/cube.obj");

        m_bigguy= read_mesh("data/zombie/ZombieSmooth.obj");
        m_texture_bigguy= read_texture(0, "data/zombie/ZombieTexture.png");
        if(m_bigguy.materials().count() == 0)
            return -1;     // pas de matieres, pas d'affichage
        m_group_bigguy = m_bigguy.groups();

        m_robot= read_mesh("data/penguin/Penguin.obj");
        m_texture_robot= read_texture(0, "data/penguin/Penguin_Texture.png");
        if(m_robot.materials().count() == 0)
            return -1;     // pas de matieres, pas d'affichage
        m_group_robot = m_robot.groups();

        
        // un autre objet
        m_objet= Mesh(GL_TRIANGLES);
        {
            // ajouter des triplets de sommet == des triangles dans objet...
        }

        //! Robot
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

        // Dimensionne le buffer actif sur array_buffer, l'alloue et l'initialise avec les positions des sommets de l'objet
        glBufferData(GL_ARRAY_BUFFER,
                     /* length */ m_robot.vertex_buffer_size() + m_robot.normal_buffer_size() + m_robot.texcoord_buffer_size(),
                     /* data */ nullptr,
                     /* usage */ GL_STATIC_DRAW); // Utilisation par draw, sans modifications
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_robot.vertex_buffer_size(), m_robot.vertex_buffer());
        glBufferSubData(GL_ARRAY_BUFFER, m_robot.vertex_buffer_size(), m_robot.normal_buffer_size(), m_robot.normal_buffer());
        glBufferSubData(GL_ARRAY_BUFFER, m_robot.vertex_buffer_size() + m_robot.normal_buffer_size(), m_robot.texcoord_buffer_size(), m_robot.texcoord_buffer());

        // Création d'un vertex array object
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        /* Positions */
        GLint attribute = 0;
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        // Tableau de 3 float.
        glVertexAttribPointer(attribute, 3, GL_FLOAT, GL_FALSE, /* stride */ 0, /* offset */ 0);
        glEnableVertexAttribArray(attribute);

        /* Normales */
        attribute = 2;
        glVertexAttribPointer(attribute, 3, GL_FLOAT, GL_FALSE, /* stride */ 0, (void*)m_robot.vertex_buffer_size()); // in vec3 normal;
        glEnableVertexAttribArray(attribute);

        /* Texture */
        attribute = 1;
        glVertexAttribPointer(attribute, 2, GL_FLOAT, GL_FALSE, /* stride */ 0, /* offset */(void*)(m_robot.vertex_buffer_size() + m_robot.normal_buffer_size())); // in vec3 normal;
        glEnableVertexAttribArray(attribute);

        vertex_count_robot = m_robot.vertex_count();
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //! Bigguy
        glGenBuffers(1, &vertex_buffer2);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer2);

        // Dimensionne le buffer actif sur array_buffer, l'alloue et l'initialise avec les positions des sommets de l'objet
        glBufferData(GL_ARRAY_BUFFER,
                     /* length */ m_bigguy.vertex_buffer_size() + m_bigguy.normal_buffer_size() + m_bigguy.texcoord_buffer_size(),
                     /* data */ nullptr,
                     /* usage */ GL_STATIC_DRAW); // Utilisation par draw, sans modifications
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_bigguy.vertex_buffer_size(), m_bigguy.vertex_buffer());
        glBufferSubData(GL_ARRAY_BUFFER, m_bigguy.vertex_buffer_size(), m_bigguy.normal_buffer_size(), m_bigguy.normal_buffer());
        glBufferSubData(GL_ARRAY_BUFFER, m_bigguy.vertex_buffer_size() + m_bigguy.normal_buffer_size(), m_bigguy.texcoord_buffer_size(), m_bigguy.texcoord_buffer());

        // Création d'un vertex array object
        glGenVertexArrays(1, &vao2);
        glBindVertexArray(vao2);

        /* Positions */
        attribute = 0;
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer2);
        // Tableau de 3 float.
        glVertexAttribPointer(attribute, 3, GL_FLOAT, GL_FALSE, /* stride */ 0, /* offset */ 0);
        glEnableVertexAttribArray(attribute);

        /* Normales */
        attribute = 2;
        glVertexAttribPointer(attribute, 3, GL_FLOAT, GL_FALSE, 0, (void*)m_bigguy.vertex_buffer_size()); // in vec3 normal;
        glEnableVertexAttribArray(attribute);

        /* Texture */
        attribute = 1;
        glVertexAttribPointer(attribute, 2, GL_FLOAT, GL_FALSE, /* stride */ 0, /* offset */ (void*)(m_bigguy.vertex_buffer_size() + m_bigguy.normal_buffer_size())); // in vec3 normal;
        glEnableVertexAttribArray(attribute);

        vertex_count_bigguy = m_bigguy.vertex_count();
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        
        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest

  	    glEnable(GL_LIGHTING); 	// Active l'�clairage
  	    glEnable(GL_LIGHT0); 	// Allume la lumi�re n�1

        return 0;   // pas d'erreur, sinon renvoyer -1
    }
    
    // destruction des objets de l'application
    int quit( )
    {
        release_program(program_texture);
        m_objet.release();
        m_repere.release();
        glDeleteBuffers(1, &vertex_buffer);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vertex_buffer2);
        glDeleteVertexArrays(1, &vao2);
        glDeleteTextures(1, &m_texture_bigguy);
        glDeleteTextures(1, &m_texture_robot);
        return 0;   // pas d'erreur
    }
    
    // dessiner une nouvelle image
    int render( )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const Materials& materials_robot= m_robot.materials();
        const Materials& materials_bigguy= m_bigguy.materials();
        
    // dessine le repere, place au centre du monde, pour le point de vue de la camera 
        draw(m_repere, /* model */ Identity(), camera());
        
    // dessine un cube, lui aussi place au centre du monde
        m_cube.default_color(White());
        Transform t= Translation(light) * Scale(0.2, 0.2, 0.2);
        draw(m_cube, t, camera());
        
        //m_cube.default_color(Blue());
        //draw(m_cube, /* model */ t, camera());

        Transform model = Identity() * Translation(-1.5, 0, 0);
        Transform model2 = Identity() * Translation(1.5, 0, 0) * Scale(0.3, 0.3, 0.3);
        Transform mvp = camera().projection() * camera().view() * model;
        //Transform mv = camera().view() * model;
        Transform mvp2 = camera().projection() * camera().view() * model2;
        //Transform mv2 = camera().view() * model2;
        Transform mp = camera().projection() * model;
        Transform mp2 = camera().projection() * model2;

        glUseProgram(program_texture);
        GLuint location;
        location = glGetUniformLocation(program_texture, "mvpMatrix");

        //! Robot
        glBindVertexArray(vao);

        glUniformMatrix4fv(location, 1, GL_TRUE, mvp.data());

        for(unsigned i= 0; i < m_group_robot.size(); i++)
        {
            const TriangleGroup& group_= m_group_robot[i];

            // recuperer la couleur de la matiere du groupe
            //Color color= materials_robot.materials[group_.index].diffuse;
            //Color multi_color = Color(1, 0, 1, 1.0f);
           
            // parametrer le shader pour dessiner avec la couleur
            //glUseProgram(program);
            program_uniform(program_texture, "mpMatrix", mp);
            program_uniform(program_texture, "vertex_direction", vec3(light.x, light.y, -light.z));
            //program_uniform(program, "vertex_color", color);
            program_uniform(program_texture, "level_number", 8);
            program_use_texture(program_texture, "texture0", 0, m_texture_robot);
           
            // dessiner les triangles du groupe
            glDrawArrays(GL_TRIANGLES, group_.first, group_.n);
        }

        //! Bigguy
        glUseProgram(program_texture);
        GLuint location_texture;
        location_texture = glGetUniformLocation(program_texture, "mvpMatrix");

        glBindVertexArray(vao2);

        glUniformMatrix4fv(location_texture, 1, GL_TRUE, mvp2.data());

        for(unsigned i= 0; i < m_group_bigguy.size(); i++)
        {
            const TriangleGroup& group_= m_group_bigguy[i];

            // recuperer la couleur de la matiere du groupe
            //Color color= materials_bigguy.materials[group_.index].diffuse;
           
            // parametrer le shader pour dessiner avec la couleur
            //glUseProgram(program_texture);
            program_uniform(program_texture, "mpMatrix", mp2);
            program_uniform(program_texture, "vertex_direction", vec3(light.x, light.y, -light.z));
            //program_uniform(program, "vertex_color", color);
            program_use_texture(program_texture, "texture0", 0, m_texture_bigguy);
            program_uniform(program_texture, "level_number", 8);
            
            // dessiner les triangles du groupe
            glDrawArrays(GL_TRIANGLES, group_.first, group_.n);
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
        
    // comment dessiner m_objet ?? 
    
    // et sans le superposer au cube deja dessine ?
        
        // continuer, afficher une nouvelle image
        // tant que la fenetre est ouverte...
        return 1;
    }

protected:
    Mesh m_objet;
    Mesh m_cube;
    Mesh m_repere;
    Mesh m_bigguy;
    Mesh m_robot;

    vec3 light = vec3(0, 2, 0);

    GLuint program_texture;
    GLuint program_cubemap;
    GLuint vao;
    GLuint vertex_buffer;
    GLuint normal_buffer;
    GLuint texture_buffer_robot;
    GLuint vao2;
    GLuint vertex_buffer2;
    GLuint normal_buffer2;
    GLuint texture_buffer_bigguy;
    GLuint m_texture_robot;
    GLuint m_texture_bigguy;
    unsigned int vertex_count_robot;
    unsigned int vertex_count_bigguy;

    std::vector<TriangleGroup> m_group_robot;
    std::vector<TriangleGroup> m_group_bigguy;
};


int main( int argc, char **argv )
{
    // il ne reste plus qu'a creer un objet application et la lancer 
    TP tp;
    tp.run();
    
    return 0;
}
