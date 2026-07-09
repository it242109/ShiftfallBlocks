//--------------------------------------------------------------------------------------
// File: Switch.cpp
//
// スイッチクラス
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "Switch.h"

// 定数の定義
const float Switch::HALF_SCALE = 0.5f;				///< 半分のサイズにする

/*
* @brief コンストラクタ
*
* @param[in]  なし
* 
* @return なし
*/
Switch::Switch()
{
}

/*
* @brief デストラクタ
*
* @param[in]  なし
* 
* @return なし
*/
Switch::~Switch()
{
}

/*
* @brief 初期化処理
*
* @param[in]  context デバイスコンテキスト
* 
* @return なし
*/
void Switch::Initialize(ID3D11DeviceContext* context)
{
	m_switches.clear();
	m_collisions.clear();
	m_switchStates.clear();

    m_collisions.resize(m_switches.size());
    m_primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);
}

/*
* @brief 更新処理
*
* @param[in]  gimmickBlocks 仕掛けブロックへのポインタ
* 
* @return なし
*/
void Switch::Update(const std::vector<std::shared_ptr<GimmickBlock>>& gimmickBlocks)
{
    // 各スイッチの状態を更新
    for (size_t i = 0; i < m_switches.size(); ++i)
    {
        // スイッチのデータを取得
        const auto& sw = m_switches[i];

        // スイッチのAABBを作成
        DirectX::SimpleMath::Vector3 half = sw.scale * HALF_SCALE;
        m_collisions[i] = AABB(sw.position - half, sw.position + half);

        bool isActivated = false;
        const auto& switchCollider = m_collisions[i];
        // 各仕掛けブロックとの当たり判定
        for (auto& block : gimmickBlocks)
        {
            if (!block || !block->GetIsVisible()) continue;

            // スイッチの種類ごとに当たり判定を分岐
            switch (sw.switchType)
            {
                // 足場ブロック
            case SwitchTargetType::SW_PLATFORM:
                if (block->GetType() == BlockType::PLATFORM &&
                    block->CheckCollision(switchCollider))
                {
                    isActivated = true;
                }
                break;
                // 鍵ブロック
            case SwitchTargetType::SW_KEY:
                if (block->GetType() == BlockType::KEY &&
                    block->CheckCollision(switchCollider))
                {
                    isActivated = true;
                }
                break;
				// ポータルブロック
            case SwitchTargetType::SW_PORTAL:
                if (block->GetType() == BlockType::PORTAL &&
                    block->CheckCollision(switchCollider))
                {
                    isActivated = true;
                }
                break;
                // アイテムブロック
            case SwitchTargetType::SW_ITEM:
                if (block->GetType() == BlockType::ITEM &&
                    block->CheckCollision(switchCollider))
                {
                    isActivated = true;
                }
                break;
				// すべてのブロック
            case SwitchTargetType::ANY:
                if (block->CheckCollision(switchCollider))
                {
                    isActivated = true;
                }
                break;
            }
        }

        // スイッチON/OFFの更新
        m_switchStates[i] = isActivated;
        if (isActivated && sw.onActivate)
        {
            sw.onActivate();
        }
    }
}

/*
* @brief 描画処理
*
* @param[in]  context デバイスコンテキスト
* @param[in]  view    ビュー行列
* @param[in]  proj 　 射影行列
* @param[in]  states  共通ステートオブジェクト
* 
* @return なし
*/
void Switch::Render(ID3D11DeviceContext* context, const DirectX::SimpleMath::Matrix& view, 
    const DirectX::SimpleMath::Matrix& proj, DirectX::CommonStates* states)
{
	if (!m_model) return;

	// 各スイッチの描画
	for (const auto& sw : m_switches)
	{
        DirectX::SimpleMath::Matrix scale = DirectX::SimpleMath::Matrix::CreateScale(sw.scale);
        DirectX::SimpleMath::Matrix trans = DirectX::SimpleMath::Matrix::CreateTranslation(sw.position);
        DirectX::SimpleMath::Matrix world = scale  * trans;

		m_model->Draw(context, *states, world, view, proj);
	}
}

/*
* @brief　各スイッチの追加処理
*
* @param[in]  data スイッチのデータ
* 
* @return なし
*/
void Switch::AddSwitch(const SwitchData& data)
{
	m_switches.push_back(data);
	m_collisions.push_back(AABB());
	m_switchStates.push_back(false);
}

/*
* @brief　スイッチが作動したら
*
* @param[in]  index 各スイッチのインデックス
* 
* @return スイッチがONならtrue、OFFならfalse
*/
bool Switch::IsSwitchOn(size_t index) const
{
	if (index >= m_switchStates.size()) return false;
	return m_switchStates[index];
}

/*
* @brief コライダーの線
*
* @param[in]  なし
* 
* @return なし
*/
void Switch::ColliderLine()
{
    if (!m_primitiveBatch) return;

    auto lineColor = DirectX::Colors::Yellow;

    m_primitiveBatch->Begin();

    for (const auto& s_col : m_collisions)
    {
        DirectX::SimpleMath::Vector3 switchCorners[8] =
        {
            { s_col.min.x, s_col.min.y, s_col.min.z },
            { s_col.max.x, s_col.min.y, s_col.min.z },
            { s_col.max.x, s_col.max.y, s_col.min.z },
            { s_col.min.x, s_col.max.y, s_col.min.z },
            { s_col.min.x, s_col.min.y, s_col.max.z },
            { s_col.max.x, s_col.min.y, s_col.max.z },
            { s_col.max.x, s_col.max.y, s_col.max.z },
            { s_col.min.x, s_col.max.y, s_col.max.z },
        };

        /*/////////////////////////////////////線の描画///////////////////////////////////////////*/

        // 前面の線を描画
        m_primitiveBatch->DrawLine({ switchCorners[0], lineColor }, { switchCorners[1], lineColor });
        m_primitiveBatch->DrawLine({ switchCorners[1], lineColor }, { switchCorners[2], lineColor });
        m_primitiveBatch->DrawLine({ switchCorners[2], lineColor }, { switchCorners[3], lineColor });
        m_primitiveBatch->DrawLine({ switchCorners[3], lineColor }, { switchCorners[0], lineColor });

        // 背面の線を描画
        m_primitiveBatch->DrawLine({ switchCorners[4], lineColor }, { switchCorners[5], lineColor });
        m_primitiveBatch->DrawLine({ switchCorners[5], lineColor }, { switchCorners[6], lineColor });
        m_primitiveBatch->DrawLine({ switchCorners[6], lineColor }, { switchCorners[7], lineColor });
        m_primitiveBatch->DrawLine({ switchCorners[7], lineColor }, { switchCorners[4], lineColor });

        // 前面と背面をつなぐ線を描画
        m_primitiveBatch->DrawLine({ switchCorners[0], lineColor }, { switchCorners[4], lineColor });
        m_primitiveBatch->DrawLine({ switchCorners[1], lineColor }, { switchCorners[5], lineColor });
        m_primitiveBatch->DrawLine({ switchCorners[2], lineColor }, { switchCorners[6], lineColor });
        m_primitiveBatch->DrawLine({ switchCorners[3], lineColor }, { switchCorners[7], lineColor });

        /*////////////////////////////////////////////////////////////////////////////////////////*/
    }
    m_primitiveBatch->End();
}