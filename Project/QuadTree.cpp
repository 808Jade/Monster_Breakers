#include "stdafx.h"
#include "QuadTree.h"
#include "Object.h"

void CQuadTree::Build(const BoundingBox& worldBounds, int maxObjects, int maxDepth)
{
    maxObjectsPerNode = maxObjects > 0 ? maxObjects : 10;
    this->maxDepth = maxDepth > 0 ? maxDepth : 4; // 기본값 4
    if (root) DeleteNode(root);
    root = new QuadTreeNode(worldBounds);
    root->depth = 0;
    PreBuild(root, 0);
}

void CQuadTree::Insert(CGameObject* object)
{
    if (!root) return;
    InsertObject(root, object);
}


void CQuadTree::Insert(const ColliderInfo& collider)
{
    if (!root) return;
    InsertCollider(root, collider);
}

void CQuadTree::PrintTree()
{
    if (!root)
    {
        std::cout << "QuadTree is empty.\n";
        return;
    }
    std::cout << "=== QuadTree Structure ===\n";
    PrintNode(root, 0);
    std::cout << "==========================\n";
}

QuadTreeNode* CQuadTree::FindNode(QuadTreeNode* node, const BoundingBox& aabb)
{
    if (!node || !node->bounds.Intersects(aabb))
        return nullptr;

    if (node->isLeaf || !node->children[0])
        return node;

    for (int i = 0; i < 4; i++)
    {
        if (node->children[i]) {
            QuadTreeNode* result = FindNode(node->children[i], aabb);
            if (result)
                return result;
        }
    }
    return node;
}

void CQuadTree::PreBuild(QuadTreeNode* node, int depth)
{
    if (!node || depth >= maxDepth)
    {
        node->isLeaf = true;
        return;
    }

    XMFLOAT3 center = node->bounds.Center;
    XMFLOAT3 extents = node->bounds.Extents;
    float halfX = extents.x * 0.5f;
    float halfZ = extents.z * 0.5f;

    BoundingBox childBounds[4];
    childBounds[0] = BoundingBox(XMFLOAT3(center.x + halfX, center.y, center.z + halfZ), XMFLOAT3(halfX, extents.y, halfZ));
    childBounds[1] = BoundingBox(XMFLOAT3(center.x - halfX, center.y, center.z + halfZ), XMFLOAT3(halfX, extents.y, halfZ));
    childBounds[2] = BoundingBox(XMFLOAT3(center.x - halfX, center.y, center.z - halfZ), XMFLOAT3(halfX, extents.y, halfZ));
    childBounds[3] = BoundingBox(XMFLOAT3(center.x + halfX, center.y, center.z - halfZ), XMFLOAT3(halfX, extents.y, halfZ));

    for (int i = 0; i < 4; i++)
    {
        node->children[i] = new QuadTreeNode(childBounds[i]);
        node->children[i]->depth = depth + 1;
        node->isLeaf = false;
        PreBuild(node->children[i], depth + 1);
    }
}

void CQuadTree::InsertObject(QuadTreeNode* node, CGameObject* object)
{
    if (!node || !object) return;

    const BoundingBox& objBounds = object->GetBoundingBox();

    if (node->isLeaf)
    {
        if (node->bounds.Intersects(objBounds))
        {
            if (std::find(node->objects.begin(), node->objects.end(), object) == node->objects.end())
            {
                node->objects.push_back(object);
                object->m_pNode = node;
            }
        }
    }
    else
    {
        for (int i = 0; i < 4; i++)
        {
            if (node->children[i])
            {
                InsertObject(node->children[i], object);
            }
        }
    }
}

