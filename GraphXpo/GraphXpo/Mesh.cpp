#include "Mesh.h"

#include <vector>
#include <fstream>


using namespace DirectX;

//Uses the data provided to create and store this mesh's vertex and index buffers.
//Also stores a bit of misc data necessary for calling DrawIndexed().
Mesh::Mesh(Vertex* vertices, int vertexCount, UINT* indices, int indexCount, ID3D11Device* device)
{
	//vertices have been loaded, determine their tangents
	CalculateTangents(vertices, vertexCount, indices, indexCount);

	//produce the index and vertex buffers
	CreateBuffers(vertices, vertexCount, indices, indexCount, device);
}

//Open and load from a file to populate the mesh data.
Mesh::Mesh(char* filename, ID3D11Device * device)
{
	// get the file extension ////////////////////////////////////////////////////////////////

	int filenameLength = 0;
	int lastPeriodIndex = 0;
	int i = 0;

	while (filename[i] != '\0') //in c/c++, strings are null terminated, so the loop should be safe
	{
		if (filename[i] == '.')
			lastPeriodIndex = i; //determine where the file extension begins

		filenameLength++; //determine how long the string itself is
		i++;
	}

	int extensionLength = filenameLength - lastPeriodIndex;

	char* extension = new char[extensionLength];
	memcpy(extension, &filename[lastPeriodIndex+1], sizeof(char) * (extensionLength)); //memcpy includes the string's null terminating char


	//load the mesh data with the appropriate format //////////////////////////////////////////////////

	if (strcmp(extension, "obj") == 0)
	{
		LoadOBJ(filename, device);
	}

	delete[] extension;
}

//Releases the stored DirectX buffers
Mesh::~Mesh()
{
	//releasing the buffers will allow DirectX to clean them up later
	//if we don't release, we have a memory leak

	if (vertexBuffer) { vertexBuffer->Release(); }
	if (indexBuffer) { indexBuffer->Release(); }
}

//Returns a pointer to the vertex buffer object. Essential for drawing the mesh.
ID3D11Buffer* Mesh::GetVertexBuffer()
{
	return vertexBuffer;
}

//Returns a pointer to the index buffer object. Essential for drawing the mesh with indexing.
ID3D11Buffer* Mesh::GetIndexBuffer()
{
	return indexBuffer;
}

//Returns the number of indices in the index buffer object. Necessary to tell DrawIndexed() how many indices to use.
int Mesh::GetIndexCount()
{
	return numIndices;
}

