//--------------------------------------------------------------------------------------
// File: DebugDraw.cpp
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//-------------------------------------------------------------------------------------

#include "pch.h"
#include "DebugDraw.h"

#include <algorithm>

namespace
{
    inline void XM_CALLCONV DrawCube(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
        DirectX::CXMMATRIX matWorld,
        DirectX::FXMVECTOR color)
    {
        static const DirectX::XMVECTORF32 s_verts[8] =
        {
            { { { -1.f, -1.f, -1.f, 0.f } } },
            { { {  1.f, -1.f, -1.f, 0.f } } },
            { { {  1.f, -1.f,  1.f, 0.f } } },
            { { { -1.f, -1.f,  1.f, 0.f } } },
            { { { -1.f,  1.f, -1.f, 0.f } } },
            { { {  1.f,  1.f, -1.f, 0.f } } },
            { { {  1.f,  1.f,  1.f, 0.f } } },
            { { { -1.f,  1.f,  1.f, 0.f } } }
        };

        static const WORD s_indices[] =
        {
            0, 1,
            1, 2,
            2, 3,
            3, 0,
            4, 5,
            5, 6,
            6, 7,
            7, 4,
            0, 4,
            1, 5,
            2, 6,
            3, 7
        };

        DirectX::VertexPositionColor verts[8];
        for (size_t i = 0; i < 8; ++i)
        {
            const DirectX::XMVECTOR v = XMVector3Transform(s_verts[i], matWorld);
            XMStoreFloat3(&verts[i].position, v);
            XMStoreFloat4(&verts[i].color, color);
        }

        batch->DrawIndexed(D3D_PRIMITIVE_TOPOLOGY_LINELIST, s_indices, static_cast<UINT>(std::size(s_indices)), verts, 8);
    }
}

void XM_CALLCONV DX::Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    const DirectX::BoundingSphere& sphere,
    DirectX::FXMVECTOR color)
{
    const DirectX::XMVECTOR origin = XMLoadFloat3(&sphere.Center);

    const float radius = sphere.Radius;

    const DirectX::XMVECTOR xaxis = DirectX::XMVectorScale(DirectX::g_XMIdentityR0, radius);
    const DirectX::XMVECTOR yaxis = DirectX::XMVectorScale(DirectX::g_XMIdentityR1, radius);
    const DirectX::XMVECTOR zaxis = DirectX::XMVectorScale(DirectX::g_XMIdentityR2, radius);

    DrawRing(batch, origin, xaxis, zaxis, color);
    DrawRing(batch, origin, xaxis, yaxis, color);
    DrawRing(batch, origin, yaxis, zaxis, color);
}

void XM_CALLCONV DX::Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    const DirectX::BoundingBox& box,
    DirectX::FXMVECTOR color)
{
    DirectX::XMMATRIX matWorld = DirectX::XMMatrixScaling(box.Extents.x, box.Extents.y, box.Extents.z);
    const DirectX::XMVECTOR position = XMLoadFloat3(&box.Center);
    matWorld.r[3] = DirectX::XMVectorSelect(matWorld.r[3], position, DirectX::g_XMSelect1110);

    DrawCube(batch, matWorld, color);
}

void XM_CALLCONV DX::Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    const DirectX::BoundingOrientedBox& obb,
    DirectX::FXMVECTOR color)
{
    DirectX::XMMATRIX matWorld = DirectX::XMMatrixRotationQuaternion(XMLoadFloat4(&obb.Orientation));
    const DirectX::XMMATRIX matScale = DirectX::XMMatrixScaling(obb.Extents.x, obb.Extents.y, obb.Extents.z);
    matWorld = XMMatrixMultiply(matScale, matWorld);
    const DirectX::XMVECTOR position = XMLoadFloat3(&obb.Center);
    matWorld.r[3] = DirectX::XMVectorSelect(matWorld.r[3], position, DirectX::g_XMSelect1110);

    DrawCube(batch, matWorld, color);
}

