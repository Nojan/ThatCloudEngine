#include "hedge.hpp"
#include <cassert>
#include <cstring>

size_t EPAComputedAllocatedSize(const EPAHalfEdgeMesh& mesh)
{
    return mesh.facesCapacity * ( sizeof(EPAHalfEdgeFace) + 3 * sizeof(EPAHalfEdge));
}

void EPAInitialize(EPAHalfEdgeMesh& mesh, void* ptr)
{
    mesh.faces = reinterpret_cast<EPAHalfEdgeFace*>(ptr);
    mesh.halfEdges = reinterpret_cast<EPAHalfEdge*>((char*)ptr + (sizeof(EPAHalfEdgeFace) * mesh.facesCapacity));
    EPAFillMemory(mesh);
}

void EPAReset(EPAHalfEdgeMesh& mesh)
{
    mesh.facesCount = 0;
    mesh.halfEdgesCount = 0;
    EPAFillMemory(mesh);
}

void EPACleanUp(EPAHalfEdgeMesh& mesh)
{
    for (uint8_t faceIdx = mesh.facesCount - 1; uint8_t(-1) != faceIdx; --faceIdx)
    {
        if (uint8_t(-1) == mesh.faces[faceIdx].halfEdge)
        {
            const uint8_t swappedIdx = mesh.facesCount - 1;
            for (uint8_t halfEdgeIdx = mesh.halfEdgesCount - 1; uint8_t(-1) != halfEdgeIdx; --halfEdgeIdx)
            {
                EPAHalfEdge& halfEddge = mesh.halfEdges[halfEdgeIdx];
                assert(halfEddge.face != faceIdx);
                if (halfEddge.face == swappedIdx)
                {
                    halfEddge.face = faceIdx;
                }
            }
            mesh.faces[faceIdx] = mesh.faces[swappedIdx];
            mesh.faces[swappedIdx].halfEdge = uint8_t(-1);
            mesh.facesCount -= 1;
        }
    }

    for (uint8_t halfEdgeIdx = mesh.halfEdgesCount - 1; uint8_t(-1) != halfEdgeIdx; --halfEdgeIdx)
    {
        EPAHalfEdge& halfEddge = mesh.halfEdges[halfEdgeIdx];
        bool removeIt = false;
        if (uint8_t(-1) == halfEddge.face)
        {
            if (uint8_t(-1) != halfEddge.opposite)
            {
                EPAHalfEdge& oppositeEdge = mesh.halfEdges[halfEddge.opposite];
                assert(oppositeEdge.opposite == halfEdgeIdx);
                if(uint8_t(-1) == oppositeEdge.face)
                {
                    oppositeEdge.opposite = -1;
                    removeIt = true;
                }
            }
            else
            {
                removeIt = true;
            }
        }
        if (removeIt)
        {
            const uint8_t swappedIdx = mesh.halfEdgesCount - 1;
            for (uint8_t halfEdgeIdy = halfEdgeIdx - 1; uint8_t(-1) != halfEdgeIdy; --halfEdgeIdy)
            {
                EPAHalfEdge& edge = mesh.halfEdges[halfEdgeIdy];
                if (edge.next == halfEdgeIdx)
                {
                    edge.next = swappedIdx;
                }
                if (edge.previous == halfEdgeIdx)
                {
                    edge.previous = swappedIdx;
                }
                if (edge.opposite == halfEdgeIdx)
                {
                    edge.opposite = swappedIdx;
                }
            }
            halfEddge = mesh.halfEdges[swappedIdx];
            mesh.halfEdges[swappedIdx].face = -1;
            mesh.halfEdgesCount -= 1;
        }
    }

    EPAFillMemory(mesh);
}

void EPAFillMemory(EPAHalfEdgeMesh& mesh)
{
    memset(mesh.faces + mesh.facesCount, 0xFF, (mesh.facesCapacity - mesh.facesCount) * sizeof(EPAHalfEdgeFace) );
    memset(mesh.halfEdges + mesh.halfEdgesCount, 0xFF, ((mesh.facesCapacity * 3) - mesh.halfEdgesCount) * sizeof(EPAHalfEdge));
}

uint8_t EPAFindEdge(const EPAHalfEdgeMesh& mesh, uint8_t v0, uint8_t v1)
{
    uint8_t result = -1;
    for (uint8_t idx = 0; idx < mesh.facesCount * 3; ++idx)
    {
        const EPAHalfEdge& halfEdge = mesh.halfEdges[idx];
        if (halfEdge.vertex == v0)
        {
            if (halfEdge.next != -1)
            {
                if (mesh.halfEdges[halfEdge.next].vertex == v1)
                {
                    result = idx;
                    break;
                }
            }
        }
    }
    return result;
}

