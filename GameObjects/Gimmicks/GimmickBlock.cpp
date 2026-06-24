//--------------------------------------------------------------------------------------
// File: GimmickBlock.cpp
//
// 仕掛けブロッククラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "GimmickBlock.h"

// 定数の定義
const float GimmickBlock::DEFAULT_FOLLOW_OFFSET_Y = -2.0f;  ///< 追尾時のデフォルトの高さオフセット
const float GimmickBlock::UNIT_CUBE_SIZE = 1.0f;            ///< AABBの基準となる単位立方体のサイズ
const float GimmickBlock::STOP_POSITION_OFFSET_Y = 0.35f;   ///< 停止位置から下げる高さのオフセット量

/*
* @brief コンストラクタ
*
* @param[in]  なし
*
* @return なし
*/
GimmickBlock::GimmickBlock()
    :
    m_followSpeed(0.0f),
    m_isCarried(false),
    m_isJustPlaced(false)
{
}

/*
* @brief 初期化処理
*
* @param[in]  pos 位置
* @param[in]  scale 大きさ
* @param[in]  type ブロックの種類
*
* @return なし
*/
void GimmickBlock::Initialize(std::shared_ptr<DirectX::Model> model, const DirectX::SimpleMath::Vector3& pos, 
    const DirectX::SimpleMath::Vector3& scale, BlockType type)
{
    m_model = model;
    m_position = pos;
    m_initialPosition = pos;
    m_scale = scale;
    m_type = type;

    // 追尾機能の初期化
    m_followOffset = DirectX::SimpleMath::Vector3(0.0f, DEFAULT_FOLLOW_OFFSET_Y, 0.0f);

    // 当たり判定のサイズを設定
    m_aabbSize = DirectX::SimpleMath::Vector3(UNIT_CUBE_SIZE, UNIT_CUBE_SIZE, UNIT_CUBE_SIZE) * m_scale;

    // 当たり判定の更新
    UpdateCollision();
}

/*
* @brief 更新処理
* 
* @param[in]  elapsedTime 前フレームからの経過時間
* 
* @return なし
*/
void GimmickBlock::Update(float elapsedTime)
{
    // 追尾処理の更新
    if (m_isFollowing)
    {
        UpdateFollowing(elapsedTime);
    }

    // ワールド行列の更新
    m_trans = DirectX::SimpleMath::Matrix::CreateScale(m_scale) * DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
}

/*
* @brief ブロックの処理を有効化
*
* @param[in]  なし
*
* @return なし
*/
void GimmickBlock::OnActivate()
{
	// ブロックを表示状態にする
    m_isVisible = !m_isVisible;
}

/*
* @brief 当たり判定の更新処理
*
* @param[in]  なし
*
* @return なし
*/
void GimmickBlock::UpdateCollision()
{
	// AABBを現在の位置とサイズで更新
    m_collision = m_collision.CreateAABB(m_position, m_aabbSize);
}

/*
* @brief 当たり判定のチェック
*
* @param[in]  otherAABB チェック対象のAABB
*
* @return 当たり判定の結果
*/
bool GimmickBlock::CheckCollision(const AABB& otherAABB)
{
    // 非表示の場合は無効化
    if(!m_isVisible) return false;
    
    return m_collision.CheckAABBCollision(m_collision, otherAABB);
}

/*
* @brief ワールド行列の取得
*
* @param[in]  なし
*
* @return　ワールド行列
*/
DirectX::SimpleMath::Matrix GimmickBlock::GetWorldMatrix() const
{
    DirectX::SimpleMath::Matrix rot = DirectX::SimpleMath::Matrix::CreateRotationY(0.0f);
    DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(m_scale);
    DirectX::SimpleMath::Matrix trans = DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
    return scale * rot * trans;
}

/*
* @brief　座標の設定
*
* @param[in]  pos 位置
*
* @return　なし
*/
void GimmickBlock::SetPosition(const DirectX::SimpleMath::Vector3& pos)
{
    m_position = pos;
    // 当たり判定の更新
    UpdateCollision();
}

/*
* @brief　大きさの設定
*
* @param[in]  scale 大きさ
*
* @return　なし
*/
void GimmickBlock::SetScale(const DirectX::SimpleMath::Vector3& scale)
{
    m_scale = scale;
    m_aabbSize = DirectX::SimpleMath::Vector3(UNIT_CUBE_SIZE, UNIT_CUBE_SIZE, UNIT_CUBE_SIZE) * m_scale;
    // 当たり判定の更新
    UpdateCollision();
}

/*
* @brief　当たり判定の取得
*
* @param[in]  なし
*
* @return 当たり判定のAABB
*/
const AABB& GimmickBlock::GetCollision() const
{
    return m_collision;
}

/*
* @brief　当たり判定の大きさの取得
*
* @param[in]  なし
*
* @return 当たり判定の大きさ
*/
const DirectX::SimpleMath::Vector3& GimmickBlock::GetAABBSize() const
{
    return m_aabbSize;
}

