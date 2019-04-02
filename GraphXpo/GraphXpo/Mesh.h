//Creates and serves as a container for DirectX buffer data

#include <d3d11.h>
#include "Vertex.h"

#pragma once
class Mesh
{
public:
	//Uses the data provided to create and store this mesh's vertex and index buffers
	Mesh(Vertex* vertices, int vertexCount, UINT* indices, int indexCount, ID3D11Device* device);

	//Open and load from a file to populate the mesh data.
	Mesh(char* filename, ID3D11Device* device);

	//Releases the stored DirectX buffers
	~Mesh();

	//accessors to the data necessary to draw to the screen
	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();
	int GetIndexCount();

private:

	///<summary>
	///Reads an obj file, interprets its data into vectors, then uses those vectors to create the mesh's buffers.
	///</summary>
	void LoadOBJ(char* objFile, ID3D11Device* device);

	///<summary>
	///Determines that tangents of each vertex in the mesh
	///Code Source: http://www.terathon.com/code/tangent.html
	///</summary>
	void CalculateTangents(Vertex* vertices, int vertexCount, UINT* indices, int indexCount);

	///<summary>
	///Helper function. Processes lists of vertices and indices into vertex and index buffers.
	///</summary>
	void CreateBuffers(Vertex* vertices, int vertexCount, UINT* indices, int indexCount, ID3D11Device* device);

	ID3D11Buffer* vertexBuffer;	//space in memory holding all vertex data (position, color, etc.) for this mesh
	ID3D11Buffer* indexBuffer;		//space in memory holding the index data (how the vertices should be combined) for this mesh

	int numIndices; //DrawIndexed() needs to know how many indices to use from the given index buffer, 
					//so we need to keep track of the max possible indices to use
};

