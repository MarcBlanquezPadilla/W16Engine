#pragma once
#include "AABB.h"
#include <vector>

class GameObject;
struct Ray;

enum class TreeType
{
    Quadtree = 4,
    Octree = 8
};

class TreeNode
{
public:
    AABB limits;
    std::vector<GameObject*> gameObjects;
    std::vector<TreeNode*> children;
    int depth;
    bool isLeaf;

    TreeNode(const AABB& bounds, int depth, int numChildren);
    ~TreeNode();

    void Clear();
    bool IsEmpty() const;
    int GetChildCount() const { return children.size(); }
};

class Tree
{
public:

    Tree(TreeType type, int maxDepth = 6, int maxObjectsPerNode = 8);
    ~Tree();

    void Build(const std::vector<GameObject*> objects, AABB worldLimits);
    void Clear();

    void QueryRay(Ray ray, std::vector<GameObject*>& results);
    void GetAllNodes(std::vector<AABB>& outNodes) const;
    void DrawDebug(glm::vec4 color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    int GetNodeCount() const;

    TreeNode* GetRoot() { return rootNode; }
    TreeType GetType() const { return type; }
    int GetChildrenPerNode() const { return childrenPerNode; }

private:
    void Insert(TreeNode* node, GameObject* object, const AABB& objectAABB);
    void Subdivide(TreeNode* node);
    void QueryRay(TreeNode* node, Ray ray, std::vector<GameObject*>& results);

    int GetChildIndex(const AABB& nodeBounds, const AABB& objectBounds);
    AABB GetChildBounds(const AABB& parentBounds, int childIndex);
    bool AABBContains(const AABB& container, const AABB& contained);

private:

    TreeNode* rootNode;
    TreeType type;
    int childrenPerNode;
    int maxDepth;
    int maxObjectsPerNode;
};

