#include "DebugUtility.h"
#include "BaseCamera.h"
#include "VectorUtility.h"
#include "MathUtility.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// デバイス・コンテキストを受け取り、描画に必要な一式を作成する
void DebugDraw::Initialize(ServiceLoacator& loacator)
{
    SetDevice(loacator.device);
    SetContext(loacator.context);

    CreateConstantBuffer();
    CreateVertexBuffer(); // この時点では_verticesが空なのでバッファは作られず、初回Render時に生成される
    CompileShader();

    CreateShader();

    CreateInputLayout();
}


void DebugDraw::CreateShader()
{
    _device->CreateVertexShader(_vertexShaderBlob->GetBufferPointer(),
        _vertexShaderBlob->GetBufferSize(), nullptr, _verticesShader.ReleaseAndGetAddressOf());

    _device->CreatePixelShader(_pixelShaderBlob->GetBufferPointer(),
        _pixelShaderBlob->GetBufferSize(), nullptr, _pixelShader.ReleaseAndGetAddressOf());
}


void DebugDraw::CreateConstantBuffer()
{
    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = sizeof(DebagConstantBuffer);
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    _device->CreateBuffer(&desc, nullptr, _constantBuffer.ReleaseAndGetAddressOf());
}


void DebugDraw::CreateVertexBuffer()
{
    _vertexCapacity = _vertices.size();
    if (_vertices.size() == 0)return; // 頂点が無ければ作る必要が無い

    D3D11_BUFFER_DESC desc{};

    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth =
        static_cast<UINT>(
            sizeof(DebagDrawVertex) * _vertexCapacity);

    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // 毎フレームMapで書き込むためCPUアクセスを許可

    _device->CreateBuffer(
        &desc,
        nullptr,
        _vertecesBlob.ReleaseAndGetAddressOf());

    //_vertices.shrink_to_fit();これをもうオブジェクトが増えないタイミングで書く
}


void DebugDraw::CompileShader()
{
    ID3DBlob* vsError = nullptr;
    ID3DBlob* psError = nullptr;

    HRESULT verticesHr = D3DCompileFromFile(
        L"DebagVertexShader.hlsl", // ファイル名
        nullptr,              // マクロ定義
        nullptr,              // #includeの処理
        "main",               // HLSLの関数名
        "vs_5_0",             // VertexShader 5.0
        0,
        0,
        _vertexShaderBlob.GetAddressOf(),
        &vsError);
    if (FAILED(verticesHr))
    {
        if (vsError)
        {
            OutputDebugStringA(
                static_cast<char*>(vsError->GetBufferPointer()));

            vsError->Release();
        }

        MessageBox(
            nullptr,
            L"VertexShader作成失敗",
            L"Error",
            MB_OK);

        return;
    }

    HRESULT pixelHr = D3DCompileFromFile(
        L"DebagPixelShader.hlsl",
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        0,
        0,
        _pixelShaderBlob.GetAddressOf(),
        &psError
    );
    if (FAILED(pixelHr))
    {
        if (psError)
        {
            OutputDebugStringA(
                static_cast<char*>(psError->GetBufferPointer()));

            psError->Release();
        }

        MessageBox(
            nullptr,
            L"PixelShader作成失敗",
            L"Error",
            MB_OK);

        return;
    }
}


void DebugDraw::CreateInputLayout()
{
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        {
            "COLOR",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            12,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
    };
    HRESULT hr = _device->CreateInputLayout(
        layout,
        ARRAYSIZE(layout),
        _vertexShaderBlob->GetBufferPointer(),
        _vertexShaderBlob->GetBufferSize(),
        _inputLayout.GetAddressOf());

    if (FAILED(hr))
    {
        MessageBox(nullptr, L"InputLayout作成失敗", L"Error", MB_OK);
    }
}


void DebugDraw::DrawLine(const XMFLOAT3& start, const XMFLOAT3& end, const XMFLOAT4& color)
{
    DebagDrawVertex v;

    v.Position = start;
    v.Color = color;
    _vertices.push_back(v);

    v.Position = end;
    v.Color = color;
    _vertices.push_back(v);
}


