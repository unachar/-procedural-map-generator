#include "createmap.h"
#include "renderer.h"
#include "room.h" 
#include "manager.h"
#include "camera.h"
#include "scene.h"
#include "ImGui/imgui.h"

// ImGuiパラメータの初期化
bool CreateRoom::s_NeedRegenerate = false;
float CreateRoom::s_TileScale = m_kTileScaleBase;
float CreateRoom::s_WallHeight = m_kWallHeightBase;
int CreateRoom::s_MaxSteps = m_kMaxStepsBase;
int CreateRoom::s_MinRoomSize = m_kMinRoomSizeBase;
int CreateRoom::s_MaxRoomSize = m_kMaxRoomSizeBase;
int CreateRoom::s_Seed = m_kSeedBase; 
float CreateRoom::s_MaxRenderDistance = m_kMaxRenderDistanceBase;

void CreateRoom::Initialize()
{
    m_Room = new Room();


    m_Room->SetTileScale(s_TileScale);
    m_Room->SetWallHeight(s_WallHeight);
    m_Room->SetMaxSteps(s_MaxSteps);
    m_Room->SetMinRoomSize(s_MinRoomSize);
    m_Room->SetMaxRoomSize(s_MaxRoomSize);
    m_Room->SetSeed(s_Seed);

    m_Room->Generate();

    vector<InstanceData> floorData, wallData;

    m_Room->CreateRnderData(floorData, wallData);

    m_FloorInstanceCount = (int)floorData.size();
    m_WallInstanceCount = (int)wallData.size();


    VERTEX_3D vertex[m_kCubeVertexCount]{};
    vertex[0].Position = XMFLOAT3(-m_kCubePosBase, -m_kCubePosBase, -m_kCubePosBase);
    vertex[1].Position = XMFLOAT3(-m_kCubePosBase,  m_kCubePosBase, -m_kCubePosBase);
    vertex[2].Position = XMFLOAT3( m_kCubePosBase,  m_kCubePosBase, -m_kCubePosBase);
    vertex[3].Position = XMFLOAT3( m_kCubePosBase, -m_kCubePosBase, -m_kCubePosBase);
    vertex[4].Position = XMFLOAT3(-m_kCubePosBase, -m_kCubePosBase,  m_kCubePosBase);
    vertex[5].Position = XMFLOAT3(-m_kCubePosBase,  m_kCubePosBase,  m_kCubePosBase); 
    vertex[6].Position = XMFLOAT3( m_kCubePosBase,  m_kCubePosBase,  m_kCubePosBase); 
    vertex[7].Position = XMFLOAT3( m_kCubePosBase, -m_kCubePosBase,  m_kCubePosBase); 

    for (int i = 0; i < m_kCubeVertexCount; i++)
    {
        vertex[i].Normal = m_kNormalBase;
        vertex[i].TexCoord = m_kTexCoordBase;
        vertex[i].Diffuse = m_kDiffuseBase;
    }

    uint16_t index[m_kCubeIndexCount] =
    {
        0,1,2, 0,2,3, 4,6,5, 4,7,6, 4,5,1, 4,1,0,
        3,2,6, 3,6,7, 1,5,6, 1,6,2, 4,0,3, 4,3,7
    };

    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertex);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA sd{ vertex };
    Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);

    bd.ByteWidth = sizeof(index);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    sd.pSysMem = index;
    Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_IndexBuffer);


    D3D11_BUFFER_DESC ibd{};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    ibd.ByteWidth = sizeof(InstanceData) * m_FloorInstanceCount;
    D3D11_SUBRESOURCE_DATA isdF{ floorData.data() };
    Renderer::GetDevice()->CreateBuffer(&ibd, &isdF, &m_InstanceBufferFloor);

    ibd.ByteWidth = sizeof(InstanceData) * m_WallInstanceCount;
    D3D11_SUBRESOURCE_DATA isdW{ wallData.data() };
    Renderer::GetDevice()->CreateBuffer(&ibd, &isdW, &m_InstanceBufferWall);


    D3D11_RASTERIZER_DESC rd{};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_BACK;
    rd.DepthClipEnable = TRUE;
    Renderer::GetDevice()->CreateRasterizerState(&rd, &m_StateSolid);

    rd.FillMode = D3D11_FILL_WIREFRAME;
    rd.CullMode = D3D11_CULL_NONE;
    rd.DepthBias = 0;
    Renderer::GetDevice()->CreateRasterizerState(&rd, &m_StateWire);


    Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\roomInstanceVS.cso");
    Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void CreateRoom::Update()
{
    // 再生成フラグが立っている場合のみマップを再生成
    if (s_NeedRegenerate)
    {
        RegenerateMap();
        s_NeedRegenerate = false;
    }


    auto cam = Manager::GetScene()->GetGameObject<Camera>();
    if (!cam || !m_Room)
    {
        return;
    }

    Vector3 camPos = cam->GetPosition();
    Vector3 camTarget = cam->GetTarget();
	XMFLOAT3 up = cam->GetUpVector();

    XMMATRIX view = XMMatrixLookAtLH(
        XMVectorSet(camPos.x, camPos.y, camPos.z, 1.f),
        XMVectorSet(camTarget.x, camTarget.y, camTarget.z, 1.f),
        XMLoadFloat3(&up));

    float fovy = cam->GetFovy();
    if (fovy <= 0.0f)
    {
        fovy = 1.0f;
    }
    XMMATRIX proj = XMMatrixPerspectiveFovLH(
        fovy,
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        1.f,
        1000.f);

    XMMATRIX vp = XMMatrixMultiply(view, proj);
    XMFLOAT4X4 m;
    XMStoreFloat4x4(&m, vp);

    // 6つの平面を抽出
    m_Planes[0] = { m._14 + m._11, m._24 + m._21, m._34 + m._31, m._44 + m._41 }; // Left
    m_Planes[1] = { m._14 - m._11, m._24 - m._21, m._34 - m._31, m._44 - m._41 }; // Right
    m_Planes[2] = { m._14 + m._12, m._24 + m._22, m._34 + m._32, m._44 + m._42 }; // Bottom
    m_Planes[3] = { m._14 - m._12, m._24 - m._22, m._34 - m._32, m._44 - m._42 }; // Top
    m_Planes[4] = {         m._13,         m._23,         m._33,         m._43 }; // Near
    m_Planes[5] = { m._14 - m._13, m._24 - m._23, m._34 - m._33, m._44 - m._43 }; // Far


    for (int i = 0; i < m_kPlanesNum; i++)
    {
        float len = sqrtf(m_Planes[i].x * m_Planes[i].x + m_Planes[i].y * m_Planes[i].y + m_Planes[i].z * m_Planes[i].z);
        if (len > 0.0f)
        {
            m_Planes[i].x /= len;
            m_Planes[i].y /= len;
            m_Planes[i].z /= len;
            m_Planes[i].w /= len;
        }
    }


    vector<InstanceData> floorData;
    vector<InstanceData> wallData;
    // 視錐台 + 距離カリングを適用
    XMFLOAT3 camPosF(camPos.x, camPos.y, camPos.z);
    m_Room->CreateRnderData(floorData, wallData, m_Planes, &camPosF, s_MaxRenderDistance);


    m_FloorInstanceCount = (int)floorData.size();
    m_WallInstanceCount = (int)wallData.size();

    if (m_InstanceBufferFloor)
    {
        m_InstanceBufferFloor->Release();
        m_InstanceBufferFloor = nullptr;
    }
    if (m_InstanceBufferWall)
    {
        m_InstanceBufferWall->Release();
        m_InstanceBufferWall = nullptr;
    }

    D3D11_BUFFER_DESC ibd{};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    if (m_FloorInstanceCount > 0)
    {
        ibd.ByteWidth = sizeof(InstanceData) * m_FloorInstanceCount;
        D3D11_SUBRESOURCE_DATA isdF{ floorData.data() };
        Renderer::GetDevice()->CreateBuffer(&ibd, &isdF, &m_InstanceBufferFloor);
    }

    if (m_WallInstanceCount > 0)
    {
        ibd.ByteWidth = sizeof(InstanceData) * m_WallInstanceCount;
        D3D11_SUBRESOURCE_DATA isdW{ wallData.data() };
        Renderer::GetDevice()->CreateBuffer(&ibd, &isdW, &m_InstanceBufferWall);
    }
}