/*
* @brief　追尾を開始
*
* @param[in]  targetPos 追尾対象の位置
* @param[in]  speed 追尾の速度
*
* @return なし
*/
void GimmickBlock::StartFollowing(const DirectX::SimpleMath::Vector3& targetPos, float speed)
{
    m_isFollowing = true;
    m_isCarried = false;
    m_targetPosition = targetPos;
    m_followSpeed = speed;
}

/*
* @brief　追尾を止める
*
* @param[in]  stopPosition　追尾停止位置
*
* @return なし
*/
void GimmickBlock::StopFollowing(const DirectX::SimpleMath::Vector3& stopPosition)
{
    m_isFollowing = false;
    m_isCarried = false;

    // 追尾を止めた位置に移動
	m_position = stopPosition; 

    // Y座標を少し下げる
    DirectX::SimpleMath::Vector3 downPos = stopPosition;
    downPos.y -= STOP_POSITION_OFFSET_Y;
    m_position = downPos;

	// 当たり判定の更新
	m_collision = m_collision.CreateAABB(m_position, m_aabbSize);
    m_targetPosition = stopPosition;
    m_targetPosition = downPos;
    m_trans = DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
}

/*
* @brief　追尾のリセット
*
* @param[in] なし
*
* @return なし
*/void GimmickBlock::Reset()
{
    m_isFollowing = false;
    m_position = m_initialPosition;
    UpdateCollision();
}

/*
* @brief　追尾状態の更新
*
* @param[in]  elapsedTime 前フレームからの経過時間  
*
* @return なし
*/ 
void GimmickBlock::UpdateFollowing(float elapsedTime)
{
    if (!m_isFollowing) return;

    // 現在のターゲット位置にオフセットを加える
    DirectX::SimpleMath::Vector3 targetPos = m_targetPosition + m_followOffset;

    // すでに運搬状態なら強制ロックする
    if (m_isCarried)
    {
        m_position = targetPos;
        UpdateCollision();
        return;
    }

    // 現在位置から目標位置へのベクトル
    DirectX::SimpleMath::Vector3 direction = targetPos - m_position;
    float distance = direction.Length();

    // 近い場合は目標位置にスナップ
    if (distance < 0.1f)
    {
        m_isCarried = true;
        m_position = targetPos;
        UpdateCollision();
        m_isJustPlaced = true;

        return;
    }
    
    if (distance > 0.0f)
    {
        // 正規化して移動
        direction.Normalize();
        DirectX::SimpleMath::Vector3 moveVector = direction * m_followSpeed * elapsedTime;

        // 移動量が残り距離より大きい場合は目標位置に直接移動
        if (moveVector.Length() > distance)
        {
            m_position = targetPos;
        }
        else
        {
            m_position += moveVector;
        }
    }
    // 当たり判定の更新
    UpdateCollision();
}

/*
* @brief コライダーの線
*
* @param[in]  なし
*
* @return なし
*/
void GimmickBlock::ColliderLine()
{
    if (!m_primitiveBatch) return;

    auto lineColor = DirectX::Colors::LimeGreen;
    m_primitiveBatch->Begin();

    if (GetIsVisible())
    {
        DirectX::SimpleMath::Vector3 platCorners[8] =
        {
            { m_collision.min.x, m_collision.min.y, m_collision.min.z },
            { m_collision.max.x, m_collision.min.y, m_collision.min.z },
            { m_collision.max.x, m_collision.max.y, m_collision.min.z },
            { m_collision.min.x, m_collision.max.y, m_collision.min.z },
            { m_collision.min.x, m_collision.min.y, m_collision.max.z },
            { m_collision.max.x, m_collision.min.y, m_collision.max.z },
            { m_collision.max.x, m_collision.max.y, m_collision.max.z },
            { m_collision.min.x, m_collision.max.y, m_collision.max.z }
        };

        // 線の描画処理（前面）
        m_primitiveBatch->DrawLine({ platCorners[0], lineColor }, { platCorners[1], lineColor });
        m_primitiveBatch->DrawLine({ platCorners[1], lineColor }, { platCorners[2], lineColor });
        m_primitiveBatch->DrawLine({ platCorners[2], lineColor }, { platCorners[3], lineColor });
        m_primitiveBatch->DrawLine({ platCorners[3], lineColor }, { platCorners[0], lineColor });

        // 線の描画処理（背面）
        m_primitiveBatch->DrawLine({ platCorners[4], lineColor }, { platCorners[5], lineColor });
        m_primitiveBatch->DrawLine({ platCorners[5], lineColor }, { platCorners[6], lineColor });
        m_primitiveBatch->DrawLine({ platCorners[6], lineColor }, { platCorners[7], lineColor });
        m_primitiveBatch->DrawLine({ platCorners[7], lineColor }, { platCorners[4], lineColor });

        // 前面と背面を繋ぐ線の描画
        m_primitiveBatch->DrawLine({ platCorners[0], lineColor }, { platCorners[4], lineColor });
        m_primitiveBatch->DrawLine({ platCorners[1], lineColor }, { platCorners[5], lineColor });
        m_primitiveBatch->DrawLine({ platCorners[2], lineColor }, { platCorners[6], lineColor });
        m_primitiveBatch->DrawLine({ platCorners[3], lineColor }, { platCorners[7], lineColor });
    }

    m_primitiveBatch->End();
}

