#pragma once

class CGameObject;

enum class ColliderType
{
    AABB,
    OBB,
    Segment,
    Sphere,
    Capsule
};

struct SegmentData
{
    XMFLOAT3 start;
    XMFLOAT3 end;
};

struct ColliderInfo
{
    ColliderType type;

    union {
        BoundingBox         aabb;
        BoundingSphere      sphere;
        BoundingOrientedBox obb;
        SegmentData         segment;
    };

    CGameObject* pOwner;      // 일반 객체일 경우 포인터 저장, 인스턴스면 nullptr
    int modelIndex;           // 인스턴싱 베이스 모델 인덱스 (일반 객체면 -1)
    int instanceID;           // 인스턴스 ID (일반 객체면 -1)

    ColliderInfo() : pOwner(nullptr), modelIndex(-1), instanceID(-1) {}

    // AABB용 생성자
    ColliderInfo(const BoundingBox& b, CGameObject* owner = nullptr, int mIdx = -1, int iID = -1)
        : type(ColliderType::AABB), aabb(b), pOwner(owner), modelIndex(mIdx), instanceID(iID) {}

    // OBB용 생성자
    ColliderInfo(const BoundingOrientedBox& o, CGameObject* owner = nullptr, int mIdx = -1, int iID = -1)
        : type(ColliderType::OBB), obb(o), pOwner(owner), modelIndex(mIdx), instanceID(iID) {}
   
    // Segment용 생성자
    ColliderInfo(const XMFLOAT3& start, const XMFLOAT3& end, CGameObject* owner = nullptr, int mIdx = -1, int iID = -1)
        : type(ColliderType::Segment), pOwner(owner), modelIndex(mIdx), instanceID(iID)
    {
        segment.start = start;
        segment.end = end;
    }

    // Sphere용 생성자
    ColliderInfo(const BoundingSphere& s, CGameObject* owner = nullptr, int mIdx = -1, int iID = -1)
        : type(ColliderType::Sphere), sphere(s), pOwner(owner), modelIndex(mIdx), instanceID(iID) {}



};

struct QuadTreeNode
{
    BoundingBox bounds;
    std::vector<CGameObject*> objects;
    std::vector<ColliderInfo> colliders;
    QuadTreeNode* children[4];
    int depth;
    bool isLeaf;

    QuadTreeNode(const BoundingBox& bounds) : bounds(bounds), isLeaf(true), depth(0)
    {
        for (int i = 0; i < 4; i++)
            children[i] = nullptr;
    }
};

class CQuadTree
{
public:
    QuadTreeNode* root;
    int maxObjectsPerNode;
    float minNodeSize;
    int maxDepth;

    CQuadTree(float minSize = 10.0f) : root(nullptr), maxObjectsPerNode(10), minNodeSize(minSize), maxDepth(0) {}
    ~CQuadTree()
    {
        DeleteNode(root);
    }

    void Build(const BoundingBox& worldBounds, int maxObjects, int maxDepth);

    void Insert(CGameObject* object);
    void Insert(const ColliderInfo& collider);

    void PrintTree();

    QuadTreeNode* FindNode(QuadTreeNode* node, const BoundingBox& aabb);
private:
    void PreBuild(QuadTreeNode* node, int depth);

    void InsertObject(QuadTreeNode* node, CGameObject* object);
    void InsertCollider(QuadTreeNode* node, const ColliderInfo& collider);

    void DeleteNode(QuadTreeNode* node);

    void PrintNode(QuadTreeNode* node, int depth);
};