bool EPAPushFace(EPAHalfEdgeMesh& mesh, uint8_t v0, uint8_t v1, uint8_t v2)
{ 
    const uint8_t faceIdx = mesh.facesCount;

    if(mesh.facesCapacity <= faceIdx)
        return false;

    EPAHalfEdgeFace& face = mesh.faces[faceIdx];

    uint8_t addedEdges = 0;

    uint8_t v0v1IdxFound = EPAFindEdge(mesh, v0, v1);
    if (uint8_t(-1) == v0v1IdxFound)
    {
        v0v1IdxFound = mesh.halfEdgesCount + addedEdges;
        addedEdges += 1;
    }

    uint8_t v1v2IdxFound = EPAFindEdge(mesh, v1, v2);
    if (uint8_t(-1) == v1v2IdxFound)
    {
        v1v2IdxFound = mesh.halfEdgesCount + addedEdges;
        addedEdges += 1;
    }

    uint8_t v2v0IdxFound = EPAFindEdge(mesh, v2, v0);
    if (uint8_t(-1) == v2v0IdxFound)
    {
        v2v0IdxFound = mesh.halfEdgesCount + addedEdges;
        addedEdges += 1;
    }

    const uint8_t v0v1Idx = v0v1IdxFound;
    const uint8_t v1v2Idx = v1v2IdxFound;
    const uint8_t v2v0Idx = v2v0IdxFound;

    face.halfEdge = v0v1Idx;

    EPAHalfEdge& v0v1 = mesh.halfEdges[v0v1Idx];
    EPAHalfEdge& v1v2 = mesh.halfEdges[v1v2Idx];
    EPAHalfEdge& v2v0 = mesh.halfEdges[v2v0Idx];

    assert(uint8_t(-1) == v0v1.face);
    v0v1.face = faceIdx;
    assert(uint8_t(-1) == v1v2.face);
    v1v2.face = faceIdx;
    assert(uint8_t(-1) == v2v0.face);
    v2v0.face = faceIdx;

    v0v1.vertex = v0;
    v1v2.vertex = v1;
    v2v0.vertex = v2;

    v0v1.next = v1v2Idx;
    v1v2.next = v2v0Idx;
    v2v0.next = v0v1Idx;

    v0v1.previous = v2v0Idx;
    v1v2.previous = v0v1Idx;
    v2v0.previous = v1v2Idx;

    v0v1.opposite = EPAFindEdge(mesh, v1, v0);
    if (uint8_t(-1) != v0v1.opposite)
    {
        mesh.halfEdges[v0v1.opposite].opposite = v0v1Idx;
    }
    v1v2.opposite = EPAFindEdge(mesh, v2, v1);
    if (uint8_t(-1) != v1v2.opposite)
    {
        mesh.halfEdges[v1v2.opposite].opposite = v1v2Idx;
    }
    v2v0.opposite = EPAFindEdge(mesh, v0, v2);
    if (uint8_t(-1) != v2v0.opposite)
    {
        mesh.halfEdges[v2v0.opposite].opposite = v2v0Idx;
    }

    mesh.halfEdgesCount += addedEdges;
    mesh.facesCount += 1;
    return true;
}

bool EPAIsFaceEquivalent(const EPAHalfEdgeMesh& mesh, const EPAHalfEdgeFace& face, uint8_t v0, uint8_t v1, uint8_t v2)
{
    const EPAHalfEdge& edge0 = mesh.halfEdges[face.halfEdge];
    const EPAHalfEdge& edge1 = mesh.halfEdges[edge0.next];
    const EPAHalfEdge& edge2 = mesh.halfEdges[edge1.next];
    if(edge2.next != face.halfEdge)
        return false;
    
    if (v0 == edge0.vertex || v0 == edge1.vertex || v0 == edge2.vertex)
    {
        if (v1 == edge0.vertex || v1 == edge1.vertex || v1 == edge2.vertex)
        {
            if (v2 == edge0.vertex || v2 == edge1.vertex || v2 == edge2.vertex)
            {
                return true;
            }
        }
    }
    return false;
}

uint8_t EPAFindFace(const EPAHalfEdgeMesh& mesh, uint8_t v0, uint8_t v1, uint8_t v2)
{
    uint8_t result(-1);
    for (uint8_t faceIdx = 0; faceIdx < mesh.facesCount; ++faceIdx)
    {
        const EPAHalfEdgeFace& face = mesh.faces[faceIdx];
        if (uint8_t(-1) == face.halfEdge)
            continue;
        if (EPAIsFaceEquivalent(mesh, face, v0, v1, v2))
        {
            result = faceIdx;
            break;
        }
    }
    return result;
}

bool EPAVerifyFaces(const EPAHalfEdgeMesh& mesh)
{
    for (uint8_t faceIdx = 0; faceIdx < mesh.facesCount; ++faceIdx)
    {
        const EPAHalfEdgeFace& face = mesh.faces[faceIdx];
        if(uint8_t(-1) == face.halfEdge)
            continue;
        if (mesh.halfEdgesCount <= face.halfEdge)
            return false;

        uint8_t edgeIdx = face.halfEdge;
        uint8_t cycleCount = 0;
        do
        {
            const EPAHalfEdge& edge = mesh.halfEdges[edgeIdx];
            if(faceIdx != edge.face)
                return false;

            if (uint8_t(-1) != edge.opposite)
            {
                if (mesh.halfEdges[edge.opposite].opposite != edgeIdx)
                {
                    return false;
                }
            }
            
            if (mesh.halfEdges[edge.previous].next != edgeIdx)
                return false;

            if (mesh.halfEdges[edge.next].previous != edgeIdx)
                return false;

            edgeIdx = edge.next;
            ++cycleCount;
            if(mesh.halfEdgesCount <= cycleCount)
                return false;
        }
        while(edgeIdx != face.halfEdge);
    }
    return true;
}
