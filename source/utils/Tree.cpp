#pragma once
#include "Tree.h"
#include "Log.h"
#include "Ray.h"
#include "../GameObject.h"
#include "../Engine.h"
#include "../Render.h"

TreeNode::TreeNode(const AABB& bounds, int depth, int numChildren)
    : limits(bounds), depth(depth), isLeaf(true)
{
    children.resize(numChildren, nullptr);
}

TreeNode::~TreeNode()
{
    Clear();
}

void TreeNode::Clear()
{
    for (int i = 0; i < children.size(); i++)
    {
        if (children[i])
        {
            children[i]->Clear();
            delete children[i];
        }
    }
    children.clear();
    gameObjects.clear();
    isLeaf = true;
}

bool TreeNode::IsEmpty() const
{
    return gameObjects.empty() && isLeaf;
}

Tree::Tree(TreeType type, int maxDepth, int maxObjectsPerNode) : type(type), maxDepth(maxDepth), maxObjectsPerNode(maxObjectsPerNode)
{
    childrenPerNode = static_cast<int>(type);
    AABB mapLimits;
    mapLimits.min = glm::vec3(-100.0f);
    mapLimits.max = glm::vec3(100.0f);
    rootNode = new TreeNode(mapLimits,0, childrenPerNode);

    const char* typeName = (type == TreeType::Quadtree) ? "Quadtree" : "Octree";
    LOG("%s created with %d children per node", typeName, childrenPerNode);
}

Tree::~Tree()
{

}

void Tree::Build(std::vector<GameObject*> gameObjects, AABB worldLimits)
{
    LOG("Building spatial tree with %d objects", gameObjects.size());

    Clear();

    rootNode->limits = worldLimits;

    for (GameObject* obj : gameObjects)
    {
        AABB globalAABB;
        if (!obj || !obj->TryGetGlobalAABB(globalAABB)) continue;

        Insert(rootNode, obj, globalAABB);
    }

    LOG("Spatial tree built with %d nodes", GetNodeCount());
}

void Tree::Clear()
{
    rootNode->Clear();
}

void Tree::Insert(TreeNode* node, GameObject* object, const AABB& objectAABB)
{
    if (!AABBContains(node->limits, objectAABB))
    {
        return;
    }

    if (node->isLeaf)
    {
        node->gameObjects.push_back(object);

        if (node->gameObjects.size() > maxObjectsPerNode && node->depth < maxDepth)
        {
            Subdivide(node);
        }
    }
    else
    {
        int childIndex = GetChildIndex(node->limits, objectAABB);

        if (childIndex >= 0 && childIndex < childrenPerNode && node->children[childIndex])
        {
            AABB childBounds = node->children[childIndex]->limits;

            if (AABBContains(childBounds, objectAABB))
            {
                Insert(node->children[childIndex], object, objectAABB);
            }
            else
            {
                node->gameObjects.push_back(object);
            }
        }
        else
        {
            node->gameObjects.push_back(object);
        }
    }
}

void Tree::Subdivide(TreeNode* node)
{
    if (!node->isLeaf) return;

    node->children.clear();

    for (int i = 0; i < childrenPerNode; i++)
    {
        AABB childBounds = GetChildBounds(node->limits, i);
        TreeNode* newNode = new TreeNode(childBounds, node->depth + 1, childrenPerNode);
        node->children.push_back(newNode);
    }

    node->isLeaf = false;

    std::vector<GameObject*> objectsToRedistribute = node->gameObjects;

    for (GameObject* obj : objectsToRedistribute)
    {
        AABB objAABB;
        if (obj->TryGetGlobalAABB(objAABB))
        {
            int childIndex = GetChildIndex(node->limits, objAABB);

            if (childIndex >= 0 && childIndex < childrenPerNode)
            {
                AABB childBounds = node->children[childIndex]->limits;

                if (AABBContains(childBounds, objAABB))
                {
                    Insert(node->children[childIndex], obj, objAABB);
                }
                else
                {
                    node->gameObjects.push_back(obj);
                }
            }
            else
            {
                node->gameObjects.push_back(obj);
            }
        }
    }
}

int Tree::GetChildIndex(const AABB& nodeBounds, const AABB& objectBounds)
{
    glm::vec3 center = (nodeBounds.min + nodeBounds.max) * 0.5f;
    glm::vec3 objCenter = (objectBounds.min + objectBounds.max) * 0.5f;

    int index = 0;

    if (type == TreeType::Octree)
    {
        if (objCenter.x > center.x) index |= 1;
        if (objCenter.y > center.y) index |= 2;
        if (objCenter.z > center.z) index |= 4;
    }
    else if (type == TreeType::Quadtree)
    {
        if (objCenter.x > center.x) index |= 1;
        if (objCenter.z > center.z) index |= 2;
    }

    return index;
}