void DebugDraw::DrawCapsule(const XMFLOAT3& s, const XMFLOAT3& e, const float& radius, const XMFLOAT4& color)
{
    Vector3 start;
    Vector3 end;

    Vector::fromF3ToV3(s, start);
    Vector::fromF3ToV3(e, end);

    // カプセルの軸方向(正規化)
    Vector3 axis = end - start;

    float length = std::sqrt(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);

    if (length <= 0.00001f)
        return; // 始点と終点がほぼ同じなら描画できない

    axis /= length;

    // 軸に垂直な2方向(right, forward)を求め、円周の基準にする
    Vector3 up = { 0.0f, 1.0f, 0.0f };

    if (fabs(Vector::Dot(axis, up)) > 0.99f)
    {
        // 軸がY軸とほぼ平行だと外積が不安定になるので、基準ベクトルを変える
        up = { 1.0f, 0.0f, 0.0f };
    }

    Vector3 right = Vector::Cross(up, axis);
    right.Normalize();

    Vector3 forward = Vector::Cross(axis, right);
    forward.Normalize();

    const int segments = 6;

    // 円柱部分: 始点側と終点側の円周、および側面の縦線を描く
    for (int i = 0; i < segments; ++i)
    {
        float theta1 = XM_2PI * i / segments;
        float theta2 = XM_2PI * (i + 1) / segments;

        Vector3 p1 =
            start +
            right * cosf(theta1) * radius +
            forward * sinf(theta1) * radius;

        Vector3 p2 =
            start +
            right * cosf(theta2) * radius +
            forward * sinf(theta2) * radius;

        Vector3 p3 =
            end +
            right * cosf(theta1) * radius +
            forward * sinf(theta1) * radius;

        Vector3 p4 =
            end +
            right * cosf(theta2) * radius +
            forward * sinf(theta2) * radius;

        DrawLine(Vector::fromV3ToF3(p1), Vector::fromV3ToF3(p2), color);
        DrawLine(Vector::fromV3ToF3(p3), Vector::fromV3ToF3(p4), color);

        // 円柱の縦線
        DrawLine(Vector::fromV3ToF3(p1), Vector::fromV3ToF3(p3), color);
    }


    const int rings = 2;

    // 始点側の半球を近似リングで描画(軸の内側=マイナス方向へ膨らませる)
    for (int j = 0; j < rings; ++j)
    {
        float phi1 = XM_PIDIV2 * j / rings;
        float phi2 = XM_PIDIV2 * (j + 1) / rings;

        float r1 = cosf(phi1) * radius;
        float h1 = sinf(phi1) * radius;

        float r2 = cosf(phi2) * radius;
        float h2 = sinf(phi2) * radius;

        for (int i = 0; i < segments; ++i)
        {
            float theta1 = XM_2PI * i / segments;
            float theta2 = XM_2PI * (i + 1) / segments;

            Vector3 p1 =
                start +
                right * cosf(theta1) * r1 +
                forward * sinf(theta1) * r1 -
                axis * h1;

            Vector3 p2 =
                start +
                right * cosf(theta2) * r1 +
                forward * sinf(theta2) * r1 -
                axis * h1;

            Vector3 p3 =
                start +
                right * cosf(theta1) * r2 +
                forward * sinf(theta1) * r2 -
                axis * h2;

            Vector3 p4 =
                start +
                right * cosf(theta2) * r2 +
                forward * sinf(theta2) * r2 -
                axis * h2;

            DrawLine(Vector::fromV3ToF3(p1), Vector::fromV3ToF3(p2), color);
            DrawLine(Vector::fromV3ToF3(p1), Vector::fromV3ToF3(p3), color);
        }
    }

    // 終点側の半球を近似リングで描画(軸の外側=プラス方向へ膨らませる)
    for (int j = 0; j < rings; ++j)
    {
        float phi1 = XM_PIDIV2 * j / rings;
        float phi2 = XM_PIDIV2 * (j + 1) / rings;

        float r1 = cosf(phi1) * radius;
        float h1 = sinf(phi1) * radius;

        float r2 = cosf(phi2) * radius;
        float h2 = sinf(phi2) * radius;

        for (int i = 0; i < segments; ++i)
        {
            float theta1 = XM_2PI * i / segments;
            float theta2 = XM_2PI * (i + 1) / segments;

            Vector3 p1 =
                end +
                right * cosf(theta1) * r1 +
                forward * sinf(theta1) * r1 +
                axis * h1;

            Vector3 p2 =
                end +
                right * cosf(theta2) * r1 +
                forward * sinf(theta2) * r1 +
                axis * h1;

            Vector3 p3 =
                end +
                right * cosf(theta1) * r2 +
                forward * sinf(theta1) * r2 +
                axis * h2;

            Vector3 p4 =
                end +
                right * cosf(theta2) * r2 +
                forward * sinf(theta2) * r2 +
                axis * h2;

            DrawLine(
                Vector::fromV3ToF3(p1),
                Vector::fromV3ToF3(p2),
                color);

            DrawLine(
                Vector::fromV3ToF3(p1),
                Vector::fromV3ToF3(p3),
                color);
        }
    }
}