void CreateRoom::Draw()
{
    Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
    Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
    Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

    Renderer::AddWorldMatrix(
        { m_Scale.x, m_Scale.y, m_Scale.z },
        { m_Rotation.x, m_Rotation.y, m_Rotation.z },
        { m_Position.x, m_Position.y, m_Position.z }
    );

    Renderer::GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT strides[] = { sizeof(VERTEX_3D), sizeof(InstanceData) };
    UINT offsets[] = { 0, 0 };

    // --- パス1: 白塗り ---
    Renderer::GetDeviceContext()->RSSetState(m_StateSolid);
    MATERIAL mat{};
    mat.Diffuse = XMFLOAT4(1, 1, 1, 1);
    Renderer::SetMaterial(mat);

    ID3D11Buffer* vbsF[] = { m_VertexBuffer, m_InstanceBufferFloor };
    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 2, vbsF, strides, offsets);
    Renderer::GetDeviceContext()->DrawIndexedInstanced(m_kCubeIndexCount, m_FloorInstanceCount, 0, 0, 0);

    ID3D11Buffer* vbsW[] = { m_VertexBuffer, m_InstanceBufferWall };
    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 2, vbsW, strides, offsets);
    Renderer::GetDeviceContext()->DrawIndexedInstanced(m_kCubeIndexCount, m_WallInstanceCount, 0, 0, 0);

    // --- パス2: 黒枠 ---
    Renderer::GetDeviceContext()->RSSetState(m_StateWire);
    mat.Diffuse = XMFLOAT4(0, 0, 0, 1);
    Renderer::SetMaterial(mat);

    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 2, vbsF, strides, offsets);
    Renderer::GetDeviceContext()->DrawIndexedInstanced(m_kCubeIndexCount, m_FloorInstanceCount, 0, 0, 0);

    Renderer::GetDeviceContext()->IASetVertexBuffers(0, 2, vbsW, strides, offsets);
    Renderer::GetDeviceContext()->DrawIndexedInstanced(m_kCubeIndexCount, m_WallInstanceCount, 0, 0, 0);

    DrawImGui();

    Renderer::GetDeviceContext()->RSSetState(nullptr);

 
}