AABB Tree::GetChildBounds(const AABB& parentBounds, int childIndex)
{
    glm::vec3 center = (parentBounds.min + parentBounds.max) * 0.5f;
    AABB bounds;

    if (type == TreeType::Octree)
    {
        bounds.min.x = (childIndex & 1) ? center.x : parentBounds.min.x;
        bounds.max.x = (childIndex & 1) ? parentBounds.max.x : center.x;

        bounds.min.y = (childIndex & 2) ? center.y : parentBounds.min.y;
        bounds.max.y = (childIndex & 2) ? parentBounds.max.y : center.y;

        bounds.min.z = (childIndex & 4) ? center.z : parentBounds.min.z;
        bounds.max.z = (childIndex & 4) ? parentBounds.max.z : center.z;
    }
    else if (type == TreeType::Quadtree)
    {
        bounds.min.x = (childIndex & 1) ? center.x : parentBounds.min.x;
        bounds.max.x = (childIndex & 1) ? parentBounds.max.x : center.x;

        bounds.min.y = parentBounds.min.y;
        bounds.max.y = parentBounds.max.y;

        bounds.min.z = (childIndex & 2) ? center.z : parentBounds.min.z;
        bounds.max.z = (childIndex & 2) ? parentBounds.max.z : center.z;
    }

    return bounds;
}

bool Tree::AABBContains(const AABB& container, const AABB& contained)
{
    return (contained.min.x >= container.min.x && contained.max.x <= container.max.x &&
        contained.min.y >= container.min.y && contained.max.y <= container.max.y &&
        contained.min.z >= container.min.z && contained.max.z <= container.max.z);
}

void Tree::GetAllNodes(std::vector<AABB>& outNodes) const
{
    std::vector<TreeNode*> stack;
    stack.push_back(rootNode);

    while (!stack.empty())
    {
        TreeNode* node = stack.back();
        stack.pop_back();

        outNodes.push_back(node->limits);

        if (!node->isLeaf)
        {
            for (TreeNode* child : node->children)
            {
                if (child)
                    stack.push_back(child);
            }
        }
    }
}

int Tree::GetNodeCount() const
{
    int count = 0;
    std::vector<TreeNode*> stack;
    stack.push_back(rootNode);

    while (!stack.empty())
    {
        TreeNode* node = stack.back();
        stack.pop_back();
        count++;

        if (!node->isLeaf)
        {
            for (TreeNode* child : node->children)
            {
                if (child)
                    stack.push_back(child);
            }
        }
    }

    return count;
}

void Tree::QueryRay(Ray ray, std::vector<GameObject*>& results)
{
    results.clear();
    QueryRay(rootNode, ray, results);
}

void Tree::QueryRay(TreeNode* node, Ray ray, std::vector<GameObject*>& results)
{
    if (!node) return;

    // Test ray-AABB intersection
    float t;
    if (!ray.RayIntersectsAABB(node->limits, t))
        return;

    // Agregar objetos de este nodo (evitando duplicados)
    for (GameObject* obj : node->gameObjects)
    {
        if (std::find(results.begin(), results.end(), obj) == results.end())
        {
            results.push_back(obj);
        }
    }

    // Si no es hoja, consultar hijos recursivamente
    if (!node->isLeaf)
    {
        for (TreeNode* child : node->children)
        {
            if (child)
            {
                QueryRay(child, ray, results);
            }
        }
    }
}

void Tree::DrawDebug(glm::vec4 _color)
{
    std::vector<AABB> allNodesAABB;
    GetAllNodes(allNodesAABB);
    glm::vec4 color = _color;


    Render* render = Engine::GetInstance().render;

    for (const AABB& box : allNodesAABB)
    {
        // 1. Calcular las 8 esquinas usando min y max
        glm::vec3 min = box.min;
        glm::vec3 max = box.max;

        glm::vec3 v0 = min;                                   // Esquina inferior izquierda atrás
        glm::vec3 v1 = glm::vec3(max.x, min.y, min.z);        // Esquina inferior derecha atrás
        glm::vec3 v2 = glm::vec3(max.x, max.y, min.z);        // Esquina superior derecha atrás
        glm::vec3 v3 = glm::vec3(min.x, max.y, min.z);        // Esquina superior izquierda atrás

        glm::vec3 v4 = glm::vec3(min.x, min.y, max.z);        // Esquina inferior izquierda frente
        glm::vec3 v5 = glm::vec3(max.x, min.y, max.z);        // Esquina inferior derecha frente
        glm::vec3 v6 = max;                                   // Esquina superior derecha frente
        glm::vec3 v7 = glm::vec3(min.x, max.y, max.z);        // Esquina superior izquierda frente


        // Cara Trasera (Z min)
        render->DrawLine(v0, v1, color);
        render->DrawLine(v1, v2, color);
        render->DrawLine(v2, v3, color);
        render->DrawLine(v3, v0, color);

        // Cara Frontal (Z max)
        render->DrawLine(v4, v5, color);
        render->DrawLine(v5, v6, color);
        render->DrawLine(v6, v7, color);
        render->DrawLine(v7, v4, color);

        // Conexiones (Profundidad)
        render->DrawLine(v0, v4, color);
        render->DrawLine(v1, v5, color);
        render->DrawLine(v2, v6, color);
        render->DrawLine(v3, v7, color);
    }
}