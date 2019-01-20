#include "Mesh.h"





Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	//this->indices.resize(indices.size() / 4);

	setupMesh();
}

void Mesh::setupMesh()
{




	// Create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);


	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// Setting vertex attribute pointer.


	// Positions

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	// Normals

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glEnableVertexAttribArray(1);
	// Texture coords


	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	glEnableVertexAttribArray(2);
	// Tangent

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	glEnableVertexAttribArray(3);
	// Bitangent


	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
	glEnableVertexAttribArray(4);
	// Bone ID


	glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)(offsetof(Vertex, boneID)));
	glEnableVertexAttribArray(5);

	// Bone Weight

	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, weight)));
	glEnableVertexAttribArray(6);




	glBindVertexArray(0);

}

void Mesh::Draw(Shader shader)
{

	

	//Loop through textures, and bind them where appropriate.
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);

		std::string number;
		std::string name = "Texture_diffuse";
									
		glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
		// Then bind the texture.
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}

	//Bind VAO and draw
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cout << "GL ERROR! " << err << std::endl;
	}
	// Set back to default
	glActiveTexture(GL_TEXTURE0);

}

void Mesh::DrawShadow(Shader shader)
{
	//Bind VAO and draw
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Mesh::Serialize(std::ofstream * stream)
{
	stream->write((char*)vertices.size(), sizeof(unsigned int));
	stream->write((char*)indices.size(), sizeof(unsigned int));
	stream->write((char*)&vertices[0], sizeof(Vertex) * vertices.size());
	stream->write((char*)&indices[0], sizeof(Vertex) * indices.size());
}

void Mesh::FromSerialize(std::ifstream * stream)
{
	unsigned int vsize, isize;
	stream->read((char*)&vsize, sizeof(unsigned int));
	stream->read((char*)&isize, sizeof(unsigned int));

	vertices.resize(vsize);
	indices.resize(isize);
	stream->read((char*)&vertices[0], sizeof(Vertex) * vertices.size());
	stream->read((char*)&indices[0], sizeof(Vertex) * indices.size());
}

void Particle::addForce(glm::vec3 f)
{
	{
		acceleration += f / mass;
	}
}

void Particle::timeStep()
{
	{
		if (movable)
		{
			glm::vec3 temp = pos;
			pos = pos + (pos - old_pos)*(float)((1.0 - DAMPING)) + acceleration*(float)(TIME_STEPSIZE2);
			old_pos = temp;
			acceleration = glm::vec3(0, 0, 0); // acceleration is reset since it HAS been translated into a change in position (and implicitely into velocity)	
		}
	}
}

void Cloth::verticesShared(Face a, Face b)
{
	// Container for any indices the two faces have in common. 
	std::vector<glm::vec2> traversed;

	// Loop through both face's indices, to see if they match eachother. 
	for (int i = 0; i < a.vertexIDs.size(); i++)
	{
		for (int k = 0; k < b.vertexIDs.size(); k++)
		{

			// If we do get a match, we push a vector into the container containing the two indices of the faces so we know which ones are equal.
			if (a.vertexIDs[i] == b.vertexIDs[k])
			{
				traversed.push_back(glm::vec2(i, k));
			}
		}
		// If we're here, if means we have an edge in common, aka that we have two vertices shared between the two faces.
		if (traversed.size() == 2)
		{
			// Get the adjacent vertices.
			unsigned int face_a_adj_ind = abs(((traversed[0].x) + (traversed[1].x)) - 3) ;
			unsigned int face_b_adj_ind =  abs(((traversed[0].y) + (traversed[1].y)) - 3);
			// Turn the stored ones from earlier and just get the ACTUAL indices from the face. Indices of indices, eh. 
			unsigned int adj_1 = a.vertexIDs[face_a_adj_ind];
			unsigned int adj_2 = b.vertexIDs[face_b_adj_ind];
			// And finally, make a bending spring between the two adjacent particles. 
			makeConstraint(particles.at(adj_1), particles.at(adj_2));
		}
	}
}



Cloth::Cloth(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
	this->textures = textures;
	this->indices = indices;
	this->vertices = vertices;
	particles.resize(vertices.size());
	// Turn vertices into particles.



	std::vector<Particle *> highestParticles;
	if (indices.size() % 3 != 0)
	{
		std::cout << "WHY THE FUCK IS THIS NOT IN THIRDS???" << std::endl;
	}
	timer.start();
	unsigned int id = 0;
	
	for (int j = 0; j < vertices.size(); j++)
	{
		Particle * particle = new Particle(vertices.at(j).Position);
		particle->index = j;


		particles.at(j) = (particle);

		if (vertices[j].boneID[0] != 0)
		{
			
			particles[j]->makeUnmovable();
			id = vertices[j].boneID[0];
		}


	}
	for (int i = 0; i < this->vertices.size(); i++)
	{
		this->vertices[i].boneID[0] = id;
		this->vertices[i].weight[0] = 1.0;
	}

	for (int i = 0; i < highestParticles.size(); i++)
	{

	}
	// Make something unmoveable so that things don't go flying off.
	//particles.at(0)->makeUnmovable();

	// Container to temporarily hold faces while we process springs
	std::vector<Face> faces;

	for (int i = 0; i < this->indices.size(); i++)
	{
		Particle * part = particles[this->indices[i]];
		for (int j = i+1; j < this->indices.size(); j++)
		{
			if(this->indices[j] != this->indices[i])
			if (particles[this->indices[j]]->pos == part->pos)
			{
					this->indices[j] = this->indices[i];
			}
		}

	}

	// Go through indices and take the ones making a triangle.
	// Indices come from assimp, so i think this is the right thing to do to get each face?
	for (int i = 0; i < this->indices.size(); i+=3)
	{
			std::vector<unsigned int> faceIds = { this->indices.at(i), this->indices.at(i + 1), this->indices.at(i + 2)};
			Face face;
			face.vertexIDs = faceIds;
			faces.push_back(face);
	}
	
	// Iterate through faces and add constraints when needed.
	for (int l = 0; l < faces.size(); l++)
	{

		// Adding edge springs.
		Face temp = faces[l];
		makeConstraint(particles.at(temp.vertexIDs[0]), particles.at(temp.vertexIDs[1]));
		makeConstraint(particles.at(temp.vertexIDs[0]), particles.at(temp.vertexIDs[2]));
		makeConstraint(particles.at(temp.vertexIDs[1]), particles.at(temp.vertexIDs[2]));


		// We need to get the bending springs as well, and i've just written a function to do that.
		for (int x = 0; x < faces.size(); x++)
		{
			Face temp2 = faces[x];
			if (l != x)
			{
				verticesShared(temp, temp2);
			}

		}
	}

	// Set up buffers 
	setupCloth();
}

void Cloth::Draw(float runningTime, glm::vec3 model, Shader shader)
{
	GLboolean x;
	glGetBooleanv(GL_CULL_FACE, &x);

    glDisable(GL_CULL_FACE);
	if (timer.getTicks() > 20)
	{
		std::vector<Constraint>::iterator constraint;
		for (int i = 0; i < 1; i++) // iterate over all constraints several times
		{
			for (constraint = constraints.begin(); constraint != constraints.end(); constraint++)
			{
				(*constraint).satisfyConstraint(colliders); // satisfy constraint.
			}
		}



		for (int k = 0; k < particles.size(); k++)
		{
			particles.at(k)->timeStep();
			vertices.at(k).Position = particles.at(k)->getPos();
		}
		timer.start();
	}
	
	//Loop through textures, and bind them where appropriate.
	unsigned int diffuseNr = 1;		
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);

		std::string number;
		std::string name = textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++); // Unsigned int -> string
		else if (name == "texture_normal")
			number = std::to_string(normalNr++); // Unsigned int -> string
		else if (name == "texture_height")
			number = std::to_string(heightNr++); // Unsigned int -> string

												 // Set the sampler to the correct texture unit
		glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
		// Then bind the texture.
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}



	//addForce(glm::vec3(0, -0.2, 0)*glm::vec3(TIME_STEPSIZE2)); // add gravity each frame, pointing down
	//windForce(glm::vec3(0.0, -0.2, -0.2)*glm::vec3(TIME_STEPSIZE2)); // generate some wind each frame







	//Bind VAO and draw
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), &vertices[0]);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);


	// Set back to default
	glActiveTexture(GL_TEXTURE0);

	//Error checking, just to make sure nothing goes wrong.
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		std::cout << "GL ERROR! " << err << std::endl;
	}
	// Set back to default
	if (x)
	{
		glEnable(GL_CULL_FACE);
	}
	
	

}

void Cloth::DrawShadow(float runningTime, glm::vec3 model, Shader shader)
{




	//Bind VAO and draw
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), &vertices[0]);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

}

void Cloth::setupCloth()
{



	// Create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);


	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);

	// Setting vertex attribute pointer.


	// Positions

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	// Normals

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glEnableVertexAttribArray(1);
	// Texture coords


	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	glEnableVertexAttribArray(2);
	// Tangent

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	glEnableVertexAttribArray(3);
	// Bitangent


	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
	glEnableVertexAttribArray(4);
	// Bone ID


	glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)(offsetof(Vertex, boneID)));
	glEnableVertexAttribArray(5);

	// Bone Weight

	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, weight)));
	glEnableVertexAttribArray(6);




	glBindVertexArray(0);




	// Set up SBO
	glGenBuffers(1, &posSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSBO);

}

float vecLength(glm::vec3 f)
{
	{
		return sqrt(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
	}
}
