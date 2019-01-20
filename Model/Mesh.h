#ifndef MESH_H
#define MESH_H
//#define string std::string
//#define vector std::vector
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glfw3.h>
#include <string>
#include <vector>
#include <iostream>
#include "../AABB.h"
#include"../Shaders/Shader.h"
#include <map>

#include "../Timer.h"
/* Some physics constants */
#define DAMPING 0.01 // how much to damp the cloth simulation each frame
#define TIME_STEPSIZE2 0.25 // how large time step each particle takes each frame
#define CONSTRAINT_ITERATIONS 15 // how many iterations of constraint satisfaction each frame (more is rigid, less is soft)




struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
	unsigned int boneID[4] = { 0, 0, 0, 0 };
	float weight[4] = { 0, 0, 0, 0 };

};

struct Face
{
	std::vector<unsigned int> vertexIDs;




};

#ifndef TEXTURES
#define TEXTURES
struct Texture
{
	unsigned int id;
	std::string type;
	std::string path;

};
#endif
class Mesh
{
public:
	unsigned int VAO;
	std::string meshname;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	AABB collider;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture>);
	void Draw(Shader shader);
	void DrawShadow(Shader shader);
	void Serialize(std::ofstream * stream);
	void FromSerialize(std::ifstream * stream);
private:
	//Render Data
	unsigned int VBO, EBO;

	void setupMesh();


};



class Particle
{
private:






public:
	Particle(glm::vec3 pos) : pos(pos), old_pos(pos), acceleration(glm::vec3(0, 0, 0)), mass(1), movable(true), accumulated_normal(glm::vec3(0, 0, 0)) {}
	Particle() {}
	unsigned int index;
	glm::vec3 pos; // the current position of the particle in 3D space
	glm::vec3 old_pos; // the position of the particle in the previous time step, used as part of the verlet numerical integration scheme
	glm::vec3 acceleration; // a vector representing the current acceleration of the particle
	glm::vec3 accumulated_normal; // an accumulated normal (i.e. non normalized), used for OpenGL soft shading
	glm::vec2 texCoord;

	bool movable; // can the particle move or not ? used to pin parts of the cloth
	float mass; // the mass of the particle (is always 1 in this example)
	void addForce(glm::vec3 f);


	/* This is one of the important methods, where the time is progressed a single step size (TIME_STEPSIZE)
	The method is called by Cloth.time_step()
	Given the equation "force = mass * acceleration" the next position is found through verlet integration*/
	void timeStep();


	glm::vec3 getPos() { return pos; }

	void resetAcceleration() { acceleration = glm::vec3(0, 0, 0); }


	void makeUnmovable() { movable = false; }

	void addToNormal(glm::vec3 normal)
	{
		accumulated_normal += glm::normalize(normal);
	}

	glm::vec3& getNormal() { return accumulated_normal; } // notice, the normal is not unit length

	void resetNormal() { accumulated_normal = glm::vec3(0, 0, 0); }

};

float vecLength(glm::vec3 f);

class Constraint
{
private:
	float rest_distance; // the length between particle p1 and p2 in rest configuration

public:
	Particle *p1, *p2; // the two particles that are connected through this constraint

	Constraint(Particle *p1, Particle *p2) : p1(p1), p2(p2)
	{
		glm::vec3 vec = p1->getPos() - p2->getPos();
		rest_distance = vecLength(vec);
	}

	/* This is one of the important methods, where a single constraint between two particles p1 and p2 is solved
	the method is called by Cloth.time_step() many times per frame*/
	void satisfyConstraint(std::vector<AABB> colliders)
	{
		glm::vec3 p1_to_p2 = p2->getPos() - p1->getPos(); // vector from p1 to p2
		float current_distance = vecLength(p1_to_p2); // current distance between p1 and p2

		glm::vec3 correctionVector;
	
		 correctionVector = p1_to_p2*(1 - rest_distance / current_distance); // The offset vector that could moves p1 into a distance of rest_distance to p2
	 // The offset vector that could moves p1 into a distance of rest_distance to p2
		glm::vec3 correctionVectorHalf = correctionVector*float(0.5); // Lets make it half that length, so that we can move BOTH p1 and p2.
		if (p1->movable)
		{
			p1->pos += (correctionVectorHalf); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.

		}


		if (p2->movable)
		{
			p2->pos -= correctionVectorHalf; // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.	
		
		}
	}

};


class Cloth
{
public:
	unsigned int VAO;
	std::string name;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<unsigned int> originalIndices;
	std::vector<Texture> textures;
	std::vector<AABB> colliders;
	int num_particles_width; // number of particles in "width" direction
	int num_particles_height; // number of particles in "height" direction
	Particle* getParticle(int x, int y) { return particles[y*num_particles_width + x]; }
	void makeConstraint(Particle *p1, Particle *p2) { constraints.push_back(Constraint(p1, p2)); }
	void verticesShared(Face a, Face b);
	Cloth(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void Draw(float runningTime, glm::vec3 model, Shader shader);
	void DrawShadow(float runningTime, glm::vec3 model, Shader shader);
	/* this is an important methods where the time is progressed one time step for the entire cloth.
	This includes calling satisfyConstraint() for every constraint, and calling timeStep() for all particles
	*/
	Timer timer;
	bool hasConstraint(Particle a, Particle b)
	{
		for (int i = 0; i < constraints.size(); i++)
		{
			if (constraints.at(i).p1->pos == a.pos && constraints.at(i).p2->pos == b.pos)
			{
				return true;
			}

			if (constraints.at(i).p1->pos == b.pos && constraints.at(i).p2->pos == a.pos)
			{
				return true;
			}
		}
		return false;
	}

	/* used to add gravity (or any other arbitrary vector) to all particles*/
	void addForce(const glm::vec3 direction)
	{

		for (int i = 0; i < particles.size(); i++)
		{
			particles.at(i)->addForce(direction); // add the forces to each particle
		}

	}

	/* used to add wind forces to all particles, is added for each triangle since the final force is proportional to the triangle area as seen from the wind direction*/
	void windForce(const glm::vec3 direction)
	{

		for (int i = 0; i < indices.size(); i += 3)
		{
			addWindForcesForTriangle(particles.at(indices.at(i)), particles.at(indices.at(i + 1)), particles.at(indices.at(i + 2)), direction);
		}


	}


private:
	//Render Data
	unsigned int VBO, EBO, posSBO;

	std::vector<Particle*> particles; // all particles that are part of this cloth
	std::vector<Constraint> constraints; // alle constraints between particles as part of this cloth


	glm::vec3 calcTriangleNormal(Particle *p1, Particle *p2, Particle *p3)
	{
		glm::vec3 pos1 = p1->getPos();
		glm::vec3 pos2 = p2->getPos();
		glm::vec3 pos3 = p3->getPos();

		glm::vec3 v1 = pos2 - pos1;
		glm::vec3 v2 = pos3 - pos1;

		return glm::cross(v1, v2);
	}					 /* A private method used by windForce() to calcualte the wind force for a single triangle
						 defined by p1,p2,p3*/
	void addWindForcesForTriangle(Particle *p1, Particle *p2, Particle *p3, const glm::vec3 direction)
	{


		glm::vec3 normal = calcTriangleNormal(p1, p2, p3);
		glm::vec3 d = glm::normalize(normal);
		glm::vec3 force = normal*(glm::dot(d, direction));
		p1->addForce(force);
		p2->addForce(force);
		p3->addForce(force);
	}

	void setupCloth();


};
#endif