void XM_CALLCONV DX::Draw(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    const DirectX::BoundingFrustum& frustum,
    DirectX::FXMVECTOR color)
{
    DirectX::XMFLOAT3 corners[DirectX::BoundingFrustum::CORNER_COUNT];
    frustum.GetCorners(corners);

    DirectX::VertexPositionColor verts[24] = {};
    verts[0].position = corners[0];
    verts[1].position = corners[1];
    verts[2].position = corners[1];
    verts[3].position = corners[2];
    verts[4].position = corners[2];
    verts[5].position = corners[3];
    verts[6].position = corners[3];
    verts[7].position = corners[0];

    verts[8].position = corners[0];
    verts[9].position = corners[4];
    verts[10].position = corners[1];
    verts[11].position = corners[5];
    verts[12].position = corners[2];
    verts[13].position = corners[6];
    verts[14].position = corners[3];
    verts[15].position = corners[7];

    verts[16].position = corners[4];
    verts[17].position = corners[5];
    verts[18].position = corners[5];
    verts[19].position = corners[6];
    verts[20].position = corners[6];
    verts[21].position = corners[7];
    verts[22].position = corners[7];
    verts[23].position = corners[4];

    for (size_t j = 0; j < std::size(verts); ++j)
    {
        XMStoreFloat4(&verts[j].color, color);
    }

    batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINELIST, verts, static_cast<UINT>(std::size(verts)));
}

void XM_CALLCONV DX::DrawGrid(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    DirectX::FXMVECTOR xAxis,
    DirectX::FXMVECTOR yAxis,
    DirectX::FXMVECTOR origin,
    size_t xdivs,
    size_t ydivs,
    DirectX::GXMVECTOR color)
{
    xdivs = std::max<size_t>(1, xdivs);
    ydivs = std::max<size_t>(1, ydivs);

    for (size_t i = 0; i <= xdivs; ++i)
    {
        float percent = float(i) / float(xdivs);
        percent = (percent * 2.f) - 1.f;
        DirectX::XMVECTOR scale = DirectX::XMVectorScale(xAxis, percent);
        scale = DirectX::XMVectorAdd(scale, origin);

        const DirectX::VertexPositionColor v1(DirectX::XMVectorSubtract(scale, yAxis), color);
        const DirectX::VertexPositionColor v2(DirectX::XMVectorAdd(scale, yAxis), color);
        batch->DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= ydivs; i++)
    {
        FLOAT percent = float(i) / float(ydivs);
        percent = (percent * 2.f) - 1.f;
        DirectX::XMVECTOR scale = DirectX::XMVectorScale(yAxis, percent);
        scale = DirectX::XMVectorAdd(scale, origin);

        const DirectX::VertexPositionColor v1(DirectX::XMVectorSubtract(scale, xAxis), color);
        const DirectX::VertexPositionColor v2(DirectX::XMVectorAdd(scale, xAxis), color);
        batch->DrawLine(v1, v2);
    }
}

void XM_CALLCONV DX::DrawRing(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    DirectX::FXMVECTOR origin,
    DirectX::FXMVECTOR majorAxis,
    DirectX::FXMVECTOR minorAxis,
    DirectX::GXMVECTOR color)
{
    constexpr size_t c_ringSegments = 32;

    DirectX::VertexPositionColor verts[c_ringSegments + 1];

    constexpr float fAngleDelta = DirectX::XM_2PI / float(c_ringSegments);
    // Instead of calling cos/sin for each segment we calculate
    // the sign of the angle delta and then incrementally calculate sin
    // and cosine from then on.
    const DirectX::XMVECTOR cosDelta = DirectX::XMVectorReplicate(cosf(fAngleDelta));
    const DirectX::XMVECTOR sinDelta = DirectX::XMVectorReplicate(sinf(fAngleDelta));
    DirectX::XMVECTOR incrementalSin = DirectX::XMVectorZero();
    static const DirectX::XMVECTORF32 s_initialCos =
    {
        { { 1.f, 1.f, 1.f, 1.f } }
    };
    DirectX::XMVECTOR incrementalCos = s_initialCos.v;
    for (size_t i = 0; i < c_ringSegments; i++)
    {
        DirectX::XMVECTOR pos = DirectX::XMVectorMultiplyAdd(majorAxis, incrementalCos, origin);
        pos = DirectX::XMVectorMultiplyAdd(minorAxis, incrementalSin, pos);
        XMStoreFloat3(&verts[i].position, pos);
        XMStoreFloat4(&verts[i].color, color);
        // Standard formula to rotate a vector.
        const DirectX::XMVECTOR newCos = DirectX::XMVectorSubtract(DirectX::XMVectorMultiply(incrementalCos, cosDelta), DirectX::XMVectorMultiply(incrementalSin, sinDelta));
        const DirectX::XMVECTOR newSin = DirectX::XMVectorAdd(DirectX::XMVectorMultiply(incrementalCos, sinDelta), DirectX::XMVectorMultiply(incrementalSin, cosDelta));
        incrementalCos = newCos;
        incrementalSin = newSin;
    }
    verts[c_ringSegments] = verts[0];

    batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, c_ringSegments + 1);
}