void CQuadTree::InsertCollider(QuadTreeNode* node, const ColliderInfo& collider)
{
    if (!node) return;

    if (node->isLeaf)
    {
        bool isIntersecting = false;

        if (collider.type == ColliderType::AABB) {
            isIntersecting = node->bounds.Intersects(collider.aabb);
        }
        else if (collider.type == ColliderType::Sphere) {
            isIntersecting = node->bounds.Intersects(collider.sphere);
        }
        else if (collider.type == ColliderType::OBB) {
            isIntersecting = node->bounds.Intersects(collider.obb);
        }
        else if (collider.type == ColliderType::Segment) {
            XMVECTOR start = XMLoadFloat3(&collider.segment.start);
            XMVECTOR end = XMLoadFloat3(&collider.segment.end);

            XMVECTOR dir = end - start;
            float segmentLength;
            XMStoreFloat(&segmentLength, XMVector3Length(dir));

            if (segmentLength > 0.0001f) {
                dir = XMVector3Normalize(dir);
                float dist = 0.0f;
                if (node->bounds.Intersects(start, dir, dist)) {
                    isIntersecting = (dist <= segmentLength);
                }
            }
            else {
                isIntersecting = (node->bounds.Contains(start) != ContainmentType::DISJOINT);
            }
        }

        if (isIntersecting)
        {
            bool isAlreadyInserted = false;
            for (const auto& existing : node->colliders)
            {
                if (collider.pOwner != nullptr && existing.pOwner == collider.pOwner) {
                    isAlreadyInserted = true;
                    break;
                }
                else if (collider.pOwner == nullptr &&
                    existing.modelIndex == collider.modelIndex &&
                    existing.instanceID == collider.instanceID) {
                    isAlreadyInserted = true;
                    break;
                }
            }
            if (!isAlreadyInserted)
            {
                node->colliders.push_back(collider);

                if (collider.pOwner)
                {
                    collider.pOwner->m_pNode = node;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < 4; i++)
        {
            if (node->children[i])
            {
                InsertCollider(node->children[i], collider);
            }
        }
    }
}

void CQuadTree::DeleteNode(QuadTreeNode* node)
{
    if (!node) return;
    for (int i = 0; i < 4; i++)
        DeleteNode(node->children[i]);
    delete node;
}

void CQuadTree::PrintNode(QuadTreeNode* node, int depth)
{
    if (!node) return;

    // 들여쓰기를 편하게 하기 위한 람다 함수
    auto PrintIndent = [](int d) {
        for (int i = 0; i < d; i++) std::cout << "  ";
        };

    PrintIndent(depth);
    std::cout << "Node (Depth " << depth << "):\n";

    PrintIndent(depth + 1);
    std::cout << "Bounds: Center(" << node->bounds.Center.x << ", " << node->bounds.Center.y << ", " << node->bounds.Center.z << ") "
        << "Extents(" << node->bounds.Extents.x << ", " << node->bounds.Extents.y << ", " << node->bounds.Extents.z << ")\n";

    PrintIndent(depth + 1);
    std::cout << "IsLeaf: " << (node->isLeaf ? "Yes" : "No")
        << ", Objects: " << node->objects.size()
        << ", Colliders: " << node->colliders.size() << "\n";

    // 1. 기존 동적 오브젝트 (CGameObject* 리스트) 출력
    if (!node->objects.empty())
    {
        PrintIndent(depth + 1);
        std::cout << "[Objects]\n";
        for (size_t i = 0; i < node->objects.size(); ++i)
        {
            PrintIndent(depth + 2);
            std::cout << "Object " << i << ": frameName = " << node->objects[i]->GetFrameName() << "\n";
        }
    }

    // 2. 새로운 정적/인스턴싱 충돌체 (ColliderInfo 리스트) 출력
    if (!node->colliders.empty())
    {
        PrintIndent(depth + 1);
        std::cout << "[Colliders]\n";
        for (size_t i = 0; i < node->colliders.size(); ++i)
        {
            PrintIndent(depth + 2);
            const auto& col = node->colliders[i];

            // ColliderType을 문자열로 변환 (switch문 사용)
            std::string typeStr;
            switch (col.type)
            {
            case ColliderType::AABB:    typeStr = "AABB"; break;
            case ColliderType::OBB:     typeStr = "OBB"; break;
            case ColliderType::Segment: typeStr = "Segment"; break;
            case ColliderType::Sphere:  typeStr = "Sphere"; break;
            case ColliderType::Capsule: typeStr = "Capsule"; break;
            default:                    typeStr = "Unknown"; break;
            }

            std::cout << "Collider " << i << " [" << typeStr << "] : ";

            // 식별자 정보 출력
            if (col.pOwner)
            {
                std::cout << "Owner = " << col.pOwner->GetFrameName() << "\n";
            }
            else
            {
                std::cout << "Instance (ModelIdx: " << col.modelIndex << ", InstID: " << col.instanceID << ")\n";
            }
        }
    }

    // 3. 자식 노드 순회
    if (!node->isLeaf)
    {
        for (int i = 0; i < 4; i++)
        {
            if (node->children[i])
            {
                PrintIndent(depth + 1);
                std::cout << "Child " << i << ":\n";
                PrintNode(node->children[i], depth + 1);
            }
        }
    }
}