void CreateRoom::DrawImGui()
{
    ImGui::Begin("Room Generation Parameters");

    bool regenerate = false;

    // 基本パラメータ
    ImGui::Text("=== Map Settings ===");
    if (ImGui::SliderFloat("Tile Scale", &s_TileScale, m_kTileScaleMin, m_kTileScaleMax)) 
    {
        regenerate = true;
    }

    if (ImGui::SliderFloat("Wall Height", &s_WallHeight, m_kWallHeightMin, m_kWallHeightMax))
    {
        regenerate = true;
    }

    ImGui::Separator();
    ImGui::Text("=== Generation Settings ===");

    if (ImGui::SliderInt("Max Steps", &s_MaxSteps, m_kMaxStepsNumMin, m_kMaxStepsNumMax))
    {
        regenerate = true;
    }

    if (ImGui::SliderInt("Min Room Size", &s_MinRoomSize, m_kMinRoomSizeNumMin, m_kMinRoomSizeNumMax))
    {
        if (s_MinRoomSize > s_MaxRoomSize)
            s_MaxRoomSize = s_MinRoomSize;
        regenerate = true;
    }

    if (ImGui::SliderInt("Max Room Size", &s_MaxRoomSize, m_kMaxRoomSizeNumMin, m_kMaxRoomSizeNumMax))
    {
        if (s_MaxRoomSize < s_MinRoomSize)
            s_MinRoomSize = s_MaxRoomSize;
        regenerate = true;
    }

    if (ImGui::SliderInt("Seed", &s_Seed, m_kSeedNumMin, m_kSeedNumMax))
    {
        regenerate = true;
    }

    ImGui::Separator();
    ImGui::Text("=== Rendering Settings ===");


    ImGui::SliderFloat("Max Render Distance", &s_MaxRenderDistance, m_kMaxRenderDistanceMin, m_kMaxRenderDistanceMax);

    ImGui::Separator();

    // 統計情報の表示
    ImGui::Text("=== Statistics ===");
    ImGui::Text("Floor Instances: %d", m_FloorInstanceCount);
    ImGui::Text("Wall Instances: %d", m_WallInstanceCount);
    ImGui::Text("Total Instances: %d", m_FloorInstanceCount + m_WallInstanceCount);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    ImGui::Separator();

    ImGui::SameLine();
    if (ImGui::Button("Random Seed"))
    {
        s_Seed = rand() % m_kRandomMax;
        regenerate = true;
    }

    if (regenerate)
    {
        s_NeedRegenerate = true;
    }

    ImGui::End();
}