void XM_CALLCONV DX::DrawRay(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    DirectX::FXMVECTOR origin,
    DirectX::FXMVECTOR direction,
    bool normalize,
    DirectX::FXMVECTOR color)
{
    DirectX::VertexPositionColor verts[3];
    DirectX::XMStoreFloat3(&verts[0].position, origin);

    DirectX::XMVECTOR normDirection = DirectX::XMVector3Normalize(direction);
    DirectX::XMVECTOR rayDirection = (normalize) ? normDirection : direction;

    DirectX::XMVECTOR perpVector = DirectX::XMVector3Cross(normDirection, DirectX::g_XMIdentityR1);

    if (DirectX::XMVector3Equal(DirectX::XMVector3LengthSq(perpVector), DirectX::g_XMZero))
    {
        perpVector = DirectX::XMVector3Cross(normDirection, DirectX::g_XMIdentityR2);
    }
    perpVector = DirectX::XMVector3Normalize(perpVector);

    XMStoreFloat3(&verts[1].position, DirectX::XMVectorAdd(rayDirection, origin));
    perpVector = DirectX::XMVectorScale(perpVector, 0.0625f);
    normDirection = DirectX::XMVectorScale(normDirection, -0.25f);
    rayDirection = DirectX::XMVectorAdd(perpVector, rayDirection);
    rayDirection = DirectX::XMVectorAdd(normDirection, rayDirection);
    XMStoreFloat3(&verts[2].position, DirectX::XMVectorAdd(rayDirection, origin));

    XMStoreFloat4(&verts[0].color, color);
    XMStoreFloat4(&verts[1].color, color);
    XMStoreFloat4(&verts[2].color, color);

    batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 2);
}

void XM_CALLCONV DX::DrawTriangle(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
    DirectX::FXMVECTOR pointA,
    DirectX::FXMVECTOR pointB,
    DirectX::FXMVECTOR pointC,
    DirectX::GXMVECTOR color)
{
    DirectX::VertexPositionColor verts[4];
    XMStoreFloat3(&verts[0].position, pointA);
    XMStoreFloat3(&verts[1].position, pointB);
    XMStoreFloat3(&verts[2].position, pointC);
    XMStoreFloat3(&verts[3].position, pointA);

    XMStoreFloat4(&verts[0].color, color);
    XMStoreFloat4(&verts[1].color, color);
    XMStoreFloat4(&verts[2].color, color);
    XMStoreFloat4(&verts[3].color, color);

    batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 4);
}

void XM_CALLCONV DX::DrawQuad(DirectX::PrimitiveBatch< DirectX::VertexPositionColor>* batch,
    DirectX::FXMVECTOR pointA,
    DirectX::FXMVECTOR pointB,
    DirectX::FXMVECTOR pointC,
    DirectX::GXMVECTOR pointD,
    DirectX::HXMVECTOR color)
{
    DirectX::VertexPositionColor verts[5];
    XMStoreFloat3(&verts[0].position, pointA);
    XMStoreFloat3(&verts[1].position, pointB);
    XMStoreFloat3(&verts[2].position, pointC);
    XMStoreFloat3(&verts[3].position, pointD);
    XMStoreFloat3(&verts[4].position, pointA);

    XMStoreFloat4(&verts[0].color, color);
    XMStoreFloat4(&verts[1].color, color);
    XMStoreFloat4(&verts[2].color, color);
    XMStoreFloat4(&verts[3].color, color);
    XMStoreFloat4(&verts[4].color, color);

    batch->Draw(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, verts, 5);
}
