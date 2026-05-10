//--------------------------------------------------------------------------------------
// File: TaskManager.h
//
// タスクを管理するクラス
//--------------------------------------------------------------------------------------
#pragma once

#include <list>
#include <set>
#include <sstream>
#include <functional>

class TaskManager;
class Task;

//////////////////////////////////////////////////////////////////////////
//																		//
//	タスク連結情報管理クラス											//
//																		//
//////////////////////////////////////////////////////////////////////////
class TaskConnectInfo
{
private:
	// タスク管理オブジェクトへのポインタ
	TaskManager* m_taskManager;

	// 親タスクへのポインタ
	Task* m_parent;

	// 子供タスクのリスト
	std::list<Task*> m_children;

public:
	TaskConnectInfo() : m_taskManager{}, m_parent{} {}
	TaskManager* GetTaskManager() const { return m_taskManager; }
	Task* GetParent() const { return m_parent; }
	std::list<Task*>* GetChildList() { return &m_children; }
	void SetTaskManager(TaskManager* taskManager) { m_taskManager = taskManager; }
	void SetParent(Task* task) { m_parent = task; }
	void AddChild(Task* task) { m_children.push_back(task); }
	bool DeleteChildList(Task* task);
};

//////////////////////////////////////////////////////////////////////////
//																		//
//	タスクの基底クラス													//
//																		//
//////////////////////////////////////////////////////////////////////////
class Task
{
private:
	// 描画順序管理用(小さいほど手前に描画される）
	int m_ot;

	// タスク連結情報
	TaskConnectInfo m_connect;

	// タスクの名前
	std::string m_name;

public:

	TaskConnectInfo& GetTaskConnectInfo() { return m_connect; }

public:

	// コンストラクタ
	Task() : m_ot(0) {}

	// デストラクタ
	virtual ~Task() {}

	//////////////////////////////////////////
	//										//
	//	オーバーライドして使う関数			//
	//										//
	//////////////////////////////////////////

	// 更新関数（falseを返すとタスクが消える）
	virtual bool Update(float elapsedTime)
	{
		UNREFERENCED_PARAMETER(elapsedTime);
		return true;
	}

	// 描画関数
	virtual void Render() {}

public:

	// タスクマネージャーの取得関数
	TaskManager* GetTaskManager() const { return m_connect.GetTaskManager(); }

	// 親タスクの取得関数
	Task* GetParent() const { return m_connect.GetParent(); }

	// 描画順設定関数（otの値が0が一番手前）
	void SetOt(int ot) { this->m_ot = ot; }

	// 描画順の取得関数
	int GetOt() const { return m_ot; }

	// 指定したタスクの子供にする変更する関数
	void ChangeParent(Task* parent);

	// タスクに名前を付ける関数
	void SetName(const std::string& name) { m_name = name; }

	// タスクの名前を取得する関数
	const std::string& GetName() { return m_name; }

	// タスク更新関数（子供のタスクまでタスク更新関数が呼び出される）
	void UpdateTasks(std::function<void(Task*)> func)
	{
		func(this);
		std::list<Task*>* children = GetTaskConnectInfo().GetChildList();
		for (const auto it : *children)
		{
			it->UpdateTasks(func);
		}
	}

};

//////////////////////////////////////////////////////////////////////////
//																		//
//	タスク管理クラス													//
//																		//
//////////////////////////////////////////////////////////////////////////
class TaskManager
{

private:
#pragma region
	// 描画順序管理テーブルへの登録用比較関数（otの値が大きい物が先頭へ）
	struct greater_ot : std::binary_function<Task*, Task*, bool>
	{
		bool operator()(Task* a, Task* b) const
		{
			return a->GetOt() > b->GetOt();
		}
	};

	// 描画順序管理テーブル
	std::multiset<Task*, greater_ot> m_ot;

	// ルートタスク
	Task* m_rootTask;

	// 実行中のタスク
	Task* m_currentTask;

	// タスクの削除関数
	void DeleteTask(Task* task);

	// 子供タスクの更新関数を実行する関数
	void ChildTaskUpdate(Task* task, float elapsedTime);

	// 子供タスクを再帰的に描画リストへ登録する関数
	void RegisterRenderList(Task* task);

	// タスク生成数
	uint64_t m_totalTaskCnt;

#pragma endregion

public:

	// コンストラクタ
	TaskManager() : m_rootTask{}, m_currentTask{}, m_totalTaskCnt{}
	{
		m_currentTask = m_rootTask = new Task();
		m_rootTask->SetName("RootTask");
	}

	// デストラクタ
	virtual ~TaskManager()
	{
		DeleteTask(m_rootTask);
	}

	// 更新関数
	virtual void Update(float elapsedTime);

	// 描画関数
	virtual void Render();

	// タスクの生成関数
	template <class T, class... Args>
	T* AddTask(Args&&... args);

	// ルートタスクを取得する関数
	Task* GetRootTask() { return m_rootTask; }

};

#pragma region
// タスクの生成関数（実行中のタスクの子供になる）
template <class T, class... Args>
T* TaskManager::AddTask(Args&&... args)
{
	// タスクを生成する
	T* task = new T(std::forward<Args>(args)...);

	// タスク管理オブジェクトを設定
	task->GetTaskConnectInfo().SetTaskManager(this);

	// 親を設定
	task->GetTaskConnectInfo().SetParent(m_currentTask);

	// 親の子供リストに追加
	m_currentTask->GetTaskConnectInfo().AddChild(task);

	// タスクに名前が付いていない場合は仮で名前を付ける
	if (task->GetName().empty())
	{
		std::ostringstream ostr;
		ostr << "Task_" << m_totalTaskCnt;
		task->SetName(ostr.str());
	}

	// タスク生成数を加算
	m_totalTaskCnt++;

	return task;
}
#pragma endregion