void CreateRoom::RegenerateMap()
{
    if (!m_Room)
        return;


    m_Room->SetTileScale(s_TileScale);
    m_Room->SetWallHeight(s_WallHeight);
    m_Room->SetMaxSteps(s_MaxSteps);
    m_Room->SetMinRoomSize(s_MinRoomSize);
    m_Room->SetMaxRoomSize(s_MaxRoomSize);
    m_Room->SetSeed(s_Seed);


    m_Room->Generate();

    vector<InstanceData> floorData, wallData;
    m_Room->CreateRnderData(floorData, wallData);

    m_FloorInstanceCount = (int)floorData.size();
    m_WallInstanceCount = (int)wallData.size();

    // 既存バッファを解放
    if (m_InstanceBufferFloor)
    {
        m_InstanceBufferFloor->Release();
        m_InstanceBufferFloor = nullptr;
    }
    if (m_InstanceBufferWall)
    {
        m_InstanceBufferWall->Release();
        m_InstanceBufferWall = nullptr;
    }

    D3D11_BUFFER_DESC ibd{};
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    if (m_FloorInstanceCount > 0)
    {
        ibd.ByteWidth = sizeof(InstanceData) * m_FloorInstanceCount;
        D3D11_SUBRESOURCE_DATA isdF{ floorData.data() };
        Renderer::GetDevice()->CreateBuffer(&ibd, &isdF, &m_InstanceBufferFloor);
    }

    if (m_WallInstanceCount > 0)
    {
        ibd.ByteWidth = sizeof(InstanceData) * m_WallInstanceCount;
        D3D11_SUBRESOURCE_DATA isdW{ wallData.data() };
        Renderer::GetDevice()->CreateBuffer(&ibd, &isdW, &m_InstanceBufferWall);
    }
}

void CreateRoom::Finalize()
{
    if (m_VertexBuffer) m_VertexBuffer->Release();
    if (m_IndexBuffer) m_IndexBuffer->Release();
    if (m_InstanceBufferFloor) m_InstanceBufferFloor->Release();
    if (m_InstanceBufferWall) m_InstanceBufferWall->Release();
    if (m_VertexLayout) m_VertexLayout->Release();
    if (m_VertexShader) m_VertexShader->Release();
    if (m_PixelShader) m_PixelShader->Release();
    if (m_StateSolid) m_StateSolid->Release();
    if (m_StateWire) m_StateWire->Release();

    if (m_Room)
    {
        delete m_Room;
        m_Room = nullptr;
    }
}