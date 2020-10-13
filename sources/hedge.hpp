#pragma once

#include <cstdint>

// Half edge for triangulare face

struct EPAHalfEdge {
    uint8_t next = -1;
    uint8_t previous = -1;
    uint8_t opposite = -1;
    uint8_t face = -1;
    uint8_t vertex = -1;
};

struct EPAHalfEdgeFace {
    uint8_t halfEdge = -1;
};

struct EPAHalfEdgeMesh {
    EPAHalfEdgeFace* faces = nullptr;
    EPAHalfEdge* halfEdges = nullptr;
    uint8_t facesCount = 0;
    uint8_t facesCapacity = 0;
    uint8_t halfEdgesCount = 0;
};

size_t EPAComputedAllocatedSize(const EPAHalfEdgeMesh& mesh);
void EPAInitialize(EPAHalfEdgeMesh& mesh, void* ptr);
void EPAReset(EPAHalfEdgeMesh& mesh);
void EPACleanUp(EPAHalfEdgeMesh& mesh);
void EPAFillMemory(EPAHalfEdgeMesh& mesh);

uint8_t EPAFindEdge(const EPAHalfEdgeMesh& mesh, uint8_t v0, uint8_t v1);
bool EPAPushFace(EPAHalfEdgeMesh& mesh, uint8_t v0, uint8_t v1, uint8_t v2);
bool EPAIsFaceEquivalent(const EPAHalfEdgeMesh& mesh, const EPAHalfEdgeFace& face, uint8_t v0, uint8_t v1, uint8_t v2);
uint8_t EPAFindFace(const EPAHalfEdgeMesh& mesh, uint8_t v0, uint8_t v1, uint8_t v2);

bool EPAVerifyFaces(const EPAHalfEdgeMesh& mesh);