///<summary>
///Reads an obj file, interprets its data into vectors, then uses those vectors to create the mesh's buffers.
///</summary>
void Mesh::LoadOBJ(char* objFile, ID3D11Device* device)
{
	// File input object
	std::ifstream obj(objFile);

	// Check for successful open
	if (!obj.is_open())
		return;

	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;     // Positions from the file
	std::vector<XMFLOAT3> normals;       // Normals from the file
	std::vector<XMFLOAT2> uvs;           // UVs from the file
	std::vector<Vertex> verts;           // Verts we're assembling
	std::vector<UINT> indices;           // Indices of these verts
	unsigned int vertCounter = 0;        // Count of vertices/indices
	char chars[100];                     // String for line reading

										 // Still have data left?
	while (obj.good())
	{
		// Get the line (100 characters should be more than enough)
		obj.getline(chars, 100);

		// Check the type of line
		if (chars[0] == 'v' && chars[1] == 'n')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 norm;
			sscanf_s(
				chars,
				"vn %f %f %f",
				&norm.x, &norm.y, &norm.z);

			// Add to the list of normals
			normals.push_back(norm);
		}
		else if (chars[0] == 'v' && chars[1] == 't')
		{
			// Read the 2 numbers directly into an XMFLOAT2
			XMFLOAT2 uv;
			sscanf_s(
				chars,
				"vt %f %f",
				&uv.x, &uv.y);

			// Add to the list of uv's
			uvs.push_back(uv);
		}
		else if (chars[0] == 'v')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 pos;
			sscanf_s(
				chars,
				"v %f %f %f",
				&pos.x, &pos.y, &pos.z);

			// Add to the positions
			positions.push_back(pos);
		}
		else if (chars[0] == 'f')
		{
			// Read the face indices into an array
			unsigned int i[12];
			int facesRead = sscanf_s(
				chars,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&i[0], &i[1], &i[2],
				&i[3], &i[4], &i[5],
				&i[6], &i[7], &i[8],
				&i[9], &i[10], &i[11]);

			// - Create the verts by looking up
			//    corresponding data from vectors
			// - OBJ File indices are 1-based, so
			//    they need to be adusted
			Vertex v1;
			v1.Position = positions[i[0] - 1];
			v1.UV = uvs[i[1] - 1];
			v1.Normal = normals[i[2] - 1];

			Vertex v2;
			v2.Position = positions[i[3] - 1];
			v2.UV = uvs[i[4] - 1];
			v2.Normal = normals[i[5] - 1];

			Vertex v3;
			v3.Position = positions[i[6] - 1];
			v3.UV = uvs[i[7] - 1];
			v3.Normal = normals[i[8] - 1];

			// The model is most likely in a right-handed space,
			// especially if it came from Maya.  We want to convert
			// to a left-handed space for DirectX.  This means we 
			// need to:
			//  - Invert the Z position
			//  - Invert the normal's Z
			//  - Flip the winding order
			// We also need to flip the UV coordinate since DirectX
			// defines (0,0) as the top left of the texture, and many
			// 3D modeling packages use the bottom left as (0,0)

			// Flip the UV's since they're probably "upside down"
			v1.UV.y = 1.0f - v1.UV.y;
			v2.UV.y = 1.0f - v2.UV.y;
			v3.UV.y = 1.0f - v3.UV.y;

			// Flip Z (LH vs. RH)
			v1.Position.z *= -1.0f;
			v2.Position.z *= -1.0f;
			v3.Position.z *= -1.0f;

			// Flip normal Z
			v1.Normal.z *= -1.0f;
			v2.Normal.z *= -1.0f;
			v3.Normal.z *= -1.0f;

			// Add the verts to the vector (flipping the winding order)
			verts.push_back(v1);
			verts.push_back(v3);
			verts.push_back(v2);

			// Add three more indices
			indices.push_back(vertCounter); vertCounter += 1;
			indices.push_back(vertCounter); vertCounter += 1;
			indices.push_back(vertCounter); vertCounter += 1;

			// Was there a 4th face?
			if (facesRead == 12)
			{
				// Make the last vertex
				Vertex v4;
				v4.Position = positions[i[9] - 1];
				v4.UV = uvs[i[10] - 1];
				v4.Normal = normals[i[11] - 1];

				// Flip the UV, Z pos and normal
				v4.UV.y = 1.0f - v4.UV.y;
				v4.Position.z *= -1.0f;
				v4.Normal.z *= -1.0f;

				// Add a whole triangle (flipping the winding order)
				verts.push_back(v1);
				verts.push_back(v4);
				verts.push_back(v3);

				// Add three more indices
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
			}
		}
	}

	// Close the file and create the actual buffers
	obj.close();

	// - "vertCounter" is BOTH the number of vertices and the number of indices
	// - Yes, the indices are a bit redundant here (one per vertex).

	//vertices have been loaded, determine their tangents
	CalculateTangents(&verts[0], vertCounter, &indices[0], vertCounter);

	CreateBuffers(&verts[0], vertCounter, &indices[0], vertCounter, device);	
}