void DebugDraw::DrawSphere(const XMFLOAT3& position, float radius, const XMFLOAT4& color)
{
    const int segments = 16;

    // XY平面の円
    for (int i = 0; i < segments; ++i)
    {
        float theta1 =
            XM_2PI * i / segments;

        float theta2 =
            XM_2PI * (i + 1) / segments;

        XMFLOAT3 p1 =
        {
            position.x + radius * cosf(theta1),
            position.y + radius * sinf(theta1),
            position.z
        };

        XMFLOAT3 p2 =
        {
            position.x + radius * cosf(theta2),
            position.y + radius * sinf(theta2),
            position.z
        };

        DrawLine(p1, p2, color);
    }

    // XZ平面の円
    for (int i = 0; i < segments; ++i)
    {
        float theta1 =
            XM_2PI * i / segments;

        float theta2 =
            XM_2PI * (i + 1) / segments;

        XMFLOAT3 p1 =
        {
            position.x + radius * cosf(theta1),
            position.y,
            position.z + radius * sinf(theta1)
        };

        XMFLOAT3 p2 =
        {
            position.x + radius * cosf(theta2),
            position.y,
            position.z + radius * sinf(theta2)
        };

        DrawLine(p1, p2, color);
    }

    // YZ平面の円
    for (int i = 0; i < segments; ++i)
    {
        float theta1 =
            XM_2PI * i / segments;

        float theta2 =
            XM_2PI * (i + 1) / segments;

        XMFLOAT3 p1 =
        {
            position.x,
            position.y + radius * cosf(theta1),
            position.z + radius * sinf(theta1)
        };

        XMFLOAT3 p2 =
        {
            position.x,
            position.y + radius * cosf(theta2),
            position.z + radius * sinf(theta2)
        };

        DrawLine(p1, p2, color);
    }
}

// AABB(min/max)を12本の線で構成されるボックスとして描画
void DebugDraw::DrawBox(
    const XMFLOAT3& min,
    const XMFLOAT3& max,
    const XMFLOAT4& color)
{
    XMFLOAT3 v[8] =
    {
        { min.x, min.y, min.z }, // 0
        { max.x, min.y, min.z }, // 1
        { max.x, max.y, min.z }, // 2
        { min.x, max.y, min.z }, // 3

        { min.x, min.y, max.z }, // 4
        { max.x, min.y, max.z }, // 5
        { max.x, max.y, max.z }, // 6
        { min.x, max.y, max.z }  // 7
    };

    // 手前
    DrawLine(v[0], v[1], color);
    DrawLine(v[1], v[2], color);
    DrawLine(v[2], v[3], color);
    DrawLine(v[3], v[0], color);

    // 奥
    DrawLine(v[4], v[5], color);
    DrawLine(v[5], v[6], color);
    DrawLine(v[6], v[7], color);
    DrawLine(v[7], v[4], color);

    // 手前と奥を繋ぐ
    DrawLine(v[0], v[4], color);
    DrawLine(v[1], v[5], color);
    DrawLine(v[2], v[6], color);
    DrawLine(v[3], v[7], color);
}


void DebugDraw::DrawRay(const XMFLOAT3& origin, const XMFLOAT3& direction, const float& maxDistance, const XMFLOAT4& color)
{
    float t1 = origin.x + direction.x * maxDistance;
    float t2 = origin.y + direction.y * maxDistance;
    float t3 = origin.z + direction.z * maxDistance;

    XMFLOAT3 result = { t1,t2,t3 };
    DrawLine(origin, result, color);
}

void DebugDraw::UpdateConstantBuffer(Camera& camera)
{
    _constantData.world =
        XMMatrixIdentity();

    _constantData.view =
        XMMatrixTranspose(camera.GetViewMatrix());

    _constantData.projection =
        XMMatrixTranspose(camera.GetProjectionMatrix());

    _context->UpdateSubresource(
        _constantBuffer.Get(),
        0,
        nullptr,
        &_constantData,
        0,
        0);
}

void DebugDraw::Render(Camera& camera)
{
    if (_vertices.empty())
        return;

    // 頂点数がバッファ容量を超えていたら作り直す
    if (_vertices.size() > _vertexCapacity) CreateVertexBuffer();

    // -------------------------
    // ConstantBuffer更新
    // -------------------------
    UpdateConstantBuffer(camera);

    // VertexBuffer更新
    // -------------------------
    D3D11_MAPPED_SUBRESOURCE mapped{};

    HRESULT hr = _context->Map(
        _vertecesBlob.Get(),
        0,
        D3D11_MAP_WRITE_DISCARD,
        0,
        &mapped);

    if (FAILED(hr))
        return;

    // CPU側の頂点データをGPUバッファへコピー
    memcpy(
        mapped.pData,
        _vertices.data(),
        sizeof(DebagDrawVertex) * _vertices.size());

    _context->Unmap(
        _vertecesBlob.Get(),
        0);

    // VertexBuffer設定
    UINT stride = sizeof(DebagDrawVertex);
    UINT offset = 0;

    _context->IASetVertexBuffers(
        0,
        1,
        _vertecesBlob.GetAddressOf(),
        &stride,
        &offset);

    // InputLayout
    _context->IASetInputLayout(
        _inputLayout.Get());

    // Shader
    _context->VSSetShader(
        _verticesShader.Get(),
        nullptr,
        0);

    _context->PSSetShader(
        _pixelShader.Get(),
        nullptr,
        0);

    // ConstantBuffer
    _context->VSSetConstantBuffers(
        0,
        1,
        _constantBuffer.GetAddressOf());

    // 線分リストとして描画
    _context->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    _context->Draw(
        static_cast<UINT>(_vertices.size()),
        0);

    // 次フレームのために描画済みの頂点リストをクリア
    ClearList();
}