///<summary>
///Determines that tangents of each vertex in the mesh
///Code Source: http://www.terathon.com/code/tangent.html
///</summary>
void Mesh::CalculateTangents(Vertex* vertices, int vertexCount, UINT* indices, int indexCount)
{
	// Reset tangents (just in case)
	for (int i = 0; i < vertexCount; i++)
	{
		vertices[i].Tangent = XMFLOAT3(0, 0, 0);
	}

	//Calculate tangents a single triangle at a time
	for (size_t i = 0; i < indexCount; i+=3)
	{
		//get the indices of this triangle
		UINT i1 = indices[i];
		UINT i2 = indices[i+1];
		UINT i3 = indices[i+2];

		//Retrieve the vertices composing this triangle
		Vertex* v1 = &vertices[i1];
		Vertex* v2 = &vertices[i2];
		Vertex* v3 = &vertices[i3];

		// Calculate vectors relative to triangle positions
		float x1 = v2->Position.x - v1->Position.x;
		float y1 = v2->Position.y - v1->Position.y;
		float z1 = v2->Position.z - v1->Position.z;

		float x2 = v3->Position.x - v1->Position.x;
		float y2 = v3->Position.y - v1->Position.y;
		float z2 = v3->Position.z - v1->Position.z;

		// Do the same for vectors relative to triangle uv's
		float s1 = v2->UV.x - v1->UV.x;
		float t1 = v2->UV.y - v1->UV.y;

		float s2 = v3->UV.x - v1->UV.x;
		float t2 = v3->UV.y - v1->UV.y;

		// Create vectors for tangent calculation
		float r = 1.0f / (s1 * t2 - s2 * t1);

		float tx = (t2 * x1 - t1 * x2) * r;
		float ty = (t2 * y1 - t1 * y2) * r;
		float tz = (t2 * z1 - t1 * z2) * r;

		// Adjust tangents of each vert of the triangle
		v1->Tangent.x += tx;
		v1->Tangent.y += ty;
		v1->Tangent.z += tz;

		v2->Tangent.x += tx;
		v2->Tangent.y += ty;
		v2->Tangent.z += tz;

		v3->Tangent.x += tx;
		v3->Tangent.y += ty;
		v3->Tangent.z += tz;
	}

	//orthogonality may have been lost in the above calculations
	//Use the Gram-Schmidt process to orthogonalize
	for (int i = 0; i < vertexCount; i++)
	{
		XMVECTOR normal = XMLoadFloat3(&vertices[i].Normal);
		XMVECTOR tangent = XMLoadFloat3(&vertices[i].Tangent);

		//orthogonalize the tangent
		tangent = XMVector3Normalize( tangent - normal * XMVector3Dot(normal, tangent));

		//store the results
		XMStoreFloat3(&vertices[i].Tangent, tangent);
	}
}

///<summary>
///Helper function. Processes lists of vertices and indices into vertex and index buffers.
///</summary>
void Mesh::CreateBuffers(Vertex* vertices, int vertexCount, UINT * indices, int indexCount, ID3D11Device * device)
{
	// Create the VERTEX BUFFER description -----------------------------------
	// - The description is created on the stack because we only need
	//    it to create the buffer.  The description is then useless.
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * vertexCount;	//the buffer should be wide enough to store each vertex
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells DirectX this is a vertex buffer
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// Create the proper struct to hold the initial vertex data
	// - This is how we put the initial data into the buffer
	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = vertices;

	// Actually create the buffer with the initial data
	// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	device->CreateBuffer(&vbd, &initialVertexData, &vertexBuffer);


	//Next, create the index buffer


	// Create the INDEX BUFFER description ------------------------------------
	// - The description is created on the stack because we only need
	//    it to create the buffer.  The description is then useless.
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(int) * indexCount;         // 3 = number of indices in the buffer
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER; // Tells DirectX this is an index buffer
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	// Create the proper struct to hold the initial index data
	// - This is how we put the initial data into the buffer
	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = indices;

	// Actually create the buffer with the initial data
	// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	device->CreateBuffer(&ibd, &initialIndexData, &indexBuffer);

	//Finally, store the index count for later retrieval in Draw calls
	numIndices = indexCount;
}
